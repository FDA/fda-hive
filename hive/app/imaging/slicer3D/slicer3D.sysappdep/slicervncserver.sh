#!/bin/bash
#/*
# *  ::718604!
# * 
# * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
# * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
# * Affiliation: Food and Drug Administration (1), George Washington University (2)
# * 
# * All rights Reserved.
# * 
# * The MIT License (MIT)
# * 
# * Permission is hereby granted, free of charge, to any person obtaining
# * a copy of this software and associated documentation files (the "Software"),
# * to deal in the Software without restriction, including without limitation
# * the rights to use, copy, modify, merge, publish, distribute, sublicense,
# * and/or sell copies of the Software, and to permit persons to whom the
# * Software is furnished to do so, subject to the following conditions:
# * 
# * The above copyright notice and this permission notice shall be included
# * in all copies or substantial portions of the Software.
# * 
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# * DEALINGS IN THE SOFTWARE.
# */

set -u
set -e

# convenience function to send feedback to stderr so stdout can go to csv
errcho(){ >&2 echo $@; }

script_dir=$(cd $(dirname $0) || exit 1; pwd)
errcho $script_dir

#
# Usage
#

Usage()
{
  echo "SlicerVNCServer v0.8 (build 20230525)"
  echo ""
  echo "Usage:  $0 -behavior <run> [<OPTIONS>]"
  echo  "       $0 -behavior <list>"
  echo  "       $0 -behavior <stop> <ContainerName>"
  echo  ""
  echo  "These calls are also possible:"
  echo  ""
  echo  "        -b|--behavior run"
  echo  "        -b|-behavior list"
  echo  "        -b|-behavior stop"
  echo  ""
  echo  "The following options are valid only for run behavior:"
  echo  "        -uID|--userID <NUMBER>"
  echo  "        -gID|--groupID <NUMBER>"
  echo  "        -sss|--slicerScreenSize <WIDTH>x<HEIGHT>"
  echo  "        -xsss|--xServerScreenSize <WIDTH>x<HEIGHT>"
  echo  "        -dcn|--dockerContainerName <STRING>"
  echo  "        -di|--dicomImportFolder <STRING>"
  echo  "        --patientMRN <patientMRN-NUMBER>"
  echo  "        --imageType <DICOM-IMAGE-TYPES>"
  echo  "        -id|--uniqueIdPath <STRING>"
  echo  "        -d|--detached"
  echo  "        -u|--user <USERNAME>"
  echo  "        -tsk|--totalSegmentatorApiKey <APIKEY-NUMBER>"
  echo  "        -tsp|--totalSegmentatorPort <PORT-NUMBER>"
  echo  "        -tsf|--totalSegmentatorFolder <STRING>"
  echo  "        -sp|--slicerPort <PORT-NUMBER>"
  echo  "        -vp|--vncPort <PORT-NUMBER>"
  echo  "        -dp|--dicomServerPort <PORT-NUMBER>"
  echo  "        -ch|--currentHostname <HOSTNAME>"
  echo  "        --processId <NUMBER>" 
  echo  "        --noGPU"
  echo  "        -t|--terminal"
  echo  "        -cts|--closeTimeoutSeconds"
  echo  ""
  echo  "In case of missing arguments the server will input some or all this information."
  echo """
      behavior=run
      userID=1001
      groupID=1001
      slicerScreenSize=1920x1080
      xServerScreenSize=1920x1080
      dockerContainerName=
      dicomImportFolder=NO_DICOM
      uniqueIdPath=$script_dir/myvar35674
      detached=
      patientMRN=
      imageType=KEEP_ALL
      user=testUser
      totalSegmentatorApiKey=87654321TS 
      totalSegmentatorPort=47517
      totalSegmentatorFolder=/TotalSegmentatorApp/store/data
      slicerPort=47518
      vncPort=47519
      dicomServerPort=47520
      processId=123
      currentHostname=localhost
      GPU_ACCELERATION_ON=YES
      SHOW_TERMINAL=NO
      closeTimeoutSeconds=300
      """
  echo
  echo """
      behavior: what mode to run the script
      userID: docker user id
      groupID: docker group id
      dockerContainerName: a unique name of the docker container that is persistant according in contrast with
        containerId that is not
      slicerScreenSize: size of the screen of Slicer window, independent of the virtual XServer screensize,
        It should be contained in the xServerScreenSize
      xServerScreenSize: Size of the virtual screen on which TurboVNC renders
      dicomImportFolder: absolute folder path that contains DICOM data to be copied to the
        DICOMDatabase. The contents of this folder should be erased after import since
        we want to import them only on the first session execution. Default: NO_DICOM
      uniqueIdPath: absolute path, unique identifier for a widget and a variable (previously called hiveWidgetFolder).
        Inside this path there will the "Incoming" and "Outgoing" folder to to upload or download data from the form.
      detached: container runs on detached mode (\"-d\" option)
      patientMRN: unique patient identifier number (medical record number)
      imageType: one (or more '|' separated) of the following CT|MRI|Ultrasound|Fluoroscopy|Immunofluorescence|Optical|Video-recording|KEEP_ALL
      user: unique username (with client permissions)
      totalSegmentatorApiKey: api key for total segmentator server requests
      totalSegmentatorPort: port for POST requests to total segmentator server
      totalSegmentatorFolder: path to the tmp folder for total segmentator server
      slicerPort: port that allows single line python function executions on Slicer through
        a POST request
      vncPort: port used to access the VNC server
      dicomServerPort: port used to access the dicom server
      processId: number of the pid session
      currentHostname: this parameter should be replaced by the public domain of the server
      GPU_ACCELERATION_ON: tell the server to look for a gpu and use it for volume rendering
        while in Slicer
      SHOW_TERMINAL: load only a terminal on the VNC session (sometimes useful for debug)
      closeTimeoutSeconds: set idle timeout seconds to save workspace and exit Slicer 
      """
}

