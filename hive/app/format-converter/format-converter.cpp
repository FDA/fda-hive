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
#include <violin/violin.hpp>
#include <qlib/QPrideProc.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>
#include <slib/utils/tbl.hpp>

#if _DEBUG
  idx debug=1;
#else
  idx debug=0;
#endif

class FormatProc: public sQPrideProc
{
public:
    FormatProc(const char * defline00, const char * srv)
        : sQPrideProc(defline00, srv)
    {
    }
    
    virtual idx OnExecute(idx);
};

class FormatOutput {

public:

    struct ReadInfo {
    char * label;
    idx length;
    idx ref_num; 
    idx tax_id;
    idx alignment_score;
    bool engineered = false;
    bool novel = false;
    bool concern = false;
    bool called = false;
    bool processed = false; 
    char * lineage;
    char * details;
    idx direction; 
    real max_score = 0.0;
};


    FormatOutput(const char *filePath): baseFilePath(filePath) {};

    bool processFiles(sStr & outputFilePath, sStr & alMatchFilePath, sStr & concernFilePath, sHiveseq &reads);

private:
    const char *baseFilePath;
    const char * _hdr = "Read ID\tRead Length\tResult Code\tTax ID\tConfidence\tResult Details";
    sTbl csvTable;
    sTbl concernTable;

    void processCSV(const char * csvFilePath, const char * concernFilePath, const char * tsvFilePath, const char * errorFilePath, sDic<ReadInfo> &dic, idx &errCount, sHiveseq & all_reads);
    void validateInitialRows(const char * filePath, idx rowsToCheck);
    idx constructResultCode(const ReadInfo &info, bool called);
    const char * getResultDetails(idx ResultInt, const char *existingDetails);
    idx findLastCommonAncestor(const char *lineage1, const char *lineage2);
    void outputSingleAlignedRead(sFil &tsvFile, const char *baseReadID, const ReadInfo &info, idx combinedLength);
    void outputPairedRead(sFil &tsvFile, const char *baseReadID, const ReadInfo &info1, const ReadInfo &info2, idx commonTaxID);
    void outputUnalignedRead(sFil &tsvFile, const char *readID, idx readLength);
    void outputUnalignedReadPair(sFil &tsvFile, const char *baseReadID, const ReadInfo &info1, const ReadInfo &info2);
};

