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

#include <ion/sIon.hpp>
#include <ssci/bio/tax-ion.hpp>
#include <qlib/QPrideProc.hpp>
#include <violin/hiveqlang.hpp>
#include <violin/hivespecobj.hpp>
#include <violin/hiveseq.hpp>
#include <violin/hiveal.hpp>
#include <qpsvc/qpsvc-dna-hexagon.hpp>
#include <violin/alignparse.hpp>
#include <xlib/dmlib.hpp>
#include <xlib/md5.hpp>

using namespace slib;
using namespace slib::qlang;
using namespace sviolin;

sHiveContext::sHiveContext()
    : TParent(), _tax_ion(0)
{
    registerDefaultBuiltins();
}

sHiveContext::sHiveContext(sUsr & usr, idx flags)
    : TParent(usr, flags), _tax_ion(0)
{
    registerDefaultBuiltins();
}

sHiveContext::~sHiveContext()
{
    delete _tax_ion;
}

void sHiveContext::init(sUsr & usr, idx flags)
{
    reset();
    TParent::init(usr, flags);
    registerDefaultBuiltins();
}

void sHiveContext::reset()
{
    delete _tax_ion;
    _tax_ion = 0;
    TParent::reset();
}

bool sHiveContext::ensureTaxIon()
{
    if( !_tax_ion ) {
        sStr tax_ion_path, error_log;
        if( !sviolin::SpecialObj::findTaxDbIonPath(tax_ion_path, *_usr, 0, 0, &error_log) ) {
            setError(EVAL_OTHER_ERROR, "%s", error_log.ptr());
            return false;
        }
        _tax_ion = new sTaxIon(tax_ion_path);
    }
    return true;
}

sHiveEngine::sHiveEngine()
    : TParent()
{
}

sHiveEngine::~sHiveEngine()
{
}

sHiveEngine::sHiveEngine(sUsr & usr, idx ctx_flags)
    : TParent(usr, ctx_flags)
{
    delete _ctx;
    _ctx = new sHiveContext(usr, ctx_flags);
}

void sHiveEngine::init(sUsr & usr, idx ctx_flags)
{
    TParent::init(usr, ctx_flags);
    delete _ctx;
    _ctx = new sHiveContext(usr, ctx_flags);
}

sHiveInternalContext::sHiveInternalContext()
    : TParent(), _tax_ion(0)
{
    registerDefaultBuiltins();
}

sHiveInternalContext::sHiveInternalContext(sQPrideBase * qp, sUsr & usr, idx flags)
    : TParent(qp, usr, flags), _tax_ion(0)
{
    registerDefaultBuiltins();
}

sHiveInternalContext::~sHiveInternalContext()
{
    delete _tax_ion;
}

void sHiveInternalContext::init(sQPrideBase * qp, sUsr & usr, idx flags)
{
    reset();
    TParent::init(qp, usr, flags);
    registerDefaultBuiltins();
}

void sHiveInternalContext::reset()
{
    delete _tax_ion;
    _tax_ion = 0;
    TParent::reset();
}

bool sHiveInternalContext::ensureTaxIon()
{
    if( !_tax_ion ) {
        sStr tax_ion_path, error_log;
        if( !sviolin::SpecialObj::findTaxDbIonPath(tax_ion_path, *_usr, 0, 0, &error_log) ) {
            setError(EVAL_OTHER_ERROR, "%s", error_log.ptr());
            return false;
        }
        _tax_ion = new sTaxIon(tax_ion_path);
    }
    return true;
}

sHiveInternalEngine::sHiveInternalEngine()
    : TParent()
{
}

sHiveInternalEngine::~sHiveInternalEngine()
{
}

sHiveInternalEngine::sHiveInternalEngine(sQPrideBase * qp, sUsr & usr, idx ctx_flags)
    : TParent(qp, usr, ctx_flags)
{
    delete _ctx;
    _ctx = new sHiveInternalContext(qp, usr, ctx_flags);
}

void sHiveInternalEngine::init(sQPrideBase * qp, sUsr & usr, idx ctx_flags)
{
    TParent::init(qp, usr, ctx_flags);
    delete _ctx;
    _ctx = new sHiveInternalContext(qp, usr, ctx_flags);
}


class sHiveContext_tax_id_info: public BuiltinFunction
{
    public:
        sHiveContext_tax_id_info(const char * name)
        {
            _name.printf("builtin %s() function", name);
        }

        bool call(sVariant & result, Context & qlctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            sHiveContext & ctx = static_cast<sHiveContext&>(qlctx);

            if( !ctx.ensureTaxIon() || !checkNArgs(ctx, nargs, 1, 1) ) {
                return false;
            }
            sStr tax_id_buf("%" UDEC, args->asUInt());
            const char * tax_result = ctx.getTaxIon()->getTaxIdInfo(tax_id_buf.ptr(), tax_id_buf.length());
            idx tax_result_len = sLen(tax_result);
            if( !tax_result || !tax_result_len ) {
                result.setNull();
                return true;
            }
            sTxtTbl result_parser;
            result_parser.parseOptions().flags = 0;
            result_parser.setBuf(tax_result, tax_result_len);
            if( !result_parser.parse() || result_parser.cols() < 5 ) {
                ctx.setError(EVAL_SYSTEM_ERROR, "Taxonomy database result in unexpected format");
                return false;
            }
            result.setDic();
            result.setElt("tax_id", result_parser.uval(0, 0));
            result.setElt("parent_tax_id", result_parser.uval(0, 1));
            sVariant rank_value;
            result_parser.val(rank_value, 0, 2);
            result.setElt("rank", rank_value);
            sVariant name_value;
            result_parser.val(name_value, 0, 3);
            result.setElt("name", name_value);
            result.setElt("num_children", result_parser.ival(0, 4));
            return true;
        }
};


