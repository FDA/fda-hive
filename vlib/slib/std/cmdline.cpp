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
#include <slib/std/file.hpp>
#include <slib/std/cmdline.hpp>

using namespace slib;

void sCmdLine::parse(const char * cmdLine, idx len, const char * separ)
{
    len=sLen(cmdLine);
    if(!separ)separ=sString_symbolsBlank;
    sStr tmp;

    sString::searchAndReplaceSymbols(&tmp,cmdLine,len,separ,0,0,1,1,1);
    for(char * ptr=tmp.ptr(); ptr ; ptr=sString::next00(ptr,1)){
        if(*ptr){
            idx which, cnt=dim();
            idx * p=set(ptr,0,&which);
            //if(which<dim()-1)
            if(cnt==dim()) // this was in the dictionary before
                p=add(1);
            *p=which;
        }
    }

}


void sCmdLine::init(idx argc, const char ** argv, const char ** envp)
{
    idx i=0;
    for(i=0;i<argc;i++) {
        const char * ptr = argv[i];
        idx which,cnt=dim();
        idx * p=set(ptr,0,&which);
        //if(which<dim()-1)
        if(cnt==dim()) // this was in the dictionary before
              p=add(1);
        *p=which;
    }

    if(envp){
        char buf[1024];
        idx icpy;

        for(i=0;envp[i];i++) {
            //const char * p=strchr(envp[i],'=');
            const char * p=envp[i];
            for( icpy=0; p[icpy]!='=' && p[icpy]; ++icpy)
                buf[icpy]=p[icpy];
            buf[icpy]=0;
            //if(p) *p=0; // remember the current
            set(buf); // envp
            if(p[icpy]){
                set(p+icpy+1);
               // *p='='; // restore the environment
            }
        }
    }
}

idx sCmdLine::exeFunCaller(sCmdLine::exeCommand * cmdexe, const char * cmd, const char * arg, const char * equCmd, sVar * vars)
{
    idx res = 0;
    if( exeFunCallBack ) {
        res = exeFunCallBack(cmdexe->param, cmd, arg, equCmd, vars);
    }
    if( !res ) {
        res = (cmdexe->cmdFun)(cmdexe->param, cmd, arg, equCmd, vars);
    }
    return res;
}