#set constants
VNCDISPLAY=:1
device_dev_dri_flag="--device /dev/dri"
gpus_all_flag="--gpus=all"
define_VGL_DISPLAY_envvar=""
detached=""
set_container_name=""

#set defaultValues
behavior="run"
dockerContainerName=""
userID=1001
groupID=1001
slicerScreenSize="1920x1080"
xServerScreenSize="1920x1080"
dicomImportFolder="NO_DICOM"
dockerDicomImportFolder="NO_DICOM"
patientMRN=
imageType="KEEP_ALL"
hiveWidgetFolder="$script_dir/myvar35674"
user="testUser"
totalSegmentatorApiKey=87654321TS
totalSegmentatorPort=47517
totalSegmentatorFolder="/TotalSegmentatorApp/store/data"
slicerPort=47518
vncPort=47519
dicomServerPort=47520
controlToken=87654321
viewToken=12345678
currentHostname=localhost
GPU_ACCELERATION_ON=YES
SHOW_TERMINAL=NO
internalIgtLinkPort=18944
processId=
closeTimeoutSeconds=300

dockerSharedFolder=/home/docker/hiveWidgetFolder/
totalSegmentatorTmpFolder=/home/docker/TotalSegmentatorApp/

ARGS=""

while [[ $# -gt 0 ]]; do
  case $1 in
    -b|--behavior)
      behavior="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -dcn|--dockerContainerName)
      dockerContainerName="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -uID|--userID)
      userID="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;  
    -gID|--groupID)
      groupID="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -sss|--slicerScreenSize)
      slicerScreenSize="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -xsss|--xServerScreenSize)
      xServerScreenSize="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -u|--user)
      user="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -id|--uniqueIdPath)
      hiveWidgetFolder="${2%/}"
      ARGS+="hiveWidgetFolder $2 "
      shift # past argument
      shift # past value
      ;;
    -tsk|--totalSegmentatorApiKey)
      totalSegmentatorApiKey="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;    
    -tsp|--totalSegmentatorPort)
      totalSegmentatorPort="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;  
    -tsf|--totalSegmentatorFolder)
      totalSegmentatorFolder="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;    
    -sp|--slicerPort)
      slicerPort="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -vp|--vncPort)
      vncPort="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -np|--dicomServerPort)
      dicomServerPort="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -processId|--processId)
      processId="$2"
      ARGS+="--processId $2 "
      shift # past argument
      shift # past value
      ;;
    -ch|--currentHostname)
      currentHostname="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    -di|--dicomImportFolder)
      dicomImportFolder="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    --patientMRN)
      patientMRN="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    --imageType)
      imageType="$2"
      ARGS+="$1 $2 "
      shift # past argument
      shift # past value
      ;;
    --noGPU)
      GPU_ACCELERATION_ON=NOT
      ARGS+="$1 "
      shift # past argument
      ;;
    -t|--terminal)
      SHOW_TERMINAL=YES
      ARGS+="$1 "
      shift # past argument
      ;;
    -d|--detached)
      detached="-d"
      ARGS+="$1 "
      shift # past argument
      ;;
    -cts|--closeTimeoutSeconds)
      closeTimeoutSeconds="$2"
      ARGS+="--closeTimeoutSeconds $2 "
      shift # past argument
      shift # past value
      ;;
    --help)
      echo
      Usage
      exit 1
      ;;
    -*|--*)
      ARGS+="$1 "
      shift # past argument
      ;;
    *)
      ARGS+="$1 "
      shift
      ;;
  esac
