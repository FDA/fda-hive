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
#include <qpsvc/dna-qc.hpp>
#include <qpsvc/qpsvc-dna-align-parser.hpp>
#include <qpsvc/archiver.hpp>
#include <qpsvc/screening.hpp>

class DnaParserProc: public sQPrideProc
{
    public:
        DnaParserProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);
};


idx DnaParserProc::OnExecute(idx req)
{
    sStr errmsg;

    sStr sourceSequenceFilePath;
    formValue("sourceSequenceFilePath", &sourceSequenceFilePath, 0);
    if( !sourceSequenceFilePath || sFile::size(sourceSequenceFilePath)==0){
        reqSetInfo(req, eQPInfoLevel_Error, "Source file '%s' missing or corrupted", sourceSequenceFilePath ? sourceSequenceFilePath.ptr() : "unspecified");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }
    const char * objS=formValue("obj");
    sUsrFile obj(sHiveId(objS), user);
    if( !obj.Id() ) {
        reqSetInfo(req, eQPInfoLevel_Error, "Invalid object '%s' sequences are missing or corrupted", objS);
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }




    logOut(eQPLogType_Info, "processing object %s and file '%s'\n", objS, sourceSequenceFilePath.ptr() );
    idx cntFound=0;






    reqSetProgress(req, cntFound, 100);
    reqSetStatus(req, eQPReqStatus_Done);

    return 0;

}


int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaParserProc backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-ionparser",argv[0]));
    return (int)backend.run(argc,argv);
}

