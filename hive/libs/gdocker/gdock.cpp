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
#include <gdocker.hpp>

idx sGDockerProc::OnExecute(idx req)
{
    return sGLauncherProc::OnExecute(req);
}
idx sGDockerProc::loadDockerImage(const char * dockerIDorName, sStr * pathDocker)
{
    sStrT cmd,buf;
    sStrT PathDocker;if(!pathDocker) pathDocker=&PathDocker;
    sUsrObj serviceAlgo;
    user->uniqueObjectAndPath( "sysdocker", dockerIDorName , &serviceAlgo, pathDocker);

    sStrT dockerImageName;serviceAlgo.propGet("docker_imagename",&dockerImageName);

    idx docker_image_id_Loaded=0;
    sPipe psex;psex.exeSys(&buf,cmd.printf(0,"docker images -q --filter \"reference=%s\"",dockerImageName.ptr()));
    if(buf.length())sscanf(buf.ptr(),"%llx",&docker_image_id_Loaded);
    idx docker_image_id_fromProp=serviceAlgo.propGetX("docker_imageid");

    if(docker_image_id_Loaded && docker_image_id_Loaded==docker_image_id_fromProp) {
        return docker_image_id_Loaded;
    }

    if(docker_image_id_Loaded) {
        buf.cut(0);psex.exeSys(&buf,cmd.printf(0,"docker kill  $(docker ps -q --filter=\"ancestor=%s\");",dockerImageName.ptr()));
        buf.cut(0);psex.exeSys(&buf,cmd.printf(0,"docker rmi %s",dockerImageName.ptr()));
    }

    buf.cut(0);psex.exeSys(&buf,cmd.printf(0,"docker load -i %s/_docker.tar",pathDocker->ptr()));
    if(buf.length())sscanf(buf.ptr(),"%llx",&docker_image_id_Loaded);

    psex.exeSys(&buf,cmd.printf(0,"docker images -q --filter \"reference=%s\"",dockerImageName.ptr()));
    if(buf.length())sscanf(buf.ptr(),"%llx",&docker_image_id_Loaded);
    user->m_SuperUserMode=true;
    serviceAlgo.propSet("docker_imageid",buf.printf("%llx",docker_image_id_Loaded));
    user->m_SuperUserMode=false;


    return docker_image_id_Loaded;
}

idx sGDockerProc::processExecute(idx req)
{
    idx dockerId;
    sPipe mps;
    sStrT dockerContainerName,oo,cmdps;
    propSet("dockerContainerName",dockerContainerName.printf("OBJ-%" DEC,objs[0].Id().objId()));

    mps.exeSys(&oo, cmdps.printf(0,"docker ps | grep '%s'",dockerContainerName.ptr()), 0,0) ;
    if(oo.length()) {
        sscanf(oo.ptr(),"%llx",(udx*)(&dockerId));
        if(dockerId)return dockerId;
    }

    loadDockerImage(formValue("sysdocker"));
    return sGLauncherProc::processExecute(req);
}

idx sGDockerProc::processLaunch(const char * cmdLine, const char * sessionFile)
{
    sPipe ps;
    sStr dst;
    ps.exeSys(&dst,cmdLine);
    if(!dst.length())return 0;
    *dst.ptr(12)=0;
    sscanf(dst.ptr(0),"%llx",&pid);
    return pid;
}

idx sGDockerProc::processCheck(idx pid)
{
    sPipe mps;
    sStr oo,cmdps;
    cmdps.printf(0,"docker ps | grep %llx", pid);
    mps.exeSys(&oo, cmdps.ptr(0), 0,0) ;
    if(!oo.length())return 0;
    sscanf(oo.ptr(0),"%llx",&pid);
    return pid;
}

idx sGDockerProc::processStop(idx pid)
{
    sStr cmdps;
    sPS ps;ps.execute(cmdps.printf(0,"docker kill %llx", pid));
    return pid;
}


