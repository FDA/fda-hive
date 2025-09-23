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
#include <slib/core/def.hpp>
#include <xlib/xls2csv/Workbook.h>
#include <xlib/xls2csv/Worksheet.h>

#include <string.h>
#include <memory>

extern "C" {
#include <libxls/xls.h>
}

using namespace std;

namespace Xls {

    Workbook::Workbook()
        : m_pWB(NULL)
    {

    }


    Workbook::~Workbook()
    {
        Close();
    }

    bool Workbook::Open(const char *xlsFile, const char *encoding)
    {
        m_pWB = xls_open(const_cast<char *>(xlsFile), const_cast<char *>(encoding));
        return m_pWB != NULL;
    }

    void Workbook::Close()
    {
        if ( m_pWB != NULL ) {
            xls_close(m_pWB);
            m_pWB = NULL;
        }

        for ( map<int, Worksheet *>::iterator it = m_Worksheets.begin(); it != m_Worksheets.end(); it++ )
            delete it->second;

        m_Worksheets.clear();
    }

    const Worksheet * Workbook::GetWorksheet(int wsNum) const
    {
        xlsWorkSheet *pWS = xls_getWorkSheet(m_pWB, wsNum);
        if ( pWS == NULL )
            return NULL;

        map<int, Worksheet *>::const_iterator it = m_Worksheets.find(wsNum);
        if ( it != m_Worksheets.end() )
            return it->second;

        xls_parseWorkSheet(pWS);

        unique_ptr<Worksheet> pws(new Worksheet(pWS, m_pWB->sheets.sheet[wsNum].name));
        const_cast<Workbook *>( this )->m_Worksheets.insert(make_pair(wsNum, pws.get()));
        return pws.release();
    }

    const Worksheet * Workbook::GetWorksheet(const char *wsName) const
    {
        if ( m_WorksheetsMap.empty() )
            const_cast<Workbook *>( this )->fillWorksheetsMap();

        map<string, int>::const_iterator it = m_WorksheetsMap.find(wsName);
        if ( it != m_WorksheetsMap.end() )
            return GetWorksheet(it->second);
        else
            return NULL;
    }

    void Workbook::fillWorksheetsMap()
    {
        for ( int i = 0; i < m_pWB->sheets.count; i++ ) {
            m_WorksheetsMap.insert(make_pair(m_pWB->sheets.sheet[i].name, i));
            m_WorksheetsNames.push_back(m_pWB->sheets.sheet[i].name);
        }
    }

    const std::list<std::string> & Workbook::GetWorksheets() const
    {
        if ( m_WorksheetsMap.empty() )
            const_cast<Workbook *>(this)->fillWorksheetsMap();

        return m_WorksheetsNames;
    }

    size_t Workbook::CountWorksheets() const
    {
        if ( m_WorksheetsMap.empty() )
            const_cast<Workbook *>( this )->fillWorksheetsMap();

        return m_WorksheetsMap.size();
    }

    int Workbook::ExportCsvs(const char *dir,) const
    {
        char *buf = (char *)alloca(strlen(dir) + 1024);
        int counter = 0;
        const list<std::string> &sheets = GetWorksheets();
        for( list<std::string>::const_iterator it = sheets.begin(); it != sheets.end(); it++ ) {
            strcpy(buf, dir);
            if( buf[strlen(buf) - 1] != '/' ) {
                strcat(buf, "/");
            }
            const char * sheetName = it->c_str();
            strcat(buf, sheetName);
            strcat(buf, ".csv");
            GetWorksheet(sheetName)->ExportCsv(buf);
            ++counter;
        }
        return counter;
    }
}
