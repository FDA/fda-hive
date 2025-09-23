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
#ifndef sLib_utils_tbl_hpp
#define sLib_utils_tbl_hpp

#include <math.h>
#include <stdint.h>
#include <slib/core/def.hpp>
#include <slib/core/str.hpp>
#include <slib/core/dic.hpp>
#include <slib/core/iter.hpp>
#include <slib/std/variant.hpp>
#include <slib/std/string.hpp>

namespace slib
{
    class sTbl {
    public:
        char * data;
        sVec <idx> fil;

        struct Element {idx start, len, icol, irow;} ;

        struct Hdr { idx cols, rows, shiftCols, shiftRows;char separators[16];};
        enum eFlags{
            fHdrCols=0x00000001,
            fHdrRows=0x00000002,
            fPreserveQuotes=0x00000004,
            fDoZero=0x000008,
            fSeparatorIsListStrings=0x00000010,
            fAllowEmptyCells=0x00000020
        };
        virtual ~sTbl(){fil.mex()->empty();}
        sTbl (const char * filestore=0, idx mode=sMex::fReadonly):
            fil(filestore ? mode : ((mode&(~sMex::fReadonly) )|sMex::fSetZero),filestore){};
        char * parse(char * src, idx len, idx flags=fPreserveQuotes,  const char * separ=",;\t", idx maxCount=sIdxMax, idx rowStart=0, idx colStart=0, idx rowCnt=sIdxMax, idx colCnt=sIdxMax, const char * endl="\r\n");


        inline idx cols() const { if(!fil.dim())return 0; Hdr * h=(Hdr *)fil.ptr();return h->cols;}
        inline idx rows() const { if(!fil.dim())return 0; Hdr * h=(Hdr *)fil.ptr();return h->rows;}
        inline idx dim() const { if(!fil.dim())return 0; Hdr * h=(Hdr *)fil.ptr();return h->rows*h->cols;}
        inline idx getShiftCols() const { const Hdr * h=(const Hdr *) fil.ptr(); return h->shiftCols;}
        inline idx getShiftRows() const { const Hdr * h=(const Hdr *) fil.ptr(); return h->shiftRows;}
        inline void setShiftCols(idx shift) { Hdr * h=(Hdr *) fil.ptr(); h->shiftCols = shift;}
        inline void setShiftRows(idx shift) { Hdr * h=(Hdr *) fil.ptr(); h->shiftRows = shift;}
        const idx * element(idx irow=0, idx icol=0) {
            if(!fil.dim())return 0;
            Hdr * h=(Hdr *) fil.ptr();
            idx * d=(idx *)(h+1);
            return d+h->cols+((h->cols*(irow+h->shiftRows))+icol+(h->shiftCols))*2;
        }

        const char * cell(idx irow, idx icol, idx * plen=0)
        {
            const idx * pel=element(irow,icol);
            if(plen)*plen=pel[1];
            return data+pel[0];
        }
        idx * types(void)  {
            if(!fil.dim())return 0;
            Hdr * h=(Hdr *) fil.ptr();
            return (idx *)(h+1);
        }

        const char * get(char * buf, idx irow, idx icol, idx szbuf=0)
        {
            const idx * pel=element(irow,icol);
            if(!szbuf)szbuf=pel[1];
            else if(szbuf>pel[1])szbuf=pel[1];
            if(szbuf)
                strncpy(buf,data+pel[0],szbuf);
            buf[szbuf]=0;
            return buf;
        }

        const char * get(sStr * out, idx irow, idx icol, idx szbuf=0)
        {
            const idx * pel=element(irow,icol);
            if(!szbuf)szbuf=pel[1];
            else if(szbuf>pel[1])szbuf=pel[1];
            idx l=out->length();
            if(szbuf)out->add(data+pel[0],szbuf);
            out->add0();
            return out->ptr(l);
        }

