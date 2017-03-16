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

extern sConSockLib _sockLib;


idx sConClient::connect(const char * strHostName,idx nHostPort, idx timeOut)
{
    tmOut=timeOut;
    _sockLib.init();

    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    
    // make a socket
    hSocket=(idx)socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);if(hSocket == SOCKET_ERROR)return err();

    // set options
    timeval t;t.tv_sec=(long)timeOut; t.tv_usec=0;
    if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t))==SOCKET_ERROR) return err();


    // get IP address from name and fill address struct 
    pHostInfo=gethostbyname(strHostName);if(!pHostInfo)return err();
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);
    Address.sin_addr.s_addr=nHostAddress; 
    Address.sin_port=htons((u_short)nHostPort);
    Address.sin_family=AF_INET;
    
    /* connect to host */
    if(::connect(hSocket,(struct sockaddr*)&Address,sizeof(Address))== SOCKET_ERROR)return err();
    
    
    return hSocket;
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/                                          _/
_/  ACTION FUNCTIONS                        _/
_/                                          _/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

/* 
        downloads HTTP text from <urlText> with CGI/POST <data> until <timeOut> .
*/

void * sConClient::getHTTP(sMex * http, const char * urlText,const char * data, idx datalen, idx timeout, idx allowreloc)
{
    sStr url;
    idx  portN=HTTPport;
    idx pos=http->pos();
    if(!urlText)return 0;

    // get the url and the port number 
    const char * p=strstr(urlText,"http://");if(!p)p=urlText;else p+=7;
    sString::copyUntil(&url,p,0,":/");
    const char * fileB=strchr(p,'/');if(!fileB)fileB="";else ++fileB;
    p=sString::searchSubstring(p,0,":",1,"?",false);if(p)portN=atoi(p+1); //  strchr(p,':')
    if(!data)data="";


    //vFile cgiM("tmp.tmp");cgiM.cut(0);
    sStr cgiM;
    
    if(!strncmp(data,"POSTMULTI",9)){data+=10; /* if not a post method */
        sStr boundary;sString::copyUntil(&boundary,data,0,"\r\r\n");
        cgiM.printf(
            "POST /%s HTTP/1.0\r\n"
            "Accept: text/html, text/plain\r\n"
            //"Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Type: multipart/form-data; boundary=%s\r\n"
            "Host: %s:%" DEC "\r\n"
            "Content-Length: %" DEC "\r\n"
            //"Connection: Keep-Alive\r\n"
            "\r\n"
            ,fileB,boundary.ptr(),url.ptr(),portN,datalen-10);
        cgiM.add(data,datalen-10);
        cgiM.add("\r\n\r\n",2);

    }
    else if(!strncmp(data,"POST",4)){data+=5; /* if not a post method */
        cgiM.printf(
            "POST /%s HTTP/1.0\r\n"
            "Accept: text/html, text/plain\r\n"
            //"User-Agent: Mozilla/4.0 (compatible; vLib)\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Host: %s:%" DEC "\r\n"
            "Content-Length: %" DEC "\r\n"
            //"Connection: Keep-Alive\r\n"
            "\r\n"
            ,fileB,url.ptr(),portN,datalen-5);
        cgiM.add(data,datalen-5);
        cgiM.add("\r\n\r\n",2);
    }
    else {if(!strncmp(data,"GET",4))data+=4;
        cgiM.printf(   /* get request */
            "GET /%s%s%s HTTP/1.0\r\n"
            "Accept: text/html, text/plain\r\n"
            "User-Agent: vLib\r\n"
            "Host: %s:%" DEC "\r\n"
            //"Connection: Keep-Alive\r\n"
            "\r\n"
            ,fileB,( data && *data) ? "?": "" , data,url.ptr(),portN);
    }
    
    
    
    sConClient con(url.ptr(),portN,timeout);if(!con.ok())return 0;
    con.send(cgiM.ptr(0),cgiM.length());
    sMex reloc;
    idx lenDownloaded=0,originalLen=http->pos();
    for( idx i=0; lenDownloaded==originalLen && i<timeout+1 ; ++i ) {    
        con.recieve(http,0,"\n\n" _ "\r\n\r\n" __, 0, allowreloc ? &reloc : 0 );
        lenDownloaded=http->pos();
        sleepSeconds(1);
    }

    if(allowreloc && reloc.pos() ) {
        sString::cleanMarkup((char *)reloc.ptr(), reloc.pos(), "Location:" __,"\n" __,"",1,true,false, false);
        sString::cleanEnds((char *)reloc.ptr(), reloc.pos(),sString_symbolsBlank,true);
        if(sLen(reloc.ptr())){
            http->cut(pos);
            //url.printf("%s",reloc.ptr()); 
            return sConClient::getHTTP(http, (const char *)reloc.ptr(),data, datalen, timeout, allowreloc-1);

        } 
    }
    //vMem::Del(srcM);

    return http->ptr();
}


/*
        sends mail from <from> to <to>  with content <content>.
*/

idx sConClient::sendMail(sStr * responseout,  const char * server, idx portN, const char * from, const char * recipients, const char * to, const char * cc,const char * subject, const char * content, idx timeout)
{
    idx code = 0;
    const char* msg = 0;
    if(!portN)portN=25;

    if( !from || !from[0] ) {
        code = 900;
        msg = "missing From";
    }
    else if( !recipients || !recipients[0] ) {
        code = 901;
        msg = "empty list of recipients";
    }
    if( code != 0 ) {
        if( responseout ) {
            responseout->printf("%" DEC " %s", code, msg);
        }
        return code;
    }

    #ifdef WIN32
        static idx socketInitialized=0;
        if(!socketInitialized) { 
            WSADATA sckdat;
            WSAStartup(2,&sckdat);
            socketInitialized=1;
        }
    #endif

    sStr rcpt;
    sString::searchAndReplaceStrings(&rcpt, recipients, 0, "," __, "\r\nrcpt to: " __, 0, true);
    sStr mailM;
    mailM.printf(
        "helo %s\r\n"
        "mail from: %s\r\n"
        "rcpt to: %s\r\n"
        "data\r\n"
        , "vlib", from, rcpt.ptr());
    mailM.printf("From: %s\r\n", from);
    if( to ) {
        mailM.printf("To: %s\r\n", to);
    }
    if( cc ) {
        mailM.printf("Cc: %s\r\n", cc);
    }
    if( subject ) {
        mailM.printf("Subject: %s\r\n", subject);
    }
    mailM.printf(
        "\r\n"
        "%s\r\n"
        ".\r\n"
        "quit\r\n\r\n"
        , content ? content : "");

    sConClient con(server, portN, timeout);
    if( !con.ok() ) {
        return 0;
    }
    con.send(mailM.ptr(0), mailM.length());

    sStr response;
    if( !responseout ) {
        responseout = &response;
    }
    con.recieve(responseout, 0, "\n\n" _ "\r\n\r\n" __);

    idx errCODESTART = 400;
    for(const char * ptr = responseout->ptr(); ptr; ptr = strchr(ptr, '\n')) {
        code = 0;
        sscanf(ptr, "%" DEC, &code);
        if(code >= errCODESTART ) {
            return code;
        }
        ++ptr;
    }
    return 0;
}
