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
#include <cflow.hpp>
#include <slib/utils/tbl.hpp>
#include <violin/hiveseq.hpp>
#include "diu-antidote-utils.hpp"
#include <slib/std/string.hpp>
using namespace sviolin;

void diu_utils::mapAcc2Tier2DB(const char* fileName, sVar * pAccToDBName)
{
    sTbl tbl;
    tbl.parseFile(fileName);
    sStr buf;
    for(idx row = 1; row < tbl.rows(); ++row)
    {
        idx aLen,dbLen,sevLen;
        const char * acc=tbl.cell(row,(idx)0,&aLen);if(!aLen)continue;
        const char * dbN=tbl.cell(row,1,&dbLen);if(!dbLen)continue;
        
        if(pAccToDBName->is(acc,aLen))
            continue;
        pAccToDBName->inp(acc,dbN,-(dbLen),aLen);
        
        const char * severity=tbl.cell(row,3,&sevLen);
        if(sevLen && severity && *severity && severity[0]!='0') {
            buf.printf(0,"%.*s-severity",(int)aLen,acc);
            pAccToDBName->inp(buf,severity,sevLen,0); 
        }
    }
    return ;
}

void diu_utils::mapGenomesToAccDB( sUsr * user, sVar * pAccToDBName)
{
    sUsrObjRes obj_res;
    sStr buf;
    user->objs2("genom", obj_res,(udx*)0,"name","_prio.fasta",0);
    for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
        sHiveseq hs(user,obj_res.id(it)->print());
        for(idx is=0;is<hs.dim();++is) {
            const char *idend, * id=hs.id(is);
            for(idend=id+1; *idend && !strchr("." sString_symbolsBlank,*idend); ++ idend){}

            const char * isaend, * isa=strstr(id,"ISAFLAG=");
            if(!isa)
                continue;

            isa+=8;
            for(isaend=isa; *isaend && !strchr(sString_symbolsBlank,*isaend); ++ isaend){}

            const char * severity="Unknown";
            switch(*isa){ 
                case 'H':severity="High";break;
                case 'M':severity="Medium";break;
                case 'L':severity="Low";break;
            }
            buf.printf(0,"%.*s-severity",(int)(idend-id),id);
            pAccToDBName->inp(buf,severity,0,0); 
        }
    }
}

void diu_utils::fillSequenceFiles(const char* accesionsFile, const char * basePath, sVar & acc2DB, sDic<Tier2DB>& dbNameToFileDic, sviolin::sHiveseq& sf)
{
    sTbl tbl;
    tbl.parseFile(accesionsFile);
    sStr buf;sFilePath FQPath; 
    for(idx row = 1; row < tbl.rows(); ++row)
    {
        idx refIdLen;
        const char * refId=tbl.cell(row,2,&refIdLen),*end=refId+refIdLen,*p=refId;
        while(p<end && !strchr("." sString_symbolsBlank,*p))++p;
        refIdLen=p-refId;
        
        const char * db=acc2DB.value(refId,0,0,refIdLen);
        if(!db)continue;
        Tier2DB* dbFile = dbNameToFileDic.set(db);
        if(!dbFile) continue;
        
        if(!dbFile->seqFile.length()){
            FQPath.makeName(buf.printf(0,"%s%s",basePath,db),"%%pathx.fq");
            sFile::remove(FQPath);
            dbFile->seqFile.init(FQPath.ptr());
        } 
        
        idx readNum = tbl.ival(row, 3);
        sf.printFastXRow(&dbFile->seqFile, true, readNum-1, 0, 0, 0, true);
    }

}
const char * diu_utils::closeDbFiles(sStr & list,sDic<Tier2DB>& dbNameToFileDic){

    for(idx i = 0; i < dbNameToFileDic.dim(); ++i) {
        dbNameToFileDic[i].seqFile.destroy();
        const char * db=(const char * )dbNameToFileDic.id(i);
        list.printf("%s%s",list.length() ? "," : "", db);
    }
    return list.ptr(0);
}

bool diu_utils::FormatOutput::processFiles(sStr & outputFilePath, sStr & alMatchFilePath, sStr & concernFilePath, sviolin::sHiveseq &reads) {

    sStr errorFilePath;
    errorFilePath.printf("%ssample_error.csv", "/tmp/");


    sDic<ReadInfo> dic;
    idx errCount = 0;

    processCSV(alMatchFilePath.ptr(), concernFilePath.ptr(), outputFilePath.ptr(), errorFilePath.ptr(), dic, errCount, reads);

    if (errCount >= 100) {
        sFil errorFile(errorFilePath.ptr());
        errorFile.printf("Too many errors encountered. Terminating program.\n");
        return false;
    }

    return true;
}

