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

#include <qpsvc/dna-cgi.hpp>
#include <slib/std/http.hpp>
#include <slib/std/string.hpp>

const char * dmDnaCgi::_svc_name = "dnaCGI";

static const char * risky_names00 = "sessionID"_"debug"__;

dmDnaCgi::dmDnaCgi(sQPride & qp, const char * url_query_string) : sQPSvc(qp)
{
    if (url_query_string) {
        sVar url_form;
        const char * fake_argv[] = {
            "dna.cgi",
            url_query_string
        };
        sHtml::inputCGI(NULL, 2, fake_argv, &url_form, 0, false, 0);
        for (idx i=0; i<url_form.dim(); i++) {
            const char * name = static_cast<const char *>(url_form.id(i));
            if (!name || sString::compareChoice(name, risky_names00, 0, true, 0, true) >= 0)
                continue;
            setVar(name, "%s", url_form.value(name, ""));
        }
        setVar("raw", "1");
    }
}
