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
#include <ulib/ulib.hpp>
#include <violin/violin.hpp>
#include <xlib/dmlib.hpp>

void getAndParse(idx gi, idx req, sStr & flnmInput, sStr & tempDir);

struct genes {
        sStr start;
        sStr end;
        sStr locus_tag;
        sStr protein_id;
        sStr db_xref;
};

class dmAnnotProc: public sQPrideProc
{
    public:
        dmAnnotProc(const char * defline00, const char * srv)
                : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);
};

void getAndParse(idx gi, idx req, sStr & flnmInput, sStr & tempDir){
    sFil myInput(flnmInput);
    sStr url;
    url.printf("http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=nuccore&rettype=gbwithparts&remode=text&id=");

    url.printf("%" DEC, gi);

    sStr wgetcmd;
    wgetcmd.printf("wget %s",url.ptr());

    dmLib::RemoteFile genbak(wgetcmd);

    if (!myInput.length()){
        ::printf("\n\tDownloading the file\n");
        }
    else {
        ::printf("\n\tThe file has been already downloaded\n");
        }

    sStr commandline;
    sStr flnmOutput;
    flnmOutput.printf("%sdmAnnot%" DEC "-%" DEC "_parsed.txt", tempDir.ptr(), req, gi);
    commandline.printf(0, "~/code/debug-Linux-x86_64/bin/genbankParser %s > %s", flnmInput.ptr(), flnmOutput.ptr() );
    sPS::execProcess(commandline);
}


idx dmAnnotProc::OnExecute(idx req)
{
    sVar rForm, *pForm = reqGetData(req, "formT.qpride", &rForm);

    idx gi = pForm->ivalue("gi", 0);
    sStr outputBlob;
    for(idx i = 0; i < 5; ++i) {
        outputBlob.printf("%lli - gi = %lli\n", i, gi);
        ::printf("%lli - gi = %lli\n", i, gi);
    }
    sStr tempDir;

    cfgStr(&tempDir, 0, "dmAnnot.gi_genbank");
    ::printf("%s\n",tempDir.ptr());


    sStr flnmInput;
    flnmInput.printf("%sdmAnnot%" DEC "-%" DEC ".gb", tempDir.ptr(), req, gi);
    sFil myInput(flnmInput);


    sStr flnmOutput;
    flnmOutput.printf("%sdmAnnot%" DEC "-%" DEC "_parsed.txt", tempDir.ptr(), req, gi);

    sFil myOutput(flnmOutput);

    if (!myOutput.length()&&!myInput.length()){logOut(eQPLogType_Info, "\n\tERROR Check GI number: %" DEC "\n",gi);}
    else if (!myOutput.length()&&myInput.length())
        {
        logOut(eQPLogType_Info, "\n\tThere is no gene in genbank file,Gi:%" DEC "\n",gi);
        }
    else if (myOutput.length()&&myInput.length())
    {
        logOut(eQPLogType_Info, "\n\tProcessing ..............\n");

        sTbl mytable;
        sStr elementcell;
        mytable.parse(myOutput.ptr(), myOutput.length(), true,  ",");
        ::printf("total cols: %" DEC "\n",mytable.cols());
        ::printf("total rows: %" DEC "\n",mytable.rows());
        mytable.get(&elementcell,mytable.rows()-1,2);
        sStr lastnodes;
        sString::searchAndReplaceSymbols(&lastnodes,elementcell,0,".",0,0,true,true,false,true);
        int totalgen=atoi(sString::next00(lastnodes.ptr(),1));
        ::printf("Total gene: %d\n",totalgen);

        sStr generallocus;
        mytable.get(&generallocus,2,0);

        sDic < sVec <struct genes> > LocusLocus;
        sVec <struct genes> *onevec;
        onevec = LocusLocus.set(generallocus.ptr());
        const char * k = (const char *)LocusLocus.id(0);
        sVec <struct genes> *v = LocusLocus.get(k);

        for (int ro=3;ro<mytable.rows();ro++)
        {

            sStr nametag;
            mytable.get(&nametag,ro,1);

            sStr nodestree; sStr nodes;
            mytable.get(&nodestree,ro,2);
            sString::searchAndReplaceSymbols(&nodes,nodestree,0,".",0,0,true,true,false,true);
            int nodesnumber=atoi(sString::next00(nodes.ptr(),1))-1;

            sStr values;
            mytable.get(&values,ro,3);
            sStr valuesclean;
            sString::cleanEnds(&valuesclean,values.ptr(),values.length(),"\n",true);


            if (strcmp(nametag,"start")==0)
            {
                onevec->add();
                onevec->ptr(nodesnumber)->start.printf("%s",values.ptr());
            }
            else if (strcmp(nametag,"end")==0)
            {
                onevec->ptr(nodesnumber)->end.printf("%s",values.ptr());
            }
            else if (strcmp(nametag,"locus_tag")==0)
            {
                onevec->ptr(nodesnumber)->locus_tag.printf("%s",values.ptr());
            }
            else if (strcmp(nametag,"protein_id")==0)
            {
                onevec->ptr(nodesnumber)->protein_id.printf("%s",values.ptr());
            }
            else if (strcmp(nametag,"db_xref")==0 && !onevec->ptr(nodesnumber)->db_xref.ptr())
            {
                onevec->ptr(nodesnumber)->db_xref.printf("%s",values.ptr());
            }

        }


        int lignenumber=0;
        sStr mylignecsv;
        for(int i = 0; i < totalgen; i++) {

            mylignecsv.printf("%s,%d,%s,%s,%s,%s,%s\n", k, lignenumber + 1, v->ptr(i)->locus_tag.ptr(), v->ptr(i)->start.ptr(), v->ptr(i)->end.ptr(), v->ptr(i)->protein_id.ptr(),v->ptr(i)->db_xref.ptr());
            reqSetProgress(req, 2, (i / 646) * 100);

            lignenumber++;
        }
        ::printf("%s",mylignecsv.ptr());
         reqSetData(req, "table_output.csv", mylignecsv.ptr());
    }

    reqSetData(req, "output.csv", &outputBlob);

    reqSetProgress(req, 0, 100);
    reqSetStatus(req, eQPReqStatus_Done);

    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv);
    dmAnnotProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dmAnnot", argv[0]));
    return (int) backend.run(argc, argv);
}