void diu_utils::FormatOutput::validateInitialRows(const char * filePath, idx rowsToCheck) {
    sTbl validationTable;
    if (!validationTable.parseFile(filePath)) {
        printf("Failed to parse the alignment file for validation: %s\n", filePath);
        exit(1);
    }

    sStr refID, tempReadID, lineage;
    sDic<bool> readIDMap;

    for (idx irow = 1; irow < rowsToCheck && irow < validationTable.rows(); ++irow) {
        validationTable.get(&refID, irow, 2);
        validationTable.get(&tempReadID, irow, 4);

        if (refID.length() == 0) {
            printf("Error: Missing Reference Identifier at row %lld. Terminating.\n", irow);
            exit(1);
        }
        if (tempReadID.length() == 0) {
            printf("Error: Missing Read Identifier at row %lld. Terminating.\n", irow);
            exit(1);
        }


    }
}

idx diu_utils::llnl_tempDic(sTbl & tbl, sDic< LLNL_BULL >& dicReads, sVar * acc2DB, const char * dbType) 
{
  
    sStr b;
    for (idx ir = 1; ir < tbl.rows(); ++ir) {

        idx readIdLen; const char * readId=tbl.cell(ir,4,&readIdLen);
        const char * end=readId+readIdLen, *p=readId;
        while(p<end && !strchr("." sString_symbolsBlank,*p))++p;
        readIdLen=p-readId;

        LLNL_BULL * bull=dicReads.set(readId,readIdLen); if(!bull) break;
        LLNL_BULL_INFO & ll = (bull->bi[0].score) ? (bull->bi[1]) : (bull->bi[0]);

        ll.refId=tbl.cell(ir,2,&ll.refIdLen);
        if (sString::searchSubstring(ll.refId,ll.refIdLen,"BIOENGINEERED=TRUE",0,0,true)) {
            const char *typePos = strstr(ll.refId, "TYPE=");
            if(!typePos) {
                ll.details = "Bioengineered";
            } else {
                typePos += 5;
                
                const char *p = typePos;
                const char *end = ll.refId + ll.refIdLen;
                while(p<end && !strchr(",\"" sString_symbolsBlank,*p))++p;

                ll.detailsLen = p - typePos;
                ll.details = typePos;
            }
        } 
        else {
            ll.details = "";
        }
        end=ll.refId+ll.refIdLen;p=ll.refId;
        while(p<end && !strchr("." sString_symbolsBlank,*p))++p;
        ll.refIdLen=p-ll.refId;
        
        ll.lineage=tbl.cell(ir,13,&ll.linLen);
        ll.taxId=tbl.ivalue(ir,12);
        ll.score=tbl.ivalue(ir,5);
        ll.serial=tbl.ivalue(ir,3)-1;
        ll.dbType=dbType;
        
        const char * sev=acc2DB->value(b.printf(0,"%.*s-severity",(int)ll.refIdLen,ll.refId));
        ll.sev=0;
        if(sev)switch(*sev) {
            case 'L':ll.sev=1;
            case 'M':ll.sev=2;
            case 'H':ll.sev=3;
        }
    }


    return dicReads.dim();
           
}

