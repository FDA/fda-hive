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
#include <ssci/bio/sVioAnnot.hpp>
#include <xlib/dmlib.hpp>

#include <violin/violin.hpp>
#include <qlib/QPrideCGI.hpp>

#ifndef sLib_sVioAnnotCGI_hpp
#define sLib_sVioAnnotCGI_hpp

 enum ePrint
    {
        ePrintGi = 0x00000001,
        ePrintLocus = 0x00000002,
        ePrintRangeStart = 0x00000004,
        ePrintRangeEnd = 0x00000008,
        ePrintIdType = 0x00000010,
        ePrintId = 0x00000020
    };

class sVioAnnotCGI: public sQPrideCGI
{
    public:
        sVioAnnotCGI(const char * defline00, const char * service, idx argc, const char * * argv, const char * * envp, FILE * readfrom, bool isCookie, bool immediate)
                :sQPrideCGI(defline00, service, argc, argv, envp, readfrom, isCookie, immediate){}
    virtual idx Cmd(const char * cmd);
};
#endif
idx sVioAnnotCGI::Cmd(const char * cmd)
{
    enum enumCommands{eGetDefinition, eGetNumberOfRange, eGetRange, eGetIdsByRangeNumber, eGetIdByKindOfId };
    const char * listCommands="vdbGetDefinition" _ "vdbGetNumberOfRange" _ "vdbGetRange" _ "vdbGetIdsByRangeNumber" _ "vdbGetIdByKindOfId" __;

    idx cmdnum =-1;
    sString::compareChoice( cmd, listCommands,&cmdnum,false, 0,true);

    if(cmdnum == -1)
        return sCGI::Cmd(cmd);

    idx giNumber = pForm->uvalue("giNumber");
    const char * locus = pForm->value("locus", 0);
    const char * idtype = pForm->value("idType",0);
    const char * id = pForm->value("id");

    const char * whatToOutPut = pForm->value("whatToOutPut",0);
    idx whatToSearch = 0;
    sStr fmt("%%b=0|giNumber=%x|locus=%x|rangeStart=%x|rangeEnd=%x|idType=%x|id=%x;",
              ePrintGi, ePrintLocus, ePrintRangeStart, ePrintRangeEnd, ePrintIdType, ePrintId);

    if (whatToOutPut){
        sString::xscanf(whatToOutPut, fmt, &whatToSearch);
    }

    sVec <sHiveId> annotIDList;
    sQPride::user -> objs("u-annot", annotIDList);

    sVec < sVioAnnot > annotList;
    for ( idx iV=0; iV < annotIDList.dim(); ++iV) {
        sUsrFile obj(annotIDList[iV], user);
        if( obj.Id() ) {
            sStr anotPath;
            if( obj.getFilePathname00(anotPath, ".vioannot") ) {
                annotList.add(1)->init(anotPath,sMex::fReadonly);
            }
        }
    }

    idx recordStart = pForm->ivalue("recordStart",1); if (recordStart < 0) recordStart = 1;
    idx cnt = pForm->ivalue("count",50); if (cnt <= 0) cnt  = 50;



    idx iv, irec;
    for ( iv=0; iv < annotList.dim(); ++iv){
        idx cntInThis = annotList[iv].getTotalRecord();
        if(recordStart < cntInThis)
            break;
        recordStart -= cntInThis;
    }

    sVec < idx > accumulatedRecords;

    idx cntPassedFilter = 0;
    sVec < idx > subsetRec ;
    for ( ; iv<annotList.dim(); ++iv){

        idx cntInThis=annotList[iv].getTotalRecord();
        idx recordEnd=cntInThis;
        subsetRec.cut(0);

        if(locus || giNumber) {

                idx found = 0;
                if (locus){
                    found = annotList[iv].getIndexFromLocus(locus);
                }
                else if (giNumber){
                    found = annotList[iv].getIndexLocusFromGi(giNumber);
                }

                if(found<recordStart )
                    continue;

                recordStart = found;
                recordEnd = recordStart+1;
        }
        else if (idtype && id){
            annotList[iv].getListOfRecordsByIdTypeId(idtype, id, &subsetRec);
            recordStart=1;
            recordEnd=subsetRec.dim();
        }
        for ( idx iRec = recordStart; iRec < recordEnd; ++iRec){
            if(subsetRec.dim())
                irec=subsetRec[iRec];
            else irec=iRec;

            ++cntPassedFilter;
            idx * pElement=accumulatedRecords.add(1);
            *pElement= ( (iv<<32)|(irec) );

            if(cntPassedFilter>=cnt)
                break;

        }
        if(cntPassedFilter>=cnt)
            break;
        recordStart=1;
     }

     sStr * out = & dataForm;
     for( idx iel=0; iel<accumulatedRecords.dim(); ++iel) {

         iv=((accumulatedRecords[iel])>>32)&0xFFFFFFFF;
         irec=(accumulatedRecords[iel])&0xFFFFFFFF;
         sVioAnnot  & Annotation=annotList[iv];

         const char * currentLocus = Annotation.getLocusFromIndex(irec);
         const char * sep="";

         sStr line;
         sStr line0;
         line0.cut(0);
         sStr rangeStart; rangeStart.cut(0);
         sStr rangeEnd; rangeEnd.cut(0);
         if (whatToSearch & ePrintGi){
              line0.printf("%s%s",sep,Annotation.getGiFromLocus(currentLocus));
              sep=",";
         }

         if (whatToSearch & ePrintLocus){
             line0.printf("%s%s",sep,currentLocus);
             sep=",";
         }

         if((whatToSearch & ePrintRangeStart) || (whatToSearch & ePrintRangeEnd) || (whatToSearch & ePrintIdType) || (whatToSearch & ePrintId )){
            Annotation.getAllRangeByGeneralLocusAndDataName(currentLocus,"CDS", rangeStart, true, false);
            Annotation.getAllRangeByGeneralLocusAndDataName(currentLocus,"CDS", rangeEnd, false, true);
            idx totalStart = sString::cnt00(rangeStart);
            sStr line1;
            line1.printf("%s", line0.ptr());
            for(idx rs = 0; rs < totalStart; rs++) {
                sStr line1aux;
                line1aux.printf("%s", line1.ptr());
                if( whatToSearch & ePrintRangeStart )
                    line1aux.printf(",%s", sString::next00(rangeStart, rs));
                if( whatToSearch & ePrintRangeEnd )
                    line1aux.printf(",%s", sString::next00(rangeEnd, rs));
                if ((whatToSearch) & (ePrintIdType|ePrintId)){
                    idx rangeIndex;
                    rangeIndex = Annotation.getRangeIndexByLocusAndRangeNumber(currentLocus, rs+1);
                    idx totalOfId = Annotation.getNumberOfIdsByRangeIndex(rangeIndex) ;
                    sStr line2;
                    line2.printf("%s", line1aux.ptr());
                    for (idx tid = 1; tid <= totalOfId; tid++){
                        sStr line2aux;
                        line2aux.printf("%s", line2.ptr());
                        idx idIndex = Annotation.getIdIndexByIdNumber(rangeIndex, tid);
                        if( whatToSearch & ePrintIdType ){
                            sStr id;
                            Annotation.getIdByIdIndex(idIndex,id,true);
                            line2aux.printf(",%s",id.ptr());
                        }
                        if (whatToSearch & ePrintId){
                            sStr id;
                            Annotation.getIdByIdIndex(idIndex,id,false);
                            line2aux.printf(",%s",id.ptr());
                        }
                        line.printf("%s\n", line2aux.ptr());
                    }
                }
                else{
                    line.printf("%s\n", line1aux.ptr());
                }
            }
         }
         else
             line.printf("%s",line0.ptr());

         switch(cmdnum) {
             case eGetDefinition:{
                 sStr definition;
                 Annotation.getDefinitionByGeneralLocus(currentLocus,definition);
             } break;


             default:break;
         };

         out->printf("%s",line.ptr());
         out->printf("\n");
     }
     out->add0();
    return sQPrideCGI::Cmd(cmd );
}


int main(int argc, const char *argv[], const char *envp[])
{
    sApp::args(argc, argv, envp);
    sBioseq::initModule(sBioseq::eACGT);
    sVioAnnotCGI qapp("config=qapp.cfg" __,"vioAnnot", argc, argv, envp, stdin, true, true);
    qapp.run();
    return 0;
}
