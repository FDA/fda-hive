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
#include <ulib/ulib.hpp>
#include <violin/violin.hpp>
#include <ssci/bio/vax-bio.hpp>

class dmAnnotProc: public sQPrideProc
{
    public:
        dmAnnotProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {
        }
        virtual idx OnExecute(idx);

        struct myStructObj {
                //any data I migfht need here
                sQPrideProc * qp;
                sVaxAnnotGB * myvax;
                idx counter;
                idx totalSize;
        };

        static idx progress_reporting_ionProviderCallback(idx record, void * param, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize);
};


idx dmAnnotProc::OnExecute(idx req)
{
    sStr errmsg;

    do {
        const sHiveId objID(formValue("obj"));
        const sHiveId qryID(formValue("qry"));
        if( !objID ) {
            errmsg.printf("Invalid objID in %" DEC " request", req);
            break;
        }
        sUsrFile obj(objID, user);
        if( !obj.Id() ) {
            errmsg.printf("Invalid obj in %" DEC " request", req);
            break;
        }
        logOut(eQPLogType_Info, "processing object %s\n", obj.Id().print());
        sStr sourceFilePath;
        formValue("sourceFilePath", &sourceFilePath, 0);
        if( !sourceFilePath ) {
            errmsg.printf("Invalid source file path in %" DEC " request", req);
            break;
        }
        logOut(eQPLogType_Info, "processing file '%s'\n", sourceFilePath.ptr());

        sFilePath outfile;
        if( !obj.addFilePathname(outfile, true, "ion") ) {
            errmsg.printf("failed to create destination");
            break;
        }
        sIon ION(outfile);

        sVaxAnnotGB vax(&ION);

        myStructObj myCallbackObj;
        myCallbackObj.qp=(this);
        myCallbackObj.myvax=&vax;
        myCallbackObj.counter=0;
        myCallbackObj.totalSize=1;

        vax.callbackParam=&myCallbackObj;

        vax.init(sVax::fUseMMap|sVax::fDoNotSupportTableHeader,sourceFilePath.ptr(0));

        //ION.providerLoad(sVax::ionProviderCallback,(void*)&vax,0,0); //,sFlag(sIon::fLoadIncompleteRelation)

        ION.providerLoad(dmAnnotProc::progress_reporting_ionProviderCallback,(void*)&vax);

        sVec<idx> recordTypesUsed;
        ION.sortRelations("annot","seqID pos", pForm->value("sortFile", "possort"), 0 /* sorting function */, 0 /* params */,&recordTypesUsed, sizeof(int),0 );
        // allocate enough space for record results for binary search tree
        sVec < sIon::RecordResult > rr;rr.resize(2*recordTypesUsed.dim());
        idx cnt=recordTypesUsed.dim() ;
        for( idx ir=0; ir<cnt; ++ir) {
            rr[ir].typeIndex=recordTypesUsed[ir];
            rr[ir+cnt].typeIndex=recordTypesUsed[ir];
        }
        ION.buildVTree("annot", pForm->value("sortFile", "possort"), pForm->value("vTreeName","max"), rr.ptr(0), rr.ptr(cnt ),cnt );


        // cast object
        //sUsrObj * annotobj=obj.cast("u-annot");
        sUsrObj * annotobj=obj.cast("u-ionAnnot");
        if( ! annotobj ) {
            errmsg.printf("Cannot cast to u-ionAnnot");
            break;
        }
        sFilePath outputDir(outfile.ptr(),"%%dir/");
        idx ssize = sDir::size(outputDir.ptr());
        annotobj->propSetI("size",ssize);

        annotobj->propSet("annot_source","genbank");

        if (qryID.objId()) {
            annotobj->propSetHiveId("reference_objs",qryID);
            annotobj->propSetHiveId("source_obj",qryID);
        }
    if(annotobj!=&obj)
      delete annotobj;
    } while( false );

    if( !errmsg ) {
        reqProgress(0, 100, 100);
        reqSetStatus(req, eQPReqStatus_Done);
    } else {
        logOut(eQPLogType_Error, "%s\n", errmsg.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
    }
    return 0;
}

idx dmAnnotProc::progress_reporting_ionProviderCallback(idx record, void * param, idx iRecord, idx fieldType, const char * fieldTypeName, const void ** recordBody, idx * recordSize)
{
    //callbackParam
    sVaxAnnotGB * tt=(sVaxAnnotGB * )param;

    myStructObj * dd = (myStructObj *) tt->callbackParam;

    if (!dd->counter) {
        dd->totalSize = tt->srcEnd - tt->srcStart;
    }
    ++dd->counter;
    idx myRecord =dd->myvax->ionProviderCallback(record,iRecord,fieldType,fieldTypeName, recordBody, recordSize);

    dd->qp->reqProgress(myRecord,0,100);
    /*
    if (!dd->qp->reqProgress(myRecord,0,dd->totalSize)){
        dd->counter = -1;
        return sIon::eProviderDestroy;
    }*/


    return myRecord;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc, argv); // remember arguments in global for future

    dmAnnotProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dmAnnot", argv[0]));
    return (int) backend.run(argc, argv);
}



// OLD FORMAT
/*sVioAnnot annot;
    annot.myCallbackFunction = reqProgressStatic;
    annot.myCallbackParam = this;
    const char * ext = strrchr(sourceFilePath, '.');
    if( ext && strcmp(ext, ".vioannot") == 0 ) {
        annot.init(sourceFilePath, sMex::fReadonly);
    } else {
        // any other file assumed to be in GenBank format
        sStr outfile;
        if( !obj.addFilePathname(outfile, true, ".vioannot") ) {
            errmsg.printf("failed to create destination");
            break;
        }
        logOut(eQPLogType_Info, "output file '%s'\n", outfile.ptr());
        idx ires = annot.ParseGBfile(sourceFilePath, outfile, true);
        if( ires < 0 ) {
            if (ires==-6){
                errmsg.printf("file format Error");
                break;
            }
            else{
                errmsg.printf("Parser returns Error Code: %" DEC, ires);
                break;
            }
        }
        annot.init(outfile, sMex::fReadonly);
}*/
