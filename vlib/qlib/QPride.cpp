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
#include <slib/std/string.hpp>
#include <qlib/QPride.hpp>
#include "QPrideCmdline.hpp"
#include "QPrideDB.hpp"

using namespace slib;

sQPride::sQPride(const char * defline00, const char * service) {
    bool dbmode = false;
    conn = 0;
    for (const char * defline = defline00; defline && !conn; defline = sString::next00(defline)) {
        if (strcmp(defline, "console") == 0) {
            conn = new sQPrideCmdline();
            dbmode = false;
        } else {
            conn = new sQPrideDB(defline);
            dbmode = true;
        }
    }
    if (dbmode) {
        appMode |= eQPApp_DB;
    } else {
        appMode |= eQPApp_Console;
    }
    if (conn) {
        sQPrideBase::init(conn, service);
    }
}

sQPride::~sQPride()
{
    if(conn)delete conn;
}

bool sQPride::usesQPSvcTable() const
{
    return (appMode & eQPApp_DB);
}

sSql * sQPride::sql(void)
{
    if(appMode & eQPApp_DB) {
        return ((sQPrideDB*)conn)->sql();
    } else {
        return 0;
    }
}
