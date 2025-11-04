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


#include <violin/violin.hpp>


class DNAUMProc: public sQPrideProc
{
    public:
        DNAUMProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {

        }
        ~DNAUMProc()
        {
        }
        virtual idx OnExecute(idx);
};

idx DNAUMProc::OnExecute(idx req)
{
    bool autoArchive= formBoolValue("autoArchive",true) ;

    sHiveseq reads(user, formValue("reads"));
    sHiveseq refs(user, formValue("references"));
    idx algorithm=formIValue("algorithm",1);


    sStr cmd,readFlnm, refFlnm,logFlnm,dataFolder,t;
    sPS ps;

    const char * name=formValue("name");


    ProcFile(t.printf(0,"refs-%s.fasta",name) ,false,&refFlnm);
    if(!sFile::exists(refFlnm.ptr())){
        sFil refF(refFlnm,sMex::fForceRemapTruncate);
        refs.printFastX(&refF, false, 0,sIdxMax, 0, true);
    }

    reqProgress(1,10, 100);
    ProcFile(t.printf(0,"%s.fastq",name) ,false,&readFlnm);
    if(!sFile::exists(readFlnm.ptr())){
        sFil readF(readFlnm,sMex::fForceRemapTruncate);
        reads.printFastX(&readF, true, 0, sIdxMax, 0, false);
    }


    reqProgress(2,20, 100);




    constructReqFilePath(logFlnm, req, "umi_collapse.log");
    constructReqFilePath(dataFolder, req, "");

    char * p=strrchr(readFlnm.ptr(0),'.'); if(p)*p=0;
    p=strrchr(refFlnm.ptr(0),'.'); if(p)*p=0;
    if(algorithm==0)cmd.printf(0, "cd %s; $(obj)umi_collapse_qp.sh %s %s", dataFolder.ptr(),readFlnm.ptr(0),refFlnm.ptr(0));
    else if(algorithm==1)cmd.printf(0, "cd %s; $(obj)umi_collapse_fgbio.sh %s %s", dataFolder.ptr(),readFlnm.ptr(0),refFlnm.ptr(0));
    cmd.printf(" >> %s",logFlnm.ptr(0));
    {
        sFil f(logFlnm,sMex::fMapRemoveFile);
        f.printf("command line\n---------------------------------\n%s\n---------------------------------\n\n\n",cmd.ptr());
    }
    sStr tt;
    replaceObjMacros( tt, cmd);
    ps.execute(tt);

    reqProgress(3,100, 100);


    sStr dst;
    ProcFile(t.printf(0,"%s_dedup.fastq",name) ,false,&dst);

    if(autoArchive) {
        sStr src("hiveseq://%s", dst.ptr());
        dmArchiver archHS(*this, dst.ptr(), src, 0, sFilePath::nextToSlash(dst.ptr()));
        archHS.setScreenFlag(*this,formIValue("launchScreening",0));
        archHS.setIndexFlag(*this,formIValue("launchIndexing",1));
        archHS.setCompressFlag(*this,formIValue("launchCompression",0));
        archHS.setQCFlag(*this,formIValue("launchQC",1));
        archHS.setFolderId(formValue("folder"));
        archHS.addObjProperty("base_tag", "umi_collapse/%" DEC ,objs[0].Id().objId());
        archHS.launch(*user, grpId);
    }

    reqSetStatus(req, eQPReqStatus_Done );

    return 0;
}

int main(int argc, const char * argv[])
{

    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);
    DNAUMProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-umi-collapse", argv[0]));
    return (int) backend.run(argc, argv);
}
