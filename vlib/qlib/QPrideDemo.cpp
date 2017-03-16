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
#include <qlib/QPrideProc.hpp>

using namespace slib;

class QPrideHunt:public sQPrideProc
{
public:
    QPrideHunt(const char * defline00,const char * srv):sQPrideProc(defline00,srv) {}
    virtual idx OnExecute(idx req);
};


idx QPrideHunt::OnExecute(idx req)
{
    printf("I am starting");

    sVar rForm, * pForm = reqGetData(req,"formT.qpride",&rForm );

    int chunkSize=pForm->ivalue("chunkSize");
    const char * query=pForm->value("query");


    // ...calculations
    sStr buf;
    buf.printf("jnfckejrn  %d",12);
    buf.add("pointer",5);

    sVec < int > array;
    array[100]=3;
    for ( int i=0; i<1000; ++i ) {
     array[i]=i*3;
    }
    sVec < double > ddd;

    sFil myfile("mfile.txt");

    myfile.printf("kerbfnekrbgekrjbg");

    for ( char * p=myfile.ptr() ; *p ; ++p ){
        p
    }

    printf("I am done");
    return 0;
}


int main(int argc, char * argv[])
{
    sApp::args(argc,argv);

    sStr tmp;
    const char * toConnect= (argc>1 && strcmp(argv[1],"-console")==0) ? "console"__ : "config=qapp.cfg"__ ;
    QPrideHunt backend(toConnect,sQPrideProc::QPrideSrvName(&tmp,"oligo1",argv[0]));
    backend.run(argc,argv);
}