class sHiveContext_tax_ids_by_name: public BuiltinFunction
{
    public:
        sHiveContext_tax_ids_by_name(const char * name)
        {
            _name.printf("builtin %s() function", name);
        }

        bool call(sVariant & result, Context & qlctx, sVariant * topic, sVariant * args, idx nargs) const
        {
            sHiveContext & ctx = static_cast<sHiveContext&>(qlctx);

            if( !ctx.ensureTaxIon() || !checkNArgs(ctx, nargs, 1, 2) ) {
                return false;
            }

            idx limit = 0;
            if( nargs > 1 ) {
                limit = args[1].asInt();
            }

            const char * tax_result = ctx.getTaxIon()->getTaxIdsByName(args->asString(), limit);
            idx tax_result_len = sLen(tax_result);
            sTxtTbl result_parser;
            result_parser.parseOptions().flags = 0;
            result_parser.setBuf(tax_result);
            if( (tax_result_len && !result_parser.parse()) || (result_parser.rows() && result_parser.cols() < 2) ) {
                ctx.setError(EVAL_SYSTEM_ERROR, "Taxonomy database result in unexpected format");
                return 0;
            }

            result.setList();
            for(idx ir = 0; ir < result_parser.rows(); ir++) {
                sVariant row;
                row.setDic();
                row.setElt("tax_id", result_parser.uval(ir, 0));
                sVariant name_val;
                result_parser.val(name_val, ir, 1);
                row.setElt("name", name_val);
                result.push(row);
            }

            return true;
        }
};

