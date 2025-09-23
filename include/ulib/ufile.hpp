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
#ifndef sLib_usrfile_h
#define sLib_usrfile_h

#include <ulib/uobj.hpp>

namespace slib {
    class sUsrFile: public sUsrObj
    {
        typedef sUsrObj TParent;

        public:
            sUsrFile(const sUsr * lusr = 0, const char * myType = "u-file")
                : sUsrObj(*lusr, myType)
            {
            }
            sUsrFile(const sHiveId & lobjid, const sUsr * lusr)
                : sUsrObj(*lusr, lobjid)
            {
            }
            sUsrFile(const sUsr& usr, const sHiveId & objId, const sHiveId * ptypeId, udx permission)
                : sUsrObj(usr, objId, ptypeId, permission)
            {
            }

            virtual const char * addFilePathname(sStr & buf, bool overwrite, const char* key, ...) const __attribute__((format(printf, 4, 5)));
            virtual bool delFilePathname(const char* key, ...) const __attribute__((format(printf, 2, 3)));

            const char* getFile(sStr& buf) const;

        protected:
            virtual const char * getFilePathnameX(sStr & buf, const sStr & key, bool check_existence) const;

        private:
            friend class dmArchiverProc;
            friend class dmCompressorProc;

            bool setFile_donotuse(const char * file_path, const char * name = 0, const char * ext = 0, udx file_sz = 0);

            void fixOldPath(void) const;
    };
}

#endif 