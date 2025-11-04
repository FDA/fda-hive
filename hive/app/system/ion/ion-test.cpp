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


#include "ion-tools.hpp"
#include <ion/sIon-core.hpp>
extern sIon * ION;




idx __on_test(sIonTools * iap, const char * cmd, const char * args, const char * ,sVar * pForm)
{
    if(sIs(cmd,"-test1")){


        idx relationTypeIndex=0;
        idx relationHasherIndex=4;

        const char * body1="23";
        const char * body2="A";


        sIon::Bucket bucket;
        idx cntList=0;

        sMex toHashBuf;
        bucket.toHash=&toHashBuf;





        idx cntRelationTargets=5;
        sIon::RecordResult recordResults[5];



        ION->getRelationBucketByHash(&bucket, relationTypeIndex, relationHasherIndex,
            &cntList,
            body1, sLen(body1) , body2, sLen(body2), 0 );



        sStr buf;
        while( bucket.found() ) {
              ION->getRelationsByBucketAndIndex(&bucket, relationTypeIndex, recordResults) ;

              for ( idx i=0 ; i<cntRelationTargets; ++i) {
                  ION->getRecordBody(recordResults+i,&buf,0);
                  ::printf(" %s",buf.ptr(0));


              }
              ::printf("\n");
        }

        ::printf("%" DEC " elements\n",cntList);


    } if(sIs(cmd,"-test2")){

        sIonWander wander;
        wander.addIon(0)->ion->init("/hive/vol_xfs/store1/578/037/3037578/ion",sMex::fReadonly);
        const char * qry="a=find.annot(id=\"$count\" , type=\"$letter\"); print(a.seqID, a.type, a.pos)";
        sIO errbuf;
        wander.traverseCompile(qry, sLen(qry), &errbuf, true);


        sIonWander::TraverseParamReference * p1=wander.getSearchDictionaryPointer("$count",6);
        sIonWander::TraverseParamReference * p2=wander.getSearchDictionaryPointer("$letter",7);
        p1->body=(void*)"23";
        p1->size=2;
        p2->body=(void*)"A";
        p2->size=1;
        wander.traverse();

        wander.traverseBuf.cut(0);

        p1->body=(void*)"23";
        p1->size=2;
        p2->body=(void*)"C";
        p2->size=1;


        wander.traverse();


        




    } else if(sIs(cmd,"-testSorter")){


        sIon::RelationIterator it;
        ION->getRelationSorterIterator("annot", args, &it);

        sIon::RecordResult rr[2];
        rr[0].typeIndex=1;
        rr[1].typeIndex=0;


        sStr Buf, * buf=&Buf;

        for(idx i=0; i<it.cntRelations; ++i){
            ION->getRelationBodyBySortIterator(&it, rr, sDim(rr) , 1 , i%4);


            buf->printf(0,"RECORD %" DEC " \n",i);
            for ( idx ic=0; ic<sDim(rr); ++ic) {
                sIon_outTextBody(buf,rr[ic].cType, rr[ic].body, rr[ic].size, ',') ;
                buf->printf(" " );
            }
            ::printf("%s\n\n",buf->ptr());
        }

        return 0;
    }
    return 0;
}