#define REGISTER_BUILTIN_FUNC(name) \
static sHiveContext_ ## name builtin_ ## name(#name); \
registerBuiltinFunction(#name, builtin_ ## name)

bool qlangCache(sQPrideBase * qp, const udx id, sStr & path, const char * fmt, ...)
{
    path.cut0cut();
    qp->configGet(&path, 0, "user.qlang-cache", 0, 0);
    if( path ) {
        path.printf("%03" UDEC "/%" UDEC "/", id % 1000, id);
        if( sDir::exists(path.ptr()) || sDir::makeDir(path.ptr()) ) {
            sCallVarg(path.vprintf, fmt);
            if( sFile::exists(path) && !sFile::size(path) && !sFile::remove(path) ) {
                return false;
            }
        } else {
            return false;
        }
    }
    return sFile::exists(path);
}



idx fastx_impl_callback(void * param, sStr * buf, idx initid, idx initseq, idx initqua, idx seqlen)
{
    sQPrideBase * qp = (sQPrideBase*) param;
    if( qp ) {
        return qp->reqProgress(qp->reqId, 10, buf->pos(), -1, 100);
    }
    return 1;
}
idx fastx_impl_callback2(void * param, idx FSSize)
{
    sQPrideBase * qp = (sQPrideBase*) param;
    if( qp ) {
        return qp->reqProgress(qp->reqId, 10, FSSize, -1, 100);
    }
    return 1;
}

const char * fasta_expected_extensions = "fasta" _ "fa" _ "fas" _ "fsa" _ "fna" __;
const char * fastq_expected_extensions = "fq" _ "fastq" __;
class sHiveContext_fastx: public sUsrQueryBuiltinFunction
{
    public:
        sHiveContext_fastx(const char * name, const bool useQualities)
            : sUsrQueryBuiltinFunction(name), _useQualities(useQualities)
        {
        }
        virtual ~sHiveContext_fastx()
        {
        }
        virtual sUsrQueryBuiltinImpl* getImpl(Context & ctx, const BuiltinFunction & func) const
        {
            sHiveInternalContext * hctx = dynamic_cast<sHiveInternalContext*>(&ctx);
            sQPrideBase * qp = hctx ? dynamic_cast<sQPrideBase*>(hctx->getQPride()) : 0;
            return new fastx_impl(qp, func, _useQualities);
        }
    private:
        const bool _useQualities;

    public:
        class fastx_impl: public sUsrQueryBuiltinBase_files
        {
                typedef sUsrQueryBuiltinBase_files TParent;

            public:

                fastx_impl(sQPrideBase * qp, const BuiltinFunction & funcObj, const bool useQualities)
                    : TParent(funcObj), _qp(qp), _qual(useQualities), _hseq(0), _ufile(0)
                {
                }
                virtual ~fastx_impl()
                {
                    if( _flags & eFX_ConcatenateObjects ) {
                        if( _ids.dim() ) {
                            sStr cached;
                            sMD5 md5;
                            for(idx i = 0; i < _ids.dim(); ++i) {
                                cached.cut0cut();
                                _ids[i].print(cached);
                                md5.parse_buffer(cached.ptr(), cached.pos());
                            }
                            cached.cut0cut();
                            dumpFile(_final, cached, qlangCache(_qp, _ids[0].objId(), cached, "%s-%" UDEC ".fast%c",
                                md5.sum(), (_flags & (eFX_ConcatenateObjects | eFP_KeepOriginalIDs)), _qual ? 'q' : 'a') );
                        } else {
                            dumpFile(_final.ptr(), 0, false);
                        }
                    }
                    delete _hseq;
                }

                sRC files(sUsrInternalContext & ctx, sUsrObj & obj, sStr & list00)
                {
                    if( isValidUFile(obj) ) {
                        list00.printf("_.%s", _ufile->propGet("ext"));
                    } else if( obj.isTypeOf("u-hiveseq+") ) {
                        list00.printf("%s.fast%c", obj.IdStr(), _qual ? 'q' : 'a');
                    } else {
                        return RC(sRC::eEvaluating, sRC::eCallback, sRC::eFile, sRC::eIgnored);
                    }
                    list00.add0(2);
                    return sRC::zero;
                }

                sRC add_extension(sUsrInternalContext & ctx, const char * src, sStr & dst)
                {
                    if(_ufile) {
                        sFilePath ext(src, "%%ext");
                        ext.shrink00();
                        dst.printf("%s%s", ext ? "." : "", ext.ptr());
                    } else {
                        dst.printf(".fast%c", _qual ? 'q' : 'a');
                    }
                    return sRC::zero;
                }

                sRC make_file(sUsrInternalContext & ctx, sUsrObj & obj, const char * src, const char * dst)
                {
                    if( _ufile ) {
                        if( !sFile::symlink(src, dst) ) {
#if _DEBUG
                            fprintf(stderr, "Failed to link for %s '%s' to '%s'\n", obj.IdStr(), src, dst);
#endif
                            return RC(sRC::eCreating, sRC::eSymLink, sRC::eOperation, sRC::eFailed);
                        }
                        return sRC::zero;
                    }
                    if( !(_flags & eFX_ConcatenateObjects) ) {
                        delete _hseq;
                        _hseq = 0;
                    }
                    if( !_hseq ) {
                        sUsr * user = ctx.getUsr();
                        if( !user ) {
                            return RC(sRC::eEvaluating, sRC::eCallback, sRC::eUser, sRC::eInvalid);
                        }
                        _hseq = new sHiveseq(user);
                    }
                    sHiveId * id = _ids.add();
                    if( id ) {
                        *id = obj.Id();
                    }
                    _hseq->parse(obj.IdStr(), sBioseq::eBioModeLong);
                    if( !(_flags & eFX_ConcatenateObjects) ) {
                        sStr cached;
                        return dumpFile(dst, cached, qlangCache(_qp, obj.Id().objId(), cached, "%s-%" UDEC ".fast%c",
                            obj.IdStr(), (_flags & (eFX_ConcatenateObjects | eFP_KeepOriginalIDs)), _qual ? 'q' : 'a') );
                    } else if( !_final ) {
                        sDir::cleanUpName(dst, _final);
                        sFil file(dst);
                        if( !file.ok() ) {
                            return RC(sRC::eCreating, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                    } else {
                        return RC(sRC::eEvaluating, sRC::eCallback, sRC::eFile, sRC::eIgnored);
                    }
                    return sRC::zero;
                }

                sRC dumpFile(const char * dst, const char * cached, const bool exists)
                {
                    if( sFile::exists(dst) ) {
                        if( !sFile::remove(dst) ) {
                            if( _qp ) {
                                _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Cannot delete existing file.");
                            }
                            return RC(sRC::eDeleting, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                    }
                    if( sDir::exists(dst) ) {
                        if( !sDir::removeDir(dst, true) ) {
                            if( _qp ) {
                                _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Cannot delete existing directory.");
                            }
                            return RC(sRC::eDeleting, sRC::eDirectory, sRC::eOperation, sRC::eFailed);
                        }
                    }
                    if( !exists ) {
                        sFil file((cached && cached[0]) ? cached : dst);
                        if( !file.ok() ) {
                            return RC(sRC::eCreating, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                        _hseq->print_callback = fastx_impl_callback;
                        _hseq->print_callbackParam = _qp;
                        if( _qp ) {
                            _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Info, "Started FAST%c export", _qual ? 'q' : 'a');
                        }
                        if( _hseq->printFastX(&file, _qual ? true : false, 0, _hseq->dim(), 0, _flags & eFP_KeepOriginalIDs, false, 0, 0) == 0 ) {
                            return RC(sRC::eCreating, sRC::eFile, sRC::eOperation, sRC::eInterrupted);
                        }
                        if( _qp ) {
                            _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Info, "Finished FAST%c export", _qual ? 'q' : 'a');
                        }
                    }
                    if( cached && cached[0] ) {
                        sFile::symlink(cached, dst);
                    }
                    return sRC::zero;
                }

                bool isValidExtension(sStr & nm) {
                    if(nm) {
                        const char * expected_extensions = _qual ? fastq_expected_extensions : fasta_expected_extensions;
                        const char * dot = strrchr(nm.ptr(), '.');
                        if( sIsExactly(dot, ".gz") || sIsExactly(dot, ".bz2") ) {
                            nm.shrink00();
                            nm.cut0cut(nm.length() - sLen(dot));
                            dot = strrchr(nm.ptr(), '.');
                        }
                        if( dot ) {
                            return sString::compareChoice(dot + 1, expected_extensions, 0, true, 0, false) != sNotIdx;
                        }
                    }
                    return false;
                }

                bool isValidUFile(sUsrObj & obj)
                {
                    _ufile = dynamic_cast<sUsrFile *>(&obj);
                    sStr nm;
                    if( !_ufile || !_ufile->isTypeOf("u-file") ) {
                        _ufile = 0;
                    } else if( obj.propGet("name", &nm) && !isValidExtension(nm) ) {
                        sRC res = RC(sRC::eValidating, sRC::eObject, sRC::eExtension, sRC::eWrongFormat);
                        _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Warning, res.print());
                        _ufile = 0;
                    }
                    return _ufile;
                }

            private:
                sQPrideBase * _qp;
                bool _qual;
                sviolin::sHiveseq * _hseq;
                sStr _final;
                sVec<sHiveId> _ids;
                sUsrFile * _ufile;

        };
};

class sHiveContext_fasta: public sHiveContext_fastx
{
    public:
        sHiveContext_fasta(const char * name)
            : sHiveContext_fastx(name, false)
        {
        }
};
class sHiveContext_fastq: public sHiveContext_fastx
{
    public:
        sHiveContext_fastq(const char * name)
            : sHiveContext_fastx(name, true)
        {
        }
};

class sHiveContext_sam: public sUsrQueryBuiltinFunction
{
    public:
        sHiveContext_sam(const char * name)
            : sUsrQueryBuiltinFunction(name)
        {
        }
        virtual sUsrQueryBuiltinImpl* getImpl(Context & ctx, const qlang::BuiltinFunction & func) const
        {
            sHiveInternalContext * hctx = dynamic_cast<sHiveInternalContext*>(&ctx);
            sQPrideBase * qp = hctx ? dynamic_cast<sQPrideBase*>(hctx->getQPride()) : 0;
            return new sam_impl(qp, func);
        }
    private:

        class sam_impl: public sUsrQueryBuiltinBase_files
        {
                typedef sUsrQueryBuiltinBase_files TParent;
                sQPrideBase * _qp;
                sUsrFile * _ufile;

            public:
                sam_impl(sQPrideBase * qp, const BuiltinFunction & funcObj)
                    : TParent(funcObj), _qp(qp), _ufile(0)
                {
                }
                virtual ~sam_impl()
                {
                }

            protected:
                sRC files(sUsrInternalContext & ctx, sUsrObj & obj, sStr & list00)
                {
                    if( isValidUFile(obj) ) {
                        list00.printf("_.%s", _ufile->propGet("ext"));
                    } else if( obj.isTypeOf("svc-align+") ) {
                        list00.printf("%s.sam", obj.IdStr());
                    } else {
                        return RC(sRC::eEvaluating, sRC::eCallback, sRC::eFile, sRC::eIgnored);
                    }
                    list00.add0(2);
                    return sRC::zero;
                }

                sRC add_extension(sUsrInternalContext & ctx, const char * src, sStr & dst)
                {
                    if(_ufile) {
                        sFilePath ext(src, "%%ext");
                        ext.shrink00();
                        dst.printf("%s%s", ext ? "." : "", ext.ptr());
                    } else {
                        dst.printf(".sam");
                    }
                    return sRC::zero;
                }

                sRC make_file(sUsrInternalContext & ctx, sUsrObj & obj, const char * src, const char * dst)
                {
                    if( _ufile ) {
                        if( !sFile::symlink(src, dst) ) {
#if _DEBUG
                            fprintf(stderr, "Failed to link for %s '%s' to '%s'\n", obj.IdStr(), src, dst);
#endif
                            return RC(sRC::eCreating, sRC::eSymLink, sRC::eOperation, sRC::eFailed);
                        }
                        return sRC::zero;
                    }
                    sRC rc;
                    sUsr * user = ctx.getUsr();
                    if( user ) {
                        sHiveal hiveal(user, obj.IdStr());
                        if( hiveal.dimAl() != 0 ) {
                            sStr subject, query;
                            QPSvcDnaHexagon::getQuery00(obj, query, ";");
                            QPSvcDnaHexagon::getSubject00(obj, subject, ";");
                            sHiveseq Sub(user, subject.ptr(), hiveal.getSubMode());
                            sHiveseq Qry(user, query.ptr(), hiveal.getQryMode());
                            if( Sub.dim() == 0 ) {
                                if( _qp ) {
                                    _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Reference '%s' sequences are missing or corrupt", subject.length() ? subject.ptr() : "unspecified");
                                }
                                rc = RC(sRC::eEvaluating, sRC::eReference, sRC::eReference, sRC::eEmpty);
                            }
                            if( Qry.dim() == 0 ) {
                                if( _qp ) {
                                    _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Query/Read '%s' sequences are missing or corrupt", query.length() ? query.ptr() : "unspecified");
                                }
                                rc = RC(sRC::eEvaluating, sRC::eQuery, sRC::eSequence, sRC::eEmpty);
                            }
                            if( !rc.isSet() ) {
                                if( sFile::exists(dst) ) {
                                    if( !sFile::remove(dst) ) {
                                        if( _qp ) {
                                            _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Cannot delete existing file.");
                                        }
                                        return RC(sRC::eDeleting, sRC::eFile, sRC::eOperation, sRC::eFailed);
                                    }
                                }
                                if( sDir::exists(dst) ) {
                                    if( !sDir::removeDir(dst, true) ) {
                                        if( _qp ) {
                                            _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Cannot delete existing directory.");
                                        }
                                        return RC(sRC::eDeleting, sRC::eDirectory, sRC::eOperation, sRC::eFailed);
                                    }
                                }
                                sStr cached;
                                const bool exists = qlangCache(_qp, obj.Id().objId(), cached, "%s.sam", obj.IdStr());
                                if( !exists ) {
                                    if( _qp ) {
                                        _qp->reqProgress(_qp->reqId, 10, 1, 10, 100);
                                        _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Info, "Started %s SAM export", obj.IdStr());
                                    }
                                    sFil samHeader;
                                    sFil samFooter;
                                    SAMHeaders(obj, samHeader, samFooter);
                                    sViosam::convertVioaltIntoSam(&hiveal,  samHeader, samFooter, -1, &Qry, &Sub, true, cached ? cached.ptr() : dst, 0, _qp, _qp ? sQPrideProc::reqProgressStatic : 0);
                                    if( _qp ) {
                                        _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Info, "Finished %s SAM export", obj.IdStr());
                                    }
                                }
                                if( cached && cached[0] ) {
                                   sFile::symlink(cached, dst);
                                }
                            }
                        } else {
                            if( _qp ) {
                                _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Warning, "Object '%s' has no alignments", obj.IdStr());
                            }
                            rc = RC(sRC::eEvaluating, sRC::eCallback, sRC::eFile, sRC::eIgnored);
                        }
                    } else {
                        rc = RC(sRC::eEvaluating, sRC::eCallback, sRC::eUser, sRC::eInvalid);
                    }
                    return rc;
                }

                bool isValidExtension(sStr & nm)
                {
                    if( nm ) {
                        const char * dot = strrchr(nm.ptr(), '.');
                        if( dot ) {
                            return sString::compareChoice(dot + 1, "sam" _ "bam" __, 0, true, 0, false) != sNotIdx;
                        }
                    }
                    return false;
                }

                bool isValidUFile(sUsrObj & obj)
                {
                    _ufile = dynamic_cast<sUsrFile *>(&obj);
                    sStr nm;
                    if( !_ufile || !_ufile->isTypeOf("u-file") ) {
                        _ufile = 0;
                    } else if( obj.propGet("ext", &nm) && !isValidExtension(nm) ) {
                        sRC res = RC(sRC::eValidating, sRC::eObject, sRC::eExtension, sRC::eWrongFormat);
                        _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Warning, res.print());
                        _ufile = 0;
                    }
                    return _ufile;
                }
        };
};

