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
#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <violin/sVioDBCGI.hpp>

void printBody(sStr & buf,  idx maxSize, idx ctype,idx bodysize,void * bodyptr,const char * show){

    if(show){
        if(strncmp(show,"Int",3)==0){
            ctype=sVioDB::eInt;
        }
        else if(strncmp(show,"String",6)==0)
            ctype=sVioDB::eString;
    }
    if(ctype==sVioDB::eInt) {
        if(*(idx *)bodyptr>INT32_MAX || *(idx *)bodyptr< INT32_MIN)
            buf.printf("not a Integer");
        else buf.printf("%"DEC"",*(idx *)bodyptr);
    }
    else if(ctype==sVioDB::eString) {
        sStr t;
        if(*(const char *)bodyptr){
            t.printf(0,"%s",(const char *)bodyptr);
            sString::escapeForCSV(buf, t.ptr(0), t.length());
        }
        else buf.printf("not a string");
    }
    else {
        sStr res;
        idx printSize=0;
        if(maxSize<bodysize)    {
            printSize=maxSize;
            buf.printf("First %"DEC" is :", printSize);
        }
        else printSize=bodysize;
        sString::printHexArr( &res, bodyptr, printSize, 16);
        buf.printf("%s",res.ptr(0));
    }

    buf.printf("\n");
}



