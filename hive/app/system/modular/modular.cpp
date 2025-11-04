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

#include <qlib/QPrideProc.hpp>
#include <slib/std/file.hpp>
#include <ion/sJson.hpp>
#include <ulib/uquery.hpp>
#include <xlib/s_curl.hpp>
#include <slib/utils/tbl.hpp>
#include <violin/hivelink.hpp>
#include <ulib/upropset.hpp>
#include <errno.h>

using namespace sviolin;


class ModularProc: public sQPrideProc
{
    public:
        sStrT error,log,myObjPath,myInternalWWW;
        bool stopOnErrors,includeSourceCode;
        idx cntPackages,cntPackagesDone,lenMyObjPath;
        sDic < idx > packagesDone;
        HIVELink HL;


        ModularProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv), packagesDone(0,sMex::fSetZero)
        {
            stopOnErrors=false;
            includeSourceCode=false;
            cntPackages=0;
            cntPackagesDone=0;
            lenMyObjPath=0;
        }
        ~ModularProc()
        {
        }
        virtual idx OnExecute(idx);


        const char * wdir(const char * dir, bool dochdir=0, sStr * path=0, bool createIfMissing=true)
        {
            if(!dir)return 0;
            sVariant results;

            if(sIs("objqry://", dir )) {
                qlang::sUsrEngine engine(*user, 0);

                if( engine.parse(dir+9, 0, &error) && engine.eval(results, &error) )
                    dir=results.asString();
                else {
                    error.printf("directory '%s' cannot be identified\n", dir);
                    return 0;
                }
            }

            idx idxDir = atoidx (dir) ;
            if (idxDir!=0) {
                sHiveId idDir (idxDir, 0);
                sUsrObj objDir (*user,idDir);
                dir=objDir.getFilePathname(*path);
            } else if(path) {
                path->printf("%s",dir);
            }


            static sStr dirP;
            dirP.cut(0);getVars(&dirP, dir);dir=dirP.ptr(0);

            if( createIfMissing && !sDir::exists(dir) )
                sDir::makeDir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH );

            if( dochdir )
                sDir::chDir(dir);

            return dir;
        }

        const char * wobj(const char * obj, sUsrObj * uobj)
        {
            if(!obj)return 0;
            sVariant results;

            if(sIs("objqry://",obj)) {
                qlang::sUsrEngine engine(*user, 0);

                if( engine.parse(obj+9, 0, &error) && engine.eval(results, &error) )
                    obj=results.asString();
                else {
                    error.printf("package or script '%s' cannot be identified\n", obj);
                    return 0;
                }

            }

            if(uobj){
                sHiveId idObj(obj);
                new (uobj) sUsrObj(*user,idObj);
            }

            return obj;
        }

        idx runPackage(const char * objid, const char * mode, bool followDependencies);
        const char * loginToHIVE(const char * domain, idx domainLen,const char * login, const char * pswd, const char * sessionID);
        idx prepareSourceFiles(const char * sourceFile, idx len, sStr * dstList);
};


idx ModularProc::runPackage(const char * package, const char * mode, bool followDependencies)
{
    idx * pCnt=packagesDone.set(package);
    (*pCnt)++;
    if(*pCnt>1) return 1;


    if(strcmp(mode,"deploy")==0) {
        sUsrPropSet upropset(*user);
        sDic<sUsrPropSet::Obj> modified_objs;
        sStr sys_package_json;sys_package_json.printf("%s/sys_package_%s.json",myObjPath.ptr(),package);
        upropset.setSrcFile(sys_package_json.ptr());
        upropset.run(&modified_objs, sUsrPropSet::fInvalidUserGroupNonFatal) ;
    }

    sUsrObjRes obj_res;
    if( !user->objs2("^sys_package", obj_res,(udx*)0,"name", package) )  {
        return 0;
    }


    sUsrObj obj(*user,*obj_res.id(obj_res.first()));
    const char * objid=obj.IdStr();
    sStrT path;
    wdir(obj.getFilePathname(path),true,0,true);
    const char * name = obj.propGet("name");


    if(followDependencies) {
        sStr depList;obj.propGet00("dependencies",&depList);
        sString::searchAndReplaceSymbols(depList.ptr(), depList.length(),",", 0, 0, true, true, false, true, false);

        for( const char * dep=depList.ptr();dep;dep=sString::next00(dep)){
            idx ret=runPackage(dep, mode, followDependencies);
            if(!ret && stopOnErrors)
                return 0;
        }

    }

    sStrT tdriver,tscript;tdriver.printf("%s_driver",mode);tscript.printf("%s_package",mode);
    sStrT t,cmdLine;

    const sUsrObjPropsNode * cmdlist=obj.propsTree()->find( "cmdlist"), * tN;
    cntPackages+=cmdlist->dim("cmdlist");

    for(const sUsrObjPropsNode * cmdListRow= cmdlist->firstChild(); cmdListRow; cmdListRow = cmdListRow->nextSibling()) {
        const char * thisDir=wdir((tN=cmdListRow->find("dir")) ? tN->value() : 0 ,true, 0, true);
        if(!thisDir && stopOnErrors) {
            error.printf("cannot find directory '%s'",tN->value());
            return 0;
        }

        const sUsrObjPropsNode * elements=cmdListRow->find(mode);

        const char * cmdname=cmdListRow->find("cmd")->value();;
        cmdLine.printf(0, "%s/%s.sh",myObjPath.ptr(),cmdname);


        const char * driver=(tN=elements->find(tdriver)) ? tN->value() : "/bin/bash";
        const char * script=elements->find(tscript)->value();


        if(!script){
            error.printf("missing script for '%s'",cmdname);
            if(stopOnErrors)break;
        }

        sVar * vars=getVars();


        vars->inp("MODULAR_WORKDIR",thisDir);
        vars->inp("MODULAR_PROCDIR",myObjPath.ptr());
        vars->inp("MODULAR_CMDNAME",cmdname);
        vars->inp("MODULAR_COMPRESS",t.printf(0,"rm -f %s%s.tgz; find %s -type f -exec tar czvf %s%s.tgz --no-recursion {} + ; echo %s,%s,%s,%s,%s,%s.tgz  >> %sresults.csv",myObjPath.ptr(),cmdname,includeSourceCode ? "" : "-not -name '*.[ch]pp'", myObjPath.ptr(),cmdname,myInternalWWW.ptr(),objs[0].Id().print(),name,objid,cmdname,cmdname,myObjPath.ptr()));
        vars->inp("MODULAR_DECOMPRESS",t.printf(0,"tar xvfz %s/%s.tgz",myObjPath.ptr(),cmdname));

        {
            sFile::remove(cmdLine);
            sFil fil(cmdLine);fil.cut(0);
            fil.printf("#");
            getVars(&fil, driver); fil.shrink00();
            fil.printf("\n");
            getVars(&fil,"source $(HIVE.environment)\n"); fil.shrink00();
            fil.printf("cd %s/\n",thisDir);
            if(strcmp(mode,"prepare")==0)fil.printf("cp sys_package* %s/\n",myObjPath.ptr());
            getVars(&fil, script); fil.shrink00();
            fil.printf("\n");
        }
        sFile::chmod(cmdLine,S_IRUSR | S_IXUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IXGRP| S_IROTH );

        cmdLine.printf(" > %s/%s.log 2>",myObjPath.ptr(),cmdname);
        const char * errFile=cmdLine.printf("%s/%s.err",myObjPath.ptr(),cmdname);
        if(system(cmdLine.ptr(0))==-1 || sFile::size(errFile)>0){
            if(errno)error.printf("ERR execute '%s': '%s'\n", cmdLine.ptr(), strerror(errno));
            else error.printf("see '%s'\n", sFilePath::nextToSlash(errFile) );
            if(stopOnErrors)break;
        }
        if(sFile::size(errFile)==0) {
            sFile::remove(errFile);
        }

        ++cntPackagesDone;
    }

    return 1;
}

