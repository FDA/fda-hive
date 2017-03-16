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
#include <slib/std.hpp>
#include <qlib/qlib.hpp>

#include <slib/utils.hpp>

#include <ssci/bio.hpp>
#include <violin/violin.hpp>


class DnaAlignMulti : public sQPrideProc
{
    public:
        DnaAlignMulti(const char * defline00,const char * srv) : sQPrideProc(defline00,srv)
        {
        }
        virtual idx OnExecute(idx);

};



idx DnaAlignMulti::OnExecute(idx req)
{
    idx cntFound=0;
    const char * subject=formValue("subject");
    sHiveseq Sub(sQPride::user, subject);Sub.reindex();
    if(Sub.dim()==0) {
        logOut(eQPLogType_Error,"Reference '%s' sequences are missing or corrupted\n",subject ? subject : "unspecified");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0; // error
    }

    // load the subject and query sequences
    const char * query=formValue("query");
    sHiveseq Qry(sQPride::user, query);Qry.reindex();
    if(Qry.dim()==0) {
        logOut(eQPLogType_Error,"Query '%s' sequences are missing or corrupted\n",query ? query : "unspecified");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0; // error
    }


    sBioseq * sub=&Sub;
    sBioseq * qry=&Qry;



    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Perform computations
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    ::printf("Subject has %" DEC " sequences inside\n",sub->dim());
    ::printf("Query has %" DEC " sequences inside\n",qry->dim());

    for( idx i=0; i< sub->dim() ; ++i ){
        const char * seq=sub->seq(i);
        idx len=sub->len(i);
        // const char * id = sub->id(i);

        for ( idx ipos=0; ipos<len; ++ipos) {
            char letter=sBioseqAlignment::_seqBits(seq, ipos, 0)  ;
            // ...
        }

    }


    // sStr  str;
    // str.printf("aaaa %s","my god");
    // str.length() and str.ptr();

    // struct ATGC_Count {idx countA, countC, countG, countT;}
    // sVec < ATGC_Count > array;
    // ATGC_Count  * pointer=array.add(100); pointer[0].countT pointer[99].countA


    // sDic < ATGC_Count > dic;
    // absolutely like sVec but has additional functions
    // dic["A"]=0;
    // dic["A"]++;
    // ATGC_Count * dic.set(keypointer, keysize);
    // ATGC_Count * c=dic.get(keypointer, keysize);
    // idx index=dic.find(keypointer, keysize)



    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Analyse results and report
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    reqSetProgress(req, cntFound, 100 );
    reqSetStatus(req, eQPReqStatus_Done);// change the status

return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future
    DnaAlignMulti backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-alignmulti",argv[0]));
    return (int)backend.run(argc,argv);
}