idx sVioDBCGI::Cmd(const char * cmd)
{
    enum enumCommands
    {
        eGetTypeCnt, eGetRecordCnt, eGetRelationCnt, eGetRelationPtr, eGetHeader, eGetRecordList, eGetRelationList, eGetRelationTree, eGetRecordDetail
    };
    const char * listCommands = "vdbGetTypeCnt"_"vdbGetRecordCnt"_"vdbGetRelationCnt"_"vdbGetRelationPtr"_"vdbGetHeader"_
        "vdbGetRecordList"_"vdbGetRelationList"_"vdbGetRelationTree"_"vdbGetRecordDetail"__;
    idx cmdnum = -1;
    sString::compareChoice(cmd, listCommands, &cmdnum, false, 0, true);
    sVec<const char*> ctype;
    ctype.add(5);
    ctype[0] = "int";
    ctype[1] = "double";
    ctype[2] = "string";
    ctype[3] = "bool";
    ctype[4] = "other";


    //"vioDBPath=ababa&vioDBFileFlag=test&cmd=vdbGetTypeCnt"
    //"vioDBPath=ababa&vioDBFileFlag=test&cmd=vdbGetRecordCnt&typeIndex=1"
    //"vioDBPath=ababa&vioDBFileFlag=test&cmd=vdbGetRelationCnt&typeIndex=1&recordIndex=1"
    //"vioDBPath=ababa&vioDBFileFlag=test&cmd=vdbGetRecordList&cnt=20&typeIndex=3&start=100"
    //"vioDBPath=ababa&vioDBFileFlag=test&cmd=vdbGetReLationList&cnt=20&typeIndex=3&start=100"
    //"ids=10303&cmd=vdbGetHeader&sessionID=user186"

    if( cmdnum == -1 ) {
        return sCGI::Cmd(cmd);
    }

    sStr myPath;

#if _DEBUG
    const char * vioDBPath = pForm->value("vioDBPath", 0);
    if( vioDBPath ) {
        myPath.printf("%s", vioDBPath);
    }
#endif
    if( !myPath ) {
        sHiveId objid(pForm->value("ids"));
        if( !objid ) {
            error("Please specify correct vioDB ID");
            outHtml();
            return 1;
        }


        std::auto_ptr<sUsrObj> infile(user->objFactory(objid));
        if( infile->Id() ) {
            const char * file = pForm->value("file", "");
            const char * ext = pForm->value("ext", "");
            if( file && file[0]) {
                infile->getFilePathname(myPath,file);
            } else if( ext && ext[0] ) {
                infile->getFilePathname(myPath,ext);
            } else {
                infile->getFilePathname(myPath,"%s","");
            }
        }
    }
    if( !myPath ) {
        error("Object file not found");
        outHtml();
        return 1;
    }
    sVioDB db(myPath);
    const char * show=pForm->value("show",0);
    vioDBFileFlag=pForm->value("vioDBFileFlag",vioDBFileFlag);
    if(!db.isok(vioDBFileFlag))
    {
        if(vioDBFileFlag && *vioDBFileFlag){
            error("Specified vioDB:%s is invalid ",vioDBFileFlag);
        }
        else error("Not in vioDB file format ");
        outHtml();
        return 1;
    }



    idx typeIndex = pForm->uvalue("typeIndex", 0);
    idx recordIndex = pForm->uvalue("recordIndex", 0);
    idx relationIndex = pForm->uvalue("relationIndex", 0);
    idx start = pForm->uvalue("start",0);
    idx cnt = pForm->uvalue("cnt",0);

    idx relationCnt, relationTypeIndex;

    switch(cmdnum) {
        case eGetTypeCnt:{
           dataForm.printf("%"DEC"",db.GetTypeCnt());
           outHtml();
        }return 1;
        case eGetHeader:{
            idx typeCnt = db.GetTypeCnt();
            sStr buf;
            buf.printf(0,"index,typeName,ctype,relCnt,recordCnt,relTypeList\n");
            sVec<sStr> tList;
            tList.add(typeCnt);
            for(idx i=0;i<typeCnt;i++){
                sVioDB::TypeList * typelist_type=db.GetTypePointer(i+1);
                sStr temp;
                tList[i].printf(0,"%s",(const char *)&typelist_type->tnameOfs);
            }
            for(idx i=0;i<typeCnt;i++){
                sVioDB::TypeList * typelist_type=db.GetTypePointer(i+1);
                idx relCnt=typelist_type->relCnt;//sizeof(typelist_type->rels)/(sizeof(sVioDB::RLST));
                buf.printf("%"DEC",%s,%s,%"DEC",%"DEC",\"",typelist_type->index,(const char *)&typelist_type->tnameOfs,ctype[(typelist_type->ctype)-1],relCnt,typelist_type->cnt);
                sVec < idx > rels;rels.add(relCnt);
                for(idx j=0;j<relCnt;j++){
                    if(j!=0) buf.printf(",");
                    const char * relationTo;
                    if(!typelist_type->rels[j].typeIndex || typelist_type->rels[j].typeIndex > typeCnt)
                        relationTo="reserved";
                    else
                        relationTo=tList[typelist_type->rels[j].typeIndex-1].ptr();
                    buf.printf("%s",relationTo);
                }
                buf.printf("\"\n");
            }

           dataForm.printf("%s",buf.ptr(0));
           outHtml();
        }return 1;
        case eGetRecordList:{
            sStr buf;
            buf.printf(0,"index,size,body\n");
            if(typeIndex>0){
                sVioDB::TypeList * typelist_type=db.GetTypePointer(typeIndex);
                idx cntShow=0,i=0;
                if(recordIndex==0){
                    cntShow=typelist_type->cnt;
                    i=start+1;
                }
                else {
                    cntShow=recordIndex;
                    i=recordIndex;
                    cnt=cnt+recordIndex;
                }
                for(;i<=cntShow &&(!cnt||i < cnt+start+1);i++){
                    idx bodysize=0;
                    void * bodyptr=db.Getbody(typeIndex, i, &bodysize);
                    buf.printf("%"DEC",%"DEC",",i,bodysize);
                    printBody(buf, 24, typelist_type->ctype,bodysize,bodyptr,0);
                }
                dataForm.printf("%s",buf.ptr(0));
                outHtml();
            }
        }return 1;
        case eGetRelationTree:{
            sStr buf;
            buf.printf(0,"Index,cnt,relTypeIndex,recordIndex,path\n");
            if(typeIndex>0 && recordIndex >0){
                sVioDB::TypeList * typelist_type=db.GetTypePointer(typeIndex);
                for(idx i=0;i<typelist_type->relCnt;i++){
                    idx cntRel=0;
                    idx relTypeIndex=0;
                    idx * relPtr=db.GetRelationPtr(typeIndex, recordIndex, i+1, &cntRel,&relTypeIndex);
                    if(cntRel==0) continue;
                    sVioDB::TypeList * typelistPtr=db.GetTypePointer(relTypeIndex);
                    for(idx j=start+1;j<=cntRel && (!cnt ||j < cnt+start+1);j++){
                        buf.printf("%"DEC",%"DEC",%"DEC",%"DEC",root/%"DEC"/%s/",i+1,cntRel,relTypeIndex,*(relPtr+j-1),i+1,(const char *)&typelistPtr->tnameOfs);
                        idx bodysize=0;
                        void * bodyptr=db.Getbody(relTypeIndex, *(relPtr+j-1), &bodysize);
                        buf.printf("%"DEC":",*(relPtr+j-1));
                        printBody(buf, 10, typelistPtr->ctype,bodysize,bodyptr,0);

                    }
                }
                dataForm.printf("%s",buf.ptr(0));
                outHtml();
            }
        }return 1;
        case eGetRelationList:{
            sStr buf;
            buf.printf(0,"relIndex,cnt,relTypeList\n");
            if(typeIndex>0 && recordIndex >0){
                sVioDB::TypeList * typelist_type=db.GetTypePointer(typeIndex);
                for(idx i=0;i<typelist_type->relCnt;i++){
                    idx cntRel=0;
                    idx relTypeIndex=0;
                    idx * relPtr=db.GetRelationPtr(typeIndex, recordIndex, i+1, &cntRel,&relTypeIndex);
                    if(cntRel==0) continue;
                    sVioDB::TypeList * typelistPtr=db.GetTypePointer(relTypeIndex);
                    buf.printf("%"DEC",%"DEC",\"%"DEC",%s:",i+1,cntRel,relTypeIndex,(const char *)&typelistPtr->tnameOfs);
                    for(idx j=start+1;j<=cntRel && (!cnt ||j < cnt+start+1);j++){
                        if(j!=0) buf.printf(",");
                        buf.printf("%"DEC"",*(relPtr+j-1));
                    }
                    buf.printf("\"\n");
                }
                dataForm.printf("%s",buf.ptr(0));
                outHtml();
            }
        }return 1;
        case eGetRecordDetail:{
            sStr buf;
      //      buf.printf(0,"typeName,ctype,recordIndex,bodysize,body\n");
            buf.printf(0,"Index,content\n");
            if(typeIndex>0 && recordIndex >0){
                sVioDB::TypeList * typelist_type=db.GetTypePointer(typeIndex);
                idx bodysize=0;
                void * bodyptr=db.Getbody(typeIndex, recordIndex, &bodysize);
                buf.printf("typeName,%s\n",(const char *)&typelist_type->tnameOfs);
                buf.printf("ctype,%s\n",ctype[(typelist_type->ctype)-1]);
                buf.printf("recordIndex,%"DEC"\n",recordIndex);
                buf.printf("bodysize,%"DEC"\n",bodysize);
                buf.printf("body,");
                printBody(buf,bodysize, typelist_type->ctype,bodysize,bodyptr,show);
                buf.printf("\n");
                for(idx ir=0; ir<typelist_type->relCnt;++ir ) {
                    buf.printf("relations-%s,%"DEC"\n",(const char *)&db.GetTypePointer(typelist_type->rels[ir].typeIndex)->tnameOfs, db.GetRecordPointerByIndex(typeIndex,recordIndex)->rels[ir].cnt);
                }
            }
            dataForm.printf("%s",buf.ptr(0));
            outHtml();

        } return 1;
        case eGetRecordCnt:{
           dataForm.printf("%"DEC"",db.GetRecordCnt(typeIndex));
           outHtml();
        } return 1;
        case eGetRelationCnt:{
           dataForm.printf("%"DEC"",db.GetRelationCnt(typeIndex, recordIndex, relationIndex));
           outHtml();
        } return 1;
        case eGetRelationPtr:{
           dataForm.printf("%"DEC"",*db.GetRelationPtr(typeIndex,recordIndex, relationIndex, &relationCnt, &relationTypeIndex));
        outHtml();
        } return 1;
        default:break;
    };
    return sCGI::Cmd(cmd ); // let the underlying Management code to deal with untreated commands
}
