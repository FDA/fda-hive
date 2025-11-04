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
#include <slib/std/app.hpp>
#include <cflow.hpp>

using namespace slib;


extern "C" {
extern idx Module_OnExecute(void * QP, idx req)
{
    CFlowProc & qp=*((CFlowProc *)QP);







    if(!qp.ensureProcess("p_downloader"))
        return 0;

    if( qp.err( qp.searchArchived("o_sample_reads","p_downloader","nuc-read"),
        "Sequence files is corrupted."))
        return 0;

    if( qp.err( qp.searchObjects("o_pathogen_references", "genome", "taxonomy","DIU_Pathogens"),
        "Pathogen list is missing."))
        return 0;



    if(!qp.ensureProcess("p_hex-alignment"))
        return 0;

    if(!qp.archiveReads("p_aligned_reads","p_hex-alignment", "fastq", 0, "dodo"))
        return 0;

    if( qp.err( qp.searchArchived("o_aligned_reads","p_aligned_reads","nuc-read"),
        "No hits to pathogen references."))
        return 0;



    if( qp.err( qp.searchObjects("o_kraken_db","special","name","krakenNIAD"),
        "Kraken DB corrupted."))
        return 0;

    if(!qp.ensureProcess("p_kraken"))
        return 0;



    qp.reqProgress(100,100, 100);
    qp.reqSetStatus(req, sQPrideBase::eQPReqStatus_Done );

    return 0;
}


}