class UnalignedPrinter
{
    public:
        virtual ~UnalignedPrinter()
        {
        }
        virtual void print(sStr & out, AlRead read, sHiveseq & Qry, bool printOriginalId) const = 0;
};
class FastaPrinter : public UnalignedPrinter
{
    public:
        virtual void print(sStr & out, AlRead read, sHiveseq & Qry, bool printOriginalId) const
        {
            Qry.printFastXRow(&out, false, read.row, 0, 0, 0, printOriginalId, false, 0, 0, sBioseq::eSeqForward, true, 0, true,false,false);
        }
};
class FastqPrinter : public UnalignedPrinter
{
    public:
        virtual void print(sStr & out, AlRead read, sHiveseq & Qry, bool printOriginalId) const
        {
            Qry.printFastXRow(&out, true, read.row, 0, 0, 0, printOriginalId, false, 0, 0, sBioseq::eSeqForward, true, 0, true,false,false);
        }
};
class SAMPrinter : public UnalignedPrinter
{
    public:
        virtual void print(sStr & out, AlRead read, sHiveseq & Qry, bool printOriginalId) const
        {
            Qry.printSAMRow(out, read.row, 0, read.samFlag(), printOriginalId, true);
        }
};

class sHiveContext_unaligned: public sUsrQueryBuiltinFunction
{
    private:
        const char * _fileExt;
        const UnalignedPrinter & _printer;