idx sCmdLine::exec(sCmdLine::exeCommand * cmds, sVar * externalVars, sStr * applog, sStr * dbglog, char * onecmd)
{
    if(!dim())return 0;
    idx len=0;//, iRemCmd=0;
    sStr EquCmd;
    char * equCmd=EquCmd.resize(sFilePath::sSizeMax);
    char * arg; // ="";
    const char * cmd="", * rCmd=0;
    char  *p ;
    sStr tmp;


    //scan and execute commands one by one
    for(idx i = 0, iCmd = 0; i < dim();)
    {
        if(isError) {
            if(applog)applog->printf(sCmdLine_ERR"stopping execution due to error(s)\n",cmd);
            break;
        }
        if(onecmd) {
            arg=onecmd;
            i=dim();
        }
        else
            arg=get(i);

        // search for the command
        for(iCmd=1; (cmd=cmds[iCmd].cmd)!=0; ++iCmd){
            len = sLen(arg);
            const char * pe=strchr(arg,'=');if(pe)len=pe-arg;
            if( !strncmp(arg,cmd ,len) && ( strcmp(cmd,"-set") == 0 || ((arg[len]=='=' || arg[len]=='.' || arg[len]==0) && cmd[len] == 0) ) ){ // //-set commands are treated specially
            //if( !strncasecmp(arg,cmd,len ) ){ // //-set commands are treated specially
                //iRemCmd=i;
                rCmd=arg;
                break;
            }
        }
        // is this a -- command ? - even if it is not in our list we should try finding the macro : so we always treat it
        if(cmd==0 && arg[0]=='-' && arg[1]=='-') {iCmd=1; cmd=rCmd=arg;len=2;}// cmdCmd


        // command not found ? skip it
        if(!cmds[iCmd].cmd || !cmds[iCmd].cmdFun){
            ++i;
            if(arg[0]=='-') {
                if(applog)applog->printf(sCmdLine_WRN"Command not found.\n",arg);
                if(applog)applog->printf(sCmdLine_ACT"See available list of commands.\n");
            }
            continue;
        }

        // prepare equCmd whyle analysing "command=equCmd argument" synthax
        if( (p=strchr(arg+len,'='))!=0 ){
            //strcpy(equCmd,arg+len+1); // if there is an equal sign , read until the next
            strcpy(equCmd,p+1); // if there is an equal sign , read until the next
            //arg[len]=0;
            *p=0;
        }
        else equCmd[0]=0;

        // is this a no-argument command ?
        if(cmds[iCmd].kind==argNone){ // no arguments, just execute

            if(applog)applog->printf(sCmdLine_EXE"%s \n",rCmd);
            isError=exeFunCaller(&cmds[iCmd],rCmd, 0, equCmd, 0) ;
            ++i;continue;
        }

        // prepare the arguments for argument having commands
        tmp.cut(0);
        // digest the description until  // to make double zero string list
        sVar vars;
        sStr cmdDesc;
        if( !externalVars ) {
            sString::copyUntil(&tmp,cmds[iCmd].descr,0,"/");
            sString::cleanEnds(tmp.ptr(),0,sString_symbolsBlank,true);
            sString::searchAndReplaceSymbols(&cmdDesc,tmp.ptr(),0,"/"sString_symbolsBlank,0,0,true,true,true);
            tmp.cut(0);
        }
        // scan all arguments until the next command
        const char * varnam=cmdDesc.ptr();
        if(!externalVars &&  varnam[0]=='=') // this is equCmd
            varnam=sString::next00(varnam);

        for( ++i; i<dim() && (arg=get(i))[0]!='-'; ++i ){
            if(arg[0]=='\\') ++arg;

            if( !externalVars && varnam) {
                //vars.inp(varnam, (arg[0]=='\\') ? arg+1 : arg);
                vars.inp(varnam, arg);
                varnam=sString::next00(varnam);
            }
            // if this is a one by one argument command ... execute it
            if(cmds[iCmd].kind==argOneByOne) {
                if(applog)applog->printf(sCmdLine_EXE"%s %s\n",rCmd, arg);
                isError=exeFunCaller(&cmds[iCmd],rCmd, arg, equCmd, externalVars? externalVars : &vars) ;
            }
            // concatenate the arguments for multiple at once commands
            else if(cmds[iCmd].kind==argAllSpacedList)
                tmp.printf("%s%s",tmp.length() ? " " : "", arg );
            else if(cmds[iCmd].kind==argAllZeroList)
                tmp.add(arg,sLen(arg)+1);
        }

        // check if argument requiring command is called without arguments
        if(!onecmd && rCmd[1]!='-' && (cmds[iCmd].kind==argAllZeroList || cmds[iCmd].kind==argAllSpacedList ) && !tmp.ptr()){
            if(applog)applog->printf(sCmdLine_ERR"command requires arguments\n",rCmd);
            if(applog)applog->printf(sCmdLine_ACT"see the description for '%s' command\n",rCmd);
            isError=1;
        }else {  // execute
            if(cmds[iCmd].kind==argAllZeroList){
                tmp.add(__,2);
                if(applog)applog->printf(sCmdLine_EXE"%s %s ... \n",rCmd,tmp.ptr());
                isError=exeFunCaller(&cmds[iCmd],rCmd, tmp.ptr(), equCmd, externalVars? externalVars : &vars) ;
            }
            else if(cmds[iCmd].kind==argAllSpacedList){
                tmp.add(__,2);
                if(dbglog)dbglog->printf(sCmdLine_EXE"%s %s\n",rCmd,tmp.ptr());
                isError=exeFunCaller(&cmds[iCmd],rCmd, tmp.ptr(), equCmd, externalVars ? externalVars : &vars) ;
            }
        }
        if(isError==sNotIdx)
            return 0;


        // end of the single command treatment
    }
    return isError;
}
