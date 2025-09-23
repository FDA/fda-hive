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

#include <qpsvc/qpsvc-dna-hexagon.hpp>
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
    sStr sSubj;
    const char * subject = QPSvcDnaHexagon::getSubject00(objs[0],sSubj);
    sHiveseq Sub(sQPride::user, subject);Sub.reindex();
    if(Sub.dim()==0) {
        logOut(eQPLogType_Error,"Reference '%s' sequences are missing or corrupted\n",subject ? subject : "unspecified");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    sStr sQry;
    const char * query=QPSvcDnaHexagon::getSubject00(objs[0],sQry);
    sHiveseq Qry(sQPride::user, query);Qry.reindex();
    if(Qry.dim()==0) {
        logOut(eQPLogType_Error,"Query '%s' sequences are missing or corrupted\n",query ? query : "unspecified");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }


    sBioseq * sub=&Sub;
    sBioseq * qry=&Qry;




    ::printf("Subject has %" DEC " sequences inside\n",sub->dim());
    ::printf("Query has %" DEC " sequences inside\n",qry->dim());

    for( idx i=0; i< sub->dim() ; ++i ){
        const char * seq=sub->seq(i);
        idx len=sub->len(i);

        for ( idx ipos=0; ipos<len; ++ipos) {
            char letter=sBioseqAlignment::_seqBits(seq, ipos)  ;
        }

    }


    reqSetProgress(req, cntFound, 100 );
    reqSetStatus(req, eQPReqStatus_Done);

return 0;
}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);
    sStr tmp;
    sApp::args(argc,argv);
    DnaAlignMulti backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-alignmulti",argv[0]));
    return (int)backend.run(argc,argv);
}