idx diu_utils::llnl_tempOut(sFil & tsvFile, sVec < sDic< LLNL_BULL > > & dicReadsSet, sviolin::sHiveseq & hs, idx isPaired) 
{
    sStr reason;
    const char * sevMap[4]={"None","Low","Medium","High"};
    for(idx is=0; is<hs.dim(); ++is) { 
        const char * readId=hs.id(is), *p=readId;
        while(*p && !strchr("." sString_symbolsBlank,*p))++p;
        idx readIdLen=p-readId;

        idx tsvLen=tsvFile.length();
        for ( idx idd=0 ; idd<dicReadsSet.dim() ; ++idd  ){ 

            sDic< LLNL_BULL > & dicReads=dicReadsSet[idd];
            LLNL_BULL * bull=dicReads.get(readId,readIdLen);if(!bull) continue;

            

            idx taxId=0;
            real score=0;
            char severity=0;
            idx resultCode=0;

            
            LLNL_BULL_INFO * l1=bull->bi, * l2=l1+1;
            const char * posLin=0,* linEnd;
            
            if(l1->taxId==l2->taxId ) { 
                taxId=l1->taxId;
                if(taxId) { 
                    posLin=l1->lineage;
                    for(linEnd=posLin; *linEnd!='|' && linEnd<posLin+l1->linLen ; ++linEnd);
                }
            } else { 
                if(l1->taxId && !l2->taxId) {
                    taxId=l1->taxId;
                    posLin=l1->lineage;
                    for(linEnd=posLin; *linEnd!='|' && linEnd<posLin+l1->linLen ; ++linEnd);
                }
                else if(!l1->taxId && l2->taxId) {
                    taxId=l2->taxId;
                    posLin=l2->lineage;
                    for(linEnd=posLin; *linEnd!='|' && linEnd<posLin+l2->linLen ; ++linEnd);
                }
                else { 
                    idx i1=l1->linLen-1, i2=l2->linLen-1;
                    while(i1>0 && l1->lineage[i1]!='|')--i1;
                    while(i2>0 && l2->lineage[i2]!='|')--i2;
                    for( ; i1>0 && i2>0; --i1, --i2) {
                        if(l1->lineage[i1]!=l2->lineage[i2]) {
                            break;
                        }
                    }
                    while(l1->lineage[i1]!='|')++i1;
                    while(l2->lineage[i2]!='|')++i2;
                    taxId=atoidx(l1->lineage+i1+1);
                    posLin=l1->lineage+i1+1;
                    for(linEnd=posLin; *linEnd!='|' && linEnd<posLin+l1->linLen ; ++linEnd);
                }
            }
            

            score=1;
            severity=l1->sev>=l2->sev ? l1->sev : l2->sev;
            resultCode=2;
            if (severity) { 
                resultCode|=1;
                if(severity==2)resultCode|=1<<4;
                if(severity==3)resultCode|=(1<<4)|(1<<5);
            }

            if(taxId && (idx)severity != 0) {
                reason.printf(0,"%sConcern",sevMap[(idx)severity]);
            } else {
                resultCode|=(1<<2);
                reason.printf(0,"%.*s",(int)l1->detailsLen,l1->details);
            }
        
            sString::searchAndReplaceSymbols(reason.ptr(),0," \t\r\n","_",0,true,true,false,true,0);
            tsvFile.printf("%.*s\t%" DEC "\t%" DEC "\t%" DEC "\t%.2lg\t%s\n",(int)readIdLen,readId,hs.len(is)*isPaired, resultCode , taxId, score , reason.length() ? reason.ptr(0) : "DbHit");
        }
        if(tsvLen==tsvFile.length())
            tsvFile.printf("%.*s\t%" DEC "\t%" DEC "\t%" DEC "\t%.2lg\t%s\n",(int)readIdLen,readId,hs.len(is)*isPaired, (idx)0 , (idx)0, (real)0.0 , "NoDBHit");

    }
    return 1;
}

