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
#include <cctype>

#include <fcntl.h>

#include <slib/core/hash2.hpp>
#include <slib/std/app.hpp>
#include <slib/utils/tbl.hpp>
#include <violin/violin.hpp>
#include <gdocker.hpp>

namespace {
    struct sRankTableEntry
    {
        char Code;
        idx Rank;
    };

    sRankTableEntry ranks__[] = {
        {'r', 0},
        {'d', 1},
        {'k', 2},
        {'p', 3},
        {'c', 4},
        {'o', 5},
        {'f', 6},
        {'g', 7},
        {'s', 8},
    };

    class sRankTable
    {
    public:
        static idx find(char code)
        {
            static const size_t arrSize(sizeof(ranks__) / sizeof(ranks__[0]));
            for(size_t i = 0; i < arrSize; ++i)
            {
                if(ranks__[i].Code == code)
                {
                    return ranks__[i].Rank;
                }
            }

            return -1;
        }
    };
    
    class sReportEntry
    {
    public:
        sReportEntry(const sStr& rankStr, const sStr& nameStr)
        : rank_(-1), indent_(0)
        {
            if(rankStr.length() > 0)
            {
                rank_ = sRankTable::find(std::tolower(rankStr[0]));
                if(*this)
                {
                    for(idx i = 0; i < nameStr.length(); ++i)
                    {
                        if(!std::isspace(nameStr[i]))
                        {
                            name_.addString(nameStr.ptr(i));
                            sString::cleanEnds(name_, 0, sString_symbolsBlank, true, 0);
                            break;
                        }

                        ++indent_;
                    }
                }
            }
        }

        operator bool() const
        {
            return rank_ != -1;
        }

        bool operator< (const sReportEntry& rhs)
        {
            return (rank_ < rhs.rank_) || ((rank_ == rhs.rank_) && (indent_ < rhs.indent_));
        }

        const sStr& name() const
        {
            return name_;
        }

    private:
        idx rank_;
        idx indent_;
        sStr name_;
    };
}

class sDNAKrakenProc: public sGDockerProc
{
public:
    sDNAKrakenProc(const char * defline00, const char * srv)
    : sGDockerProc(defline00, srv)
    {
    }

    virtual idx OnExecute(idx req);
    idx extractRefs (char * resultFilePtr, idx resultFileLen, const char * outFile);

private:
    bool normalizeOutput(sFil& inputFile, sFil& outputFile)
    {
        sTbl tbl;
        tbl.parse(inputFile.ptr(), inputFile.length(), 0, "\t");
        if((tbl.rows() == 0) || (tbl.cols() < 6))
        {
            return false;
        }

        sStr hitCount, rank, taxId, name, nameLineage, taxIdLineage;
        char percent[8] = { 0 };
        for(idx row = 0; row < tbl.rows(); ++row)
        {
            rank.cut(0);
            tbl.get(&rank, row, 3);
            name.cut(0);
            tbl.get(&name, row, 5);
            sReportEntry entry(rank, name);
            if(!entry)
            {
                continue;
            }

            sString::changeCase(rank, 0, sString::eCaseLo);
            name.cutAddString(0, entry.name());
            tbl.get(percent, row, (idx)0);
            const char* pPercent = percent;
            while(std::isspace(*pPercent)) ++pPercent;
            hitCount.cut(0);
            tbl.get(&hitCount, row, 1);
            taxId.cut(0);
            tbl.get(&taxId, row, 4);
            nameLineage.cut(0);
            nameLineage.printf("%s__%s", rank.ptr(), name.ptr());
            taxIdLineage.cutAddString(0, taxId);
            buildLineage(entry, tbl, row - 1, nameLineage, taxIdLineage);
            outputFile.printf("%s,%s,%s,%s,%s\n", name.ptr(), nameLineage.ptr(), taxIdLineage.ptr(), pPercent, hitCount.ptr());
        }

        return true;
    }