done

ARGS+="--dockerSharedFolder=$dockerSharedFolder "
ARGS+="--totalSegmentatorTmpFolder=$totalSegmentatorTmpFolder "
SlicerLogsFolder="${hiveWidgetFolder}/Logs/Slicer/"
TurboVNCLogsFolder="${hiveWidgetFolder}/Logs/TurboVNC/"
DICOMDatabase="${hiveWidgetFolder}/DICOMDatabase"
Sessions="${hiveWidgetFolder}/Sessions"
IncomingFolder="${hiveWidgetFolder}/Incoming/"
OutgoingFolder="${hiveWidgetFolder}/Outgoing/"

if [ "$behavior" == "run" ]
then
  # remove all networks not used currently by any containers
  docker network prune --force
  # create network if doesn't exist
  networkName="$(basename $hiveWidgetFolder)"
  docker network inspect $networkName >/dev/null 2>&1 || \
      docker network create --driver bridge $networkName
  #
  connect_to_network="--network $networkName"
  #
  echo userID=$userID
  echo groupID=$groupID
  echo slicerScreenSize=$slicerScreenSize
  echo xServerScreenSize=$xServerScreenSize
  echo uniqueIdPath=$hiveWidgetFolder
  echo user=$user
  echo totalSegmentatorApiKey=$totalSegmentatorApiKey
  echo totalSegmentatorPort=$totalSegmentatorPort
  echo totalSegmentatorFolder=$totalSegmentatorFolder
  echo slicerPort=$slicerPort
  echo vncPort=$vncPort
  echo dicomServerPort=$dicomServerPort
  echo processId=$processId
  echo currentHostname=$currentHostname
  echo networkName=$networkName
  echo dicomImportFolder=$dicomImportFolder
  echo detached=$detached
  echo patientMRN=$patientMRN
  echo imageType=$imageType
  echo GPU_ACCELERATION_ON=$GPU_ACCELERATION_ON
  echo SHOW_TERMINAL=$SHOW_TERMINAL
  #
  mkdir -p $SlicerLogsFolder
  mkdir -p $TurboVNCLogsFolder
  mkdir -p $DICOMDatabase
  mkdir -p $Sessions
  mkdir -p $IncomingFolder
  mkdir -p $OutgoingFolder
  # delete previous cidfile if exists
  rm -f ${hiveWidgetFolder}/temp_*
fi

bind_mount_dicom_data=""
if [ "$dicomImportFolder" != "NO_DICOM" ]; then
  dockerDicomImportFolder=/home/docker/DicomFolder
  bind_mount_dicom_data="-v $dicomImportFolder:$dockerDicomImportFolder"
fi
ARGS+="--dockerDicomImportFolder=$dockerDicomImportFolder"

if [ "$dockerContainerName" != "" ]; then
  set_container_name="--name $dockerContainerName"
fi

