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
/*
 #include <qpsvc/qpsvc-dna-alignx.hpp>


QPSvcDnaAlignx::QPSvcDnaAlignx(sQPride& qp) : TParent(qp)
{
}


QPSvcDnaAlignx::QPSvcDnaAlignx(sQPride& qp, const char * qry, const char * sub, eAlgorithm algorithm, bool ifMarkResultFileDiff, const char *param)
    : TParent(qp)
{
    algorithmtype = algorithm;
    setQuery(qry);
    setSubject(sub);
    if (param){
         blastNTParam.cut(0);
         blastNTParam.printf("%s", param);
     }
    setAlgorithm();


    if (ifMarkResultFileDiff){
        setVar ("resultFileTemplate","balignment.hiveal");
    }
}

QPSvcDnaAlignx::~QPSvcDnaAlignx()
{
}

void QPSvcDnaAlignx::setQuery(const char * qry)
{
    setVar("query", "%s", qry);
}

void QPSvcDnaAlignx::setSubject(const char * sub)
{
    setVar("subject", "%s", sub);
}

void QPSvcDnaAlignx::setAlgorithm()
{
    //setVar("submitterSvc", "dna-screener");
    setVar("submitterSvc", "censuScope");
    if (algorithmtype == eBlast){
        setVar("alignSelector", "svc-align-blast");
        setVar("blastProgram", "blastn");
        setVar("evalueFilter", "-6");
        setVar("scissors", "27");
        setVar("complexityRefEntropy", "0");
        setVar("complexityRefWindow", "0");
        setVar("complexityEntropy", "0");
        setVar("complexityWindow", "0");
        //setVar("alignSelector", "svc-align-blast");
        setVar("slice", "10000");
        setVar("seedSize", "28");
        setVar("resultFileTemplate", "balignment.hiveal");
    }
    else if (algorithmtype == eBowtie){
        setVar("alignSelector", "svc-align-bowtie");
        setVar("cmdLine", "-n 2 -e 1000 -I 0 -X 250 --maxbts 125 --pairtries 100 --chunkmbs 64 -k 1");
        setVar("slice", "1000000");
    }
    else if (algorithmtype == eTophat){
        setVar("alignSelector", "svc-align-tophat");
        setVar("cmdLine", "-n 0.00002 -M 7 -R 25");
        setVar("slice", "1000000");
        }
    else {
        setVar("alignSelector", "svc-align-bowtie");
        setVar("cmdLine", "-n 2 -e 1000 -I 0 -X 250 --maxbts 125 --pairtries 100 --chunkmbs 64 -k 1");
        setVar("slice", "1000000");
    }
}

void QPSvcDnaAlignx::setobjDefaultParameters()
{
    if (algorithmtype == eBlast){
        if (blastNTParam){
            setVar("cmdLine", blastNTParam.ptr());
        }
        else {
            setVar("cmdLine", "-evalue 1e-4 -max_target_seqs 1 -max_hsps_per_subject 1");
            //setVar("maxMissQueryPercent", "10");
        }
    }
}

*/
