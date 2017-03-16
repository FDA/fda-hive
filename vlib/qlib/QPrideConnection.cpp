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
#include <slib/std.hpp>
#include "QPrideConnection.hpp"

#include <regex.h>

using namespace slib;

bool sQPrideConnection::hostNameMatch(const char * host, const char * hostName, idx * pmaxjob)
{
    if( strstr(hostName, "regex:") == hostName ) {
        return 0;
    }
    bool ret = false;
    bool isRX = false;
    if( strstr(host, "regex:") == host ) {
        isRX = true;
        host += 6;
    }
    sStr h(sMex::fExactSize);
    const char * colon = strchr(host, ':');
    if( colon ) {
        h.printf("%.*s", (int)(colon - host), host);
    } else {
        h.printf("%s", host);
    }
    if( isRX ) {
        regex_t re;
        if( regcomp(&re, h, REG_EXTENDED | REG_ICASE) == 0 ) {
            if( regexec(&re, hostName, 0, NULL, 0) == 0 ) {
                ret = true;
            }
            regfree(&re);
        }
    } else if( strcasecmp(h, hostName) == 0 ) {
        ret = true;
    }
    if( ret && colon && pmaxjob ) {
        sscanf(colon + 1, "%"DEC, pmaxjob);
    }
    return ret;
}

bool sQPrideConnection::hostNameListMatch(const char * hosts, const char * hostName, idx * pmaxjob, idx * pwhich)
{
    sStr tmp;
    bool isNot = false;
    if( hosts[0] == '!' ) {
        ++hosts;
        isNot = true;
    }
    sString::searchAndReplaceSymbols(&tmp, hosts, 0, "/", 0, 0, true, true, true, true);
    if( pwhich ) {
        *pwhich = -1;
    }
    idx insideDomain = 0;
    for(char * rp = tmp.ptr(); rp; rp = sString::next00(rp), ++insideDomain) {
        if( hostNameMatch(rp, hostName, pmaxjob) ) {
            if( pwhich ) {
                (*pwhich) = insideDomain;
            }
            return !isNot;
        }
    }
    return isNot;
}
