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

#include <violin/hivespecobj.hpp>
#include <ulib/ufile.hpp>

using namespace slib;

const char * sviolin::SpecialObj::default_taxdb_version_prefix = "3";

static idx cmpVersion(const char * v1, const char * v2)
{
    if( !v1 || !*v1 ) {
        if( !v2 || !*v2 ) {
            return 0;
        }
        return -1;
    }
    if( !v2 || !*v2 ) {
        return 1;
    }

    char *end1 = 0, *end2 = 0;
    idx i1 = strtoidx(v1, &end1, 10);
    idx i2 = strtoidx(v2, &end2, 10);
    if( i1 != i2 ) {
        return i1 - i2;
    }

    return cmpVersion(*end1 ? end1 + 1 : end1, *end2 ? end2 + 1 : end2);
}

//static
bool sviolin::SpecialObj::find(sHiveId & result, const sUsr & user, const char * desired_meaning, const char * version_prefix)
{
    sStr rebuf;
    sUsrObjRes obj_res;

    if( !desired_meaning ) {
        desired_meaning = sStr::zero;
    }
    if( !version_prefix ) {
        version_prefix = sStr::zero;
    }

    const idx version_prefix_len = sLen(version_prefix);

    rebuf.printf("^%s$,^", desired_meaning);
    for(idx i = 0; version_prefix[i]; i++) {
        if( version_prefix[i] == '.' ) {
            // escape dots (special regexp meaning)
            rebuf.addString("\\");
        }
        rebuf.addString(version_prefix + i, 1);
    }
    if( rebuf[rebuf.length() - 1] != '.' ) {
        rebuf.addString("(\\.|$)");
    }
    // e.g. buf.ptr() is "^ncbiTaxonomy$,^3.2(\\.|$)" to search for version 3.2 or 3.2.x

    user.objs2("^special$", obj_res, 0, "meaning,version", rebuf.ptr(), "meaning,version");
    const char * max_version = 0;
    for(sUsrObjRes::IdIter it = obj_res.first(); obj_res.has(it); obj_res.next(it)) {
        const sUsrObjRes::TObjProp * o = obj_res.get(it);
        const char * meaning = obj_res.getValue(obj_res.get(*o, "meaning"));
        const char * version = obj_res.getValue(obj_res.get(*o, "version"));
        if( meaning && version && !strcmp(meaning, desired_meaning) && !strncmp(version, version_prefix, version_prefix_len) && (version[version_prefix_len] == 0 || version[version_prefix_len] == '.') ) {
            if( !max_version || cmpVersion(version, max_version) > 0 ) {
                result = *obj_res.id(it);
                max_version = version;
            }
        }
    }
    if( max_version ) {
        return true;
    } else {
        result = sHiveId::zero;
        return false;
    }
}

//static
const char * sviolin::SpecialObj::findTaxDbIonPath(sStr & path, const sUsr & user, const char * version_prefix/* = 0*/, sHiveId * result_id/* = 0*/, sStr * log/* = 0 */)
{
    sHiveId tax_db_id;
    idx path_start = path.length();
    bool found = false;
    if( findTaxDb(tax_db_id, user, version_prefix) ) {
        sUsrFile ufile(tax_db_id, &user);
        if( ufile.Id() == tax_db_id ) {
            if( ufile.getFilePathname(path, "ncbiTaxonomy.ion") ) {
                found = true;
                // remove .ion extension for use in sTaxIon
                if( path.length() > 4 && !strcmp(path.ptr(path.length() - 4), ".ion") ) {
                    path.cut0cut(path.length() - 4);
                }
            } else if( log ) {
                log->printf("taxonomy database object %s ionDB file is missing", tax_db_id.print());
            }
        } else if( log ) {
            log->printf("taxonomy database object %s could not be opened as a file object", tax_db_id.print());
        }
    } else if( log ) {
        log->addString("taxonomy database is missing");
    }
    if( result_id ) {
        *result_id = tax_db_id;
    }
    return found ? path.ptr(path_start) : 0;
}
