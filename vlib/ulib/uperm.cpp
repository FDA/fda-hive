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

#include <slib/core.hpp>
#include <slib/utils.hpp>
#include <slib/utils/json/printer.hpp>
#include "uperm.hpp"

using namespace slib;

//! read |-delimeted list of permissions
udx slib::permPermParse(const char * src, idx len/*=0*/)
{
    udx perm = ePermNone;
    static sStr fmt("%%b=0|browse=%x|read=%x|write=%x|exec=%x|del=%x|admin=%x|share=%x|download=%x;",
                    ePermCanBrowse, ePermCanRead, ePermCanWrite, ePermCanExecute, ePermCanDelete, ePermCanAdmin, ePermCanShare, ePermCanDownload);
    if( !len ) {
        len = sLen(src);
    }
    if( src && src[0] && len ) {
        sString::xbufscanf(src, src + len, fmt, &perm);
    }
    return perm;
}

//! read |-delimeted list of permission flags
udx slib::permFlagParse(const char * src, idx len/*=0*/)
{
    udx flags = eFlagNone;
    static sStr fmt("%%b=0|allow=0|active=0|deny=%x|down=%x|up=%x|hold=%x|revoke=%x;",
                    eFlagRestrictive, eFlagInheritDown, eFlagInheritUp, eFlagOnHold, eFlagRevoked);
    if( !len ) {
        len = sLen(src);
    }
    if( src && src[0] && len ) {
        sString::xbufscanf(src, src + len, fmt, &flags);
    }
    return flags;
}

char * slib::permPrettyPrint(sStr &dst, const udx perm, const udx flags)
{
    idx pos = dst.length();
    if( perm != ePermNone ) {
        sStr fmt("%%b=|browse=%x|read=%x|write=%x|exec=%x|del=%x|admin=%x|share=%x|download=%x",
            ePermCanBrowse, ePermCanRead, ePermCanWrite, ePermCanExecute, ePermCanDelete, ePermCanAdmin, ePermCanShare, ePermCanDownload);
        sString::xprintf(&dst, fmt, perm);
    }
    dst.printf(",");
    if( flags != eFlagNone ) {
        sStr fmt("%%b=|deny=%x|down=%x|up=%x|hold=%x|revoke=%x",
            eFlagRestrictive, eFlagInheritDown, eFlagInheritUp, eFlagOnHold, eFlagRevoked);
        sString::xprintf(&dst, fmt, flags);
    }
    return dst.ptr(pos);
}

void slib::permPretty2JSON(sJSONPrinter & printer, udx num_group, const char * pretty_group, const char * perm_pretty_print, udx perm_perm, udx perm_flags)
{
    if( perm_pretty_print ) {
        const char * comma = strchr(perm_pretty_print, ',');
        perm_perm = permPermParse(perm_pretty_print, comma ? comma - perm_pretty_print : 0);
        perm_flags = comma ? permFlagParse(comma + 1) : 0;
    }

    printer.startObject();
    printer.addKey("party");
    if( pretty_group ) {
        printer.addValue(pretty_group);
    } else {
        printer.addValue(num_group);
    }

    {
        printer.addKey("act");
        printer.startObject();
        if( perm_perm & ePermCanBrowse ) {
            printer.addKeyValue("browse", perm_flags & eFlagRestrictive ? false : true);
        }
        if( perm_perm & ePermCanRead ) {
            printer.addKeyValue("read", perm_flags & eFlagRestrictive ? false : true);
        }
        if( perm_perm & ePermCanWrite ) {
            printer.addKeyValue("write", perm_flags & eFlagRestrictive ? false : true);
        }
        if( perm_perm & ePermCanExecute ) {
            printer.addKeyValue("exec", perm_flags & eFlagRestrictive ? false : true);
        }
        if( perm_perm & ePermCanDelete ) {
            printer.addKeyValue("del", perm_flags & eFlagRestrictive ? false : true);
        }
        if( perm_perm & ePermCanAdmin ) {
            printer.addKeyValue("admin", perm_flags & eFlagRestrictive ? false : true);
        }
        if( perm_perm & ePermCanShare ) {
            printer.addKeyValue("share", perm_flags & eFlagRestrictive ? false : true);
        }
        if( perm_perm & ePermCanDownload ) {
            printer.addKeyValue("download", perm_flags & eFlagRestrictive ? false : true);
        }
        printer.endObject();
    }

    if( perm_flags & eFlagInheritDown ) {
        printer.addKey("_infect");
        printer.startObject();
        printer.addKey("party");
        printer.startArray();
        printer.addValue("member");
        printer.endArray();
        printer.endObject();
    }

    if( perm_flags & eFlagInheritUp ) {
        printer.addKey("_upfect");
        printer.startObject();
        printer.addKey("party");
        printer.startArray();
        printer.addValue("member");
        printer.endArray();
        printer.endObject();
    }

    if( perm_flags & (eFlagOnHold | eFlagRevoked) ) {
        printer.addKey("flags");
        printer.startArray();
        if( perm_flags & eFlagOnHold ) {
            printer.addValue("hold");
        }
        if( perm_flags & eFlagRevoked ) {
            printer.addValue("revoke");
        }
        printer.endArray();
    }

    printer.endObject();
}
