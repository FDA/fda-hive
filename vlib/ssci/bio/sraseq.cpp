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
#include <slib/std/dll.hpp>
#include <slib/std/string.hpp>
#include <ssci/bio/sraseq.hpp>

using namespace slib;

#define LIBSRA_PREFIX "libSRA_"
const char* proc_names[] = {
        LIBSRA_PREFIX"Version",
        LIBSRA_PREFIX"SetReadAsDna",
        LIBSRA_PREFIX"SetReadAs4na",
        LIBSRA_PREFIX"SetReadAs2na",
        LIBSRA_PREFIX"SRAMgrOpenTableRead",
        LIBSRA_PREFIX"SRATableRelease",
        LIBSRA_PREFIX"SRATableRows",
        LIBSRA_PREFIX"GetReadNumber",
        LIBSRA_PREFIX"GetSeqLen",
        LIBSRA_PREFIX"GetSeqName",
        LIBSRA_PREFIX"GetSeq",
        LIBSRA_PREFIX"GetQual"
};


static union {
    sDll::Proc p[12];
    struct {
        udx (*ver)(void);
        void (*asDna)(void);
        void (*as4na)(void);
        void (*as2na)(void);
        const void* (*open)(const char* path, udx cache_size);
        void (*close)(const void* handle);
        udx (*dim)(const void* handle);
        udx (*num)(const void* handle, udx row, udx read_types);
        udx (*len)(const void* handle, udx row, idx read_num);
        idx (*id)(const void* handle, udx row, idx read_num, void* buf, idx buf_sz);
        idx (*seq)(const void* handle, udx row, idx read_num, void* buf, idx buf_sz);
        idx (*qua)(const void* handle, udx row, idx read_num, void* buf, idx buf_sz);
    } f;
} vtbl = {{0,0,0,0,0,0,0,0,0,0,0,0}};

static bool OpenDll(void)
{
    static sDll s_dll;
    const char* lib_names[] = { "libsra.2", "libsra" };
    bool success = true;

    if(vtbl.f.ver == 0) {
        for(udx i = 0; i < sizeof(lib_names) / sizeof(lib_names[0]); ++i) {
            success = true;
            for(udx j = 0; j < sizeof(proc_names) / sizeof(proc_names[0]); ++j) {
                vtbl.p[j] = s_dll.loadProc(lib_names[i], proc_names[j]);
                if(vtbl.p[j] == 0) {
                    success = false;
                    break;
                }
            }
        }
    }
    if(success) {
        udx ver = vtbl.f.ver();
        if((ver >> 24) != 2) {
            success = false;
        }
    }
    return success;
}

sSRASeq::sSRASeq(const char* path, udx cache_size)
        : sBioseq(), m_handle(0)
{
    if(OpenDll()) {
        m_handle = vtbl.f.open(path, cache_size);
        vtbl.f.as2na();
        m_seq.resize(sSizePage);
        m_id.resize(sSizePage);
        m_qual.resize(sSizePage);
    }
}

sSRASeq::~sSRASeq()
{
    if( m_handle ) {
        vtbl.f.close(m_handle);
    }
}

idx sSRASeq::dim(void)
{
    return m_handle ? vtbl.f.dim(m_handle) : 0;
}

idx sSRASeq::num(idx num, idx readtypes)
{
    return vtbl.f.num(m_handle, num, readtypes);
}

idx sSRASeq::len(idx num, idx iread)
{
    return vtbl.f.len(m_handle, num, iread);
}

inline
const char * sSRASeq_readbuf(const void* m_handle, idx num, idx iread, sStr& buf,
        idx (*func)(const void* handle, udx row, idx read_num, void* buf, idx buf_sz))
{
    idx i = 0;
    do {
        idx q = func(m_handle, num, iread, buf.ptr(), buf.total());
        if(q <= buf.total()) {
            break;
        }
        buf.resize(q);
    } while(++i < 10);
    return buf.ptr();
}

const char * sSRASeq::seq(idx num, idx iread, idx ipos, idx ilen )
{
    return sSRASeq_readbuf(m_handle, num, iread, m_seq, vtbl.f.seq);
}

const char * sSRASeq::id(idx num, idx iread)
{
    return sSRASeq_readbuf(m_handle, num, iread, m_id, vtbl.f.id);
}

const char * sSRASeq::qua(idx num, idx iread, idx ipos, idx ilen )
{
    return sSRASeq_readbuf(m_handle, num, iread, m_qual, vtbl.f.qua);
}