if [ "$behavior" == "run" ]
then
  if [ "$GPU_ACCELERATION_ON" == "NOT" ]
  then
          device_dev_dri_flag=""
          gpus_all_flag=""
  else
          if [ -z ${VGL_DISPLAY+x} ]; then
                  export VGL_DISPLAY=/dev/dri/card0
                  if [ ! -c /dev/dri/card0 ]; then
                          echo ERROR: /dev/dri/card0 does not exist.
                          exit 1
                  fi
                  if [ ! -r /dev/dri/card0 ]; then
                          echo ERROR: /dev/dri/card0 is not readable.
                          exit 1
                  fi
                  if [ ! -c /dev/dri/renderD128 ]; then
                          echo ERROR: /dev/dri/renderD128 does not exist.
                          exit 1
                  fi
                  if [ ! -r /dev/dri/renderD128 ]; then
                          echo ERROR: /dev/dri/renderD128 is not readable.
                          exit 1
                  fi
          fi
          define_VGL_DISPLAY_envvar="-e VGL_DISPLAY=$VGL_DISPLAY"
  fi

  # pulseaudio
  #export PULSE_SERVER=unix:${XDG_RUNTIME_DIR}/pulse/native

  echo """
  docker run --user "$userID:$groupID" -it \
          $set_container_name \
          $detached \
          $device_dev_dri_flag \
          $gpus_all_flag \
          -p $vncPort:6080 \
          -p $slicerPort:2016 \
          --expose $totalSegmentatorPort \
          --expose $dicomServerPort \
          --expose $internalIgtLinkPort \
          --cidfile=$hiveWidgetFolder/temp_$processId \
          -v $hiveWidgetFolder:$dockerSharedFolder \
          -v $totalSegmentatorFolder:$totalSegmentatorTmpFolder \
          $bind_mount_dicom_data \
          --rm \
          -m 100g \
          --cpus=4 \
          $define_VGL_DISPLAY_envvar \
          $connect_to_network \
          -e "xServerScreenSize=$xServerScreenSize" \
          -e "VNCDISPLAY=$VNCDISPLAY" \
          -e "processId=$processId" \
          -e "GPU_ACCELERATION_ON=$GPU_ACCELERATION_ON" \
          -e "SHOW_TERMINAL=$SHOW_TERMINAL" \
          -e "AllVariables=$ARGS" \
          -v ~/.config/pulse/cookie:/root/.config/pulse/cookie \
          hive/slicervncwidget:latest
  """
  dockerContainerId=$(docker run --user "$userID:$groupID" -it \
          $set_container_name \
          $detached \
          $device_dev_dri_flag \
          $gpus_all_flag \
          -p $vncPort:6080 \
          -p $slicerPort:2016 \
          --expose $totalSegmentatorPort \
          --expose $dicomServerPort \
          --expose $internalIgtLinkPort \
          -v $hiveWidgetFolder:$dockerSharedFolder \
          -v $totalSegmentatorFolder:$totalSegmentatorTmpFolder \
          --cidfile=$hiveWidgetFolder/temp_$processId \
          $bind_mount_dicom_data \
          --rm \
          -m 100g \
          --cpus=4 \
          $define_VGL_DISPLAY_envvar \
          $connect_to_network \
          -e "xServerScreenSize=$xServerScreenSize" \
          -e "VNCDISPLAY=$VNCDISPLAY" \
          -e "processId=$processId" \
          -e "GPU_ACCELERATION_ON=$GPU_ACCELERATION_ON" \
          -e "SHOW_TERMINAL=$SHOW_TERMINAL" \
          -e "AllVariables=$ARGS" \
          -v ~/.config/pulse/cookie:/root/.config/pulse/cookie \
          hive/slicervncwidget:latest)
  #
  
  ContainerName=$(docker ps --filter "id=$dockerContainerId" --format "{{.Names}}")
  errcho
  errcho Execute:
  errcho docker stop $ContainerName
  errcho "to stop the container."
  errcho
  #
  CONTROL_URL="http://${currentHostname}:$vncPort/vnc.html?port=$vncPort&resize=scale&autoconnect=true&reconnect=true&password=$controlToken&quality=2&compression=7&reconnect_delay=500"
  VIEW_URL="http://${currentHostname}:$vncPort/vnc.html?port=$vncPort&resize=scale&autoconnect=true&reconnect=true&password=$viewToken&quality=2&compression=7&reconnect_delay=500"
  errcho "Control URL is ${CONTROL_URL}"
  errcho "View URL is ${VIEW_URL}"
  #
  VNCServerStartIsStarting=
  while [ "$VNCServerStartIsStarting" = "" ]; do
          sleep 1
          VNCServerStartIsStarting=$(cat ${TurboVNCLogsFolder%/}/pid_${processId}_error.log | grep "Log file is" | sed 's/.Log file is//g')
          #echo $VNCServerStartIsStarting
  done
  #
  #Set a OTP and a viewOnly OTP
  docker exec $ContainerName sh -c "echo $controlToken$'\n'$viewToken| /opt/TurboVNC/bin/vncpasswd -f -v >/home/docker/.vnc/passwd 2>/dev/null"
  #
  errcho
  #
  errcho
  errcho Open the NoVNC url, click connect, input one of the two passwords, use Slicer. 
  errcho
  errcho "The hive API server will start."
  errcho Do a POST request:
  REST_API_URL=http://${currentHostname}:${slicerPort}
  errcho curl -X POST "${REST_API_URL}"/hive/loadworkspacefromhash --data 12345678
  errcho
  #
else
  if [ "$behavior" == "stop" ]
  then
    dockerContainerId=$(docker stop ${ContainerName})
    errcho Stopped $ContainerName
  else
    if [ "$behavior" == "list" ]
    then
      docker ps
    else
      echo "Unknown behavior $behavior"
      exit 1
    fi
  fi
fi


