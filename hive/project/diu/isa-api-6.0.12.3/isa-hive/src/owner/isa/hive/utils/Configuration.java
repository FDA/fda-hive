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
package owner.isa.hive.utils;

import com.google.common.base.Strings;
import org.aeonbits.owner.ConfigFactory;
import mil.army.nvl.common.base.MoreProperties;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Properties;

public class Configuration {
  static public String MY_UCI = "client-no-deploy";
  static public String CONTROLLER_UCI = "controller-no-deploy";
  static public String CONTROLLER_IP = "127.0.0.1";
  static public String CONTROLLER_PORT = "9950";
  static public String KEY_DIRECTORY = "./keys";
  static public String MASTER_KEY_FILENAME = "master.key";
  static public String MIL2525_DEFAULT_TARGET_ID = "SUGP-----------";
  static public boolean IS_LOCAL_CONTROLLER = true;
  static public long HEARTBEAT_INTERVAL = 30L;
  static public long OBSERVATIONS_POLLING_INTERVAL = 60L;
  static public String COMMAND_DIRECTORY = "";
  static public String HEALTH_STATUS_DIRECTORY = ".";
  static public String HEALTH_STATUS_FILE = "last-seen.txt";
  static public String HIVE_TOOL_PATH = "dna.cgi";
  static public String HIVE_SESSION_ID = "";
  static public String LOG_FILES_DIRECTORY = "./logs";
  static public int LOG_FILES_LIMIT = 1024 * 1024 * 1024;
  static public int LOG_FILES_COUNT = 5;
  static public String PID_FILE="pidfile.pid";
  static public String DEFAULT_GEOLOCATION = "0.0,0.0";

  public static void load(String[] args) {
    Properties props = new Properties();
    try {
      props.load(Files.newInputStream(new File("general.properties").toPath()));
    } catch (IOException ignored) {
    }

    AppConfig config = ConfigFactory.create(AppConfig.class, props, MoreProperties.fromArguments(args));
    MY_UCI = config.getMyUCI();
    CONTROLLER_UCI = config.getControllerUCI();
    CONTROLLER_IP = config.getControllerIP();
    CONTROLLER_PORT = config.getControllerPort();
    KEY_DIRECTORY = config.getKeyDirectory();
    MASTER_KEY_FILENAME = config.getMasterKeyFilename();
    MIL2525_DEFAULT_TARGET_ID = config.getMil2525DefaultTatgetID();
    IS_LOCAL_CONTROLLER = CONTROLLER_IP.equals("127.0.0.1") || CONTROLLER_IP.equals("localhost");
    HEARTBEAT_INTERVAL = Long.parseLong(config.getHeartbeatInterval());
    OBSERVATIONS_POLLING_INTERVAL = Long.parseLong(config.getObservationsPollingInterval());
    String cmdDir = config.getCommandDirectory();
    if(!cmdDir.isEmpty()) {
      COMMAND_DIRECTORY = Paths.get(cmdDir).toAbsolutePath().normalize().toString();
    }

    HEALTH_STATUS_DIRECTORY = Paths.get(config.getHealthStatusDirectory()).toAbsolutePath().normalize().toString();
    HEALTH_STATUS_FILE = config.getHealthStatusFile();
    HIVE_TOOL_PATH = config.getHiveToolPath();
    HIVE_SESSION_ID = config.getHiveSessionID();

    String logsDir = config.getLogFilesDirectory();
    if(Strings.isNullOrEmpty(logsDir)) {
      logsDir = LOG_FILES_DIRECTORY;
    }

    LOG_FILES_DIRECTORY = Paths.get(logsDir).toAbsolutePath().normalize().toString();
    LOG_FILES_LIMIT = Integer.parseInt(config.getLogFilesLimit());
    LOG_FILES_COUNT = Integer.parseInt(config.getLogFilesCount());

    PID_FILE = Paths.get(config.getPidFile()).toAbsolutePath().normalize().toString();
    DEFAULT_GEOLOCATION = config.getDefaultGeolocation();
  }
}