void diu_utils::FormatOutput::processCSV(const char *alMatchFilePath, const char *concernFilePath, const char *tsvFilePath, const char *errorFilePath, sDic<ReadInfo> &dic, idx &errCount, sviolin::sHiveseq &all_reads) {
    sTbl concernTable;

    sStr alMatch_splitPath;
    sString::searchAndReplaceSymbols(&alMatch_splitPath,alMatchFilePath,0,";",0,0,true,true,true,true,0);

    if (!csvTable.parseFile(alMatch_splitPath.ptr(0))) {
        printf("Failed to parse the main CSV file: %s\n", alMatchFilePath);
        return;
    }
    if (!concernTable.parseFile(concernFilePath, sTbl::fPreserveQuotes, ",")) {
        printf("Failed to parse the concern file: %s\n", concernFilePath);
        return;
    }

    sDic<bool> concernMap;
    sStr cell;
    sStr concernColumn;
    for (idx irow = 1; irow < concernTable.rows(); ++irow) {
        cell.cut(0); concernColumn.cut(0);
        concernTable.get(&cell, irow,(idx) 0);
        bool *concern = concernMap.set(cell.ptr(), cell.length());


        concernTable.get(&concernColumn, irow, 2);

        if (concernColumn.length() == 0 || strncmp(concernColumn.ptr(),"0",1) == 0) {
            *concern = false;
        } else {
            *concern = true;
        }
    }

    sFil tsvFile(tsvFilePath, sMex::fSetZero | sMex::fMapRemoveFile);
    sFil errorFile(errorFilePath, sMex::fSetZero | sMex::fMapRemoveFile);

    tsvFile.printf("%s\n", _hdr);

    sDic<bool> processedReads;
    sStr fullReadKey;
    sStr baseReadID;
    sStr refID, tempReadID, lineage;
    
    for (const char * alM_path=alMatch_splitPath.ptr(0); alM_path; alM_path=sString::next00(alM_path)) {
        sTbl csvTable;
        if (!csvTable.parseFile(alM_path)) {
            continue;
        }
        if (csvTable.rows()<2) {
            continue;
        }
        for (idx i = 1; i < csvTable.rows(); ++i) {
            refID.cut(0); tempReadID.cut(0); lineage.cut(0);

            idx refNum = csvTable.ivalue(i, 1);
            csvTable.get(&refID, i, 2);
            csvTable.get(&tempReadID, i, 4);
            idx length = csvTable.ivalue(i, 7);
            idx taxID = csvTable.ivalue(i, 12);
            idx alignment_score = csvTable.ivalue(i, 5);
            csvTable.get(&lineage, i, 13);

            const char* spacePos = strchr(tempReadID.ptr(), ' ');
            if (!spacePos) {
                continue;
            }

            baseReadID.cut(0);
            baseReadID.addString(tempReadID.ptr(), spacePos - tempReadID.ptr());

            idx direction = csvTable.ivalue(i, 6);

            fullReadKey.cut(0);
            fullReadKey.printf("%s %s", baseReadID.ptr(),  (direction==1 ? "1:" : "2:"));

            ReadInfo *existingInfo = dic.get(fullReadKey.ptr(), fullReadKey.length());
            if (existingInfo) {
                if (alignment_score > existingInfo->alignment_score) {
                    existingInfo->length = length;
                    existingInfo->tax_id = taxID;
                    existingInfo->alignment_score = alignment_score;
                    existingInfo->direction = direction;
                    existingInfo->lineage = (lineage.length() == 0) ? strdup("No Lineage Available") : strdup(lineage.ptr());
                    existingInfo->concern = concernMap.get(refID.ptr()) ? *concernMap.get(refID.ptr()) : false;

                    existingInfo->engineered = (strstr(refID.ptr(), "BIOENGINEERED=TRUE") != NULL);
                    if (existingInfo->engineered) {
                        char *typePos = strstr(refID.ptr(), "TYPE=");
                        if (typePos) {
                            typePos += 5;
                            char *endPos = strchr(typePos, ' ');
                            if (!endPos) endPos = refID.ptr() + refID.length();
                            existingInfo->details = strndup(typePos, endPos - typePos);
                        }
                    }
                    existingInfo->novel = (strstr(refID.ptr(), "NOVEL=TRUE") != NULL);
                    existingInfo->called = true;
                    existingInfo->ref_num = refNum;
                }
            } else {
                ReadInfo *newInfo = dic.set(fullReadKey.ptr(), fullReadKey.length());
                newInfo->length = length;
                newInfo->tax_id = taxID;
                newInfo->alignment_score = alignment_score;
                newInfo->direction = direction;
                newInfo->lineage = (lineage.length() == 0) ? strdup("No Lineage Available") : strdup(lineage.ptr());
                newInfo->concern = concernMap.get(refID.ptr()) ? *concernMap.get(refID.ptr()) : false;

                    newInfo->engineered = (strstr(refID.ptr(), "BIOENGINEERED=TRUE") != NULL);
                if (newInfo->engineered) {
                    char *typePos = strstr(refID.ptr(), "TYPE=");
                    if (typePos) {
                        typePos += 5;
                        char *endPos = strchr(typePos, ' ');
                        if (!endPos) endPos = refID.ptr() + refID.length();
                        newInfo->details = strndup(typePos, endPos - typePos);
                    }
                }
                newInfo->novel = (strstr(refID.ptr(), "NOVEL=TRUE") != NULL);
                newInfo->called = true;
                newInfo->ref_num = refNum;
            }
        }
    }
    

    idx numCalls = 0;
    for (idx i = 0; i < all_reads.dim(); ++i) {
        const char *readID = all_reads.id(i);
        sStr baseReadID;
        const char* spacePos = strchr(readID, ' ');

        if (spacePos) {
            baseReadID.addString(readID, spacePos - readID); 
        }

        if (processedReads.get(baseReadID.ptr())) {
            continue;
        }

        sStr key1, key2;
        key1.printf("%s 1:", baseReadID.ptr());
        key2.printf("%s 2:", baseReadID.ptr());

        ReadInfo* infoR1 = dic.get(key1.ptr());
        ReadInfo* infoR2 = dic.get(key2.ptr());
        idx commonTaxID = 0;

        if (infoR1 && !infoR2) {
            outputSingleAlignedRead(tsvFile, baseReadID.ptr(), *infoR1, all_reads.len(i));
            infoR1->processed = true;
        } else if (infoR2 && !infoR1) {
            outputSingleAlignedRead(tsvFile, baseReadID.ptr(), *infoR2, all_reads.len(i));
            infoR2->processed = true;
        } else if (infoR1 && infoR2) {
            if (infoR1->ref_num != infoR2->ref_num) {
                numCalls++;
                commonTaxID = findLastCommonAncestor(infoR1->lineage, infoR2->lineage);

                if (commonTaxID != -1) {
                    outputPairedRead(tsvFile, baseReadID.ptr(), *infoR1, *infoR2, commonTaxID);
                } else {
                    outputPairedRead(tsvFile, baseReadID.ptr(), *infoR1, *infoR2, infoR1->tax_id);
                }
            } else {
                outputPairedRead(tsvFile, baseReadID.ptr(), *infoR1, *infoR2, commonTaxID);
            }

            infoR1->processed = true;
            infoR2->processed = true;
        } else {
            outputUnalignedRead(tsvFile, baseReadID.ptr(), all_reads.len(i));
        }

        *processedReads.set(baseReadID.ptr()) = true;
    }
    printf("%lld", numCalls);
}



