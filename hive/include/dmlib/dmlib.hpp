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
#ifndef sDmLib_hpp
#define sDmLib_hpp

#include <slib/std/file.hpp>

using namespace slib;

class dmRemoteFile
{
    public:

        dmRemoteFile(slib::sPipe::callbackFuncType cbFunc = 0, void * cbParam = 0, idx cbSecs = 10)
        {
            init(cbFunc, cbParam, cbSecs);
        }

        dmRemoteFile * init(slib::sPipe::callbackFuncType cbFunc = 0, void * cbParam = 0, idx cbSecs = 10)
        {
            m_callback = cbFunc;
            m_callbackParam = cbParam;
            m_callbackSecs = cbSecs;
            return this;
        }

        sRC getFile(const char * destFilename, idx * size = 0, const char * wget_opts = 0, const char * url = 0, ...);

    private:
        slib::sPipe::callbackFuncType * m_callback;
        void * m_callbackParam;
        idx m_callbackSecs;
};

class dmLib
{
    public:
        class File
        {
            public:
                File();
                bool init(const char * path, const char * name);

                const char * const name(sStr * buf = 0) const
                {
                    return _get(_name, buf);
                }
                const char * const path(sStr * buf = 0) const
                {
                    return _get(_path, buf);
                }
                const char * const dir(sStr * buf = 0) const
                {
                    return _get(_dir, buf);
                }
                const char * const location(sStr * buf = 0) const
                {
                    return _get(_location, buf);
                }
                idx size() const
                {
                    return _size;
                }
                const char * const MD5() const
                {
                    return _md5;
                }

            protected:

                friend class dmLib;

                const char * const _get(const char * val, sStr * buf = 0) const
                {
                    if( buf && val ) {
                        buf->printf("%s", val);
                    }
                    return val;
                }

                sStr _location;
                sStr _path;
                sStr _dir;
                const char * _name;
                idx _size;
                char _md5[33];
                idx _id;
        };

        dmLib(udx max_unpack_depth = ~0, udx max_collect_depth = ~0)
            : _max_unpack_depth(max_unpack_depth), _max_collect_depth(max_collect_depth), _unpack_depth(0), _collect_depth(0)
        {
        }

        idx dim() const
        {
            return _list.dim();
        }
        const File * first() const
        {
            return _list.ptr(0);
        }
        const File * next(const File * curr) const
        {
            return (curr && curr->_id < _list.dim()) ? _list.ptr(curr->_id) : 0;
        }
        bool end(const File * curr) const
        {
            return !curr || (curr->_id > _list.dim());
        }

        typedef idx (*callbackFunc)(void * param, idx FSSize);

        bool unpack(const char * path, const char * name = 0, sStr * log = 0, sStr * msg = 0, callbackFunc callb = 0, void * callbParam = 0, idx callbsecs = 10);
        static void clean(const char * path);

        enum EPackAlgo {
            eUnspecified = 0,
            eNone,
            eTar,
            eGzip,
            eBzip2,
            eZip,
        };

        static idx arcDim(const char * path);
        static bool pack(const char * srcPath, sStr & dstPath, EPackAlgo algo, sStr * log = 0, sStr * msg = 0, sStr * name = 0, callbackFunc callb = 0, void * callbParam = 0, idx callbSecs = 10, bool deletePackedFiles = false, bool packAllFiles = false);

    protected:

        bool collect(const char * path, const char * meta, sStr & log, sStr & msg, idx level, callbackFunc callb, void * callbParam, idx callbsecs);

        const udx _max_unpack_depth, _max_collect_depth;
        udx _unpack_depth, _collect_depth;
        sVec<File> _list;

};

#endif 