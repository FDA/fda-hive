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
#include <qpsvc/archiver.hpp>


class DNAPEMergerProc: public sQPrideProc
{
    public:
        DNAPEMergerProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {

        }
        ~DNAPEMergerProc()
        {
        }
        virtual idx OnExecute(idx);
};

enum eAlgo { ePear=0, eFlash , eFastP };
idx DNAPEMergerProc::OnExecute(idx req)
{

    idx minLength=formIValue("minLength",80) ;
    bool trimPolyG= formBoolValue("trimPolyG",true) ;

    bool autoArchive= formBoolValue("autoArchive",true) ;

    sVec<sHiveId> ids;
    formHiveIdValues("fwd", &ids);
    formHiveIdValues("rev", &ids);

    sStr cmd,fwdD, revD, dst;
    sPS ps;

    {
        sStr fwdS;
        sUsrObj fwd(*user,ids[0]);
        fwd.getFilePathname(fwdD, "_.fastq.gz");
        if(!fwdD.length()) {
            fwd.getFilePathname(fwdS, "_.zip");
            reqAddFile(fwdD, "fwd.fastq");
            cmd.printf(0, "/bin/unzip -p %s > %s", fwdS.ptr(0), fwdD.ptr(0) );

            ps.execute(cmd);
        }
    }
    reqProgress(1,33, 100);
    {
        sStr revS;
        sUsrObj rev(*user,ids[1]);
        rev.getFilePathname(revD, "_.fastq.gz");
        if(!revD.length()){
            rev.getFilePathname(revS, "_.zip");
            reqAddFile(revD, "rev.fastq");
            cmd.printf(0, "/bin/unzip -p %s > %s", revS.ptr(0), revD.ptr(0) );
            ps.execute(cmd);
        }
    }
    reqProgress(2,66, 100);

    const char * name=formValue("name",0,"merged");
    idx algo=formIValue("algo",1);
    sStr a1;const char * adapter_r1=formValue("adapter_r1",&a1,0);if (adapter_r1 && (*adapter_r1)==0)adapter_r1=0;
    sStr a2;const char * adapter_r2=formValue("adapter_r2",&a2,0);if (adapter_r2 && (*adapter_r2)==0)adapter_r2=0;
    reqAddFile(dst, "merged-%s",name);
    sStrT html,json,logfile,cmdfile,lerrfile;
    reqAddFile(html, "fastP.html");
    reqAddFile(json, "fastP.json");
    reqAddFile(logfile, "fastP.log");
    reqAddFile(lerrfile, "err.log");
    reqAddFile(cmdfile, "cmd.log");


    sStrT  tt;
    prepareMacroExeLaunch(tt);

    if(algo==ePear) {
        tt.printf( "$(obj)per.osLinux -f %s -r %s -o %s", fwdD.ptr(), revD.ptr(0), dst.ptr());
    } else if(algo==eFlash) {
        tt.printf("$(obj)flash.osLinux %s %s -o %s -d \"\" ", fwdD.ptr(), revD.ptr(0), dst.ptr());
    }else if(algo==eFastP) {

        tt.printf( "$(obj)fastP.osLinux "
                "--in1 '%s' --in2 '%s' "
                "--out1 %s.R1.trim.fastq.gz --out2 %s.R2.trim.fastq.gz "
                "--merge --merged_out %s "
                "--json %s --html %s "
                "--thread 4 "
                "--length_required %" DEC " "
                "--correction --reads_to_process 0"
                , fwdD.ptr(), revD.ptr(0)
                , dst.ptr(), dst.ptr()
                , dst.ptr()
                ,json.ptr(0),html.ptr(0)
                ,minLength);


        if(trimPolyG)tt.printf(" --trim_poly_g");
        if(!adapter_r1 && !adapter_r2)tt.printf(" --detect_adapter_for_pe");
        if(adapter_r1)tt.printf(" --adapter_sequence=%s",adapter_r1);
        if(adapter_r2)tt.printf(" --adapter_sequence_r2=%s",adapter_r2);
        tt.printf(" --verbose");
        tt.printf(" >> %s 2>&1",logfile.ptr(0));

        tt.printf("; echo $USER $LD_LIBRARY_PATH $? > %s",lerrfile.ptr(0));
    }
    cmd.cut(0);
    replaceObjMacros(cmd, tt);

    {
        sFil f(cmdfile,sMex::fMapRemoveFile);
        f.printf("command line\n---------------------------------\n%s\n---------------------------------\n\n\n",cmd.ptr());
    }

    system(cmd);

    reqProgress(3,100, 100);

    if(autoArchive) {
        sStr src("hiveseq://%s", dst.ptr());
        dmArchiver archHS(*this, dst.ptr(), src, 0, sFilePath::nextToSlash(dst.ptr()));
        archHS.setScreenFlag(*this,formIValue("launchScreening",0));
        archHS.setIndexFlag(*this,formIValue("launchIndexing",1));
        archHS.setCompressFlag(*this,formIValue("launchCompression",0));
        archHS.setQCFlag(*this,formIValue("launchQC",1));
        archHS.setFolderId(formValue("folder"));
        archHS.addObjProperty("base_tag", "merger/%" DEC ,objs[0].Id().objId());
        archHS.launch(*user, grpId);
    }

    reqSetStatus(req, eQPReqStatus_Done );

    return 0;
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    DNAPEMergerProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-pemerger", argv[0]));
    return (int) backend.run(argc, argv);
}