void diu_utils::FormatOutput::outputSingleAlignedRead(sFil &tsvFile, const char *baseReadID, const ReadInfo &info, idx combinedLength) {
    real score = (info.alignment_score) * 1.0 / (combinedLength * 5);
    idx code = constructResultCode(info, true);
    const char *resultDetails = getResultDetails(code, info.details);
    tsvFile.printf("%s\t%lld\t%lld\t%lld\t%f\t%s\n", 
        baseReadID,
        combinedLength, 
        code, 
        info.tax_id, 
        score,
        resultDetails);
}

void diu_utils::FormatOutput::outputPairedRead(sFil &tsvFile, const char *baseReadID, const ReadInfo &info1, const ReadInfo &info2, idx commonTaxID) {
    real combinedScore = (info1.alignment_score + info2.alignment_score) * 1.0 / 
                         ((info1.length + info2.length) * 5);
    idx code = constructResultCode(info1, true);  
    const char *resultDetails = getResultDetails(code, info1.details);
    tsvFile.printf("%s\t%lld\t%lld\t%lld\t%f\t%s\n", 
        baseReadID, 
        info1.length + info2.length, 
        code, 
        commonTaxID, 
        combinedScore,
        resultDetails);
}

void diu_utils::FormatOutput::outputUnalignedRead(sFil &tsvFile, const char *baseReadID, idx readLength) {
    tsvFile.printf("%s\t%lld\t%d\t%d\t%f\t%s\n", 
        baseReadID,
        readLength,
        0, 
        -1, 
        1.0, 
        "NoRefInDB");
}

void diu_utils::FormatOutput::outputUnalignedReadPair(sFil &tsvFile, const char *baseReadID, const ReadInfo &info1, const ReadInfo &info2) {
    idx combinedLength = info1.length + info2.length;
    tsvFile.printf("%s\t%lld\t%d\t%d\t%f\t%s\n", 
        baseReadID,
        combinedLength,
        0, 
        -1, 
        1.0, 
        "NoRefInDB");
}

idx diu_utils::FormatOutput::findLastCommonAncestor(const char *lineage1, const char *lineage2) {
    const char* taxonomicRanks[] = {
        "strain", "species",  "genus", "family", "order", "class", "phylum", "superkingdom", "cellular organisms"
           
    };
    const idx numRanks = sizeof(taxonomicRanks) / sizeof(taxonomicRanks[0]);

    sDic<idx> lineageMap1;
    sDic<idx> lineageMap2;

    sStr tokenized1, tokenized2;

    tokenized1.addString(lineage1);
    char *token1 = strtok(tokenized1.ptr(), "|");
    while (token1) {
        char *rankStart = strchr(token1, ':');
        if (rankStart) {
            rankStart++;
            char *rankEnd = strchr(rankStart, ':');
            if (rankEnd) {
                sStr rankPart;
                rankPart.addString(rankStart, rankEnd - rankStart);
                idx taxID = strtoidx(token1, NULL, 10);
                *(lineageMap1.set(rankPart.ptr())) = taxID;
            }
        }
        token1 = strtok(NULL, "|");
    }

    tokenized2.addString(lineage2);
    char *token2 = strtok(tokenized2.ptr(), "|");
    while (token2) {
        char *rankStart = strchr(token2, ':');
        if (rankStart) {
            rankStart++;
            char *rankEnd = strchr(rankStart, ':');
            if (rankEnd) {
                sStr rankPart;
                rankPart.addString(rankStart, rankEnd - rankStart);
                idx taxID = strtoidx(token2, NULL, 10);
                *(lineageMap2.set(rankPart.ptr())) = taxID;
            }
        }
        token2 = strtok(NULL, "|");
    }

    idx lastCommonAncestor = -1;
    for (idx i = 0; i < numRanks; ++i) {
        const char* rank = taxonomicRanks[i];
        idx *taxID1 = lineageMap1.get(rank);
        idx *taxID2 = lineageMap2.get(rank);

        if (!taxID1 || !taxID2){
            continue;
        }
        
        if (*taxID1 == *taxID2) {
            lastCommonAncestor = *taxID1;
            break;
        }
    }

    return lastCommonAncestor;
}