        idx ivalue(idx irow, idx icol, idx defValue=0 ) {
            sStr oneCell;
            get(&oneCell,irow,icol);
            idx val=defValue;
            sscanf(oneCell.ptr(),"%" DEC,&val);
            return val;
        }
        real rvalue(idx irow, idx icol, real defValue=0 ) {
            sStr oneCell;
            get(&oneCell,irow,icol);
            real val=defValue;
            sscanf(oneCell.ptr(),"%lf",&val);
            return val;
        }

    };

    class sTabular
    {
    protected:
        struct ValReinterp {
            real r;
            bool numeric;
            ValReinterp(): r(0), numeric(false) {}
        };
        sVec<sVec<ValReinterp> > _colReinterp;
        sDic<sVec<idx> > _colIds;
        sDic<sStr> _table_metadata;
        mutable sVariant _conversion_val, _conversion_val2;

    public:
        enum eReinterpretType {
            eNone,
            eUnique,
            eBool
        };
        enum eReinterpretFlags {
            fCaseSensitive=1<<0,
            fMissingAsNaN=1<<1,
            fNumbersAsStrings=1<<2
        };
        enum ePrintFlags {
            fNoUnescape=1<<0,
            fForCSV=1<<1
        };

        enum ECellEncoding {
            eEncodingUnspecified,
            eEncodingNone,
            eEncodingCSV
        };

        struct RCell
        {
            const sTabular * tbl;
            idx irow;
            idx icol;

            RCell(const sTabular * t=0) { sSet(this, 0); tbl = t; }
            RCell(const sTabular * t, idx ir, idx ic): tbl(t), irow(ir), icol(ic) {}
            RCell(const RCell & rhs) { memcpy(this, &rhs, sizeof(RCell)); }
            inline RCell& operator=(const RCell & rhs)
            {
                memcpy(this, &rhs, sizeof(RCell));
                return *this;
            }
            inline real operator-(const RCell & rhs) const { return tbl->rdiff(irow, icol, rhs.irow, rhs.icol); }
            inline operator real() const { return tbl->rval(irow, icol); }
        };

        class RIter : public sIter<RCell, RIter>
        {
        private:
            const sTabular * _tbl;
            idx _i;
            idx _ifixed;
            const sVec<idx> * _ivals;
            bool _vertical;

            void init(const sTabular * tbl)
            {
                _tbl = tbl;
                _i = _ifixed = 0;
                _ivals = 0;
                _vertical = false;
            }
            void init(const RIter & rhs)
            {
                _tbl = rhs._tbl;
                _i = rhs._i;
                _ifixed = rhs._ifixed;
                _ivals = rhs._ivals;
                _vertical = rhs._vertical;
            }
            inline bool ivalinrange() const
            {
                if (_ivals && _ivals->dim()) {
                    return _i < _ivals->dim();
                }
                return true;
            }
            inline idx inonfixed() const
            {
                if (_ivals && _ivals->dim()) {
                    return _i < _ivals->dim() ? (*_ivals)[_i] : (*_ivals)[_ivals->dim() - 1] + 1;
                }
                return _i;
            }

        public:
            RIter() { init(0); }
            RIter(const sTabular * tbl, bool vertical, idx ifixed, const sVec<idx> * ivals=0)
            {
                init(tbl);
                _ifixed = ifixed;
                _ivals = ivals;
                _vertical = vertical;
            }
            RIter(const RIter & rhs) { init(rhs); }
            RIter * clone_impl() const { return new RIter(*this); }
            inline void requestData_impl() {}
            inline void releaseData_impl() {}
            inline bool readyData_impl() const { return true; }
            inline idx irow() const { return _vertical ? inonfixed() : _ifixed; }
            inline idx icol() const { return _vertical ? _ifixed : inonfixed(); }
            bool validData_impl() const { return _tbl && ivalinrange() && irow() < _tbl->rows() && icol() < _tbl->cols(); }
            inline idx pos_impl() const { return _i; }
            inline idx segment_impl() const { return 0; }
            inline idx segmentPos_impl() const { return pos_impl(); }
            RIter & increment_impl() { _i++; return *this; }
            RCell dereference_impl() const { return RCell(_tbl, irow(), icol()); }
            RIter & operator=(const RIter & rhs) { init(rhs); return *this; }
        };

