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

#include <slib/std/string.hpp>


#include <slib/std/online.hpp>

using namespace slib;

sConSockLib _sockLib;


idx sConServer::listen( idx nHostPort,idx timeOut)
{
    _sockLib.init();
    tmOut=timeOut;

    struct sockaddr_in Address; 
    idx AddressSize=sizeof(Address);
    
    if((hSocket=(idx)socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))== SOCKET_ERROR)return err();

    idx yes=1;
    if (setsockopt(hSocket,SOL_SOCKET,SO_REUSEADDR,(char * )&yes,sizeof(idx)) == SOCKET_ERROR)return err();
    timeval t;t.tv_sec=(long)timeOut; t.tv_usec=0;
    if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t))==SOCKET_ERROR) return err();
    
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons((u_short)nHostPort);
    Address.sin_family=AF_INET;
    if(bind(hSocket,(struct sockaddr*)&Address,sizeof(Address))== SOCKET_ERROR)return err();
    
    #if defined (SLIB64) && defined (WIN32)
        getsockname( hSocket, (struct sockaddr *) &Address,(int *)&AddressSize);
    #else
        getsockname( hSocket, (struct sockaddr *) &Address,(socklen_t *)&AddressSize);
    #endif

    if(::listen(hSocket,SOMAXCONN) == SOCKET_ERROR)return err();


    return hSocket;
}

idx sConServer::accept( sConIP * transactionCon)
{
    struct sockaddr_in Address; 
    idx AddressSize=sizeof(Address);
    
    #if defined (SLIB64) && defined (WIN32)
        transactionCon->hSocket=(idx)::accept(hSocket,(struct sockaddr*)&Address,(int *)&AddressSize);if(transactionCon->hSocket==SOCKET_ERROR) return err();
    #else
        transactionCon->hSocket=(idx)::accept(hSocket,(struct sockaddr*)&Address,(socklen_t *)&AddressSize);if(transactionCon->hSocket==SOCKET_ERROR) return err();
    #endif
    transactionCon->tmOut=tmOut;
    return transactionCon->hSocket;
}

idx sConIP::send(const char * writeBuf,idx writeLen)
{
    if(hSocket<1)return err();
    if(writeBuf && !writeLen) writeLen=(idx )strlen(writeBuf);
    writeLen=::send(hSocket,writeBuf,(int)writeLen , 0 );
    
    return writeLen;
}

void sConIP::disconnect(void) 
{ 
    if(hSocket>0)
        closesocket(hSocket);
    hSocket=0; 
}

idx sConIP::recieve(sMex * rd, idx maxLen, const char * filterHeader00, idx loops, sMex * filtout)
{
    if(hSocket<1)return err();
    sStr tmp; char * tmpBuf=tmp.resize(sSizePage);
    char * filterDone=0;
    idx pos, pos0, il, ln,it=0;

    u_long iMode = 1;
    ioControl(hSocket, FIONBIO, &iMode);
    for(pos=0, pos0=rd->pos(), il=0, ln=sNotIdx; ln!=0; ){

        ln=sSizePage;
        if(maxLen && pos+ln>maxLen ) ln=maxLen-pos;
        if( (ln=recv(hSocket,tmpBuf,(int)ln ,0 ))==0) return err();
        if(ln==SOCKET_ERROR ){
            if( it<tmOut ){sleepSeconds(1);++it;continue;}
            else return sNotIdx; 
        }
        
        it=0;
        rd->add(tmpBuf,ln);

        if(filterHeader00 && !filterDone) {
            if( (filterDone=sString::searchSubstring((const char *)rd->ptr(pos0),rd->pos()-pos0,filterHeader00,1,0,0))!=0 ){
                filterDone+=sString::compareChoice(filterDone,filterHeader00,0,0,0);
                if(filtout){filtout->add(rd->ptr(),(filterDone-((char*)rd->ptr()))); filtout->add(__,2);}
                rd->del(pos0,(idx)(filterDone-((char*)rd->ptr())));
                
            }
        }
        pos+=ln;
        ++il;
        if(loops && il>=loops)break;
    }

    return (idx)(rd->pos()-pos0);
}






idx sConIP::initUDP(idx port)
{
    idx yes=1;
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    

    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((u_short)port);
    
    hSocket = (idx)socket(AF_INET, SOCK_DGRAM, 0) ;
    isErr=setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(idx));if(isErr==SOCKET_ERROR)return 0; 
    #ifdef SO_REUSEPORT
        isErr=setsockopt(hSocket, SOL_SOCKET, SO_REUSEPORT, (char *)&yes, sizeof(idx));if(isErr==SOCKET_ERROR)return 0;
    #endif
    isErr=bind(hSocket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); if(isErr==SOCKET_ERROR)return 0;
    return hSocket;
}

#ifdef SLIB_WIN
    #define SETSOCKOPTPARAM const char *
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #define SETSOCKOPTPARAM const void *
#endif



idx sConIP::sendUDP(const char * serveraddr, idx port, const char * buffer, idx bufSize, bool isSingular)
{
    int sock, n, yes=1;
    struct sockaddr_in server;
    struct hostent *hp;
     memset(&server, 0,sizeof(server));
    server.sin_port = htons((u_short)port);
    int isErr=0;
    
    if(!strcmp(serveraddr,"broadcast")){
        
        sock = (int)socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        isErr=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(int) );
 
        server.sin_addr.s_addr = htons((u_short)-1);
        server.sin_family = PF_INET;
    } else {
        sock= (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);if (sock < 0) return 0;

    
        if(!isSingular)
            isErr=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(int) );
     
            hp = gethostbyname(serveraddr);if (hp==0) return 0;
            memcpy((char *)&server.sin_addr,(char *)hp->h_addr, hp->h_length );
        
        server.sin_family = AF_INET;

    }

    if(!bufSize)bufSize=sLen(buffer);
    n=sendto(sock,buffer, (int)bufSize,0,(sockaddr*)&server,sizeof(struct sockaddr_in));if (n < 0) return 0 ;
    closesocket(sock);
    return isErr ? 0 : n ;
}