    void buildLineage(const sReportEntry& curEntry, sTbl& tbl, idx row, sStr& nameLineage, sStr& taxIdLineage)
    {
        if(row <= 0)
        {
            return;
        }

        sStr rank, taxId, name;
        tbl.get(&rank, row, 3);
        tbl.get(&name, row, 5);
        sReportEntry entry(rank, name);
        if(!entry)
        {
            return;
        }

        const sReportEntry* pCurEntry = &curEntry;
        if(entry < curEntry)
        {
            sStr namesStack, taxIdsStack;
            tbl.get(&taxId, row, 4);
            sString::changeCase(rank, 0, sString::eCaseLo);
            namesStack.printf("%s__%s|", rank.ptr(), entry.name().ptr());
            namesStack.addString(nameLineage);
            nameLineage.cutAddString(0, namesStack);
            taxIdsStack.printf("%s|", taxId.ptr());
            taxIdsStack.addString(taxIdLineage);
            taxIdLineage.cutAddString(0, taxIdsStack);
            pCurEntry = &entry;
        }

        buildLineage(*pCurEntry, tbl, --row, nameLineage, taxIdLineage);
    }
};

idx sDNAKrakenProc::extractRefs(char * resultFilePtr, idx resultFileLen, const char * outFile)
{
    char rank_refs[32]; strcpy(rank_refs,formValue("rank_refs",0,"-"));
    if(rank_refs[0]=='-' || !outFile) return 0;

    sHiveTaxAcc hax(user,formValue("ntSource"),outFile);
    if(!ok)return 0;

    idx threshold_coverage=formIValue("threshold_coverage");
    idx threshold_hits=formIValue("threshold_hits");
    idx maxAccPerSpecies=formIValue("max_per_species",10);
    idx topSpecies=formIValue("top_species",20);

    sTbl tbl;tbl.parse(resultFilePtr, resultFileLen, 0, "\t");

    sVec < real > perc;perc.add(tbl.rows());
    sVec < idx  > order;order.add(2*tbl.rows());

    for(idx row = 0; row < tbl.rows(); ++row)
        perc[row]=tbl.ival(row,0);
    sSort::sort(tbl.rows(), perc.ptr(), order.ptr());

    char rank[24],taxid[256];
    idx cnt=0;
    for(idx iRow = 0; iRow < tbl.rows(); ++iRow){
        idx row = topSpecies ? order[tbl.rows()-1-iRow] : iRow;

        tbl.get(rank,row,3,sDim(rank));
        if(!sIs(rank_refs,rank))
            continue;
        idx hitself=tbl.ival(row,2);
        if(!hitself)
            continue;
        tbl.get(taxid,row,4);
        if(atoidx(taxid)<2)
            continue;



        hax.parseTaxIds("0,1,9606", 0, sHiveTaxAcc::eExclude, ',');

        sVec < sHiveTaxAcc::TAX2ACC > t2a;
        hax.listAccByTaxId(taxid,&t2a,maxAccPerSpecies);
        for( idx i=0; i<t2a.dim() ; ++i) {
            sHiveTaxAcc::TAX2ACC * ta=t2a.ptr(i);if(!ta->len)continue;
            if(threshold_coverage && hitself*100/ta->len<threshold_coverage)continue;
            if(threshold_hits && hitself<threshold_hits )continue;

            hax.copySequences( t2a.ptr(i) );
        }

        ++cnt;
        if(topSpecies && cnt > topSpecies) break;
    }
    return tbl.rows();
}

idx sDNAKrakenProc::OnExecute(idx req)
{
    processExecute(req);


    sFil resultFile(ProcFile("out.txt", false), sMex::fReadonly);
    if(resultFile.length() == 0)
    {
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        reqSetInfo(req, eQPInfoLevel_Error,"Output file is missing");
        return 0;
    }

    extractRefs(resultFile.ptr(), resultFile.length(), ProcFile("reference_set.fa", false));
    sStr dest;
    if (strcmp(formValue("destination",0,""),"sequence")==0) {
        sUsrObj seqObj(*user,sHiveId(formValue("input1")));
        if (seqObj.Id()) {
            seqObj.addFilePathname(dest,1,"kraken-normalized-out.txt");
        }
    }
    if (dest.length()==0){
        dest.printf("%s",ProcFile("normalized-out.txt", false));
    }
    sFil outputFile(dest.ptr(), sMex::fMapRemoveFile | sMex::fDirectFileStream);
    if(!normalizeOutput(resultFile, outputFile))
    {
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        reqSetInfo(req, eQPInfoLevel_Error,"Failed to normalize the output");
        return 0;
    }

    reqSetStatus(reqId, SR.status );
    reqProgress(SR.progress, SR.progress100, SR.progress100);
    return 0;
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    sDNAKrakenProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, (argc>=3 && strcmp(argv[1],"-svc")==0) ? argv[2] :"dna-kraken", argv[0]));
    return (int) backend.run(argc, argv);
}