idx diu_utils::FormatOutput::constructResultCode(const ReadInfo &info, bool called) {
    idx r_code = 0;
    r_code = info.concern ? r_code | 1 : r_code;
    r_code = called ? r_code | (1 << 1) : r_code;
    r_code = info.engineered ? r_code | (1 << 2) : r_code;
    r_code = info.novel ? r_code | (1 << 3) : r_code;
    return r_code;
}

const char * diu_utils::FormatOutput::getResultDetails(idx resultInt, const char *existingDetails) {
    if (existingDetails && *existingDetails) {
        return existingDetails; 
    }

    if (resultInt == 0) {
        return "NoRefInDB";
    } else if (resultInt == 1) {
        return "HitConcernDB";
    } else if (resultInt == 2) {
        return "NoConcernBio";
    } else if (resultInt == 8) {
        return "ExpertDetermination";
    } else {
        return "MultipleFlags";
    }
}

idx diu_utils::createDIEventJson (const char * file, JSNode & resinf, sStr & b, idx & cntEvents, sVar * acc2DB)
{
    #define GET_SUPERKINGDOM(lineage) \
        (   strstr((lineage), "Viruses") ? "Viruses" :  \
            strstr((lineage), "Bacteria") ? "Bacteria" : \
            strstr((lineage), "Fungi") ? "Fungi" :       \
            strstr((lineage), "Protozoa") ? "Protozoa" : \
                    "Bioengineered")

     #define SEVERITY_LEVEL(severity) \
                ( (severity) == NULL ? 0 :\
                    (strcmp((severity), "") == 0) ? 0 :\
                    strcmp((severity), "Low") == 0 ? 1 : \
                    strcmp((severity), "Medium") == 0 ? 2 : \
                    strcmp((severity), "High") == 0 ? 3 : 0)

    sTbl table; table.parseFile(file, sTbl::fPreserveQuotes, ",");
    sStr buf, tb;
    idx countTotal = 0;

    sStr speciesLeft, speciesRight, lineageBuf, signal;
    sStr tmpLeft, tmpRight;
    const char * taxonomicRanks = ":strain:" _ ":species:" _ ":genus:" _ ":family:" _ ":order:" _ ":class:" _ ":phylum:" _ ":superkingdom:" __;

    for (idx ir = 1; ir < table.rows(); ++ir) {
        countTotal += table.ival(ir, 6);
        countTotal += table.ival(ir, 7);
    }

    for (idx ir = 1; ir < table.rows(); ++ir) {
        idx taxId1 = table.ival(ir, 8);
        idx taxId2 = table.ival(ir, 10);

        idx lineageLeftLen, lineageRightLen;
        const char * lineageLeft = table.cell(ir, 9, &lineageLeftLen);
        const char * lineageRight = table.cell(ir, 11, &lineageRightLen);

        idx subLeftLen, subRightLen;
        const char * subLeft = table.cell(ir, (idx)0, &subLeftLen);
        const char * subRight = table.cell(ir, 3, &subRightLen);
 
        const char * endLeft=subLeft+subLeftLen;
        const char *p=subLeft;
        while(p<endLeft && !strchr("." sString_symbolsBlank,*p))++p;
        subLeftLen=p-subLeft;
        tmpLeft.cut(0); tmpLeft.addString(p, endLeft - p);

        const char * endRight=subRight+subRightLen;
        p=subRight;
        while(p<endRight && !strchr("." sString_symbolsBlank,*p))++p;
        subRightLen=p-subRight;
        tmpRight.cut(0); tmpRight.addString(p,endRight-p);

        if (strncmp(subLeft,subRight,subLeftLen)==0) {
            continue;
        }

        const char * bioengTypeLeft = 0; idx bioengTypeLeftLen=0;
        if (strstr(tmpLeft.ptr(),"BIOENGINEERED=TRUE")!=NULL) {
            const char * typePos = strstr(tmpLeft.ptr(),"TYPE=");
            if (typePos) {
                typePos+=5;
                p = typePos;
                while(p<endLeft && !strchr(",\"" sString_symbolsBlank,*p))++p;
                bioengTypeLeftLen = p - typePos;
                bioengTypeLeft = typePos;
            }
        }

        const char * bioengTypeRight = 0; idx bioengTypeRightLen=0;
        if (strstr(tmpRight.ptr(),"BIOENGINEERED=TRUE")!=NULL) {
            const char * typePos = strstr(tmpRight.ptr(),"TYPE=");
            if (typePos) {
                typePos+=5;
                p = typePos;
                while(p<endRight && !strchr(",\"" sString_symbolsBlank,*p))++p;
                bioengTypeRightLen = p - typePos;
                bioengTypeRight = typePos;
            }
        }

        const char * superkingdomLeft = "Bioengineered";
        const char * superkingdomRight = "Bioengineered";

        idx forward = table.ival(ir, 6);
        idx reverse = table.ival(ir, 7);
        idx sum = forward + reverse;

        speciesLeft.cut(0); speciesRight.cut(0);
        if (lineageLeftLen) {
            buf.cut(0); buf.addString(lineageLeft,lineageLeftLen);
            for (const char * p=taxonomicRanks; p; p=sString::next00(p)) {
                const char * lspeciesLeft = strstr(buf.ptr(), p);
                if (lspeciesLeft) {
                    const char * pp=lspeciesLeft+sLen(p);
                    lspeciesLeft=lspeciesLeft+sLen(p);
                    while(*pp && !strchr(":|" ,*pp)) ++pp;
                    speciesLeft.addString(lspeciesLeft,pp-lspeciesLeft);
                    break;
                }
            }
            superkingdomLeft = GET_SUPERKINGDOM(buf.ptr());
        }

        if (lineageRightLen){
            buf.cut(0); buf.addString(lineageRight,lineageRightLen);
            for (const char * p=taxonomicRanks; p; p=sString::next00(p)) {
                const char * lspeciesRight = strstr(buf.ptr(), p);
                if (lspeciesRight) {
                    const char * pp=lspeciesRight+sLen(p);
                    lspeciesRight=lspeciesRight+sLen(p);
                    while(*pp && !strchr(":|",*pp)) ++pp;
                    speciesRight.addString(lspeciesRight,pp-lspeciesRight);
                    break;
                }
            }
            superkingdomRight = GET_SUPERKINGDOM(buf.ptr());
        }
        
        if (strcmp(superkingdomLeft, "Bioengineered") == 0 && strcmp(superkingdomRight, "Bioengineered") == 0) {
            signal.printf(0, "Recombinant:");
            if (bioengTypeLeftLen) {
                signal.printf("%.*s", (int)bioengTypeLeftLen, bioengTypeLeft);
            } else signal.printf("%.*s", (int)subLeftLen,subLeft);
            signal.printf("---");
            if (bioengTypeRightLen) {
                signal.printf("%.*s", (int)bioengTypeRightLen, bioengTypeRight);
            } else signal.printf("%.*s",(int)subRightLen, subRight);
            lineageBuf.printf(0, "BIOENGINEERED:%.*s;BIOENGINEERED:%.*s", (int)subLeftLen, subLeft, (int)subRightLen, subRight);            
        } else if (strcmp(superkingdomLeft, "Bioengineered") == 0) {
            signal.printf(0, "Recombinant:");
            if (bioengTypeLeftLen) {
                signal.printf("%.*s", (int)bioengTypeLeftLen, bioengTypeLeft);
            } else signal.printf("%.*s", (int)subLeftLen,subLeft);
            signal.printf("---%s",speciesRight.ptr());            
            lineageBuf.printf(0, "BIOENGINEERED:%.*s;%" DEC"", (int)subLeftLen, subLeft, taxId2);
        } else if (strcmp(superkingdomRight, "Bioengineered") == 0) {
            signal.printf(0, "Recombinant:%s---", speciesLeft.ptr());
            if(bioengTypeRightLen) {
                signal.printf("%.*s", (int)bioengTypeRightLen,bioengTypeRight);
            } else signal.printf("%.*s",(int)subRightLen,subRight);
            lineageBuf.printf(0, "%" DEC ";BIOENGINEERED:%.*s", taxId1, (int)subRightLen, subRight);
        } else {
            signal.printf(0, "Recombinant:%s---%s", speciesLeft.ptr(), speciesRight.ptr());
            lineageBuf.printf(0, "%" DEC";%" DEC"", taxId1, taxId2);
        }

        const char * eventClass = NULL;
        if (strcmp(superkingdomLeft, "Bioengineered") == 0 || strcmp(superkingdomRight, "Bioengineered") == 0) {
            eventClass = "Bioengineered";
        } else if (strcmp(superkingdomLeft, superkingdomRight) == 0) {
            eventClass = superkingdomLeft;
        } else {
            eventClass = "Novel";
        }

        const char * sev = "low";

        if (acc2DB) {
            idx sevLeft = SEVERITY_LEVEL(acc2DB->value(tb.printf(0, "%.*s-severity", (int)subLeftLen, subLeft)));
            idx sevRight = SEVERITY_LEVEL(acc2DB->value(tb.printf(0, "%.*s-severity", (int)subRightLen, subRight)));

            idx maxSev = (sevLeft >= sevRight) ? sevLeft : sevRight;

            switch (maxSev) {
                    case 0: sev = "safe"; break;
                    case 1: sev = "low"; break;
                    case 2: sev = "medium"; break;
                    case 3: sev = "high"; break;
            }
        }

        JSNode & res = resinf.linkobj("#");
        res.link("serial_no", cntEvents + 1);
        res.link("signal", signal.ptr());
        res.link("confidence", sum );
        res.link("severity", sev);
        res.link("class", eventClass);
        res.link("lineage", lineageBuf.ptr());
        res.link("description", buf.printf(0, "%" DEC ", %" DEC, taxId1, taxId2));

        ++cntEvents;
    }

    return cntEvents;
}

