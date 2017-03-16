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
#include <violin/violin.hpp>
#include <dna-denovo/dna-denovoextension.hpp>
#include <math.h>

class DnaDenovoAssemblyProc: public sQPrideProc
{
    public:

        DnaDenovoAssemblyProc(const char * defline00, const char * srv)
                : sQPrideProc(defline00, srv)
        {
            sBioseq::initModule(sBioseq::eACGT);
        } //sQPrideProc(defline00,srv) {}
        virtual idx OnExecute(idx);

};


idx DnaDenovoAssemblyProc::OnExecute(idx req)
{
    sVar rForm, *pForm = reqGetData(req, "formT.qpride", &rForm);

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Read input parameters
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//    sStr path;
//    sBioseq Qry(cfgPath(&path, 0, pForm->value("query"), "violin.queryRepository") , true );
//    if(Qry.dim()==0) {
//        logOut(eQPLogType_Error,"Query sequences are missing or corrupted\n");
//        reqSetStatus(req, eQPReqStatus_ProgError); // set the error status
//        return 0; // error
//    }
    //sStr path;
    //sUsrFile::getPathOrObj( sQPride::user , pForm->value("query"), &path);

//    sStr filepath;

//    strstr();

    const char * query = pForm->value("query");
    sHiveseq hs(sQPride::user, query);
    hs.reindex();
    if( hs.dim() == 0 ) {
        logOut(eQPLogType_Error, "query %s sequences are missing or corrupted\n", query);
        reqSetStatus(req, eQPReqStatus_ProgError); // set the error status
        return 0; // error
    }
    sBioseq & Qry = hs;
    ::printf("%" DEC " sequences are here\n", Qry.dim());


//    sHiveseq ref(sQPride::user, "/home/lsantana/sabin2/Sabin2.vioseq2");
//    sBioseq & Ref = ref;

//    for (idx pos = 0; pos < Ref.dim(); pos++){
//        ::printf("%s pos = %lld rpt = %lld sim = %lld \n",Ref.id(pos), pos, Ref.rpt(pos), Ref.sim(pos));
//        sStr decompressedATGC;
//        sBioseq::uncompressATGC(&decompressedATGC, Ref.seq(pos), 0, Ref.len(pos));
//        ::printf ( "%s\n", decompressedATGC.ptr() );
//    }

//    ::printf("%s pos = %lld rpt = %lld sim = %lld \n",Ref.id(0), (idx)0, Ref.rpt(0), Ref.sim(0));
//    sStr decompressedReference;
//    sBioseq::uncompressATGC(&decompressedReference, Ref.seq(0), 0, Ref.len(0));
//    ::printf ( "%s\n", decompressedReference.ptr() );
//
//    ::printf("%s pos = %lld rpt = %lld sim = %lld \n",Ref.id(1), (idx)0, Ref.rpt(1), Ref.sim(1));
//    sStr decompressedReferenceRev;
//    sBioseq::uncompressATGC(&decompressedReferenceRev, Ref.seq(1), 0, Ref.len(1));
//    ::printf ( "%s\n", decompressedReferenceRev.ptr() );

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ create the tree
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//    sFile::remove("mytestfilename.txt");
//    SeqTree tree(&Qry,"mytestfilename.txt");
    BioseqTree tree(&Qry, 0);
    DnaDenovoAssembly dnv(this);

    idx sizemer = 16;
    idx sizefilter = 1;
    idx rptfilter = 0;

    dnv.dnaDenovoExtension(Qry, tree, sizemer, sizefilter, rptfilter);

//    dnv.dnaDenovoExtension(Qry, tree, sizemer, sizefilter, rptfilter);

    // Print nodes
//    tree.printTree();
//    cleanSeqs();

//    ::printf("\n\n*********** Second Stage Results ***************\n\n");
//
//    }


    ::printf ("\n");
//    char let1;
//    idx seq;
//    idx sequence = 10;
//    bool isReverse;
//    for (idx i=0; i < seqOrder[sequence].contiglen; i++){
//
//        getValues (sequence, i, &seq, &seqpos, &isReverse);
//        let1 = sBioseqAlignment::_seqBits(Qry.seq(seq), seqpos, 0);
//        BioseqTree::printlet(let1);
//    }

    ::printf ("\n");
    //    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//    // _/
//    // _/ output results 
//    // _/
//    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//    ::printf( "\n contigs = %lld \n", seqCount);
//    ::printf( "\n repeated sequences = %lld \n", removeCount);
//      ::printf( "\n unique sequences = %lld \n", uniqueCount);
//    ::printf( "\n maximum listDim = %lld \n", uniqueCount + removeCount);

    reqSetStatus(req, eQPReqStatus_Done); // change the status
    reqProgress(0, 100, 100);
    return 0;
}

int main(int argc, const char * argv[])
{

    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future

    DnaDenovoAssemblyProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-denovoassembly", argv[0]));
    return (int) backend.run(argc, argv);
}
