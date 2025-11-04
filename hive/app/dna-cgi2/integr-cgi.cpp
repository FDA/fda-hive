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
#include <ion/sJson.hpp>
#include <qlib/QPrideCGI.hpp>
using namespace slib;

enum enumIntegrCommands
{
    eIntegrSlicerInit,
    eIntegrLast
};
#define PATIENT_MRN_UID

idx DnaCGI::CmdIntegr(idx cmd)
{
    switch(cmd) {
        case eIntegrSlicerInit: {
            const char * serviceInt=pForm->value("service","slicer3D");
            idx minPort=48000,maxPort=49000,stpPort=4, portIn=0;

            raw=1;noOutHTML=true;
            pForm->inp("raw", "1");

            sStrT uid,prt,fmt;
            #ifdef PATIENT_MRN_UID
                uid.printf("%s",pForm->value("mrn",m_User.Email()));
            #else
                uid.printf("%s.",pForm->value("objType",""));
                uid.printf("%s.",pForm->value("objID",""));
                uid.printf("%s.",pForm->value("widgetID",""));
                uid.printf("%s",pForm->value("user",m_User.Email()));
            #endif


            idx ret=0;
            const char* procID=0;

            sUsrObjRes obj_res;
            if( m_User.objs2(fmt.printf("^svc-%s",serviceInt), obj_res,(udx*)0,"uID", uid.ptr()) ) {
                sUsrObj proc( m_User, *obj_res.id(obj_res.first()) );
                procID=proc.IdStr();

                idx reqID=proc.propGetI("reqID");
                Request r;if(reqID && !requestGet(reqID, &r))reqID=0;

                if(reqID ) {
                        ret=reqReSubmit(reqID);
                        reqSetAction(reqID, eQPReqAction_Run);

                } else {

                    pForm->inp("cmd", "-qpProcSubmit");
                    pForm->inp("svc",serviceInt);
                    pForm->inp("reuseProc",procID);
                    ret = sQPrideCGI::Cmd("-qpProcSubmit");
                }
            }
            else {

                fmt.printf(0,"prop.svc-%s.",serviceInt);
                idx fl=fmt.length(),grp=1;
                #define INP(_v_n,_v_fmt,_v_v) if((_v_v)){fmt.printf(fl,"%s.%" DEC,(_v_n),grp++);prt.printf(0,(_v_fmt),(_v_v));pForm->inp(fmt.ptr(),prt.ptr());}
                #define INP2(_v_n,_v_fmt,_v_v,_v_v2) if((_v_v)){fmt.printf(fl,"%s.%" DEC,(_v_n),grp++);prt.printf(0,(_v_fmt),(_v_v),(_v_v2));pForm->inp(fmt.ptr(),prt.ptr());}

                pForm->inp("cmd", "-qpProcSubmit");
                pForm->inp("svc",serviceInt);
                INP("_type","svc-%s",serviceInt);
                INP("svc","%s",serviceInt);
                INP("action","%s", "2");
                INP("folder","%s","Inbox");
                INP("svcTitle","%s service",serviceInt);
                INP("submitter","%s",serviceInt);
                INP("name","patient-%s",uid.ptr());

                INP("uID","%s",uid.ptr());
                INP("mrn","%s",pForm->value("mrn"));
                INP("objID","%s",pForm->value("objID"));
                INP("widgetID","%s",pForm->value("widgetID",""));
                INP("demo","%s",pForm->boolvalue("demo") ? "true" : "false" );
                INP("share","%s" ,pForm->value("share") );
                INP("user","%s",pForm->value("user",m_User.Email()));
                INP("screenSize","%s",pForm->value("screenSize","1920x1080"));
                INP("imageType","%s" ,pForm->value("imageType") );



                idx portOK=false;
                sConClient cl;
                portIn=minPort;
                for ( idx ip=minPort; ip<maxPort; ip+=stpPort) {
                    portIn=minPort+4*(((idx)getpid() + rand())%((maxPort-minPort)/4));

                    idx hSocket=cl.connect("localhost",portIn, 2);
                    if(!hSocket) {
                        portOK=true;
                        break;
                    }

                    ::close(hSocket);


                }
                if(portOK) {
                    INP("slicerPort","%" DEC ,portIn+0);
                    INP("VNCPort","%" DEC ,portIn+1);
                    INP("noVNCPort","%" DEC ,portIn+2);
                    INP("igtLinkPort","%" DEC ,portIn+3);
                    INP2("url","http://localhost:%" DEC "/vnc.html?port=%" DEC "&resize=scale&autoconnect=true&reconnect=true&password=87654321&quality=2&compression=7&reconnect_delay=500",portIn+2,portIn+1);

                    ret=sQPrideCGI::Cmd("-qpProcSubmit");
                    idx reqID=0, objI=0;
                    if(ret && dataForm.length()) {
                        sscanf(dataForm.ptr(0),"%" DEC ",%" DEC, &reqID,&objI);dataForm.cut(0);
                        if(objI)procID=prt.printf(0,"%" DEC, objI);
                    }
                } else procID=0;
            }

            noOutHTML=false;
            if(!procID) {
                dataForm.printf("{\"error\":\"Cannot initialize or find appropriate %s process on port %" DEC "\"}",serviceInt,portIn);
                outHtml();
                return 1;
            }else {
                pForm->inp("cmd","propget");
                pForm->inp("mode","json");
                pForm->inp("ids",procID);

                return sQPrideCGI::Cmd("propget");
            }

        }
        break;
        default:
            break;
    }

    return 1;
}