        virtual ~sTabular() {}

        idx dim() const
        {
            return rows() * cols();
        }

        virtual idx rows() const = 0;
        virtual idx cols() const = 0;
        virtual sVariant::eType coltype(idx icol) const = 0;
        virtual bool setColtype(idx icol, sVariant::eType t) { return false; }
        virtual sVariant::eType type(idx irow, idx icol) const
        {
            return coltype(icol);
        }
        virtual idx colId(const char * colname, idx colname_len=0, idx ic=0) const
        {
            const sVec<idx> * ids = _colIds.get(colname, colname_len);
            if (likely(ids && ic >= 0 && ic < ids->dim())) {
                return *ids->ptr(ic);
            }
            return -sIdxMax;
        }
        virtual idx colIdDim(const char * colname, idx colname_len=0) const
        {
            const sVec<idx> * ids = _colIds.get(colname, colname_len);
            return likely(ids) ? ids->dim() : 0;
        }
        virtual const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=0, const char * defValue=0, idx flags=0) const = 0;
        virtual const char * cell(idx irow, idx icol, idx * cellLen) const = 0;
        virtual ECellEncoding cellEncoding(idx irow, idx icol) const { return eEncodingUnspecified; }
        virtual bool missing(idx irow, idx icol) const = 0;
        virtual bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const = 0;
        virtual idx ival(idx irow, idx icol, idx defValue=0, bool noReinterpret=false, bool blankIsNull=false) const
        {
            bool have_val = val(_conversion_val, irow, icol, noReinterpret, blankIsNull);
            return have_val && !_conversion_val.isNull() ? _conversion_val.asInt() : defValue;
        }
        virtual udx uval(idx irow, idx icol, udx defValue=0, bool noReinterpret=false, bool blankIsNull=false) const
        {
            bool have_val = val(_conversion_val, irow, icol, noReinterpret, blankIsNull);
            return have_val && !_conversion_val.isNull() ? _conversion_val.asUInt() : defValue;
        }
        virtual bool boolval(idx irow, idx icol, bool defValue=false, bool noReinterpret=false, bool blankIsNull=false) const
        {
            bool have_val = val(_conversion_val, irow, icol, noReinterpret, blankIsNull);
            return have_val && !_conversion_val.isNull() ? _conversion_val.asBool() : defValue;
        }
        virtual real rval(idx irow, idx icol, real defValue=0., bool noReinterpret=false, bool blankIsNull=false) const
        {
            bool have_val = val(_conversion_val, irow, icol, noReinterpret, blankIsNull);
            return have_val && !_conversion_val.isNull() ? _conversion_val.asReal() : defValue;
        }
        virtual real rdiff(idx irow1, idx icol1, idx irow2, idx icol2) const;
        virtual idx dimTopHeader() const = 0;
        virtual idx dimLeftHeader() const = 0;
        virtual const char * printTopHeader(sStr & out, idx icol, idx irow=0, idx maxLen=0, const char * defValue=0, idx flags=0) const
        {
            if (likely(dimTopHeader()))
                return printCell(out, irow - dimTopHeader(), icol, maxLen, defValue, flags);
            if (defValue) {
                if (flags & fForCSV)
                    return sString::escapeForCSV(out, defValue);
                else
                    return out.printf("%s", defValue);
            }
            return 0;
        }
        virtual const char * printLeftHeader(sStr & out, idx irow, idx icol=0, idx maxLen=0, const char * defValue=0, idx flags=0) const
        {
            if (likely(dimLeftHeader()))
                return printCell(out, irow, icol - dimLeftHeader(), maxLen, defValue, flags);

            if (defValue) {
                if (flags & fNoUnescape)
                    return out.printf("%s", defValue);
                else
                    return sString::unescapeFromCSV(out, defValue);
            }
            return 0;
        }
        virtual const char * printCSV(sStr & out, idx irowStart=0, idx icolStart=0, idx numRows=sIdxMax, idx numCols=sIdxMax, bool withHeaders=true) const;