bool FormatOutput::processFiles(sStr & outputFilePath, sStr & alMatchFilePath, sStr & concernFilePath, sviolin::sHiveseq &reads) {

    sStr errorFilePath;
    errorFilePath.printf("%ssample_error.csv", "/tmp/");

    validateInitialRows(alMatchFilePath.ptr(), 20);

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

void FormatOutput::validateInitialRows(const char * filePath, idx rowsToCheck) {
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


void FormatOutput::processCSV(const char *alMatchFilePath, const char *concernFilePath, const char *tsvFilePath, const char *errorFilePath, sDic<ReadInfo> &dic, idx &errCount, sviolin::sHiveseq &all_reads) {
    sTbl csvTable;
    sTbl concernTable;

    if (!csvTable.parseFile(alMatchFilePath)) {
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
        concernTable.get(&cell, irow, 0);
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
    for (idx i = 1; i < csvTable.rows(); ++i) {
        refID.cut(0); tempReadID.cut(0); lineage.cut(0);

        idx refNum = csvTable.ivalue(i, 1);
        csvTable.get(&refID, i, 2);
        csvTable.get(&tempReadID, i, 4);
        idx length = csvTable.ivalue(i, 7);
        idx taxID = csvTable.ivalue(i, 12);
        idx alignment_score = csvTable.ivalue(i, 5);
        csvTable.get(&lineage, i, 13);
        idx direction = csvTable.ivalue(i, 6);

        const char* spacePos = strchr(tempReadID.ptr(), ' ');
        if (spacePos) {
            baseReadID.cut(0);
            baseReadID.addString(tempReadID.ptr(), spacePos - tempReadID.ptr());
        } else {
            
            baseReadID.cut(0);
            baseReadID.addString(tempReadID.ptr());
        }

        fullReadKey.cut(0);
        if (spacePos) {
            fullReadKey.printf("%s %s", baseReadID.ptr(), (direction == 1 ? "1:" : "2:"));
        } else {
            fullReadKey.printf("%s %s", baseReadID.ptr(), "1:"); 
        }

        

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

    idx numCalls = 0;
    for (idx i = 0; i < all_reads.dim(); ++i) {
        
        const char *readID = all_reads.id(i);
        sStr baseReadID;

        const char* spacePos = strchr(readID, ' ');
        if (spacePos) {
            baseReadID.cut(0);
            baseReadID.addString(readID, spacePos - readID);
        } else {
            
            baseReadID.cut(0);
            baseReadID.addString(readID);
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



void FormatOutput::outputSingleAlignedRead(sFil &tsvFile, const char *baseReadID, const ReadInfo &info, idx combinedLength) {
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

void FormatOutput::outputPairedRead(sFil &tsvFile, const char *baseReadID, const ReadInfo &info1, const ReadInfo &info2, idx commonTaxID) {
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

void FormatOutput::outputUnalignedRead(sFil &tsvFile, const char *baseReadID, idx readLength) {
    tsvFile.printf("%s\t%lld\t%d\t%d\t%f\t%s\n", 
        baseReadID,
        readLength,
        0, 
        -1, 
        1.0, 
        "NoRefInDB");
}

void FormatOutput::outputUnalignedReadPair(sFil &tsvFile, const char *baseReadID, const ReadInfo &info1, const ReadInfo &info2) {
    idx combinedLength = info1.length + info2.length;
    tsvFile.printf("%s\t%lld\t%d\t%d\t%f\t%s\n", 
        baseReadID,
        combinedLength,
        0, 
        -1, 
        1.0, 
        "NoRefInDB");
}

idx FormatOutput::findLastCommonAncestor(const char *lineage1, const char *lineage2) {
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



idx FormatOutput::constructResultCode(const ReadInfo &info, bool called) {
    idx r_code = 0;
    r_code = info.concern ? r_code | 1 : r_code;
    r_code = called ? r_code | (1 << 1) : r_code;
    r_code = info.engineered ? r_code | (1 << 2) : r_code;
    r_code = info.novel ? r_code | (1 << 3) : r_code;
    return r_code;
}

const char * FormatOutput::getResultDetails(idx resultInt, const char *existingDetails) {
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

idx createDIEventJson (const char * file, JSNode & resinf, sStr & b, idx & cntEvents, sVar * acc2DB)
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
        const char * subLeft = table.cell(ir, 0, &subLeftLen); 
        const char * subRight = table.cell(ir, 3, &subRightLen); 
        
        const char * endLeft=subLeft+subLeftLen;
        const char *p=subLeft;
        while(p<endLeft && !strchr("." sString_symbolsBlank,*p))++p;
        subLeftLen=p-subLeft;

        const char * endRight=subRight+subRightLen;
        p=subRight;
        while(p<endRight && !strchr("." sString_symbolsBlank,*p))++p;
        subRightLen=p-subRight;
         
        const char * superkingdomLeft = "Bioengineered";
        const char * superkingdomRight = "Bioengineered";

        idx forward = table.ival(ir, 6);
        idx reverse = table.ival(ir, 7);
        idx sum = forward + reverse; 

        speciesLeft.cut(0); speciesRight.cut(0);
        if (lineageLeftLen) {
            for (const char * p=taxonomicRanks; p; p=sString::next00(p)) {
                const char * lspeciesLeft = strstr(lineageLeft, p);
                if (lspeciesLeft) {
                    const char * pp=lspeciesLeft+9;
                    lspeciesLeft=lspeciesLeft+9;
                    while(*pp && !strchr(":|" ,*pp)) ++pp;
                    speciesLeft.addString(lspeciesLeft,pp-lspeciesLeft);
                    break;
                }
            }
            superkingdomLeft = GET_SUPERKINGDOM(lineageLeft);
        }

        if (lineageRightLen){
            for (const char * p=taxonomicRanks; p; p=sString::next00(p)) {
                const char * lspeciesRight = strstr(lineageRight, p);
                if (lspeciesRight) {
                    const char * pp=lspeciesRight+9;
                    lspeciesRight=lspeciesRight+9;
                    while(*pp && !strchr(":|",*pp)) ++pp;
                    speciesRight.addString(lspeciesRight,pp-lspeciesRight);
                    break;
                }
            }
            superkingdomRight = GET_SUPERKINGDOM(lineageRight);
        }

        if (strcmp(superkingdomLeft, "Bioengineered") == 0 && strcmp(superkingdomRight, "Bioengineered") == 0) {
            signal.printf(0, "Recombinant:%.*s---%.*s", (int)subLeftLen,subLeft, (int)subRightLen, subRight);
            lineageBuf.printf(0, "BIOENGINEERED:%.*s;BIOENGINEERED:%.*s", (int)subLeftLen, subLeft, (int)subRightLen, subRight);
        } else if (strcmp(superkingdomLeft, "Bioengineered") == 0) {
            signal.printf(0, "Recombinant:%.*s---%s", (int)subLeftLen, subLeft, speciesRight.ptr());
            lineageBuf.printf(0, "BIOENGINEERED:%.*s;%" DEC"", (int)subLeftLen, subLeft, taxId2);
        } else if (strcmp(superkingdomRight, "Bioengineered") == 0) {
            signal.printf(0, "Recombinant:%s---%.*s", speciesLeft.ptr(), (int)subRightLen,subRight);
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
                    case 0: sev = "unknown"; break;
                    case 1: sev = "low"; break;
                    case 2: sev = "medium"; break;
                    case 3: sev = "high"; break;
            }
        }
        
        JSNode & res = resinf.linkobj("#");
        res.link("serial_no", cntEvents + 1);
        res.link("signal", signal.ptr());
        res.link("confidence", sum*1.0/countTotal);
        res.link("severity", sev);
        res.link("class", eventClass);
        res.link("lineage", lineageBuf.ptr());
        res.link("description", buf.printf(0, "%" DEC ", %" DEC, taxId1, taxId2));

        ++cntEvents;
    }

    return cntEvents;
}


idx FormatProc::OnExecute(idx req) {
    sHiveseq all_reads(user, "/nest3/store/833/010/10833/_.vioseqlist;/nest3/store/832/010/10832/_.vioseqlist");
    const char *basePath = "/home/skeeney/code/hive/app/format-converter/temp/";

    sStr csvFilePath;
    sStr concernFilePath;
    sStr tsvFilePath;

    tsvFilePath.printf("%ssample.tsv", basePath);
    csvFilePath.printf("%ssample.csv", basePath);
    concernFilePath.printf("%ssample_virusDB.csv", basePath);


    sStr b;
    sJson newJson;
    JSNode root(&newJson);
    
    
        JSNode & resinf=root.linkarr("result_info");
        idx cntEvents=0;

    createDIEventJson("/home/lam/code/hive/app/format-converter/temp/sample.csv", resinf, b, cntEvents, 0);

    newJson.save("/home/lam/code/hive/app/format-converter/temp/myJSON.json");
    

    
    return 0;
}

int main(int argc, const char * argv[]) {
    sStr tmp;
    sApp::args(argc,argv);
    FormatProc bacinend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp,"format-converter",argv[0]));
    return (int)bacinend.run(argc,argv);
}
