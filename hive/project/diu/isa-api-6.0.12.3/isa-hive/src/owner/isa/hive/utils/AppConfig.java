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

import org.aeonbits.owner.Accessible;
import org.aeonbits.owner.Config.DefaultValue;
import org.aeonbits.owner.Config.Key;

public interface AppConfig extends Accessible {
  @DefaultValue("client-no-deploy")
  @Key("my_uci")
  String getMyUCI();

  @DefaultValue("controller-no-deploy")
  @Key("controller_uci")
  String getControllerUCI();

  @DefaultValue("127.0.0.1")
  @Key("controller_ip")
  String getControllerIP();

  @DefaultValue("9950")
  @Key("controller_port")
  String getControllerPort();

  @DefaultValue("./keys")
  @Key("key_directory")
  String getKeyDirectory();

  @DefaultValue("master.key")
  @Key("master_key_filename")
  String getMasterKeyFilename();

  @DefaultValue("SUGP-----------")
  @Key("mil2525_default_target_id")
  String getMil2525DefaultTatgetID();

  @DefaultValue("30")
  @Key("heartbeat_interval")
  String getHeartbeatInterval();

  @DefaultValue("60")
  @Key("observations_polling_interval")
  String getObservationsPollingInterval();

  @DefaultValue("")
  @Key("command_directory")
  String getCommandDirectory();

  @DefaultValue(".")
  @Key("health_status_directory")
  String getHealthStatusDirectory();

  @DefaultValue("last-seen.txt")
  @Key("health_status_file")
  String getHealthStatusFile();

  @DefaultValue("dna.cgi")
  @Key("hive_tool_path")
  String getHiveToolPath();

  @DefaultValue("")
  @Key("hive_session_id")
  String getHiveSessionID();

  @DefaultValue("./log")
  @Key("log_files_directory")
  String getLogFilesDirectory();

  @DefaultValue("1073741824")
  @Key("log_files_limit")
  String getLogFilesLimit();

  @DefaultValue("5")
  @Key("log_files_count")
  String getLogFilesCount();

  @DefaultValue("pidfile.pid")
  @Key("pid_file")
  String getPidFile();

  @DefaultValue("0.0,0.0")
  @Key("default_geolocation")
  String getDefaultGeolocation();
}
