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
        }
        virtual idx OnExecute(idx);

};

idx DnaDenovoAssemblyProc::OnExecute(idx req)
{

    sHiveId objID;
    objID = objs[0].Id();
    sUsrObj obj(*user, objID);

    if( !obj.Id() ) {
        logOut(eQPLogType_Info, "Object %s not found or access denied", objID.print());
        reqSetInfo(req, eQPInfoLevel_Error, "Object %s not found or access denied", objID.print());
        return 1;
    } else {
        logOut(eQPLogType_Info, "processing object %s\n", objID.print());
    }



    sVec<sHiveId> objids;

    obj.propGetHiveIds("query_objId", objids);
    sHiveseq hs(user, objids[0].print(), sBioseq::eBioModeShort);

    if( hs.dim() == 0 ) {
        logOut(eQPLogType_Error, "query %s sequences are missing or corrupted\n", objids[0].print());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    sBioseq & Qry = hs;
    ::printf("%" DEC " sequences are here\n", Qry.dim());





    BioseqTree tree(&Qry, 0);
    DnaDenovoAssembly dnv(this);

    idx sizemer = 64;
    idx sizefilter = 1;
    idx rptfilter = 0;

    dnv.setInitialParameters(sizemer, sizefilter, rptfilter, true, 1000);
    dnv.setMissmatchesPercent(100);
    dnv.dnaContigExtension(Qry, tree, sizemer, sizefilter, rptfilter, false);





    fprintf(stderr, "\n");

    fprintf(stderr, "\n");

    {
        sStr path_buf;
        sFil outFile;
        const char *name = "results";
        const char *ext = ".fasta";
        const char * path = reqAddFile(path_buf, "%s%s", name, ext);
        if( path && outFile.init(path) && outFile.ok() ) {
    #ifdef _DEBUG
            logOut(eQPLogType_Trace, "Created %s", path);
    #endif
        } else {
            logOut(eQPLogType_Error, "Failed to create %s%s", name, ext);
            return false;
        }

        idx seqCount = dnv.printResult(&outFile, &Qry, 1000);
        fprintf(stderr, "we generated: %" DEC " sequences\n", seqCount);
    }
    {
        sFil outFile;
        sStr path_buf;

        const char * stats = reqAddFile(path_buf, "stats.txt");
        if( stats && outFile.init(stats) && outFile.ok() ) {
    #ifdef _DEBUG
            logOut(eQPLogType_Trace, "Created %s", stats);
    #endif
        } else {
            logOut(eQPLogType_Error, "Failed to create %s", stats);
            return false;
        }
        dnv.someStats(&outFile, 1000);
    }

    reqSetStatus(req, eQPReqStatus_Done);
    reqProgress(-1, 100, 100);

    return 0;
}

int main(int argc, const char * argv[])
{

    sStr tmp;
    sApp::args(argc, argv);

    DnaDenovoAssemblyProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-denovoassembly", argv[0]));
    return (int) backend.run(argc, argv);
}