idx ModularProc::prepareSourceFiles(const char * sourceFile, idx len, sStr * dstList)
{
    sIO selfSourceFile;
    if(strcmp(sourceFile,"sourceDomain")!=0) {
        HL.setDomain(sourceFile);
        sourceFile=HL.getFile(strrchr((char*)sourceFile,'/')+1,"results.csv");
        len=HL.lenResponse;
    }
    sTbl tbl;tbl.parse((char*)sourceFile, len);
    for( idx ir=1; ir<tbl.rows(); ++ir){

        HL.loginToHIVE(tbl.get0(0,ir,(idx)0));

        const char * objid=tbl.get0(0,ir,1);

        tbl.get(&myObjPath,ir,5);{idx ll=sLen(myObjPath.ptr());if(*(myObjPath.ptr(ll-1))=='\n')*myObjPath.ptr(ll-1)=0;}
        HL.getFile(myObjPath.ptr(),objid,myObjPath.ptr(lenMyObjPath));

        myObjPath.printf(lenMyObjPath,"sys_package_");tbl.get(&myObjPath,ir,2);myObjPath.shrink00();myObjPath.add(".json");
        HL.getFile(myObjPath.ptr(),objid,myObjPath.ptr(lenMyObjPath));

        myObjPath.cut(lenMyObjPath);
        if(dstList) {
            if(ir>1)dstList->printf(",");
            tbl.get(dstList,ir,2);dstList->shrink00();
        }
    }
    return tbl.rows()-1;
}

idx ModularProc::OnExecute(idx req)
{
    cntPackages=0;
    cntPackagesDone=0;
    error.cut(0);

    stopOnErrors=formBoolValue("stopOnErrors");
    bool followDependencies=formBoolValue("followDependencies");
    sStr packageList,rePackageList;formValues00("package",&packageList) ;
    const char * package = 0;
    const char * cmd=formValue("cmd",0,"prepare");

    myInternalWWW.cut(0);cfgStr(&myInternalWWW,0,"internalWWW");
    HL.initFromProcess(this);

    myObjPath.cut(0);objs[0].getFilePathname(myObjPath);
    lenMyObjPath=myObjPath.length();
    if(strcmp(cmd,"prepare")==0){
        myObjPath.add("results.csv");
        sFile::remove(myObjPath.ptr());
        sFil fl(myObjPath.ptr());
        fl.printf("sourceDomain,sourceObject,packageName,packageID,cmdName,resultFie\n");


        sString::searchAndReplaceSymbols(packageList.ptr(), packageList.length(),",", 0, 0, true, true, false, true, false);
        package=packageList.ptr();
    } else {
        prepareSourceFiles(packageList.ptr(), packageList.length(),&rePackageList);
        sString::searchAndReplaceSymbols(rePackageList.ptr(), rePackageList.length(),",", 0, 0, true, true, false, true, false);
        package=rePackageList.ptr();
    }
    myObjPath.cut0cut(lenMyObjPath);

    for(; package; package= sString::next00(package)) {

        if(!runPackage(package,cmd,followDependencies) && stopOnErrors)
            break;

        reqProgress(cntPackagesDone,cntPackagesDone, cntPackages);
    }

    if(error.length()){
        error.add0();logOut(eQPLogType_Error, error.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
    }
    else {
        reqProgress(cntPackagesDone , cntPackagesDone, cntPackagesDone);
        reqSetStatus(req, eQPReqStatus_Done );
    }

    return 0;
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    ModularProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "modular", argv[0]));
    return (int) backend.run(argc, argv);
}





