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
#pragma once
#ifndef sLib_std_online_hpp
#define sLib_std_online_hpp

#include <slib/core/str.hpp>


    #ifdef WIN32
        #ifndef AF_IPX
        #include <Winsock2.h>
        #endif
        typedef idx socklen_t;
    #else
        #include <sys/ioctl.h>
        #include <netinet/in.h>
        #include <netdb.h>
        #include <sys/socket.h>
        #define closesocket close
        #define SOCKET_ERROR (-1)
    #endif

namespace slib {

    // this class initalizes the library at first use and deinitializes at the end 
    class sConSockLib
    {
    public:
        idx libInit;
        
        idx init(void) {
            if(libInit)return 1;
            #ifdef WIN32
                static WSADATA ws;
                libInit=WSAStartup( MAKEWORD( 2, 2 ) , &ws) ? 0 : 1;
            #endif
            return libInit;
        }
        sConSockLib() {libInit=0;}
        ~sConSockLib() {
            #ifdef WIN32
                WSACleanup( );
            #endif
        }
    };


    class sConIP
    {
        public:
            idx isErr;
            idx hSocket;
            idx tmOut;
            
        public:
            sConIP(){hSocket=0;tmOut=0;isErr=0;}
            
            ~sConIP(void){disconnect();}

            void disconnect(void);
            
            idx send(const char * writeBuf,idx writeLen=0);
            idx recieve(sMex * rd, idx maxLen=0, const char * filterHeader00=0, idx loops=0, sMex * filtout=0);
            bool ok(void) {return isErr ? false : true;}
            idx err(idx errCode=SOCKET_ERROR){isErr=errCode;return 0;}
            
            idx initUDP(idx port);
            static idx sendUDP(const char * serveraddr, idx port, const char * buffer, idx bufSize, bool isSingular);


            
    };

    class sConClient:public sConIP
    {
            
        public:
            sConClient(const char * strHostName,idx nHostPort, idx timeOut=0)
                {connect(strHostName,nHostPort, timeOut);}
            sConClient(){}

            idx connect(const char * strHostName,idx nHostPort, idx timeOut=0);

            static const idx HTTPport=80;
            static void * getHTTP(sMex * http, const char * urlText,const char * data=0,idx datalen=0, idx timeout=30, idx allowreloc=0);
            // Difference between recipients and to is that they may include cc and bcc!!
            // recipients, to, cc can be CSV
            static idx sendMail(sStr * responseOut, const char * server, idx portN, const char * from, const char * recipients, const char * to, const char * cc, const char * subject, const char * content, idx timeout=30);
            template <class Tobj> static void * getHTTPObj(Tobj * pVal, const char * urlText,const char * data=0,idx datalen=0, idx timeout=30)
            {
                sMex http;
                if( !getHTTP(&http, urlText,data=0,timeout) )return 0;
                return (Tobj*)memmove(pVal,http.ptr(),sizeof(Tobj));
            }


    };

    class sConServer:public sConIP
    {
        public:
            sConServer(idx nHostPort, idx timeOut=0)
                {listen(nHostPort, timeOut);}
            sConServer(){}

            idx listen( idx nHostPort,idx timeOut);
            idx accept( sConIP * transaction);
    };

    
}
#endif