        virtual bool reinterpretCol(idx icol, eReinterpretType type, idx flags=0, const char * ref=0, idx reflen=0);
        virtual const char * getTableMetadata(const char * name, idx irow=0, idx icol=0) const
        {
            const sStr * pval = _table_metadata.get(name);
            return pval && pval->length() ? pval->ptr(0) : 0;
        }
        virtual bool setTableMetadata(const char * name, const char * value, idx irow=0, idx icol=0)
        {
            sStr * pval = _table_metadata.setString(name);
            if (!(pval->flags & sMex::fExactSize)) {
                pval->destroy();
                pval->init(sMex::fExactSize);
            }
            pval->cutAddString(0, value);
            return true;
        }
        static sVariant::eType mergeTypes(sVariant::eType t1, sVariant::eType t2);
    };

    class sTblIndex {
    public:
        enum eFlags {
            fColsep00 = 1<<0,
            fColsepCanRepeat = 1<<1,
            fPuntQuotes = 1<<2,
            fSaveRowEnds = 1<<3,
            fTopHeader = 1<<4,
            fLeftHeader = 1<<5,
            fRaggedEdge = 1<<6
        };

    protected:
        struct Header {
            char version[17];
            int64_t sourceTime;
            int64_t sourceSize;
            int64_t indexRangeStart;
            int64_t indexRangeLast;
            int64_t totalCols;
            int64_t totalRows;
            uint8_t topHeader;
            uint8_t leftHeader;
            int64_t segments;
            int64_t flags;
            int64_t colsepPos;
            int64_t rowsepPos;
            int64_t quotesPos;
            int64_t colTypesPos;
            int64_t segInfoPos;
            int64_t segSize;

            Header() { init(0, 0); }

            void init(idx total_cols, idx flags, idx dimTopHeader=1, idx dimLeftHeader=1);
            idx read(sMex * mex, idx pos);
            idx write(sMex * mex, idx pos, bool finished) const;
            idx size() const __attribute__((const));
        } _hdr;

        struct SegInfo {
            enum eFlags {
                fUInt8 = 1<<0,
                fUInt16 = 1<<1,
                fRowMaxAbsCol = 1<<2,
                fCompressRowMaxAbsCol = 1<<3
            };

            int64_t segPos;
            int64_t index;
            int64_t firstAbsCol;
            int64_t firstAbsRow;
            int64_t lastAbsCol;
            int64_t lastAbsRow;
            int64_t maxAbsCol;
            int64_t flags;

            idx minRowMaxAbsCol;
            idx maxRowMaxAbsCol;
            idx maxColIndex;
            idx rowSize;
            idx colSize;
            idx rowMaxAbsColSize;

            void init() { sSet(this, 0); minRowMaxAbsCol = sIdxMax; }
            SegInfo() { init(); }
            SegInfo(const SegInfo & rhs) { memcpy(this, &rhs, sizeof(SegInfo)); }

            idx read(sMex * mex, idx pos);
            idx write(sMex * mex, idx pos) const;
            static idx size() __attribute__((const));

            void updateSegSize();
            idx segSize() const __attribute__((pure));
            bool isCompressed() const { return flags & (fUInt8|fUInt16|fCompressRowMaxAbsCol); }

            idx segRowPos(idx absrow) const { return segPos + (absrow - firstAbsRow) * rowSize; }

            idx getSegMaxAbsCol(const sMex * mex, idx absrow) const __attribute__((pure));
            idx getSegIndex(const sMex * mex, idx absrow, idx abscol, idx * nextIndexInRow=0) const __attribute__((pure));
            idx setSegIndexRow(sMex * mex, idx absrow, const idx * indices, idx indices_first_abscol, idx indices_last_abscol);

            void segExpand(sMex * mex, idx newMaxAbsCol, bool raggedize);
            void segCompress(sMex * mex);

            idx maxAllowedColIndex() const __attribute__((pure));
            idx maxAllowedRowMaxAbsCol() const __attribute__((pure));
        };

        sVec<idx> _colsepLen;
        sVec<sVariant::eType> _coltypes;
        sVec<SegInfo> _segs;
        sMex _mex;

        mutable idx _curSeg;
        idx _segMaxOffset;

        int cmpCellSeg(idx absrow, idx abscol, idx iseg) const __attribute__((pure));
        idx findSeg(idx absrow, idx abscol, idx iseg=0) const __attribute__((pure));
        idx getIndexAtSeg(idx absrow, idx abscol, idx iseg) const __attribute__((pure));
        inline SegInfo & lastSeg() { return *_segs.ptr(_hdr.segments-1); }
        inline const SegInfo & lastSeg() const { return *_segs.ptr(_hdr.segments-1); }
        void compressLastSeg();

    public:
        sTblIndex();
        sMex * mex() { return &_mex; }
        const sMex * mex() const { return &_mex; }
        bool readHeader();
        idx totalCols() const { return _hdr.totalCols; }
        idx totalRows() const { return _hdr.totalRows; }
        idx topHeader() const { return _hdr.flags & fTopHeader ? _hdr.topHeader : 0; }
        idx leftHeader() const { return _hdr.flags & fLeftHeader ? _hdr.leftHeader : 0; }
        sVariant::eType coltype(idx abscol) const { return likely(abscol < _coltypes.dim()) ? _coltypes[abscol] : sVariant::value_NULL; }
        bool setColtype(idx abscol, sVariant::eType t)
        {
            if (unlikely(abscol < 0))
                return false;
            *_coltypes(abscol) = t;
            return true;
        }
        idx index(idx absrow, idx abscol, idx * nextIndexInRow=0) const;
        idx totalColsInRow(idx absrow) const;
        const char * colsep() const { return _mex.pos() ? static_cast<const char*>(_mex.ptr(_hdr.colsepPos)) : ","; }
        bool colsep00() const { return _hdr.flags & fColsep00; }
        const idx * colsep00len() const { return _colsepLen.ptr(); }
        bool colsepCanRepeat() const { return _hdr.flags & fColsepCanRepeat; }
        const char * rowsep() const { return _mex.pos() ? static_cast<const char *>(_mex.ptr(_hdr.rowsepPos)) : "\r\n"; }
        const char * quotes() const { return _mex.pos() ? static_cast<const char *>(_mex.ptr(_hdr.quotesPos)) : "\""; }
        bool isStandardCSV() const;
        bool puntQuotes() const { return _hdr.flags & fPuntQuotes; }
        bool saveRowEnds() const { return _hdr.flags & fSaveRowEnds; }
        bool raggedEdge() const { return _hdr.flags & fRaggedEdge; }
        idx sourceTime() const { return _hdr.sourceTime; }
        idx sourceSize() const { return _hdr.sourceSize; }
        idx indexRangeStart() const { return _hdr.indexRangeStart; }
        idx indexRangeLast() const { return _hdr.indexRangeLast; }
        void setIndexRangeLast(idx m) { _hdr.indexRangeLast = m; }
        void initHeader(idx totalCols, idx flags=0, const char * colsep=",", const char * rowsep="\r\n", const char * quotes="\"", idx dimTopHeader=1, idx dimLeftHeader=1);
        bool initialized() const { return _segs.dim(); }
        void setTopHeader(idx rows)
        {
            if (rows > 0) {
                _hdr.topHeader = rows;
                _hdr.flags |= fTopHeader;
            } else {
                _hdr.flags &= ~(int64_t)fTopHeader;
            }
        }
        void setLeftHeader(idx cols)
        {
            if (cols > 0) {
                _hdr.leftHeader = cols;
                _hdr.flags |= fLeftHeader;
            } else {
                _hdr.flags &= ~(int64_t)fLeftHeader;
            }
        }
        void setSourceInfo(idx time, idx size)
        {
            _hdr.sourceTime = time;
            _hdr.sourceSize = size;
        }
        void addRow(const idx * indices, idx numIndices);
        void finish();

#ifdef _DEBUG
        void setSegMaxOffset(idx m) { _segMaxOffset = m; }
#endif
        idx getSegMaxOffset() const { return _segMaxOffset; }
    };

    class sTxtTbl : public sTabular {
    public:
        typedef idx (*progressCb)(void * param, idx items, idx progress, idx progressMax);
        struct ParseOptions {
            idx flags;
            const char * colsep;
            const char * rowsep;
            const char * quotes;
            const char * comment;
            idx absrowStart;
            idx abscolStart;
            idx absrowCnt;
            idx abscolCnt;
            idx dimTopHeader;
            idx dimLeftHeader;
            idx initialOffset;
            idx headerOffset;
            idx maxLen;
            progressCb progressCallback;
            void * progressParam;

            ParseOptions() { sTxtTbl::resetParseOptions(*this); }
        };

    protected:
        sTblIndex _index;
        sStr _indexPath, _indexWritablePath;
        sFil _tableFil;
        const char * _tableBuf;
        idx _tableBufLen;
        idx _tableBufTime;
        sVec<idx> _addedCellsIndices;
        idx _addedCellsDim;
        idx _addedRows;

        mutable sStr _conversion_buf;

        ParseOptions _parse_options;
        static const ParseOptions _default_parse_options;

        void setBufInternal(const char * buf, idx len, idx time);
        void updateColIds();
        bool ensureIndexWritable(bool truncate_index = false);
        const char * getRowBuf(idx irow, idx * len_out) const;
        const char * getRowsBuf(idx irow, idx nrows, idx * len_out, idx * first_len_out) const;

    public:
        sTxtTbl(const char * indexPath=0, bool indexReadOnly=false);
        virtual ~sTxtTbl();
        void setBuf(const char * buf, idx len=0, idx time=sIdxMax) { setBufInternal(buf, len ? len : sLen(buf), time); }
        void setBuf(const sStr * buf, idx time=sIdxMax) { setBufInternal(buf->ptr(), buf->length(), time); }
        void setFile(const char * path, idx mexflags=sMex::fReadonly);
        void borrowFile(sMex * mex, idx time=sIdxMax)
        {
            _tableFil.borrow(mex);
            setBufInternal(_tableFil.ptr(), _tableFil.length(), time < sIdxMax ? time : _tableFil.mtime());
        }
        const sFil * getFile(sMex::Pos * pos_out = 0) const
        {
            if( _tableBuf == _tableFil.ptr() ) {
                if( pos_out ) {
                    pos_out->pos = _index.indexRangeStart();
                    pos_out->size = _index.indexRangeLast() - _index.indexRangeStart();
                }
                return &_tableFil;
            } else {
                return 0;
            }
        }
        const sTblIndex & getIndex() const { return _index; }
        sTblIndex & getIndex() { return _index; }
        virtual bool isParsed() const __attribute__((pure));

        inline ParseOptions & parseOptions() { return _parse_options; }
        inline static const ParseOptions & defaultParseOptions() { return _default_parse_options; }
        inline const ParseOptions & parseOptions() const { return _parse_options; }
        static void resetParseOptions(ParseOptions & p);

        virtual const char * parse();

        static bool segmentizeBuffer(sVec<sMex::Pos> & segments, const char * buf, idx buflen, const ParseOptions * parseOptions=0, idx maxSegSize=0);

        virtual idx rows() const { return _index.totalRows() - _index.topHeader(); }
        virtual idx cols() const {
            idx ret = _index.totalCols() - _index.leftHeader();
            return _index.saveRowEnds() ? ret - 1 : ret;
        }
        virtual sVariant::eType coltype(idx icol) const { return _index.coltype(icol + dimLeftHeader()); }
        virtual bool setColtype(idx icol, sVariant::eType t) { return _index.setColtype(icol + dimLeftHeader(), t); }
        virtual const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=sIdxMax, const char * defValue=0, idx flags=0) const;
        virtual const char * printCSV(sStr & out, idx irowStart=0, idx icolStart=0, idx numRows=sIdxMax, idx numCols=sIdxMax, bool withHeaders=true) const;
        virtual const char * cell(idx irow, idx icol, idx * cellLen) const;
        virtual ECellEncoding cellEncoding(idx irow, idx icol) const { return _index.isStandardCSV() ? eEncodingCSV : eEncodingUnspecified; }
        virtual bool missing(idx irow, idx icol) const
        {
            idx cellLen;
            return !cell(irow, icol, &cellLen) || !cellLen;
        }
        virtual bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const;
        virtual idx dimTopHeader() const { return _index.topHeader(); }
        virtual idx dimLeftHeader() const { return _index.leftHeader(); }
        virtual bool isReadonly() const
        {
            if (_tableBuf != _tableFil.ptr())
                return true;
            return _tableFil.flags & sMex::fReadonly;
        }
        virtual void remapReadonly();

        virtual void setDimTopHeader(idx rows)
        {
            _index.setTopHeader(rows);
        }

        virtual void setDimLeftHeader(idx cols)
        {
            _index.setLeftHeader(cols);
        }

        virtual bool initWritable(idx cols, idx flags, const char * colsep=",", const char * rowsep="\r\n", const char * quotes="\"", idx dimTopHeader=1, idx dimLeftHeader=1);
        virtual bool addCell(const char * val=0, idx len=0, idx typeHint=-1);
        virtual bool addCell(const sVariant & val);
        virtual bool addCell(const sHiveId & val);
        virtual bool addICell(idx ival);
        virtual bool addUCell(udx uval);
        virtual bool addRCell(real rval);
        virtual bool addBoolCell(bool bval);
        virtual bool addEndRow();
        virtual bool finish();