    public:
        sHiveContext_unaligned(const char * name, const char * fileExt, UnalignedPrinter & printer)
            : sUsrQueryBuiltinFunction(name), _fileExt(fileExt), _printer(printer)
        {
        }
        virtual sUsrQueryBuiltinImpl* getImpl(Context & ctx, const qlang::BuiltinFunction & func) const
        {
            sHiveInternalContext * hctx = dynamic_cast<sHiveInternalContext*>(&ctx);
            sQPrideBase * qp = hctx ? dynamic_cast<sQPrideBase*>(hctx->getQPride()) : 0;
            return new unaligned_impl(qp, func, _fileExt, _printer);
        }
        class unaligned_impl: public sUsrQueryBuiltinBase_files
        {
                typedef sUsrQueryBuiltinBase_files TParent;
                sQPrideBase * _qp;

            public:
                unaligned_impl(sQPrideBase * qp, const BuiltinFunction & funcObj, const char * fileExt, const UnalignedPrinter & printer)
                    : TParent(funcObj), _qp(qp), _curQry(0), _filesToPrint(), _fileExt(fileExt), _printer(printer), _printedForCurObj(false), _alBitMap()
                {
                    firstFlNms.tmpName = "_tmp_R1_unaligned";
                    firstFlNms.name = "_R1_unaligned";
                    secondFlNms.tmpName = "_tmp_R2_unaligned";
                    secondFlNms.name = "_R2_unaligned";
                    combinedFlNms.tmpName = "_tmp_unaligned";
                    combinedFlNms.name = "_unaligned";
                }
                virtual ~unaligned_impl()
                {
                    delete _curQry;
                }

            protected:

                typedef struct {
                        const char * tmpName;
                        const char * name;
                } FileNames;
                FileNames firstFlNms;
                FileNames secondFlNms;
                FileNames combinedFlNms;

                sHiveseq * _curQry;
                sStack<FileNames> _filesToPrint;
                const char * _fileExt;
                const UnalignedPrinter & _printer;
                bool _printedForCurObj;
                AlBitMap _alBitMap;

                void populateFlNmStack()
                {
                    if ( _flags & eUO_PairInOne ) {
                        *_filesToPrint.push() = combinedFlNms;
                    }
                    if ( _flags & eUO_PairSplit ) {
                        *_filesToPrint.push() = firstFlNms;
                        *_filesToPrint.push() = secondFlNms;
                    }
                }

                sRC initQuery(sUsr & user, sUsrObj & obj)
                {
                    sStr qryIds;
                    QPSvcDnaHexagon::getQuery00(obj, qryIds, ";");
                    _curQry = new sHiveseq(&user, qryIds.ptr(), sBioseq::eBioModeLong);
                    if( _curQry->dim() == 0 ) {
                        if( _qp ) {
                            _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Query/Read '%s' sequences are missing or corrupt", qryIds.length() ? qryIds.ptr() : "unspecified");
                        }
                        return RC(sRC::eEvaluating, sRC::eQuery, sRC::eFile, sRC::eIgnored);
                    }
                    return sRC::zero;
                }