idx diu_utils::createEventJson (const char * fl, JSNode & resinf, sStr & b, idx & cntEvents, idx & highestSeverity, bool isTax, sVar * acc2DB)
{
    sTbl tbl;tbl.parseFile(fl);
    sStr buf,tb;
    idx cntTotal=0;
    for( idx ir=1; ir<tbl.rows(); ++ir) {
        idx taxid=tbl.ival(ir,1);if(!taxid)continue;
        cntTotal+=tbl.ival(ir,isTax ? 3 : 4);
    }

    for( idx ir=1; ir<tbl.rows(); ++ir) {
        idx taxid=tbl.ival(ir,1);if(!taxid)continue;
        idx hitsUnique=tbl.ival(ir,isTax ? 3 : 4);if(hitsUnique<1)continue;
        
        idx lname=0;const char * name = tbl.cell(ir,isTax ? 5 : 2,&lname);
        idx lline=0;const char * line = isTax ? tbl.cell(ir,7,&lline) : 0;
        idx lskdm=13;const char * skdm = isTax ? tbl.cell(ir,9,&lskdm) : "Bioengineered";
        idx lorigin=0;const char * origin = tbl.cell(ir,(idx)0,&lorigin);

        const char  *  sev="safe";
        if(isTax && acc2DB){
            idx maxSev=0, curSev=0;
            idx lacc,lracc; const char * acc=tbl.cell(ir,8,&lacc);
            if(lacc && acc) { 
                buf.cut(0);sString::searchAndReplaceSymbols(&buf,acc,lacc,",",0,0,true,true,0,true,0);
                for ( const char * t=buf.ptr(0); t; t=sString::next00(t)){
                    for(lracc=0;lracc<lacc && t[lracc]!='.';++lracc);
                    sev=acc2DB->value(tb.printf(0,"%.*s-severity",(int)lracc,t));
                    if(!sev){sev="safe";continue;}
                    switch(sev[0]){
                        case 'L':curSev=1;break;
                        case 'M':curSev=2;break;
                        case 'H':curSev=3;break;
                        default:curSev=0;break;
                    }
                    if(curSev>maxSev)maxSev=curSev;
                }
                switch(maxSev){
                    case 0:sev="safe";break;
                    case 1:sev="low";break;
                    case 2:sev="medium";break;
                    case 3:sev="high";break;
                }
            }
            if (maxSev>highestSeverity){
                highestSeverity=maxSev;
            }
        } 

        JSNode & res=resinf.linkobj("#");
        res.link("serial_no",cntEvents+1);
        res.link("signal",b.printf(0,"%.*s",(int)lname,name));
        res.link("confidence",(idx)hitsUnique);
        res.link("severity",sev);
        res.link("class",lskdm ? b.printf(0,"%.*s",(int)lskdm,skdm) : "Unknown");
        if(line)res.link("lineage",b.printf(0,"%.*s",(int)lline,line));
        res.link("description",buf.printf(0,"%" DEC "/%.*s",taxid,(int)lorigin,origin));
        ++cntEvents;
    }
    return cntEvents;
}
