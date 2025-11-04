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

idx sCmdLine::isError=0;
idx sCmdLine::verbose=0;

void sCmdLine::parse(const char * cmdLine, idx len, const char * separ, bool onlyCommands)
{
    len=sLen(cmdLine);
    if(!separ)separ=sString_symbolsBlank;
    sStr tmp;

    sString::searchAndReplaceSymbols(&tmp,cmdLine,len,separ,0,0,1,1,1);
    bool prvCmd=false;
    for(char * ptr=tmp.ptr(); ptr ; ptr=sString::next00(ptr,1)){
        if(*ptr){
            idx which, cnt=dim();
            if(onlyCommands) {
                if( sIs("cmd",ptr))  { 
                    prvCmd=true;
                    continue;
                }
                if(!prvCmd)
                    continue;
            }
            
            idx * p=set(ptr,0,&which);
            if(onlyCommands && prvCmd)
                prvCmd=false;
                
            if(cnt==dim())
                p=add(1);
            *p=which;
        }
    }

}


void sCmdLine::init(idx argc, const char ** argv, const char ** envp)
{
    if( memcmp(argv[0]+sLen(argv[0])-4,".cgi",4)==0 ) {
        initCgi(argc,argv,true);
        return ;
    }
    if(argc<2 ) { 
        argc=2;
        static const char * Argv[3]={argv[0],"-help",0};
        argv=Argv;
    }    
    else if( strcmp(argv[1],"cgi")==0 ) {
        initCgi(argc-1,argv+1,true);
        return ;
    }
    idx i=0;
    for(i=0;i<argc;i++) {
        const char * ptr = argv[i];
        idx which,cnt=dim();
        idx * p=set(ptr,0,&which);
        if(cnt==dim())
              p=add(1);
        *p=which;
    }

    if(envp){
        char buf[1024];
        idx icpy;

        for(i=0;envp[i];i++) {
            const char * p=envp[i];
            for( icpy=0; p[icpy]!='=' && p[icpy]; ++icpy)
                buf[icpy]=p[icpy];
            buf[icpy]=0;
            set(buf);
            if(p[icpy]){
                set(p+icpy+1);
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
    if(!dim())
       return 0;
        
    if(!externalVars && cgiVars.dim())externalVars=&cgiVars;
    idx len=0;
    sStr EquCmd;
    char * equCmd=EquCmd.resize(sFilePath::sSizeMax);
    char * arg;
    const char * cmd="", * rCmd=0;
    char  *p ;
    sStr tmp;

    if(doDefaultCommands) {
    }
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

        if(doDefaultCommands){
            if( strcmp(arg,"-help")==0 ) {
                const char * par= (i+1<dim() ? get(i+1) : "-" );if(par[0]!='-') ++i; else par=0;
                for ( idx ir=0; cmds[ir].param!=sNotPtr ; ++ir ) {
                    if( par && strcmp(par,cmds[ir].cmd+1)!=0)continue;
                    ::printf("\t%s%s\n", cmds[ir].cmd,cmds[ir].descr);
                }
                ++i;
                continue;
            }
            else if( strcmp(arg,"-version")==0 ) {
                printf("%s\n",__TIMESTAMP__);
                ++i;
                continue;
            }
            else  if( strcmp(arg,"-verbose")==0 ) {
                const char * par=get(i+1);if(par[0]!='-') ++i; else par=0;
                if(par)verbose=atoidx(par);
                ++i;
                continue;
            }else if(strcmp(arg,"-calc")==0 ){
                const char * expression=get(i+1);if(expression[0]!='-') ++i; else expression=0;
                sIO langIO, errIO;
                sLang lang(&langIO,&errIO);
                lang.parse(expression, sLen(expression) );
                printf("%s\n",lang.reslt.ptr(0));
                ++i;
                continue;
            }
        }
        
        sCmdLine::exeCommand * cmd_cur=cmds;
        for(iCmd=1; (cmd=cmds[iCmd].cmd)!=0; ++iCmd){
            len = sLen(arg);
            const char * pe=strchr(arg,'=');if(pe)len=pe-arg;
            if( !strncmp(arg,cmd ,len) && ( strcmp(cmd,"--set") == 0 || ((arg[len]=='=' || arg[len]=='.' || arg[len]==0) && (cmd[len] == 0 || arg[len] == 0 ) ) ) ){
                rCmd=arg;
                cmd_cur=cmds+iCmd;
                break;
            }
        }
        

        if(cmd==0 && arg[0]=='-' && arg[1]=='-') {iCmd=1; cmd=rCmd=arg;len=2;}

        sDll::Proc dllProc=0;
        sCmdLine::exeCommand * dllCmdExec=0;
        idx iDllCmd=-1;
        if(cmd==0 && arg[0]=='-' && modulePrefix ) {
            const char * module=strchr(arg+1,'-');
            if(module) { 
                sFilePath moduleName;moduleName.makeName(get(0),"%%dir/%s%.*s.so",modulePrefix,(int)(module-arg-1),arg+1);
                
                sDll::Handle dll=cmdDlls.dllopen(moduleName);
                if(dll) {
                    dllProc=cmdDlls.dllproc(dll,moduleName.printf(0,"__on_%.*s",(int)(module-arg-1),arg+1));
                    dllCmdExec=(sCmdLine::exeCommand*)cmdDlls.dllproc(dll,"cmdExes");
                    if(dllProc && dllCmdExec) { 
                        for(iDllCmd=1; (cmd=dllCmdExec[iDllCmd].cmd)!=0; ++iDllCmd){
                            len = sLen(arg);
                            const char * pe=strchr(arg,'=');if(pe)len=pe-arg;
                            if( !strncmp(arg,cmd ,len) && ( strcmp(cmd,"-set") == 0 || ((arg[len]=='=' || arg[len]=='.' || arg[len]==0) && cmd[len] == 0) ) ){
                                rCmd=arg;
                                cmd_cur=dllCmdExec+iDllCmd;
                                break;
                            }
                        }
                    }
                }
            }
        }
        

        if((!cmd_cur->cmd || !cmd_cur->cmdFun) && (!dllProc)){
            ++i;
            if(arg[0]=='-') {
                if(applog)applog->printf(sCmdLine_WRN"Command not found.\n",arg);
                if(applog)applog->printf(sCmdLine_ACT"See available list of commands.\n");
            }
            continue;
        }

        if( (p=strchr(arg+len,'='))!=0 ){
            strcpy(equCmd,p+1);
            *p=0;
        }
        else equCmd[0]=0;

        if(cmd_cur->kind==argNone){

            if(applog)applog->printf(sCmdLine_EXE"%s \n",rCmd);
            if(dllProc)isError=(idx)dllProc(&cmds[iCmd],arg, 0, equCmd, 0) ;
            else isError=exeFunCaller(&cmds[iCmd],rCmd, 0, equCmd, 0) ;
            ++i;continue;
        }

        tmp.cut(0);
        sStr cmdDesc;
        if( !externalVars ) {
            sString::copyUntil(&tmp,cmd_cur->descr,0,"/");
            sString::cleanEnds(tmp.ptr(),0,sString_symbolsBlank,true);
            sString::searchAndReplaceSymbols(&cmdDesc,tmp.ptr(),0,"/" sString_symbolsBlank,0,0,true,true,true);
            tmp.cut(0);
        }
        const char * varnam=cmdDesc.ptr();
        if(!externalVars &&  varnam[0]=='=')
            varnam=sString::next00(varnam);

        for( ++i; i<dim() && (arg=get(i))[0]!='-'; ++i ){
            if(arg[0]=='\\') ++arg;

            if( !externalVars && varnam) {
                vars.inp(varnam, arg);
                varnam=sString::next00(varnam);
            }
            if(cmd_cur->kind==argOneByOne) {
                if(applog)applog->printf(sCmdLine_EXE"%s %s\n",rCmd, arg);
                if(dllProc)isError=(idx)dllProc(&cmds[iCmd],rCmd, arg, equCmd, externalVars? externalVars : &vars) ;
                else isError=exeFunCaller(&cmds[iCmd],rCmd, arg, equCmd, externalVars? externalVars : &vars) ;
            }
            else if(cmd_cur->kind==argAllSpacedList)
                tmp.printf("%s%s",tmp.length() ? " " : "", arg );
            else if(cmd_cur->kind==argAllZeroList)
                tmp.add(arg,sLen(arg)+1);
        }

        if(!onecmd && (!externalVars || !(externalVars->dim())) && rCmd[1]!='-' && (cmd_cur->kind==argAllZeroList || cmd_cur->kind==argAllSpacedList ) && !tmp.ptr()){
            if(applog)applog->printf(sCmdLine_ERR"command requires arguments\n",rCmd);
            if(applog)applog->printf(sCmdLine_ACT"see the description for '%s' command\n",rCmd);
            isError=1;
        }else {
            if(cmd_cur->kind==argAllZeroList){
                tmp.add(__,2);
                if(applog)applog->printf(sCmdLine_EXE"%s %s ... \n",rCmd,tmp.ptr());
                if(dllProc)isError=(idx)dllProc(&cmds[iCmd],rCmd, tmp.ptr(), equCmd, externalVars? externalVars : &vars) ;
                else isError=exeFunCaller(&cmds[iCmd],rCmd, tmp.ptr(), equCmd, externalVars? externalVars : &vars) ;
            }
            else if(cmd_cur->kind==argAllSpacedList){
                tmp.add(__,2);
                if(dbglog)dbglog->printf(sCmdLine_EXE"%s %s\n",rCmd,tmp.ptr());
                if(dllProc) isError=(idx)dllProc(&cmds[iCmd],rCmd, tmp.ptr(), equCmd, externalVars ? externalVars : &vars) ;
                else isError=exeFunCaller(&cmds[iCmd],rCmd, tmp.ptr(), equCmd, externalVars ? externalVars : &vars) ;
            }
        }
        if(isError==sNotIdx)
            return 0;


    }
    return isError;
}
