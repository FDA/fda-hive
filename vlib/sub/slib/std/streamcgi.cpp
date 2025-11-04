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

#include <slib/std/streamcgi.hpp>
#include <slib/std/file.hpp>

using namespace slib;


idx sStreamCGI::parseInput(int argc, const char * argv[], const char *envp[], FILE * fp)
{
    sStr singleVar, singleVarName;
    if(!fp)fp=(argc>1) ? fopen(argv[1],"r") : stdin;
    if(gDebug>0)
    {
        {
            sFil f("tmp/post.in",sMex::fMapRemoveFile);
            f.readIO(fp);
        }
        if(fp!=stdin){fclose(fp);}
        fp=fopen("tmp/post.in","r");
    }

    const char * p;
    const char * method=getenv("REQUEST_METHOD") ; if (!method) method="GET";
    const char * content_type=getenv("CONTENT_TYPE");
    const char * multipart_boundary=(content_type && (memcmp(content_type,(p="multipart/form-data; boundary="),30)==0)) ? content_type+30 : argv[2];
    idx content_length= ( argc<2 ) ? ((p=getenv("CONTENT_LENGTH")) ? atoidx(p) : sIdxMax) : sFile::size(argv[1]);

    sStr mp;
    multipart_boundary=mp.printf("--%s",multipart_boundary);

    struct {
        const char * look;
        idx len;
    } srch[]={
        {multipart_boundary,sLen(multipart_boundary)}
        ,{"Content-Disposition: form-data; ",32}
        ,{"Content-Type: ",14}
    };


    sStr Buf;Buf.add(0,streamBufSize+256);
    char * buf=Buf.ptr();buf[0]='\r'; buf[1]='\n';

    enum eMode {expectBoundary=0,expectContentDisposition,expectContentType,expectContentBody,expectEmpty};
    idx mode=sFlag(0),len=-1,pos=0,cur=2,nxt,it=expectBoundary;
    const char * input_name=0,* input_filename=0;
    idx input_name_length=0,input_filename_length=0,content_type_length=0, tot_len=0, tot_pos=0;

    while( tot_len<content_length ) {

        len=streamBufSize-cur;
        len=(idx )stream_fread(buf+cur,1,len,fp);
        if(!len)break;
        tot_len+=len;
        len+=cur;
        if(!len && stream_feof(fp)) break;

        cur=0;
        for(pos=0; pos<len-1;++pos, ++tot_pos) {
            if(buf[pos]=='\r' && buf[pos+1]=='\n'){
                if(cur<len-2 && buf[cur]=='\r' && buf[cur+1]=='\n') {
                    if( mode&sFlag(expectContentBody) )
                        it=expectContentBody;
                    else it=expectEmpty;
                }
                else {
                    for ( it=0; it<sDim(srch) ; ++it ) {
                        if( mode&sFlag(it) && memcmp(buf+cur,srch[it].look,srch[it].len)==0 ) {
                            cur+=srch[it].len;
                            break;
                        }
                    }
                }

                if(it==expectContentBody && memcmp(buf+pos+2,srch[expectBoundary].look,srch[expectBoundary].len)!=0)
                    continue;


                if(it==expectBoundary) {
                    mode=sFlag(expectContentDisposition);
                }
                else if(it==expectEmpty) {
                    mode=sFlag(expectBoundary);
                    if(tot_pos)mode|=sFlag(expectContentBody);
                }
                else if(it==expectContentDisposition) {
                    mode=sFlag(expectContentType)|sFlag(expectEmpty);

                    input_name=0;input_filename=0;
                    input_name_length=0;input_filename_length=0;

                    for(nxt=cur; nxt<len && (buf[nxt]!='\r' || buf[nxt+1]!='\n') ; ++nxt ){
                        if(memcmp(buf+nxt,"name=",5)==0 ) {
                            nxt+=5;
                            input_name=buf+nxt;
                        }
                        if(buf[nxt]==';' && input_name && !input_name_length)
                            input_name_length=buf+nxt-input_name;
                        if(memcmp(buf+nxt,"filename=",9)==0 ) {
                            nxt+=9;
                            input_filename=buf+nxt;
                        }
                        if(buf[nxt]==';' && input_filename && !input_filename_length)
                            input_filename_length=buf+nxt-input_filename;
                    }
                    if(!input_name_length && input_name)input_name_length=buf+nxt-input_name;

                    if(!input_filename_length && input_filename)input_filename_length=buf+nxt-input_filename;
                    char ch;
                    if( input_name){
                        ch=input_name[0];
                        if(ch=='\'' || ch=='\"') {
                            input_name++;input_name_length--;
                            if(input_name[input_name_length-1]==ch)
                                input_name_length--;
                        }
                    }
                    if(input_filename) {
                        ch=input_filename[input_filename_length-1];
                        if( ch=='\'' || ch=='\"' ){
                            input_filename_length--;
                            if(input_filename[0]==ch)
                                {input_filename++;input_filename_length--;}
                        }
                    }
                    pos=nxt;
                    tot_pos+=nxt-cur;
                    content_type=0;
                    content_type_length=0;

                    if(read_var || read_var_len) {
                        read_var=0;
                        read_var_len=0;
                    } else if(loadMode){
                        if(dataIsOpen){
                            dataIsOpen=false;
                            dataClose();
                        }
                    }
                }
                else if(it==expectContentType) {
                    mode=sFlag(expectEmpty);
                    content_type=buf+cur;
                    content_type_length=pos-cur;
                }
                else if(mode&sFlag(expectContentBody)){
                    if(!dataIsOpen) {
                        if((!input_filename) && (!content_type)) {
                            read_var=input_name;read_var_len=input_name_length;
                        } else if(loadMode){
                            if(!dataIsOpen){
                                dataIsOpen=true;
                                dataOpen(content_type, content_type_length,input_name,input_name_length, input_filename, input_filename_length);
                            }
                        }
                    }

                    mode=sFlag(expectContentBody)|sFlag(expectBoundary);
                    if(pos>cur) {
                        if(read_var || read_var_len) {
                            if(singleVar.length()) {
                                singleVar.add(buf+cur,(pos-cur));
                                vars.inp(singleVarName.ptr(0),singleVar.ptr(0),-singleVar.length());
                                singleVar.cut(0);
                                singleVarName.cut(0);
                            }else {
                                vars.inpf(buf+cur,-(pos-cur),"%.*s",(int)read_var_len,read_var);
                            }
                            if(read_var_len==4 && memcmp(read_var,"load",4)==0 && atoidx(buf+cur) )
                                loadMode=1;
                            if(read_var_len==7 && memcmp(read_var,"verbose",7)==0 && atoidx(buf+cur) )
                                verbose=true;
                        } else if(loadMode){
                            if(dataIsOpen){
                                idx leftover=dataStream(buf+cur,(pos-cur));
                                pos-=leftover;
                            }
                        }
                    }
                }
                else {
                }
                pos++;
                tot_pos++;
                cur=pos+1;
            }
        }

        if(cur<len){
            if(mode&sFlag(expectContentBody)){
                if(pos>cur){
                    if(!dataIsOpen) {
                        if((!input_filename) && (!content_type)) {
                            read_var=input_name;read_var_len=input_name_length;
                        } else if(loadMode){
                            if(!dataIsOpen){
                                dataIsOpen=true;
                                dataOpen(content_type, content_type_length,input_name,input_name_length, input_filename, input_filename_length);
                            }
                        }
                    }
                    if(read_var || read_var_len) {
                        singleVar.add(buf+cur,(len-cur));
                        if(!singleVarName.length())
                            singleVarName.add(read_var,read_var_len);
                        cur=0;
                    } else if(loadMode){
                        if(dataIsOpen){
                            idx leftover=dataStream(buf+cur,(len-cur));
                            memmove(buf,buf+len-leftover,leftover);
                            cur=leftover;
                        }
                    }
                }
                else
                    cur=0;
            }
            else
                memmove(buf,buf+cur,len-cur);
        }
    }

    if(read_var || read_var_len) {
        read_var=0;
        read_var_len=0;
    } else if(loadMode){
        if(dataIsOpen){
            dataIsOpen=false;
            dataClose();
        }
    }

    if(fp && fp!=stdin && stream_fread==fread)
        fclose(fp);


    if(loadMode)
        streamFinished();
    else {
        const char *cmd=pForm->value("cmd");
        if(!cmd)cmd=pForm->value("cmdr");
        return Cmd(cmd);
    }

    return 0;


}

void sStreamCGI::copyMetadata(sVar * meta, const char * input_name, idx input_name_length)
{
    for (idx iv=0; iv<vars.dim() ; ++iv) {
        idx ksz,dsz;
        const char * key=(const char * )vars.id(iv,&ksz);
        idx pos=sString::compareNUntil(key,input_name,input_name_length,".",false); if(!pos)continue;
        const void * data=vars.value(key,0,&dsz,ksz);
        meta->inpf(data,-dsz,"%.*s",(int)(ksz-(pos+1)),key+pos+1);
    }

}


