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
#include <cstring>
#include <slib/std/app.hpp>
#include <slib/utils/tbl.hpp>
#include <gdocker.hpp>

namespace {
    struct sRankTableEntry
    {
        char Code;
        idx Rank;
    };

    sRankTableEntry ranks__[] = {
        {'d', 0},
        {'k', 1},
        {'p', 2},
        {'c', 3},
        {'o', 4},
        {'f', 5},
        {'g', 6},
        {'s', 7},
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
}

class sDNAMetaPhlAnProc: public sGDockerProc
{
public:
    sDNAMetaPhlAnProc(const char * defline00, const char * srv)
    :sGDockerProc(defline00, srv)
    {
    }

    virtual idx OnExecute(idx req);

private:
    bool normalizeOutput(sFil& inputFile, sFil& outputFile)
    {
        sTbl tbl;
        tbl.parse(inputFile.ptr(), inputFile.length(), 0, "\t");
        if((tbl.rows() == 0) || (tbl.cols() < 4))
        {
            return false;
        }

        sStr nameLineage, taxIdLineage, name;
        char percent[8] = { 0 };
        for(idx row = 0; row < tbl.rows(); ++row)
        {
            nameLineage.cut(0);
            tbl.get(&nameLineage, row, (idx)0);
            if((nameLineage.length() == 0) || (nameLineage[0] == '#'))
            {
                continue;
            }

            taxIdLineage.cut(0);
            tbl.get(&taxIdLineage, row, 1);
            tbl.get(percent, row, 2);
            sString::cleanEnds(percent, 0, sString_symbolsBlank, true, 0);
            char* pLastName = std::strrchr(nameLineage.ptr(), '|');
            if(pLastName)
            {
                ++pLastName;
            }
            else
            {
                pLastName = nameLineage.ptr();
            }

            char* pHead = std::strstr(pLastName, "__");
            if(!pHead || (static_cast<size_t>(pHead - pLastName) > sizeof(char)) || (sRankTable::find(*pLastName) == -1))
            {
                continue;
            }

            pLastName += 3 * sizeof(char);
            name.cutAddString(0, pLastName);
            outputFile.printf("%s,%s,%s,%s,%s\n", name.ptr(), nameLineage.ptr(), taxIdLineage.ptr(), percent, "1");
        }

        return true;
    }
};

idx sDNAMetaPhlAnProc::OnExecute(idx req)
{
    processExecute(req);
    if(SR.status != eQPReqStatus_Done)
    {
        return 0;
    }

    sFil resultFile(ProcFile("out.txt",false),sMex::fReadonly);
    if(resultFile.length()==0)
    {
        reqSetStatus(reqId, eQPReqStatus_ProgError );
        reqSetInfo(req, eQPInfoLevel_Error,"Output file is missing");
        return 0;
    }

    sFil outputFile(ProcFile("normalized-out.txt", false), sMex::fMapRemoveFile | sMex::fDirectFileStream);
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
    sDNAMetaPhlAnProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, (argc>=3 && strcmp(argv[1],"-svc")==0) ? argv[2] :"dna-metaphlan", argv[0]));
    return (int) backend.run(argc, argv);
}
