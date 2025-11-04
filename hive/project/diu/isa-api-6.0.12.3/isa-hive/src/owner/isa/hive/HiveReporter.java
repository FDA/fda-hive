package owner.isa.hive;

import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.logging.*;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.base.Function;
import com.google.common.base.Optional;
import com.google.common.base.Strings;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableSet;
import com.google.common.net.InetAddresses;
import com.google.common.util.concurrent.Futures;

import gov.isa.behaviors.AbstractComponentBehavior;
import gov.isa.behaviors.BehaviorControls;
import gov.isa.behaviors.ComponentBehavior;
import gov.isa.behaviors.SubscriptionManager;
import gov.isa.capabilities.*;
import gov.isa.capabilities.StandardCapabilities.PositionObservableHandler;
import gov.isa.capabilities.StandardCapabilities.IdentityObservableHandler;
import gov.isa.capabilities.StandardCapabilities.BiologicalReadingObservableHandler;
import gov.isa.capabilities.cbrn.CBRNCapabilities.BiologicalLevelObservableHandler;
import gov.isa.capabilities.extended.ExtendedCapabilities.BackcastObservableHandler;
import gov.isa.components.Component;
import gov.isa.components.PrimaryComponent;
import gov.isa.components.PrimaryComponentDeclaration;
import gov.isa.model.*;
import gov.isa.model.List;
import gov.isa.model.cbrn.*;
import gov.isa.model.extended.ExtendedModelFactory;
import gov.isa.model.extended.Posit;
import gov.isa.net.*;
import gov.isa.spi.*;
import gov.isa.util.TimeUtils;

import owner.isa.hive.utils.BcfksKeystores;
import owner.isa.hive.utils.BioMaterial;
import owner.isa.hive.utils.Configuration;
import owner.isa.hive.utils.DirectoryMonitor;
import owner.isa.hive.utils.GeolocationResolver;
import owner.isa.hive.utils.JksKeystores;
import owner.isa.hive.utils.ProcessRunner;

public class HiveReporter extends Configuration {

  private static final Logger LOG = Logger.getLogger("HIVE-ISA-REPORTER");

  private static final ScheduledExecutorService rebootExecutor = Executors.newScheduledThreadPool(1);

  private static final AtomicBoolean clientConnected = new AtomicBoolean(false);

  private static final AtomicBoolean clientSubscribed = new AtomicBoolean(false);

  private static final ConcurrentHashMap<String, AbstractMap.SimpleImmutableEntry<BioMaterial, Function<BioMaterial, Integer>>> publications = new ConcurrentHashMap<>();

  public static void main( final String[] args ) {

    initLogger();

    ensureSingleProcessInstance();

    LOG.info("Current working directory is: " + Paths.get(".").toAbsolutePath().normalize().toString());

    Configuration.load(args);

    try {
      Files.createDirectories(Paths.get(HEALTH_STATUS_DIRECTORY));
    } catch (IOException e) {};

    // Get a component factory manager
    ComponentFactoryManager manager = new ComponentFactoryManager();

    // Get the default isa api provider from the component factory manager
    ComponentFactoryProvider provider = manager.getDefaultProvider();

    bootClient(provider);
  }

  private static void initLogger() {
    try {
      final String filePattern = "isa-hive%u.log";
      final String logPattern =  Paths.get(LOG_FILES_DIRECTORY, filePattern).toAbsolutePath().normalize().toString();

      Files.createDirectories(Paths.get(LOG_FILES_DIRECTORY));
      FileHandler fileHandler = new FileHandler(logPattern, LOG_FILES_LIMIT, LOG_FILES_COUNT, true);
      fileHandler.setFormatter(new SimpleFormatter());
      LOG.addHandler(fileHandler);
    } catch(IOException e) {
      System.err.println("Failed to initialize log file handler: " + e.getMessage());
    }
  }

