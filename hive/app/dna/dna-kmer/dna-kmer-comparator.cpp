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
#include <ssci/bio/bioseq.hpp>
#include <ssci/bio/bioseqraw.hpp>
#include <ssci/bio/bioseqkmers.hpp>
#include <ssci/bio/bioseqalign.hpp>


#include <violin/violin.hpp>
#include <slib/std/app.hpp>

using namespace slib;

class DnaKMerComparatorProc: public sQPrideProc
{
    private:
        
    public:
        DnaKMerComparatorProc(const char * defline00, const char * srv) : sQPrideProc(defline00, srv)
        {
           idCnt=0;
        };

        virtual idx OnExecute(idx);

    
        idx myCallbackComp(sBioseqKMers * myThis, const char * id, idx idNum, idx curLength, idx kmerSeq, const char *kmerLetters);


        static idx myCallbackCompStatic(sBioseqKMers * myThis, void * param, const char * id, idx idNum, idx curLength, idx kmerSeq, const char * kmerLetters) 
        {
            return  ((DnaKMerComparatorProc * )param)->myCallbackComp(myThis, id, idNum, curLength, kmerSeq, kmerLetters);
        }

        sVec < sBioseqKMers> perReferenceFile;
        sDic < sDic <idx> > summary;

        sDic <idx> idNumDic;
        idx idCnt;

};


idx DnaKMerComparatorProc::myCallbackComp(sBioseqKMers * myThis, const char * id, idx idNum, idx curLength, idx kmerSeq, const char * kmerLetters){
   idx totRefObjs = perReferenceFile.dim();
   idx totRefs = 0, found=0;
   for (idx irefObj=0; irefObj< totRefObjs; ++irefObj) {
        sBioseqKMers * curRef = perReferenceFile.ptr(irefObj);
        totRefs = curRef->getBigMapDim()/curRef->getCombinations();
        for (idx iref=0; iref<totRefs; ++iref) {
            found = curRef->kmerCount(iref, kmerSeq);
            if (found) {
                if (!summary.find(curRef->m_ref.ref_path.ptr(), curRef->m_ref.ref_path.length())){
                    summary.set(curRef->m_ref.ref_path.ptr(), curRef->m_ref.ref_path.length());
                }
                sDic <idx> * ref = summary.get(curRef->m_ref.ref_path.ptr(), curRef->m_ref.ref_path.length());
                if (!ref->find((const void *)(&iref), sizeof(iref))) {
                    *(ref->set((const void *)(&iref), sizeof(iref)))=0;
                }
                *(ref->get((const void *)(&iref), sizeof(iref)))+=1;
            }
        }
        
   }
   
   if (!idNumDic.find(id)) {
        *(idNumDic.set(id))=1;
        idCnt+=1;
   }
   return 1;
}


idx DnaKMerComparatorProc::OnExecute(idx req)
{
    idx kmer_length = 8;
    sStr subPath;


    const char * genomes = formValue("genome");
    sStr gObjs00;
    sString::searchAndReplaceSymbols(&gObjs00,genomes,0,";",0,0,true,true,true,true);

    
    for (const char * g=gObjs00; g; g=sString::next00(g)) {
        sUsrFile file(g, user);
        subPath.cut(0);
        file.getFile(subPath);
        sFilePath gfile(subPath,"%%dir/%s", "_");

        sBioseqKMers * pNew= perReferenceFile.add(1);
        new (pNew) sBioseqKMers(gfile.ptr(),kmer_length,true,false,sBioseqKMers::eKmerTable_None);
    }


    const char * reads = formValue("reads");
    sStr rObjs00;
    sString::searchAndReplaceSymbols(&rObjs00,reads,0,";",0,0,true,true,true,true);

    for (const char * r=rObjs00; r; r=sString::next00(r)) {
        sUsrFile file(r, user);
        subPath.cut(0);

        file.getFile(subPath);
        sFilePath rfile(subPath,"%%dir/%s", "_");

        sBioseqKMers myReadFile(rfile.ptr(),kmer_length, false, true, sBioseqKMers::eKmerTable_None);
        myReadFile.setCallback((void *)this, DnaKMerComparatorProc::myCallbackCompStatic);
        myReadFile.processRaw();

    }

    if (summary.dim()) {
        idx ilen=0, threshold = 100;
        sFil out_summary(ProcFile("ref-counts.csv", false), sMex::fMapRemoveFile | sMex::fDirectFileStream);

        out_summary.printf(0,"RefPath,iRef,count\n");

        for (idx ipath=0; ipath<summary.dim(); ++ipath) {
            const char * ppath = (const char * )summary.id(ipath, &ilen);
            sDic <idx> * ref = summary.ptr(ipath);
            for (idx ir=0; ir<ref->dim(); ++ir) {
                idx * curRef = (idx *)ref->id(ir);
                idx * cnt = ref->ptr(ir);
                
                if (*cnt < threshold) continue;
                out_summary.printf("\"%.*s\",%lld,%lld\n",(int)ilen,ppath,*curRef,*cnt);

            }

        }
    }

    ::printf("====\n Total: %lld\n",idNumDic.dim());
    ::printf("====\n Total Cnt: %lld\n",idCnt);

    reqProgress(100, 100, 100);
    reqSetStatus(req, eQPReqStatus_Done);

 
    return 0;

}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc,argv);
    DnaKMerComparatorProc backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-kmer-comparator",argv[0]));
    return (int)backend.run(argc,argv);
}