                sRC initAlBitMap(sUsr & user, sUsrObj & obj)
                {
                    sHiveal curAl(&user, obj.IdStr());
                    sStr qryIds;
                    QPSvcDnaHexagon::getQuery00(obj, qryIds, ";");
                    sHiveseq Qry(&user, qryIds.ptr(), curAl.getQryMode());
                    if( Qry.dim() == 0 ) {
                        if( _qp ) {
                            _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Query/Read '%s' sequences are missing or corrupt", qryIds.length() ? qryIds.ptr() : "unspecified");
                        }
                        return RC(sRC::eEvaluating, sRC::eQuery, sRC::eFile, sRC::eIgnored);
                    }
                    _alBitMap.build(curAl, Qry.dim());
                    return sRC::zero;
                }

                sRC parseArgs(Context &ctx, sVariant *args, const idx& nargs)
                {
                    sRC rc = TParent::parseArgs(ctx, args, nargs);
                    if ( !( _flags & eUO_PairInOne ) && !( _flags & eUO_PairSplit ) ) {
                        _flags |= eUO_PairInOne;
                    }
                    return rc;
                }

                sRC files(sUsrInternalContext & ctx, sUsrObj & obj, sStr & list00)
                {
                    if( !obj.isTypeOf("svc-align+") ) {
                        return RC(sRC::eEvaluating, sRC::eCallback, sRC::eFile, sRC::eIgnored);
                    }

                    sUsr * user = ctx.getUsr();
                    if ( !user ) {
                        return RC(sRC::eEvaluating, sRC::eCallback, sRC::eUser, sRC::eInvalid);
                    }

                    delete _curQry;
                    _curQry = 0;
                    if ( sRC rc = initQuery(*user, obj) ) {
                        return rc;
                    }

                    if ( sRC rc = initAlBitMap(*user, obj) ) {
                        return rc;
                    }

                    _filesToPrint.cut();
                    populateFlNmStack();

                    _printedForCurObj = false;

                    for (idx i = 0; i < _filesToPrint.dim(); ++i) {
                        list00.printf("%s%s", obj.IdStr(), _filesToPrint[i].name);
                        list00.add0();
                    }
                    list00.add0();
                    return sRC::zero;
                }

                sRC add_extension(sUsrInternalContext & ctx, const char * src, sStr & dst)
                {
                    if ( _filesToPrint.dim() == 0 ) {
                        return RC(sRC::ePopping, sRC::eStack, sRC::eOperation, sRC::eFailed);
                    }
                    dst.printf("%s%s", _filesToPrint.top()->name, _fileExt);
                    return sRC::zero;
                }

                bool tempName(sStr & out, const char * name, const sHiveId & id)
                {
                    const bool exists = qlangCache(_qp, id.objId(), out, "%s", name);
                    if( !out ) {
                        out.printf(0, "%s", name);
                    }
                    return exists;
                }

