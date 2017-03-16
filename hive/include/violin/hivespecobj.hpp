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
#ifndef violin_hivespecobj_hpp
#define violin_hivespecobj_hpp

#include <slib/core.hpp>
#include <ulib/usr.hpp>

namespace sviolin {
    class SpecialObj {
        public:
            static bool find(sHiveId & result, const sUsr & user, const char * desired_meaning, const char * version_prefix);

            static const char * default_taxdb_version_prefix;
            static bool findTaxDb(sHiveId & result, const sUsr & user, const char * version_prefix = 0)
            {
                return find(result, user, "ncbiTaxonomy", version_prefix ? version_prefix : default_taxdb_version_prefix);
            }
            //! return taxonomy database ION path, without .ion extension
            static const char * findTaxDbIonPath(sStr & path, const sUsr & user, const char * version_prefix = 0, sHiveId * result_id = 0, sStr * log = 0);
    };
};

#endif