  private static void ensureSingleProcessInstance() {
    Path pidFilePath = Paths.get(PID_FILE);
    String prevPid = null;
    if(Files.exists(pidFilePath)) try {
      prevPid = new String(Files.readAllBytes(pidFilePath)).trim();
    } catch (IOException ex) {
      LOG.warning("Failed to open PID file: " + ex.getMessage());
    }

    if(!Strings.isNullOrEmpty(prevPid)) {
      String os = System.getProperty("os.name");
      String killCmd;
      if (os.startsWith("Win")) {
        killCmd = String.format("taskkill /F /T /PID %s", prevPid.trim());
      } else {
        killCmd = String.format("kill %s", prevPid.trim());
      }
      try {
        LOG.warning("Killing previous process by command: " + killCmd);
        Runtime.getRuntime().exec(killCmd);
      } catch (IOException ex) {
        LOG.warning("Failed to kill previous process: " + ex.getMessage());
      }
    }

    String name = ManagementFactory.getRuntimeMXBean().getName();
    String pidStr = name.split("@")[0];
    LOG.info("Current PID: " + pidStr);
    try {
      Files.write(pidFilePath, pidStr.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
    } catch (IOException ex) {
      LOG.warning("Failed to write PID file: " + ex.getMessage());
    }
  }

  private static void updateHealthStatus(final Path healthStatusFile) {
    final SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    sdf.setTimeZone(TimeZone.getTimeZone("GMT"));
    String timeStamp = sdf.format(new java.util.Date());
    try {
      Files.write(healthStatusFile, timeStamp.getBytes());
    } catch (IOException ex) {
      LOG.log(Level.SEVERE, "Failed to write health status file: " + ex.getMessage());
    }
  }

  private static void bootClient(final ComponentFactoryProvider provider) {
    try {
      updateHealthStatus(Paths.get(HEALTH_STATUS_DIRECTORY, HEALTH_STATUS_FILE));

      final Path keyDirectoryPath = Paths.get(KEY_DIRECTORY);

      // Initialize the isa api provider and get the component factory
      final ComponentFactory componentFactory = provider.initialize(
          new StandardComponentFactoryOptions.ComponentKeyStoreRetriever(
              IS_LOCAL_CONTROLLER ? BcfksKeystores.createKeystoreRetriever(keyDirectoryPath, MASTER_KEY_FILENAME) : JksKeystores.createKeystoreRetriever(keyDirectoryPath)
          ),
          new StandardComponentFactoryOptions.ComponentKeyPasswordRetriever(
              IS_LOCAL_CONTROLLER ? BcfksKeystores.createPasswordRetriever(keyDirectoryPath, MASTER_KEY_FILENAME) : JksKeystores.createPasswordRetriever(keyDirectoryPath)
          ),
          new StandardComponentFactoryOptions.ComponentTrustStoreRetriever(
              IS_LOCAL_CONTROLLER ? BcfksKeystores.createTruststoreRetriever(keyDirectoryPath, MASTER_KEY_FILENAME) : JksKeystores.createTruststoreRetriever(keyDirectoryPath)
          )
      );

      // Get a model factory from the component factory.  It is used to construct all data types
      ModelFactory modelFactory = componentFactory.getModelFactory();

      // Create an uci for this component
      UCI clientUCI = modelFactory.newUCI(MY_UCI);

      // Create a component declaration for this component.  It is used to declare capabilities for the component.
      PrimaryComponentDeclaration clientDeclaration = componentFactory.createPrimaryComponentDeclaration(clientUCI);

      double latitude = 0, longitude = 0;
      String[] geolocation = GeolocationResolver.resolve();
      if((geolocation == null) || (geolocation.length != 2)) {
        geolocation = DEFAULT_GEOLOCATION.split(",", 2);
      }

      if((geolocation != null) && (geolocation.length == 2)) {
        latitude = Double.parseDouble(geolocation[0]);
        longitude = Double.parseDouble(geolocation[1]);
      }

      final GeographicPosition clientPosition = modelFactory.newGeographicPosition(
          modelFactory.newDegrees(latitude),
          modelFactory.newDegrees(longitude),
          Optional.<Meters>absent(),
          Optional.<Meters>absent(),
          Optional.<Meters>absent(),
          Optional.<Meters>absent(),
          Optional.<Degrees>absent(),
          Optional.<Degrees>absent()
      );

      // TODO: obtain mil2525 target identifier.
      final String targetID = MIL2525_DEFAULT_TARGET_ID;
      final StandardIdentity targetIdentity = modelFactory.newStandardIdentity(modelFactory.newSIDC(targetID), Optional.<Percent>absent());

      clientDeclaration.declare(createBiologicalMaterialBehavior(modelFactory, clientPosition, targetIdentity));
      clientDeclaration.declare(createHeartBeatBehavior(modelFactory, clientPosition, targetIdentity));
      try {
        // Create a behavior to handle self subscription.
        final String subscription = String.format("select all matching UCI{\"%s\"} == source() with loopback", MY_UCI);
        clientDeclaration.declare(createSubscriptionBehavior(modelFactory, subscription));
      } catch (DataQueryParserException ex) {
        Optional<String> reason = ex.getStartIndex().transform(
            new Function<Integer, String>() {
              @Override
              public String apply( Integer index ) {
                return "near index " + index.toString();
              }
            }
        );

        LOG.log(Level.SEVERE, "Error while parsing subscription " + reason.or("at unknown location."));
        System.exit( 1 );
      }

      // Create a primary component using the component declaration
      final PrimaryComponent clientComponent = componentFactory.createPrimaryComponent(clientDeclaration);

      // Create an uci for the controller you are connecting to.
      UCI serverUCI = modelFactory.newUCI(CONTROLLER_UCI);

      // Create an endpoint descriptor for the controller you want to connect to.
      final EndpointDescriptor serverEndpoint = new EndpointDescriptor(
          InetAddresses.forString(CONTROLLER_IP),
          Integer.parseInt(CONTROLLER_PORT),
          serverUCI
      );

      // Connect this client to the controller and get a monitor that will watch the connection.
      ConnectionMonitor monitor = clientComponent.addClientToEndpoint(serverEndpoint);
      monitor.addLifeCycleListener(
        new LifeCycleListener() {
          @Override public void onLifeCycleChange(LifeCyclePhase prev, LifeCyclePhase next) {
            LOG.info(prev.getPhaseID() + " => " + next.getPhaseID());
            synchronized(clientConnected){
              if(!clientConnected.get()) {
                if(next.getPhaseID().equals(LifeCyclePhase.PhaseID.OPERATION)) {
                  clientConnected.set(true);
                  clientConnected.notifyAll();
                }
              }
              else if(next.getPhaseID().equals(LifeCyclePhase.PhaseID.IDLE)) {
                clientConnected.set(false);
                clientSubscribed.set(false);
                LOG.log(Level.SEVERE, "The connection to the Controller has been dropped, scheduling the reboot.");
                rebootExecutor.schedule(new Runnable() {
                  @Override
                  public void run() {
                    clientComponent.removeClientToEndpoint(serverEndpoint);
                    componentFactory.getBehavioralScheduler().shutdown();
                    componentFactory.getNetworkingScheduler().shutdown();
                    publications.clear();
                    bootClient(provider);
                  }
                }, 5L, TimeUnit.SECONDS);
              }
            }
          }
        }
      );

    } catch (ComponentFactoryException e) {
      LOG.log(Level.SEVERE, "Problem initializing the component factory: " + e.getMessage());
      System.exit( 1 );
    }
  }

  private static void waitForCondition(final AtomicBoolean waitObject) {
    while(!waitObject.get()) {
      synchronized (waitObject) {
        try {
          waitObject.wait(1000);
        } catch (InterruptedException ex) {}
      }
      try {
        // if wait() will not wait - must be outside synchronized block, or it may cause freeze thread with notifyAll()
        java.lang.Thread.sleep(100);
      } catch (InterruptedException ex) {}
    }
  }

  private static ComponentBehavior createHeartBeatBehavior(
          final ModelFactory factory,
          final GeographicPosition clientPosition,
          final StandardIdentity targetIdentity
  ) {

    final StandardCapabilities.IdentityPropertyHandler identityHandler = new StandardCapabilities.IdentityPropertyHandler(
        factory,
        factory.newMutability(Mutability.Predefined.PERMANENT),
        targetIdentity,
        ReadyState.READY,
        ReportingState.REPORTING
    );

    final StandardCapabilities.PositionPropertyHandler positionHandler = new StandardCapabilities.PositionPropertyHandler(
        factory,
        factory.newMutability(Mutability.Predefined.READ_ONLY),
        clientPosition,
        ReadyState.READY,
        ReportingState.REPORTING
    );

    final Path healthStatusFile = Paths.get(HEALTH_STATUS_DIRECTORY, HEALTH_STATUS_FILE);

    return new AbstractComponentBehavior() {
      @Override
      public ImmutableSet<PropertyHandler> getProperties() {
        return ImmutableSet.of(
            identityHandler,
            positionHandler
        );
      }

      @Override
      public void initialize(final BehaviorControls controls) {
        controls.getScheduler().schedule(
          new Runnable() {
            @Override
            public void run() {
              waitForCondition(clientConnected);
              LOG.info("Starting heartbeat sending process");
              controls.getScheduler().scheduleAtFixedRate(
                new Runnable() {
                  @Override
                  public void run() {
                    GeographicPosition oldPosition = positionHandler.getValueSafely();
                    positionHandler.setValueSafely(oldPosition);
                    updateHealthStatus(healthStatusFile);
                  }
                }, 1L, HEARTBEAT_INTERVAL, TimeUnit.SECONDS
              );
            }
          }, 1, TimeUnit.SECONDS
        );
      }
    };
  }

  private static ComponentBehavior createSubscriptionBehavior(ModelFactory factory, final String subscription)
          throws DataQueryParserException {

    // Create a data query parser manager that will give us a provider
    final DataQueryParserManager manager = new DataQueryParserManager();

    // Get the data query parser provider, which will give us a data query parser
    final DataQueryParserProvider provider = manager.getDefaultProvider();

    // Get the parser, which will give us a data query once we pass it the subscription
    final DataQueryParser parser = provider.createDataQueryParser(factory);

    // Get the data query, which is what we can pass to the live data engine
    final DataQuery dataQuery = parser.parseLanguage( subscription );

    // Create the component behavior. Once it is initialized with the Behavior Controls,
    // it will submit the subscriptions to the subscription manager (live data engine)
    return new AbstractComponentBehavior() {
      @Override
      public void initialize(final BehaviorControls controls) {
        controls.getScheduler().schedule(
          new Runnable() {
            @Override
            public void run() {
              waitForCondition(clientConnected);
              LOG.info("Subscribing with query: [ " + subscription + " ]");
              controls.getSubscriptionManager().submitSubscription(
                "HIVE_self_subscription", // unique name for subscription
                dataQuery,
                new SubscriptionManager.SubscriptionReceiver() {
                  @Override
                  public void onSubscribedMessage(
                          SubscriptionID subscription,
                          Message message,
                          Header header,
                          MetaData meta
                  ) {
                    LOG.info("Received " + message.toString() );
                    if(!clientSubscribed.get()) {
                      synchronized(clientSubscribed) {
                        clientSubscribed.set(true);
                        clientSubscribed.notifyAll();
                      }
                    }

                    if(publications.isEmpty()) {
                      return;
                    }

                    if(message instanceof Event) {
                      for(NameValuePair nvp : ((Event)message).getObservables()) {
                        if(nvp.getName().equals("Biological Level")) {
                          Object val = nvp.getValue();
                          if(val instanceof List) {
                            ImmutableList<Object> values = ((List)val).getValues();
                            for(Object obj : values) {
                              if(obj instanceof CustomType) {
                                ImmutableList<NameValuePair> fields = ((CustomType)obj).getFields();
                                for(NameValuePair fnvp : fields) {
                                  if(fnvp.getName().equals("sample id")) {
                                    Object sampleVal = fnvp.getValue();
                                    if(sampleVal instanceof String) {
                                      String sampleID = (String)sampleVal;
                                      AbstractMap.SimpleImmutableEntry<BioMaterial, Function<BioMaterial, Integer>> replyData = publications.get(sampleID);
                                      if(replyData != null) {
                                        replyData.getValue().apply(replyData.getKey());
                                        publications.remove(sampleID);
                                        break;
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                },
                true // persist this subscription until this component is shut down.
              );
            }
          }, 1, TimeUnit.SECONDS
        );
      }
    };
  }

  private static String getHiveOutput() {
    StringBuilder sb = new StringBuilder();
    if(!Strings.isNullOrEmpty(HIVE_SESSION_ID)) {
      sb.append(String.format("sessionID=%s&", HIVE_SESSION_ID));
    }

    sb.append("cmdr=daMessageList");
    String args = sb.toString();
    LOG.info("getHiveOutput: Running process [" + HIVE_TOOL_PATH + "] with args: " + args);
    String jsonString = ProcessRunner.run(ImmutableSet.of(0), HIVE_TOOL_PATH, args);
    /*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
    return jsonString.replaceAll("\0", "");
  }

  private static ArrayList<BioMaterial> parseHiveOutput(final String hiveOutput, final ModelFactory factory) {
    if(Strings.isNullOrEmpty(hiveOutput)) {
      return null;
    }

    String jsonString = hiveOutput.trim();
    int idx = jsonString.indexOf("[{");
    if(idx > 0) {
      jsonString = jsonString.substring(idx);
    }

    JsonNode rootObj;
    try {
      ObjectMapper mapper = new ObjectMapper();
      rootObj = mapper.readTree(jsonString);
    } catch (JsonProcessingException ex) {
      LOG.warning("Error parsing the JSON string: " + ex.getMessage());
      return null;
    }

    if(!rootObj.isArray() || rootObj.isEmpty()) {
      return null;
    }

    JsonNode item = rootObj.get(0);
    JsonNode objs = item.get("objs");
    if((objs == null) || !objs.isArray() || objs.isEmpty()) {
      return null;
    }

    ArrayList<BioMaterial> observations = new ArrayList<>();
    for(JsonNode obj : objs) {
      JsonNode id = obj.get("_id");
      if((id == null) || !id.isIntegralNumber()) {
        LOG.warning("Invalid '_id' field, the observation ignored");
        continue;
      }

      String hiveObjectID = String.valueOf(id.asLong());
      JsonNode result_info = obj.get("result_info");
      if((result_info == null) || !result_info.isArray() || result_info.isEmpty()) {
        LOG.warning("Missing or empty 'result_info' array, the observation ignored");
        continue;
      }

      GeographicPosition materialPosition = null;
      JsonNode sampleCollection = obj.get("sample_collection");
      if((sampleCollection != null) && sampleCollection.isObject()) {
        JsonNode geolocationNode = sampleCollection.get("geoLocation");
        if((geolocationNode != null) && geolocationNode.isTextual()) {
          String pairedVal = geolocationNode.asText();
          if(!Strings.isNullOrEmpty(pairedVal)) {
            String[] location = pairedVal.split(",", 2);
            if((location != null) && (location.length == 2)) {
              double latitude = Double.parseDouble(location[0]), longitude = Double.parseDouble(location[1]);
              materialPosition = factory.newGeographicPosition(
                  factory.newDegrees(latitude),
                  factory.newDegrees(longitude),
                  Optional.<Meters>absent(),
                  Optional.<Meters>absent(),
                  Optional.<Meters>absent(),
                  Optional.<Meters>absent(),
                  Optional.<Degrees>absent(),
                  Optional.<Degrees>absent()
              );
            }
          }
        }
      }

      for(JsonNode result : result_info) {
        JsonNode severityNode = result.get("severity");
        if(severityNode == null || !severityNode.isTextual()) {
          LOG.warning("Invalid 'severity' field, the observation ignored");
          continue;
        }

        boolean isHarmful = true;
        String severity = severityNode.asText();
        switch(severity.trim().toLowerCase()) {
          case "low":
          case "medium":
          case "high":
          case "critical":
            break;
          default:
            isHarmful = false;
            break;
        }

        JsonNode serial_no = result.get("serial_no");
        if((serial_no == null) || !serial_no.isIntegralNumber()) {
          LOG.warning("Invalid 'serial_no' field, the observation ignored");
          continue;
        }

        String hiveSampleID = String.valueOf(serial_no.asLong());
        JsonNode signal = result.get("signal");
        if(signal == null || !signal.isTextual()) {
          LOG.warning("Invalid 'signal' field, the observation ignored");
          continue;
        }

        String materialName = signal.asText();
        if(Strings.isNullOrEmpty(materialName)) {
          LOG.warning("Invalid material name, the observation ignored");
          continue;
        }

        JsonNode confidenceNode = result.get("confidence");
        if(confidenceNode == null || !confidenceNode.isIntegralNumber()) {
          LOG.warning("Invalid 'confidence' field, the observation ignored");
          continue;
        }

        float confidence = (float)confidenceNode.asInt() / 100;
        JsonNode classNode = result.get("class");
        if(classNode == null || !classNode.isTextual()) {
          LOG.warning("Invalid 'class' field, the observation ignored");
          continue;
        }

        BiologicalMaterial.Predefined materialClass = BiologicalMaterial.Predefined.UNKNOWN;
        String hiveClass = classNode.asText();
        switch(hiveClass.trim().toLowerCase()) {
          case "bacteria":
            materialClass = BiologicalMaterial.Predefined.BACTERIA;
            break;
          case "viruses":
            materialClass = BiologicalMaterial.Predefined.VIRAL;
            break;
          case "fungi":
            materialClass = BiologicalMaterial.Predefined.SPORE;
            break;
          case "eukaryota":
          case "archea":
            materialClass = BiologicalMaterial.Predefined.PATHOGEN;
            break;
          case "bioengineered":
            materialClass = BiologicalMaterial.Predefined.UNSPECIFIED;
            break;
          default:
            break;
        }

        observations.add(new BioMaterial(
            materialName,
            materialClass,
            String.format("%s-%s-%s", UUID.randomUUID().toString().replace("-", ""), hiveObjectID, hiveSampleID),
            isHarmful,
            confidence,
            hiveObjectID,
            hiveSampleID,
            materialPosition));
      }
    }

    return observations;
  }

  private static synchronized ArrayList<BioMaterial> getObservations(final ModelFactory factory) {
    ArrayList<BioMaterial> observations = parseHiveOutput(getHiveOutput(), factory);
    if(observations == null || observations.isEmpty()) {
      LOG.info("No new HIVE observations");
    }

    return observations;
  }

  private static Integer confirmPublishing(BioMaterial input) {
    LOG.info("Received Event for sampleID " + input.getSampleID() + ", the publishing was successful.");
    StringBuilder sb = new StringBuilder();
    if(!Strings.isNullOrEmpty(HIVE_SESSION_ID)) {
      sb.append(String.format("sessionID=%s&", HIVE_SESSION_ID));
    }

    sb.append(String.format("cmdr=daMessageReceived&ids=%s&subids=%s", input.getHiveObjectID(), input.getHiveSampleID()));
    String args = sb.toString();
    LOG.info("confirmPublishing: Running process [" + HIVE_TOOL_PATH + "] with args: " + args);
    String result = ProcessRunner.run(ImmutableSet.of(0), HIVE_TOOL_PATH, args);
    final String statusToken = "Status:";
    int idx = result.indexOf(statusToken);
    if(idx == -1) {
      LOG.warning("No HTML status code in the output");
      return -1;
    }

    String statusCode = result.substring(idx + statusToken.length());
    idx = statusCode.indexOf('\n');
    if(idx == -1) {
      LOG.warning("Missing new line after status code in the output");
      return -1;
    }

    String status = statusCode.substring(0, idx).trim();
    if(!status.equals("200")) {
      LOG.warning("The result is not 200 OK:" + status);
      return -1;
    }

    String objectID = statusCode.substring(idx).trim();
    boolean match = input.getHiveObjectID().equals(objectID);
    LOG.info("Result of HIVE confirmation for sampleID " + input.getSampleID() + ": " + match);
    return match ? 0 : -1;
  }

  private static ComponentBehavior createBiologicalMaterialBehavior(
          final ModelFactory factory,
          final GeographicPosition clientPosition,
          final StandardIdentity targetIdentity
  ) {
    final BiologicalReadingObservableHandler biologicalReadingObservable = new BiologicalReadingObservableHandler(factory, ReadyState.READY, ReportingState.REPORTING);

    final BackcastObservableHandler backcastObservable = new BackcastObservableHandler(factory, ReadyState.READY, ReportingState.REPORTING);

    final BiologicalLevelObservableHandler biologicalLevelObservable = new BiologicalLevelObservableHandler(factory, ReadyState.READY, ReportingState.REPORTING);

    final PositionObservableHandler clientPositionObservable = new PositionObservableHandler(factory, ReadyState.READY, ReportingState.REPORTING);

    final IdentityObservableHandler targetIdentityObservable = new IdentityObservableHandler(factory, ReadyState.READY, ReportingState.REPORTING);

    return new AbstractComponentBehavior() {

      @Override
      public ImmutableSet<ObservableHandler> getObservables() {
        return ImmutableSet.of(
            biologicalReadingObservable,
            backcastObservable,
            biologicalLevelObservable,
            clientPositionObservable,
            targetIdentityObservable
        );
      }

      @Override
      public void initialize(final BehaviorControls controls) {
        controls.getScheduler().schedule(
          new Runnable() {
            @Override
            public void run() {
              waitForCondition(clientSubscribed);
              LOG.info("Starting biological observations polling process");
              controls.getScheduler().scheduleAtFixedRate(
                new Runnable() {
                  @Override
                  public void run() {
                    final ArrayList<BioMaterial> observations = getObservations(factory);
                    if(observations != null && !observations.isEmpty()) {
                      for(BioMaterial bioMaterial : observations) {
                        publishBiologicalObservation(
                            factory,
                            controls.getScheduler(),
                            controls.getComponent(),
                            clientPosition,
                            targetIdentity,
                            bioMaterial,
                            biologicalReadingObservable,
                            backcastObservable,
                            biologicalLevelObservable,
                            clientPositionObservable,
                            targetIdentityObservable
                        );
                      }
                    }
                  }
                }, 1L, OBSERVATIONS_POLLING_INTERVAL, TimeUnit.SECONDS
              );

              if(!COMMAND_DIRECTORY.isEmpty()) {
                LOG.info("Starting command directory [" + COMMAND_DIRECTORY + "] monitoring process");
                controls.getScheduler().schedule(
                  new Runnable() {
                    @Override
                    public void run() {
                      try {
                        Path cmdDir = Paths.get(COMMAND_DIRECTORY);
                        if(Files.exists(cmdDir)) {
                          Files.walkFileTree(cmdDir, new SimpleFileVisitor<Path>() {
                            @Override
                            public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                              Files.delete(file);
                              return FileVisitResult.CONTINUE;
                            }

                            @Override
                            public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                              Files.delete(dir);
                              return FileVisitResult.CONTINUE;
                            }
                          });
                        }

                        Files.createDirectories(cmdDir);
                      } catch (IOException e) {
                        LOG.warning("Failed to create command directory: " + e.getMessage());
                      }

                      DirectoryMonitor monitor = new DirectoryMonitor();
                      Function<AbstractMap.SimpleImmutableEntry<Path, Integer>, Integer> callback = new Function<AbstractMap.SimpleImmutableEntry<Path, Integer>, Integer>() {
                        @Override
                        public Integer apply(AbstractMap.SimpleImmutableEntry<Path, Integer> input) {
                          LOG.info("Event for command file: " + input.getKey() + ", type: " + input.getValue());
                          if(input.getValue() == DirectoryMonitor.EVENT_TYPE_CREATE) {
                            final ArrayList<BioMaterial> observations = getObservations(factory);
                            if(observations != null && !observations.isEmpty()) {
                              for(BioMaterial bioMaterial : observations) {
                                publishBiologicalObservation(
                                    factory,
                                    controls.getScheduler(),
                                    controls.getComponent(),
                                    clientPosition,
                                    targetIdentity,
                                    bioMaterial,
                                    biologicalReadingObservable,
                                    backcastObservable,
                                    biologicalLevelObservable,
                                    clientPositionObservable,
                                    targetIdentityObservable
                                );
                              }
                            }

                            try {
                              Files.delete(input.getKey());
                            } catch (IOException e) { }
                          }

                          return 0;
                        }
                      };

                      monitor.start(COMMAND_DIRECTORY, callback);
                    }
                  }, 1L, TimeUnit.SECONDS
                );
              }
            }
          }, 1, TimeUnit.SECONDS
        );
      }
    };
  }

  private static synchronized void publishBiologicalObservation(
      final ModelFactory factory,
      final Executor executor,
      final Component component,
      final GeographicPosition clientPosition,
      final StandardIdentity targetIdentity,
      final BioMaterial bioMaterial,
      final BiologicalReadingObservableHandler biologicalReadingObservable,
      final BackcastObservableHandler backcastObservable,
      final BiologicalLevelObservableHandler biologicalLevelObservable,
      final PositionObservableHandler clientPositionObservable,
      final IdentityObservableHandler targetIdentityObservable
  ) {
    Futures.submit(
      new Runnable() {
        @Override
        public void run() {
          try {
            LOG.info(String.format("Publishing the event with biomaterial: sampleID:%s, name:%s, class:%s, harmful:%b, confidence:%f", bioMaterial.getSampleID(), bioMaterial.getMaterialName(), bioMaterial.getMaterialClass().toString(), bioMaterial.isHarmful(), bioMaterial.getConfidence()));
            final BiologicalMaterial biologicalMaterial = factory.newBiologicalMaterial(bioMaterial.getMaterialClass());
            final BiologicalReading biologicalReading = factory.newBiologicalReading(
                Optional.fromNullable(biologicalMaterial),
                Optional.<Count>absent(),
                Optional.<MeasuredPpm3>absent(),
                bioMaterial.isHarmful(),
                Optional.<Count>absent(),
                Optional.<MeasuredSecs>absent());

            final GeographicPosition observationPosition = bioMaterial.getGeoPosition() != null ? bioMaterial.getGeoPosition() : clientPosition;
            UTC now = TimeUtils.currentTimeUTC(factory);
            ExtendedModelFactory extendedModelFactory = ExtendedModelFactory.newInstance(factory);
            final NameValuePair nvp = factory.newNameValuePair("Position", observationPosition);
            final Posit posit = extendedModelFactory.newPosit(now, factory.newPercent(bioMaterial.getConfidence()), ImmutableList.of(nvp));

            CBRNModelFactory cbrnModelFactory = CBRNModelFactory.newInstance(factory);
            final BiologicalReadingLevel biologicalReadingLevel = cbrnModelFactory.newBiologicalReadingLevel(
                Optional.fromNullable(biologicalMaterial),
                Optional.fromNullable(bioMaterial.getMaterialName()),
                Optional.<Count>absent(),
                Optional.of(bioMaterial.isHarmful()),
                Optional.<MeasuredSecs>absent(),
                factory.newPercent(bioMaterial.getConfidence()),
                Optional.<String>absent(),
                Optional.<MeasuredPpm3>absent(),
                Optional.fromNullable(bioMaterial.getSampleID()),
                ImmutableList.<NameValuePair>of(),
                Optional.<String>absent(),
                Optional.<String>absent(),
                Optional.<MeasuredPsecpm3>absent()
            );

            Function<BioMaterial, Integer> callback = new Function<BioMaterial, Integer>() {
              @Override
              public Integer apply(BioMaterial input) {
                return confirmPublishing(input);
              }
            };

            publications.put(bioMaterial.getSampleID(), new AbstractMap.SimpleImmutableEntry<>(bioMaterial, callback));
            component.observe(
                now,
                Optional.<UTC>of(TimeUtils.after(factory, now, 30.0)), // stale interval
                factory.newEventType(bioMaterial.isHarmful() ? EventType.Predefined.ALERT : EventType.Predefined.MEASUREMENT),
                biologicalReadingObservable.observeSafely(biologicalReading),
                backcastObservable.observeSafely(ImmutableList.of(posit)),
                biologicalLevelObservable.observeSafely(ImmutableList.of(biologicalReadingLevel)),
                clientPositionObservable.observeSafely(observationPosition),
                targetIdentityObservable.observeSafely(targetIdentity)
            );
            LOG.info("Done publishing the event with sampleID " + bioMaterial.getSampleID());
          } catch (Exception e) {
            LOG.warning("Failed to publish the event with sampleID " + bioMaterial.getSampleID() + ": " + e.getMessage());
          }
        }
      }, executor
    );
  }
}