                sRC initTmpFile(sFil & out, const char * name, const sHiveId & id)
                {
                    sStr filePath;
                    const bool exists = tempName(filePath, name, id);
                    if( !exists ) {
                        out.init(filePath.ptr());
                        if( !out.ok() ) {
                            return RC(sRC::eOpening, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                    }
                    return sRC::zero;
                }

                sRC dumpUnalignedPaired(const sHiveId & id)
                {
                    sFil firstR;
                    sFil secondR;
                    sFil combinedR;
                    bool printOriginalIds = _flags & eFP_KeepOriginalIDs;

                    if ( _flags & eUO_PairInOne ) {
                        if ( sRC rc = initTmpFile(combinedR, combinedFlNms.tmpName, id) ) {
                            return rc;
                        }
                    }
                    if ( _flags & eUO_PairSplit ) {
                        if ( sRC rc = initTmpFile(firstR, firstFlNms.tmpName, id) ) {
                            return rc;
                        }
                        if ( sRC rc = initTmpFile(secondR, secondFlNms.tmpName, id) ) {
                            return rc;
                        }
                    }

                    PairedReadIter iter(*_curQry, _alBitMap);
                    sVec<AlRead> reads;
                    for (; iter.pos() < iter.dim(); iter.next()) {
                        iter.get(reads);
                        if ( _flags & eUO_PairInOne ) {
                            for (idx i = 0; i < reads.dim(); i++) {
                                if ( !reads[i].aligned ) {
                                    _printer.print(combinedR, reads[i], *_curQry, printOriginalIds);
                                }
                            }
                        }
                        if ( _flags & eUO_PairSplit ) {
                            if ( reads.dim() > 1 && !reads[0].aligned && !reads[1].aligned ) {
                                _printer.print(firstR, reads[0], *_curQry, printOriginalIds);
                                _printer.print(secondR, reads[1], *_curQry, printOriginalIds);
                            }
                        }
                    }

                    return sRC::zero;
                }

                bool isPairedAl(sUsrObj & obj)
                {
                    sStr buf;
                    obj.propGet00("query_paired", &buf, ";");
                    return buf.length() > 0;
                }

                sRC dumpUnaligned(const sHiveId & id)
                {
                    if ( _flags & eUO_PairInOne ) {
                        bool printOriginalIds = _flags & eFP_KeepOriginalIDs;
                        sFil combinedR;
                        if ( sRC rc = initTmpFile(combinedR, combinedFlNms.tmpName, id) ) {
                            return rc;
                        }

                        UnpairedReadIter iter(*_curQry, _alBitMap);
                        sVec<AlRead> reads;
                        for (; iter.pos() < iter.dim(); iter.next()) {
                            iter.get(reads);
                            for (idx i = 0; i < reads.dim(); i++) {
                                if ( !reads[i].aligned ) {
                                    _printer.print(combinedR, reads[i], *_curQry, printOriginalIds);
                                }
                            }
                        }
                    }
                    return sRC::zero;
                }

                sRC make_file(sUsrInternalContext & ctx, sUsrObj & obj, const char * src, const char * dst)
                {
                    if ( _filesToPrint.dim() == 0 ) {
                        return RC(sRC::ePopping, sRC::eStack, sRC::eOperation, sRC::eFailed);
                    }

                    if( sFile::exists(dst) ) {
                        if( !sFile::remove(dst) ) {
                            if( _qp ) {
                                _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Cannot delete existing file.");
                            }
                            _filesToPrint.pop();
                            return RC(sRC::eDeleting, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                    }
                    if( sDir::exists(dst) ) {
                        if( !sDir::removeDir(dst, true) ) {
                            if( _qp ) {
                                _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Cannot delete existing directory.");
                            }
                            _filesToPrint.pop();
                            return RC(sRC::eDeleting, sRC::eDirectory, sRC::eOperation, sRC::eFailed);
                        }
                    }
                    if( _qp ) {
                        _qp->reqProgress(_qp->reqId, 10, 1, 10, 100);
                        _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Info, "Started %s unaligned export", obj.IdStr());
                    }
                    if ( !_printedForCurObj ) {
                        sRC dumpRC;
                        if ( isPairedAl(obj) ) {
                            dumpRC = dumpUnalignedPaired(obj.Id());
                        } else {
                            dumpRC = dumpUnaligned(obj.Id());
                        }
                        _printedForCurObj = true;
                        if ( dumpRC ) {
                            if ( _qp ) {
                                _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Cannot dump unaligned reads: %s", dumpRC.print());
                            }
                            _filesToPrint.pop();
                            return dumpRC;
                        }
                    }
                    sStr tmpName;
                    tempName(tmpName, _filesToPrint.top()->tmpName, obj.Id());
                    if( sFile::exists(tmpName.ptr()) ) {
                        if( !sFile::symlink(tmpName, dst) ) {
                            if( _qp ) {
                                _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Error, "Cannot rename existing file.");
                            }
                            _filesToPrint.pop();
                            return RC(sRC::eRenaming, sRC::eFile, sRC::eOperation, sRC::eFailed);
                        }
                    } else {
                        _filesToPrint.pop();
                        return RC(sRC::eAccessing, sRC::eFile, sRC::eFile, sRC::eIgnored);
                    }
                    if( _qp ) {
                        _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Info, "Finished %s unaligned export", obj.IdStr());
                    }
                    _filesToPrint.pop();
                    return sRC::zero;
                }
        };
};

class sHiveContext_unaligned_fasta: public sHiveContext_unaligned
{
    private:
        FastaPrinter _fastaPrinter;
    public:
        sHiveContext_unaligned_fasta(const char * name)
            : sHiveContext_unaligned(name, ".fasta", _fastaPrinter)
        {
        }
};
class sHiveContext_unaligned_fastq: public sHiveContext_unaligned
{
    private:
        FastqPrinter _fastqPrinter;
    public:
        sHiveContext_unaligned_fastq(const char * name)
            : sHiveContext_unaligned(name, ".fastq", _fastqPrinter)
        {
        }
};
class sHiveContext_unaligned_sam: public sHiveContext_unaligned
{
    private:
        SAMPrinter _samPrinter;
    public:
        sHiveContext_unaligned_sam(const char * name)
            : sHiveContext_unaligned(name, ".sam", _samPrinter)
        {
        }
};


class sHiveContext_ngs_index: public sUsrQueryBuiltinFunction
{
    public:
        sHiveContext_ngs_index(const char * name)
            : sUsrQueryBuiltinFunction(name)
        {
        }
        virtual sUsrQueryBuiltinImpl* getImpl(Context & ctx, const qlang::BuiltinFunction & func) const
        {
            sHiveInternalContext * hctx = dynamic_cast<sHiveInternalContext*>(&ctx);
            sQPrideBase * qp = hctx ? dynamic_cast<sQPrideBase*>(hctx->getQPride()) : 0;
            return new ngs_index_impl(qp, func);
        }
        class ngs_index_impl: public sUsrQueryBuiltinBase_files
        {
                typedef sUsrQueryBuiltinBase_files TParent;

            public:
                ngs_index_impl(sQPrideBase * qp, const BuiltinFunction & funcObj)
                    : TParent(funcObj), _qp(qp)
                {
                    *_map.set("bam") = "samtools-index" _ "bai" __;
                    *_map.set("cram") = "samtools-index" _ "crai" __;
                    *_map.set("fastq")  = "samtools-fqidx" _ "fai" __;
                    *_map.set("fq") = "samtools-fqidx" _ "fai" __;
                    *_map.set("fasta")  = "samtools-faidx" _ "fai" __;
                    *_map.set("fa") = "samtools-faidx" _ "fai" __;
                }

            protected:
                sHiveId & getToolpkg(sUsrInternalContext & ctx)
                {
                    static sHiveId tool;
                    if( !tool ) {
                        sUsr * user = ctx.getUsr();
                        sUsrObjRes r;
                        sStr v("ngstools,%s", _version);
                        user->objs2("tool_pkg", r, 0, "name,version", v.ptr());
                        if( r.dim() ) {
                            tool = *r.firstId();
                        }
                    }
                    return tool;
                }
                sHiveId getAlgo(sUsrInternalContext & ctx, const char * svc)
                {
                    sUsr * user = ctx.getUsr();
                    sUsrObjRes r;
                    sStr v("%s,%s", svc, _version);
                    user->objs2("algorithm-script", r, 0, "name,version", v.ptr());
                    return r.dim() ? *r.firstId() : sHiveId();
                }
                sRC parseArgs(Context &ctx, sVariant *args, const idx& nargs)
                {
                    _version = _func.getArgAsString(0, args, nargs);
                    return TParent::parseArgs(ctx, &args[1], nargs - 1);
                }

