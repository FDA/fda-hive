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
#pragma once

#include <stdlib.h>
#include <list>
#include <map>
#include <string>

struct xlsWorkBook;

namespace Xls {

    class Worksheet;

    class Workbook
    {
    private:
        xlsWorkBook *m_pWB;
        std::map<int, Worksheet *> m_Worksheets;
        std::map<std::string, int> m_WorksheetsMap;
        std::list<std::string> m_WorksheetsNames;

    private:
        void fillWorksheetsMap();

    public:
        Workbook();
        ~Workbook();

        bool Open(const char *xlsFile, const char *encoding = "iso-8859-15//TRANSLIT");
        void Close();

        const Worksheet * GetWorksheet(int wsNum) const;
        const Worksheet * GetWorksheet(const char *wsName) const;

        size_t CountWorksheets() const;
        const std::list<std::string> & GetWorksheets() const;
        
        int ExportCsvs(const char *dir, const char *reSheetNameFilter = NULL) const;
    };

}