#ifdef _DEBUG
        void setSegMaxOffset(idx m) { _index.setSegMaxOffset(m); }
#endif
    };

    class sVcfTbl : public sTxtTbl {
    public:
        sVcfTbl(const char * indexPath=0, bool indexReadOnly=false) : sTxtTbl(indexPath, indexReadOnly)
        {
            resetParseOptions(parseOptions());
        }
        virtual ~sVcfTbl() {}
        static void resetParseOptions(ParseOptions & p);
        virtual const char * parse();
    };

    class sGtfTbl : public sTxtTbl {
    protected:
        sStr _colname_buf;
        sVec<idx> _colname_offsets;
    public:
        sGtfTbl(const char * indexPath=0, bool indexReadOnly=false);
        virtual ~sGtfTbl() {}
        static void resetParseOptions(ParseOptions & p);
        virtual const char * parse();
        virtual idx dimTopHeader() const { return 1; }
        virtual const char * cell(idx irow, idx icol, idx * cellLen=0) const;
    };

    class sVariantTbl : public sTabular {
    protected:
        mutable sVec<sVariant> _cells;
        sVec<sVariant::eType> _coltypes;
        idx _dim_left_header;

        bool inrange(idx irow, idx icol) const { return irow >= -1 && icol >= -_dim_left_header && irow < rows() && icol < cols() && cellsIndex(irow, icol) < _cells.dim(); }
        idx cellsIndex(idx irow, idx icol) const { return (irow + 1) * (cols() + _dim_left_header) + icol + _dim_left_header; }

    public:
        sVariantTbl(idx numcols=0, idx dim_left_header=0): _coltypes(sMex::fExactSize|sMex::fSetZero)
        {
            _dim_left_header = dim_left_header;
            _cells.resize(numcols + dim_left_header);
            _coltypes.resize(numcols + dim_left_header);
        }
        idx rows() const
        {
            idx ret = _cells.dim() / (cols() + _dim_left_header) - 1;
            if (_cells.dim() % (cols() + _dim_left_header))
                ret++;
            return ret;
        }
        idx cols() const { return _coltypes.dim() - _dim_left_header; }
        virtual sVariant::eType coltype(idx icol) const { return likely(inrange(0, icol)) ? _coltypes[icol + _dim_left_header] : sVariant::value_NULL; }
        virtual bool setColtype(idx icol, sVariant::eType t)
        {
            if (!inrange(0, icol))
                return false;
            _coltypes[icol + _dim_left_header] = t;
            return true;
        }
        virtual sVariant::eType type(idx irow, idx icol) const { return likely(inrange(irow, icol)) ? _cells[cellsIndex(irow, icol)].getType() : sVariant::value_NULL; }
        virtual const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=0, const char *defValue=0, idx flags=0) const;
        virtual const char * cell(idx irow, idx icol, idx * cellLen) const;
        virtual ECellEncoding cellEncoding(idx irow, idx icol) const { return eEncodingNone; }
        virtual bool missing(idx irow, idx icol) const { return type(irow, icol) == sVariant::value_NULL; }
        virtual bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const;
        virtual idx dimTopHeader() const { return 1; }
        virtual idx dimLeftHeader() const { return _dim_left_header; }

        bool setVal(idx irow, idx icol, sVariant & val);
        bool setVal(idx irow, idx icol, const char * s)
        {
            sVariant val(s);
            return setVal(irow, icol, val);
        }
        bool setVal(idx irow, idx icol, idx i)
        {
            sVariant val(i);
            return setVal(irow, icol, val);
        }
        bool setVal(idx irow, idx icol, real r)
        {
            sVariant val(r);
            return setVal(irow, icol, val);
        }
        const char * printJSON(sStr & out, const sVariant::Whitespace * whitespace=0, idx indent_level=0) const;
    };

    class sVariantTblData : public sVariant::sVariantData {
    private:
        sVariantTbl _tbl;
        idx _refCount;
        static const char * _typeName;

    public:
        sVariantTblData(idx numcols=0, idx dim_left_header=0): _tbl(numcols, dim_left_header), _refCount(0) {}
        virtual ~sVariantTblData() {}

        sVariantData * clone()
        {
            incrementRefCount();
            return this;
        }
        void incrementRefCount() { _refCount++; }
        void decrementRefCount() { _refCount--; }
        bool hasRefs() const { return _refCount > 0; }

        bool asBool() const { return _tbl.dim(); }
        bool isNullish() const { return _tbl.dim() == 0; }
        idx asInt() const { return _tbl.dim(); }
        real asReal() const { return _tbl.dim(); }
        void getNumericValue(sVariant & numeric) const { numeric.setInt(_tbl.dim()); }
        void print(sStr & s, sVariant::ePrintMode mode, const sVariant::Whitespace * whitespace=0, idx indent_level=0) const
        {
            if (mode == sVariant::eJSON)
                _tbl.printJSON(s, whitespace, indent_level);
            else
                _tbl.printCSV(s);
        }
        const char * getTypeName() const { return _typeName; }

        bool operator==(const sVariantData & rhs) const { return this == &rhs; }
        bool operator<(const sVariantData & rhs) const { return this < &rhs; }
        bool operator>(const sVariantData & rhs) const { return this > &rhs; }

        sVariantTbl & getTable() { return _tbl; }
        const sVariantTbl & getTable() const { return _tbl; }
    };
}

#endif 