                sRC files(sUsrInternalContext & ctx, sUsrObj & obj, sStr & list00)
                {
                    if( !isValidUFile(obj) ) {
                        return RC(sRC::eEvaluating, sRC::eCallback, sRC::eFile, sRC::eIgnored);
                    }
                    const char * ext = _ufile->propGet("ext");
                    list00.printf("_.%s", ext);
                    list00.add0(1);
                    const char * ix = list00.printf("_.%s.%s", ext, sString::next00(_svc));
                    list00.add0(2);
                    sStr buf;
                    if( !_ufile->getFilePathname(buf, "%s", ix) ) {
                        sStr svcName("%s-%s", _svc, _version);
                        sVar form;
                        form.inp("prop.svc.input.1", obj.IdStr());
                        sHiveId algo = getAlgo(ctx, _svc);
                        form.inp("algo", algo.print());
                        sHiveId tool = getToolpkg(ctx);
                        form.inp("prop.svc.toolpkg.1", tool.print());
                        form.inp("type", svcName.ptr());
                        const idx req = _qp->reqProcSubmit(1, &form, svcName.ptr(), 0, sQPrideBase::eQPReqAction_Run);
                        if( !req ) {
                            return RC(sRC::eEvaluating, sRC::eCallback, sRC::eIndex, sRC::eFailed);
                        }
                        *_wait.add(1) = req;

                    }
                    return sRC::zero;
                }

                sRC add_extension(sUsrInternalContext & ctx, const char * src, sStr & dst)
                {
                    if(_ufile) {
                        sFilePath ext(src, "%%ext");
                        sFilePath nm(src, "%%flnm");
                        if( sIs("_.", nm) ) {
                            ext.printf(0, "%s", nm.ptr(2));
                        }
                        ext.shrink00();
                        dst.printf("%s%s", ext ? "." : "", ext.ptr());
                    }
                    return sRC::zero;
                }

                sRC make_file(sUsrInternalContext & ctx, sUsrObj & obj, const char * src, const char * dst)
                {
                    sRC rc = sRC::zero;
                    if( _wait.dim() ) {
                        bool done = false;
                        while( !done && rc.isUnset() ) {
                            done = true;
                            for(idx i = 0; i < _wait.dim(); ++i) {
                                if( _wait[i] ) {
                                    const idx status = _qp->reqGetStatus(_wait[i]);
                                    if( status == sQPrideBase::eQPReqStatus_Running ) {
                                        sleepSeconds(5);
                                        _qp->grpAssignReqID(_wait[i], _qp->grpId);
                                    } else if( status == sQPrideBase::eQPReqStatus_Done ) {
                                        _qp->grpAssignReqID(_wait[i], _qp->grpId);
                                        _wait[i] = 0;
                                    } else if( status > sQPrideBase::eQPReqStatus_Done ) {
                                        _qp->grpAssignReqID(_wait[i], _qp->grpId);
                                        _wait[i] = 0;
                                        rc = RC(sRC::eEvaluating, sRC::eCallback, sRC::eIndex, sRC::eFailed);
                                    } else {
                                        done = false;
                                    }
                                }
                            }
                        }
                        _wait.empty();
                    }
                    if( _ufile && !rc.isSet() ) {
                        if( !sFile::symlink(src, dst) ) {
#if _DEBUG
                            fprintf(stderr, "Failed to link for %s '%s' to '%s'\n", obj.IdStr(), src, dst);
#endif
                            rc = RC(sRC::eCreating, sRC::eSymLink, sRC::eOperation, sRC::eFailed);
                        }
                    }
                    return rc;
                }

                const char * const findTool(sStr & nm) {
                    if(nm) {
                        const char * dot = strrchr(nm.ptr(), '.');
                        if( sIsExactly(dot, ".gz") ) {
                            nm.shrink00();
                            nm.cut0cut(nm.length() - sLen(dot));
                            dot = strrchr(nm.ptr(), '.');
                        }
                        if( dot ) {
                            const char * const * p = _map.get(dot + 1);
                            return p ? *p : nullptr;
                        }
                    }
                    return nullptr;
                }

                bool isValidUFile(sUsrObj & obj)
                {
                    _ufile = dynamic_cast<sUsrFile *>(&obj);
                    sStr nm;
                    if( !_ufile || !_ufile->isTypeOf("u-file") ) {
                        _ufile = 0; _svc = 0;
                    } else if( obj.propGet("name", &nm) && !(_svc = findTool(nm)) ) {
                        sRC res = RC(sRC::eValidating, sRC::eObject, sRC::eExtension, sRC::eWrongFormat);
                        _qp->reqSetInfo(_qp->reqId, _qp->eQPInfoLevel_Warning, res.print());
                        _ufile = 0;
                    }
                    return _ufile && _svc;
                }

            private:
                const char * _version;
                sQPrideBase * _qp;
                sDic<const char *> _map;
                sUsrFile * _ufile;
                const char * _svc;
                sVec<idx> _wait;
        };
};

void sHiveContext::registerDefaultBuiltins()
{
    REGISTER_BUILTIN_FUNC(tax_id_info);
    REGISTER_BUILTIN_FUNC(tax_ids_by_name);
}


void sHiveInternalContext::registerDefaultBuiltins()
{
    TParent::registerDefaultBuiltins();
    REGISTER_BUILTIN_FUNC(tax_id_info);
    REGISTER_BUILTIN_FUNC(tax_ids_by_name);
    REGISTER_BUILTIN_FUNC(fasta);
    REGISTER_BUILTIN_FUNC(fastq);
    REGISTER_BUILTIN_FUNC(sam);
    REGISTER_BUILTIN_FUNC(unaligned_fasta);
    REGISTER_BUILTIN_FUNC(unaligned_fastq);
    REGISTER_BUILTIN_FUNC(unaligned_sam);
    REGISTER_BUILTIN_FUNC(ngs_index);
}
