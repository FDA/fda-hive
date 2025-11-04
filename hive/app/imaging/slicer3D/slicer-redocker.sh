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
export QAPP='/home/qpride/bin/qapp -user qapp';


for DN in "docker-slicer3D" "HIVEOrtancDICOMServer"; do

    if [ "$DN" == "docker-slicer3D" ] ; then
        export CN="hive/slicervncwidget"
    elif [ "$DN" == "HIVEOrtancDICOMServer" ] ; then
        export CN="hive/ortancdicomserver"
    fi
    
 
    export lastDockerIDFile="/home/qpride/bin/docker-"$DN".id"
    if [ -f "$lastDockerIDFile" ] ; then
        export lastDockerID=`cat $lastDockerIDFile`
    else
        export lastDockerID="0"
    fi
    
    export dockerid=`$QAPP -query "a=alloftype('user-script',{name:'$DN'}); return a[0];"`

    if [ "$lastDockerID" == "$dockerid" ] ; then
        echo "docker image for $DN has not changed, old one "$lastDockerID" is in place "
    else 
        echo "docker "$DN" has changed, setting up a new one "$dockerid 
        export dockerpath=`$QAPP -qcd $dockerid`
        export dockerfile=`ls -a "$dockerpath"*.tar`
        echo $dockerid > $lastDockerIDFile
        chown qpride.users $lastDockerIDFile
        docker kill  $(docker ps -q --filter="ancestor="$CN)
        docker rmi $CN
        docker load -i $dockerfile
        
        if [ "$DN" == "HIVEOrtancDICOMServer" ] ; then
            cp -f $dockerpath/Configuration.json  /home/qpride/data/orthanc/config/
            chown qpride.users /home/qpride/data/orthanc/config/Configuration.json
        fi
    fi


    
    if [ "$DN" == "HIVEOrtancDICOMServer" ] ; then
        export alive=`echoscu 127.0.0.1 2104 2>&1`
        if [ ! -z "$alive" ] ; then
            echo "docker "$DN" is not responding; restarting" 
            docker run --rm -p 2104:2104 -p 8042:8042 -v /home/qpride/data/orthanc/db:/var/lib/orthanc/db/ --detach -v /home/qpride/data/orthanc/config:/etc/orthanc hive/ortancdicomserver
        fi
    fi
done;
