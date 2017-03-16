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
#include <ssci/bio/bioproject.hpp>

#define TAXID_TYPE 1
#define BIOPROJECT_TYPE 2

void bioproject::Parsefile(const char* srcfile, const char* inputfilename, bool combineFile)
{

    // _/_/_/_/_/_/_/_/_/ _/_/_/_/
    // _/
    // _/ Prepare input files 
    // _/
    //constructor, the first parameter is the final vioDB file name, the second thing is tag, and the thrid is  how many tyeps
    //you will have in your viodb file, and the fourth is the maximum of the realtionships you will have
    sVioDB db(inputfilename,"bioproject",2,1);//constructor: 1 types, 1 is the maximum raltionship 
    //read the source file
    sFil srcFile(srcfile,sMex::fReadonly);
    const char * src=srcFile.ptr();
    idx length=srcFile.length();

    if(!src)::printf("Sorce file %s is unreadable\n",srcfile);

    if(!src)return;
    //some buffer for the element you will put in the vioDB file format
    sStr line;
    char buf[sSizePage];//change it to a bufer
    idx taxid=0;
    idx projectid=0;
    idx genomid=0;
    //This is a an array, that you tell which realtion it points to
    idx relationlistTAXID[1]={2}; // the realtionships here are telling that taxID is linked to parent-taxid 1->1 ,  to child-taxids 1->1 , to names 1->2 and to rsanks 1->3
    idx relationlistBIOPRO[1]={1}; // the realtionships here are telling that ranks are linked to taxids 2->1
    // After it , you can do the first step, addtype
    //the first parameter is the etype of the thing, the second one is how many relationship it will have
    //the third one is the realtion list you build before. next is the name you want to name your
    //type, and give the onbased index of this type
    db.AddType(sVioDB::eInt,1,relationlistTAXID,"taxID", 1); // taxids are integers having 4 out of four relations (children, parents, ranks and names) 
    db.AddType(sVioDB::eInt,1,relationlistBIOPRO,"pID",2); // ranks ahve a single 1/4 relations to taxids 

    for ( idx i=0 ; i<length && *src; ++i)
    {
        src=getonelinefromBuffer(buf,src);
        if(buf[0]=='>') ParseOneline(3,buf+1,0,&genomid,&projectid,&taxid); // this function reads two integers followed by a string 
        //char * rank=string.ptr(0);

        idx TAXIDrecordsize=sizeof(taxid);
        idx PROJECTrecordsize=sizeof(projectid);

        //idx myRecordID=typelist_taxid->cnt+1;
        //idx myRecordID=0;
        //first is the type of the record. second is the body of the thing you put into the fileformat
        //and thrid one is the body's size
        db.AddRecord(TAXID_TYPE,(const void *)&taxid, TAXIDrecordsize);   
        //but at this time, you only tell it, for a specific record, 
        //one of this relationship ++
        //taxid type record's first realtionship(the third parameter)++
        //this is the non-hash example.
        //see the ncbitax.cpp to a hash example.
        db.AddRecordRelationshipCounter(TAXID_TYPE, 0, 1); // 2 here means that this node has one more parent relationship 
        
        db.AddRecord(BIOPROJECT_TYPE,(const void *)&projectid, PROJECTrecordsize);
        db.AddRecordRelationshipCounter(BIOPROJECT_TYPE, 0, 1); // 0 here means that this node has one more child relationship
    }
    //after you got all the realtionship, you allocate space for it.
    db.AllocRelation();
   //do the same for loop for your source file
    src=srcFile.ptr();
    for ( idx k=0 ; k< length  && *src ; ++k) {
    //but this time you need to give the real body of the realationship you want for a 
        //specific record. 
        src=getonelinefromBuffer(buf,src);    
        if(buf[0]=='>') ParseOneline(3,buf+1,0,&genomid,&projectid,&taxid);
        //bool AddRelation(idx typeIndexOneBased_from, idx typeIndexOneBased_to, idx reocrdIndexOneBased_from, idx recordIndexOneBased_to,idx relationIndexOneBased_to);
        ;//for the taxid type record, you want to fill the index k+1 one with its first relationship with the index of k+1
        db.AddRelation(TAXID_TYPE, 1, k+1, k+1 ); // BIOPROJECT_TYPE
        db.AddRelation(BIOPROJECT_TYPE, 1, k+1, k+1); // TAXID_TYPE
    }
    db.Finalize(combineFile);

}    


idx bioproject::getTaxidByProjectID(idx srchprojectid){
    if(srchprojectid){
        for(idx i=1;i <= DB.GetRecordCnt(BIOPROJECT_TYPE);i++){
            idx bodysize;
            idx projectid = *(idx *) DB.Getbody(BIOPROJECT_TYPE,i,&bodysize);
            if(srchprojectid==projectid){
                idx relationCnt;
                idx relationTypeIndex;
                idx index= *DB.GetRelationPtr(BIOPROJECT_TYPE, i, 1,&relationCnt,&relationTypeIndex);
                return *(idx *) DB.Getbody(relationTypeIndex,index,&bodysize);
            }
        }
    }
    return 0;
}
     
idx bioproject::getProjectIDByTaxID(idx srchtaxid){
    if(srchtaxid){
        for(idx i=1;i <= DB.GetRecordCnt(TAXID_TYPE);i++){
            idx bodysize;
            idx taxid = *(idx *) DB.Getbody(TAXID_TYPE,i,&bodysize);
            if(srchtaxid==taxid){
                idx relationCnt;
                idx relationTypeIndex;
                idx index= *DB.GetRelationPtr(TAXID_TYPE, i, 1,&relationCnt,&relationTypeIndex);
                return *(idx *) DB.Getbody(relationTypeIndex,index,&bodysize);
            }
        }
    }
    return 0;
}


void bioproject::ParseOneline(idx idCnt, const char * buf, sStr * string, idx * id, ...)
{
    va_list marker;
    va_start(marker, id );
    sStr line;
    sString::searchAndReplaceSymbols(&line, buf ,0, sString_symbolsBlank, 0, 0, true , true, false , true);
    const char * ptr1=line.ptr(); 
    for(idx i=0; i<idCnt ;i++)
    {
        //const char * ptr1=line.ptr();//get the taxid
        sscanf(ptr1,"%" DEC,id);
        //sFile::copy(filenamesrc,filenamedest, true);
        id = va_arg( marker, idx *);
        ptr1=sString::next00(ptr1);
    }
    va_end(marker);
    if(string){
        string->cut(0);
        string->printf(0,"%s",ptr1);
        sString::searchAndReplaceSymbols(string->ptr(0), 0, sString_symbolsBlank, " ", 0, true , true, false , true);
        sString::cleanEnds(string->ptr(0),0,sString_symbolsBlank,true);
    }
    //string->ptrint(0,"%s",ptr1);
}

const char * bioproject::getonelinefromBuffer(char * buf, const char * fileContent)
{
    int i;
    if(!fileContent[0])
    {
        return fileContent;
    }        
    for (i=0;fileContent[i]!='\n' ; ++ i)  
        buf[i]=fileContent[i];          
    buf[i]='\0';
    return fileContent+i+1;

}
