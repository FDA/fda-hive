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
            fil(filestore ? mode : ((mode&(~sMex::fReadonly) )|sMex::fSetZero),filestore){};//{fil.mex()->flags|=sMex::fSetZero|mode;}
        char * parse(char * src, idx len, idx flags=fPreserveQuotes,  const char * separ=",;\t", idx maxCount=sIdxMax, idx rowStart=0, idx colStart=0, idx rowCnt=sIdxMax, idx colCnt=sIdxMax, const char * endl="\r\n");
        //void parse(const char * filename, idx flags, const char * separ=",");


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
            //out->add0();
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

    //! Abstract interface for reading tabular data from a buffer
    class sTabular
    {
    protected:
        struct ValReinterp {
            real r;
            bool numeric;
            ValReinterp(): r(0), numeric(false) {}
        };
        sVec<sVec<ValReinterp> > _colReinterp; // abscol -> row -> reinterpreted value
        sDic<sVec<idx> > _colIds; // name -> list of column ids
        sDic<sStr> _table_metadata;
        mutable sVariant _conversion_val, _conversion_val2; // temporaries for converting values

    public:
        enum eReinterpretType {
            eNone, //!< Remove reinterpretation
            eUnique, //!< Map equal strings to unique integer values
            eBool //!< Map strings that equal the reference (or "false", "off", or "0" if no reference was given) to false, and others to true
        };
        enum eReinterpretFlags {
            fCaseSensitive=1<<0, //!< Perform case-sensitive comparison when comparing a cell's string value to reference and already seen values
            fMissingAsNaN=1<<1, //!< If missing() == true, interpret cell as NAN
            fNumbersAsStrings=1<<2 //!< If cell's string looks like a number, interpret as a string, not as that number
        };
        enum ePrintFlags {
            fNoUnescape=1<<0, //!< Don't unescape CSV-quoted data when retrieving data
            fForCSV=1<<1      //!< Escape string for use as a CSV cell when printing
        };

        enum ECellEncoding {
            eEncodingUnspecified, //!< Encoding not specified
            eEncodingNone, //!< No encoding - return value of cell() can be used directly
            eEncodingCSV //!< CSV encoding - return value of cell() needs to be CSV-decoded
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

        //! number of data cells in the table (not counting headers)
        idx dim() const
        {
            return rows() * cols();
        }

        //! number of data rows (not counting header)
        virtual idx rows() const = 0;
        //! number of data columns (not counting left header)
        virtual idx cols() const = 0;
        //! type of cells in specified data column
        /*! \returns an sVariant::eType value */
        virtual sVariant::eType coltype(idx icol) const = 0;
        //! set type of data cells in specified column
        /*! \param t an sVariant::eType value
            \returns true if setting the column type succeded */
        virtual bool setColtype(idx icol, sVariant::eType t) { return false; }
        //! type of given data cell
        /*! \returns an sVariant::eType value */
        virtual sVariant::eType type(idx irow, idx icol) const
        {
            return coltype(icol);
        }
        //! find column with given title
        /*! \param colname title of column to find
            \param colname_len length of \a colname; if 0, \a colname is assumed to be 0-terminated
            \param ic if there are multiple columns with this title, which one to use
            \returns index of the ic-th column with specified title, or -sIdxMax on error */
        virtual idx colId(const char * colname, idx colname_len=0, idx ic=0) const
        {
            const sVec<idx> * ids = _colIds.get(colname, colname_len);
            if (likely(ids && ic >= 0 && ic < ids->dim())) {
                return *ids->ptr(ic);
            }
            return -sIdxMax;
        }
        //! find number of columns with given title
        /*! \param colname title of column to find
            \param colname_len length of \a colname; if 0, \a colname is assumed to be 0-terminated */
        virtual idx colIdDim(const char * colname, idx colname_len=0) const
        {
            const sVec<idx> * ids = _colIds.get(colname, colname_len);
            return likely(ids) ? ids->dim() : 0;
        }
        //! unescape and decode cell's string content
        /*! \param[out] out buffer to store result
            \param irow row index (0-based; negative for header)
            \param icol column index (0-based; negative for left header)
            \param maxLen max length of data to return; 0 means no limit
            \param defValue fallback value if specified cell does not exist; not used if 0
            \param flags bitwise-or of sTabular::ePrintFlags values
            \returns pointer to start of printed string in \a out if the cell was found (or
                     if \a defValue was given), or 0 otherwise */
        virtual const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=0, const char * defValue=0, idx flags=0) const = 0;
        //! access to cell's content in the underlying buffer without unescaping
        /*! \param irow row index (0-based; negative for header)
            \param icol column index (0-based; negative for left header)
            \param[out] cellLen number of bytes in underlying buffer for given cell; not used if 0
            \returns pointer to beginning of cell content in table's underlying buffer, or 0 if the cell does not exist
            \warning result is usually not 0-terminated, so don't forget cellLen. */
        virtual const char * cell(idx irow, idx icol, idx * cellLen) const = 0;
        //! get encoding of cell's content in the underlying buffer
        virtual ECellEncoding cellEncoding(idx irow, idx icol) const { return eEncodingUnspecified; }
        //! check whether the cell is considered "missing" in the table's data model
        virtual bool missing(idx irow, idx icol) const = 0;
        //! cell's content as a variant value
        /*! \param[out] out cell's variant value
            \param irow row index (0-based; negative for header)
            \param icol column index (0-based; negative for header)
            \param noReinterpret if true, obtain the cell's original value instead of the reinterpretation resulting from reinterpretCol()
            \returns whether icol/irow are valid for the table's geometry
            \note If a cell's value is missing() but irow/icol are valid, val() will return true */
        virtual bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const = 0;
        //! cell's content as a signed integer
        virtual idx ival(idx irow, idx icol, idx defValue=0, bool noReinterpret=false, bool blankIsNull=false) const
        {
            bool have_val = val(_conversion_val, irow, icol, noReinterpret, blankIsNull);
            return have_val && !_conversion_val.isNull() ? _conversion_val.asInt() : defValue;
        }
        //! cell's content as an unsigned integer
        virtual udx uval(idx irow, idx icol, udx defValue=0, bool noReinterpret=false, bool blankIsNull=false) const
        {
            bool have_val = val(_conversion_val, irow, icol, noReinterpret, blankIsNull);
            return have_val && !_conversion_val.isNull() ? _conversion_val.asUInt() : defValue;
        }
        //! cell's content as a boolean
        virtual bool boolval(idx irow, idx icol, bool defValue=false, bool noReinterpret=false, bool blankIsNull=false) const
        {
            bool have_val = val(_conversion_val, irow, icol, noReinterpret, blankIsNull);
            return have_val && !_conversion_val.isNull() ? _conversion_val.asBool() : defValue;
        }
        //! cell's content as a real
        virtual real rval(idx irow, idx icol, real defValue=0., bool noReinterpret=false, bool blankIsNull=false) const
        {
            bool have_val = val(_conversion_val, irow, icol, noReinterpret, blankIsNull);
            return have_val && !_conversion_val.isNull() ? _conversion_val.asReal() : defValue;
        }
        //! numeric difference between two cells
        virtual real rdiff(idx irow1, idx icol1, idx irow2, idx icol2) const;
        //! number of rows in top header (i.e. in column titles)
        virtual idx dimTopHeader() const = 0;
        //! number of columns in left header (i.e. in row titles)
        virtual idx dimLeftHeader() const = 0;
        //! unescape and decode a cell in the top header (i.e. a column title)
        /*! \param[out] out buffer to store result
            \param icol column index (0-based; negative for left header)
            \param irow row index within header (0-based, so 0 means first row of header)
            \param maxLen max length of data to return; 0 means no limit
            \param defValue fallback value if specified cell does not exist; not used if 0
            \param flags bitwise-or of sTabular::ePrintFlags values
            \returns pointer to start of printed string in \a out if the cell was found (or
                     if \a defValue was given), or 0 otherwise */
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
        //! unescape and decode a cell in the left header (i.e. a row title)
        /*! \param[out] out buffer to store result
            \param irow row index (0-based; negative for top header)
            \param icol column index within header (0-based, so 0 means first column of left header)
            \param maxLen max length of data to return; 0 means no limit
            \param defValue fallback value if specified cell does not exist; not used if 0
            \param flags bitwise-or of sTabular::ePrintFlags values
            \returns pointer to start of printed string in \a out if the cell was found (or
                     if \a defValue was given), or 0 otherwise */
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
        //! unescape and decode a rectangular region of a table and print as CSV
        /*! \param[out] out buffer to store result
            \param irowStart starting row index (0-based)
            \param icolStart starting column index (0-based)
            \param numRows maximum number of rows to output
            \param numCols maximum number of columns to output
            \param withHeaders include top and left headers for the specified region */
        virtual const char * printCSV(sStr & out, idx irowStart=0, idx icolStart=0, idx numRows=sIdxMax, idx numCols=sIdxMax, bool withHeaders=true) const;

        //! reinterpret a column's string values as numeric values
        /*! \param icol column index within header (0-based)
            \param type choice of reinterpretation algorithm - see sTabular::eReinterpretType.
                        Use sTabular::eNone to cancel reinterpretation.
            \param flags bitwise-or of sTabular::eReinterpretFlags
            \param ref reference value (strings equal to it will be interpreted as 0 / false)
            \param reflen length of ref; if 0, ref is treated as a 0-terminated string
            \returns true if reinterpretation succeeded
            \note Reinterpretation affects val() / rval() / ival() etc., but not cell() / printCell() / printCSV()
            \note Rerun reintepretCol() after modifying, adding, or removing cells */
        virtual bool reinterpretCol(idx icol, eReinterpretType type, idx flags=0, const char * ref=0, idx reflen=0);
        //! return metadata value associated with a table or one of the table's regions
        /*! \param name name of metadata variable ("name", "obj", etc.)
            \param icol column index (if different regions of the table have different names)
            \param irow row index (if different regions of the table have different names) */
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
        //! utility function for finding the "least common denominator" of two sVariant::eType types
        static sVariant::eType mergeTypes(sVariant::eType t1, sVariant::eType t2);
    };

    //! Cell position index for a text table (e.g. CSV or tab-delimeted)
    class sTblIndex {
    public:
        enum eFlags {
            fColsep00 = 1<<0, //!< column separators as a 00-terminated list of 0-delimeted strings (otherwise, treated as a 0-terminated list of characters)
            fColsepCanRepeat = 1<<1, //!< a sequence of column separator characters/strings counts as one logical separator; so a,,b means [a,b] instead of [a, "", b]
            fPuntQuotes = 1<<2, //!< discard outermost quote characters when storing index. \warning breaks standard CSV encoding!
            fSaveRowEnds = 1<<3, //!< save row end indices; results in larger index, but faster retrieval; required if there are gaps between rows
            fTopHeader = 1<<4, //!< table has a top header (i.e. column titles)
            fLeftHeader = 1<<5, //!< table has a left header (i.e. row titles)
            fRaggedEdge = 1<<6 //!< different rows may have different numbers of columns; results in a larger index
        };

    protected:
        struct Header {
            char version[17]; //!< 16 bytes from file + terminal zero
            int64_t sourceTime; //!< source file timestamp, used to detect if source file changed
            int64_t sourceSize; //!< source file size, used to detect if source file changed
            int64_t indexRangeStart; //!< hints for table embedded in middle of file
            int64_t indexRangeLast; //!< hints for table embedded in middle of file
            int64_t totalCols; //!< total number of columns, including header
            int64_t totalRows; //!< total mumber of rows, including header
            uint8_t topHeader; //!< number of rows in top header \note not used if !(flags & fTopHeader)
            uint8_t leftHeader; //!< number of columns in left header \note not used if !(flags &fLeftHeader)
            int64_t segments; //!< number of segments, each covering a 32-bit index subspace
            int64_t flags; //!< bitwise-or of sTblIndex::eFlags
            int64_t colsepPos; //!< offset of colsep character/string list within sTblIndex::mex()
            int64_t rowsepPos; //!< offset of rowsep character list within sTblIndex::mex()
            int64_t quotesPos; //!< offset of quote character list within sTblIndex::mex()
            int64_t colTypesPos; //!< offset of coltype list within sTblIndex::mex()
            int64_t segInfoPos; //!< offset of SegInfo list within sTblIndex::mex()
            int64_t segSize; //!< size of each element of SegInfo list \note may be larger than sizeof(SegInfo) for compatibility

            Header() { init(0, 0); }

            void init(idx total_cols, idx flags, idx dimTopHeader=1, idx dimLeftHeader=1);
            idx read(sMex * mex, idx pos);
            idx write(sMex * mex, idx pos, bool finished) const;
            idx size() const __attribute__((const));
        } _hdr;

        struct SegInfo {
            enum eFlags {
                fUInt8 = 1<<0, //!< column indices are uint8_t (default is int32_t)
                fUInt16 = 1<<1, //!< column indices are uint16_t (default is int32_t)
                fRowMaxAbsCol = 1<<2, //!< in each row, store number of columns that are in use (for sTblIndex::fRaggedEdge)
                fCompressRowMaxAbsCol = 1<<3 //!< the per-row number of in-use columns is stored in same format as column indices themselves (default is to store as int64_t)
            };

            int64_t segPos; //!< offset of segment within sTblIndex::mex()
            int64_t index; //!< offset of segment's first cell within table source file
            int64_t firstAbsCol; //!< column number of segment's first cell
            int64_t firstAbsRow; //!< row mumber of segment's first cell
            int64_t lastAbsCol; //!< column number of segment's last cell
            int64_t lastAbsRow; //!< row number of segment's last cell
            int64_t maxAbsCol; //!< maximum column number of any cell within the segment
            int64_t flags; //!< bitwise-or of SegInfo::eFlags

            /* Not stored; statistics for compressing the segment */
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
            //! storage size of a SegInfo structure in sTblIndex::mex()
            static idx size() __attribute__((const));

            void updateSegSize();
            //! storage size of the segment described by this SegInfo in sTblIndex::mex()
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
        //! access or modify the index file's buffer
        sMex * mex() { return &_mex; }
        //! access the index file's buffer
        const sMex * mex() const { return &_mex; }
        //! parse the header from mex(); run this after opening or modifying mex()
        bool readHeader();
        //! total number of columns, incluging left header
        idx totalCols() const { return _hdr.totalCols; }
        //! total number of rows, including top header
        idx totalRows() const { return _hdr.totalRows; }
        //! number of rows in the top header (column titles)
        idx topHeader() const { return _hdr.flags & fTopHeader ? _hdr.topHeader : 0; }
        //! number of columns in the left header (row titles)
        idx leftHeader() const { return _hdr.flags & fLeftHeader ? _hdr.leftHeader : 0; }
        //! type of data cells in specified column
        /*! \param abscol column number (0-based, count includes left header)
            \returns an sVariant::eType value */
        sVariant::eType coltype(idx abscol) const { return likely(abscol < _coltypes.dim()) ? _coltypes[abscol] : sVariant::value_NULL; }
        //! set type of data cells in specified column
        /*! \param abscol column number (0-based, count includes left header)
            \param t an sVariant::eType value */
        bool setColtype(idx abscol, sVariant::eType t)
        {
            if (unlikely(abscol < 0))
                return false;
            *_coltypes(abscol) = t;
            return true;
        }
        //! offset of specified cell within source file
        idx index(idx absrow, idx abscol, idx * nextIndexInRow=0) const;
        //! total number of columns (including left header) in specified row
        idx totalColsInRow(idx absrow) const;
        //! list of column separators used within source file
        const char * colsep() const { return _mex.pos() ? static_cast<const char*>(_mex.ptr(_hdr.colsepPos)) : ","; }
        //! check whether colsep() is a list of strings or list of characters
        /*! \returns true if colsep() is a 00-terminated list of 0-delimeted strings; false if colsep() is a 0-terminated list of characters */
        bool colsep00() const { return _hdr.flags & fColsep00; }
        //! array of lengths of colsep() strings if colsep00() is true
        const idx * colsep00len() const { return _colsepLen.ptr(); }
        //! whether a sequence of column separator characters/strings counts as one logical separator (so a,,b would mean [a,b] instead of [a, "", b])
        bool colsepCanRepeat() const { return _hdr.flags & fColsepCanRepeat; }
        //! list of row separator characters
        const char * rowsep() const { return _mex.pos() ? static_cast<const char *>(_mex.ptr(_hdr.rowsepPos)) : "\r\n"; }
        //! list of quote characters
        const char * quotes() const { return _mex.pos() ? static_cast<const char *>(_mex.ptr(_hdr.quotesPos)) : "\""; }
        //! check if colsep and rowsep options are standard CSV
        bool isStandardCSV() const;
        //! whether the outermost quote characters are discarded when storing the index
        /* \warning breaks standard CSV encoding */
        bool puntQuotes() const { return _hdr.flags & fPuntQuotes; }
        //! whether the row end indices are saved; results in a larger index file, but faster retrieval
        bool saveRowEnds() const { return _hdr.flags & fSaveRowEnds; }
        //! whether different rows may have different numbers of columns; results in a larger index file
        bool raggedEdge() const { return _hdr.flags & fRaggedEdge; }
        //! source file timestamp, used to detect if source file changed
        idx sourceTime() const { return _hdr.sourceTime; }
        //! source file size, used to detect if source file changed
        idx sourceSize() const { return _hdr.sourceSize; }
        //! start of the indexed part of the source file
        idx indexRangeStart() const { return _hdr.indexRangeStart; }
        //! offset beyond end of the indexed part of the source file
        idx indexRangeLast() const { return _hdr.indexRangeLast; }
        //! set offset beyond end of the index part of the source file
        void setIndexRangeLast(idx m) { _hdr.indexRangeLast = m; }
        //! empty and reset the index
        /*! \param totalCols total number of columns to store; must be > 0; get this right from the start for maximum efficiency;
            \param flags bitwise-or of sTblIndex::eFlags
            \param colsep list of column separator characters or strings
            \param rowsep list of row separator characters
            \param quotes list of quote characters
            \param dimTopHeader number of rows in top header; not used if !(flags & fTopHeader)
            \param dimLeftHeader number of rows in left header; not used if !(flags & fLeftHeader) */
        void initHeader(idx totalCols, idx flags=0, const char * colsep=",", const char * rowsep="\r\n", const char * quotes="\"", idx dimTopHeader=1, idx dimLeftHeader=1);
        //! whether the index had been initialized
        bool initialized() const { return _segs.dim(); }
        //! set number of rows in top header (column titles)
        void setTopHeader(idx rows)
        {
            if (rows > 0) {
                _hdr.topHeader = rows;
                _hdr.flags |= fTopHeader;
            } else {
                _hdr.flags &= ~(int64_t)fTopHeader;
            }
        }
        //! set number of rows in left header (row titles)
        void setLeftHeader(idx cols)
        {
            if (cols > 0) {
                _hdr.leftHeader = cols;
                _hdr.flags |= fLeftHeader;
            } else {
                _hdr.flags &= ~(int64_t)fLeftHeader;
            }
        }
        //! set source file timestamp and size
        void setSourceInfo(idx time, idx size)
        {
            _hdr.sourceTime = time;
            _hdr.sourceSize = size;
        }
        //! append array of offsets of cells in a source file row
        /*! \param indices array of offsets to add; must be non-0
            \param numIndices number of elements in \a indices; must be >= 1 */
        void addRow(const idx * indices, idx numIndices);
        //! update and compress mex()
        void finish();

#ifdef _DEBUG
        void setSegMaxOffset(idx m) { _segMaxOffset = m; }
#endif
        idx getSegMaxOffset() const { return _segMaxOffset; }
    };

    //! Indexed text table, for example CSV or tab-delimeted
    /*! Basic usage example:
    \code
    sTxtTbl tbl("mytable.idx"); // mytable.idx is the index file
    tbl.setFile("mytable.csv"); // mytable.csv is the data to be indexed
    if (!tbl.isParsed()) { // if the index does not exist or is outdated
        tbl.parseOptions().flags = sTblIndex::fTopHeader | sTblIndex::fSaveRowEnds;
        tbl.parse();
    }
    if (!tbl.rows()) {
        fprintf(stderr, "No data found!\n");
    } else {
        tbl.reinterpretCol(0, sTabular::eUnique); // reinterpret strings in column 0 as unique integers
        sStr buf;
        tbl.printCell(buf, 0, 0);
        fprintf(stderr, "Cell 0,0 is %s as string, or %0.2f as numeric\n", buf.ptr(), tbl.rval(0, 0));
    }
    \endcode */
    class sTxtTbl : public sTabular {
    public:
        //*! Same interface as sQPrideProc::reqProgress()
        typedef idx (*progressCb)(void * param, idx items, idx progress, idx progressMax);
        struct ParseOptions {
            idx flags;           //!< bitwise-or of sTblIndex::eFlags; sTblIndex::fTopHeader|sTblIndex::fSaveRowEnds by default
            const char * colsep; //!< 0-terminated list of possible column separator characters (or 00-terminated, 0-delimeted list if flags & sTblIndex::fColsep00 is true); "," by default
            const char * rowsep; //!< 0-terminated list of possible row separator characters; "\r\n" by default
            const char * quotes; //!< 0-terminated list of quote characters; "\"" by default
            const char * comment;//!< strings with this prefix at the beginning of the buffer will be skipped; 0 by default
            idx absrowStart;     //!< start indexing table at this row; 0 by default
            idx abscolStart;     //!< start indexing table at this column; 0 by default
            idx absrowCnt;       //!< index at most this number of rows; sIdxMax by default
            idx abscolCnt;       //!< index at most this number of columns; sIdxMax by default
            idx dimTopHeader;    //!< if flags & sTblIndex::fTopHeader is true, number of rows to interpret as the top header; 1 by default
            idx dimLeftHeader;   //!< if flags & sTblIndex::fLeftHeader is true, number of columns to interpret as the left header; 1 by default
            idx initialOffset;   //!< number of bytes to completely skip at the beginning of the buffer, before starting any parsing or row/column counting; 0 by default
            idx headerOffset;    //!< if >= 0, offset of header line(s) to use if different from initialOffset; -1 by default
            idx maxLen;          //!< maximum number of bytes to examine (following initialOffset) during parsing
            progressCb progressCallback; //!< progress-reporting callback if non-0; same interface as sQPrideProc::reqProgress
            void * progressParam; //!< parameter for progressCb

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
        bool ensureIndexWritable();
        const char * getRowBuf(idx irow, idx * len_out) const;
        const char * getRowsBuf(idx irow, idx nrows, idx * len_out, idx * first_len_out) const;

    public:
        /*! \param indexPath path where the table's index file is stored; will be read as if it exists, and created if it doesn't
         *  \param indexReadOnly if indexPath is an invalid file, do not immediately delete it and start writing a new index */
        sTxtTbl(const char * indexPath=0, bool indexReadOnly=false);
        virtual ~sTxtTbl();
        //! use specified buffer as the table's data
        /*! \param buf start of data buffer
            \param len length of data buffer; if 0, buf is treated as 0-terminated string
            \param time data timestamp, used to check if data was already parsed by the index */
        void setBuf(const char * buf, idx len=0, idx time=sIdxMax) { setBufInternal(buf, len ? len : sLen(buf), time); }
        //! use specified buffer as the table's data
        /*! \param buf data buffer
            \param time data timestamp, used to check if data was already parsed by the index
            \warning if \a buf->ptr() changes, for example because \a buf
                grows/shrinks and its underlying buffer is reallocated,
                the table will become invalid! */
        void setBuf(const sStr * buf, idx time=sIdxMax) { setBufInternal(buf->ptr(), buf->length(), time); }
        //! use specified file as the table's data
        /*! \param path data file path
            \param mexflags bitwise-or of sMex::bitFlags for opening the file */
        void setFile(const char * path, idx mexflags=sMex::fReadonly);
        //! use specified buffer as the table's data
        /*! \param mex existing data buffer
            \param time data timestamp; if equal to sIdxMax, \a mex->mtime()                will be used as the timestamp instead
            \warning After this call, the underlying buffer/file of \a mex
                will be owned by the table, and \a mex will no longer be
                able to access it */
        void borrowFile(sMex * mex, idx time=sIdxMax)
        {
            _tableFil.borrow(mex);
            setBufInternal(_tableFil.ptr(), _tableFil.length(), time < sIdxMax ? time : _tableFil.mtime());
        }
        //! get the table's data file if it was set using setFile() or borrowFile()
        /*! \param pos_out retrieves the indexed part of the data file
            \returns data file, or 0 on failure */
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
        //! check if the data is parsed (meaning that the data's timestamp and size matches what is recorded in the index file)
        virtual bool isParsed() const __attribute__((pure));

        inline ParseOptions & parseOptions() { return _parse_options; }
        inline static const ParseOptions & defaultParseOptions() { return _default_parse_options; }
        inline const ParseOptions & parseOptions() const { return _parse_options; }
        static void resetParseOptions(ParseOptions & p);

        //! parse the data buffer and record cell offsets in the index
        /*! \note customize fields of parseOptions() to control parser behavior
            \returns pointer past the end of the interpreted part of the buffer, or 0 on parse failure */
        virtual const char * parse();

        //! try to split a CSV/TSV buffer into segments such that segment boundaries fall on row ends; O(len/maxSegSize).
        /*! \param[out] segments segments of up to maxSegSize bytes
            \note Due to the time complexity of distinguishing the inside and outside of a quoted
                  csv cell, the result may not be correct - check for sanity by parsing. */
        static bool segmentizeBuffer(sVec<sMex::Pos> & segments, const char * buf, idx buflen, const ParseOptions * parseOptions=0, idx maxSegSize=0);

        /* sTabular interface */
        //! number of data rows (not counting header)
        virtual idx rows() const { return _index.totalRows() - _index.topHeader(); }
        //! number of data columns (not counting left header)
        virtual idx cols() const {
            idx ret = _index.totalCols() - _index.leftHeader();
            return _index.saveRowEnds() ? ret - 1 : ret;
        }
        virtual sVariant::eType coltype(idx icol) const { return _index.coltype(icol + dimLeftHeader()); }
        virtual bool setColtype(idx icol, sVariant::eType t) { return _index.setColtype(icol + dimLeftHeader(), t); }
        virtual const char * printCell(sStr & out, idx irow, idx icol, idx maxLen=sIdxMax, const char * defValue=0, idx flags=0) const;
        virtual const char * printCSV(sStr & out, idx irowStart=0, idx icolStart=0, idx numRows=sIdxMax, idx numCols=sIdxMax, bool withHeaders=true) const;
        //! access to cell's content in the data buffer without unescaping
        /*! \param irow row index (0-based; negative for header)
            \param icol column index (0-based; negative for left header)
            \param[out] cellLen number of bytes in data buffer for given cell; not used if 0
            \returns pointer to beginning of cell content in data buffer, or 0 if the cell is not in the table
            \warning result is usually not 0-terminated, so don't forget cellLen. */
        virtual const char * cell(idx irow, idx icol, idx * cellLen) const;
        //! get encoding of cell's content in the underlying buffer
        virtual ECellEncoding cellEncoding(idx irow, idx icol) const { return _index.isStandardCSV() ? eEncodingCSV : eEncodingUnspecified; }
        //! check whether the cell is considered "missing" - i.e. is not in the table or has length 0
        virtual bool missing(idx irow, idx icol) const
        {
            idx cellLen;
            return !cell(irow, icol, &cellLen) || !cellLen;
        }
        //! cell's content as a variant value
        /*! \param[out] out cell's variant value
            \param irow row index (0-based; negative for header)
            \param icol column index (0-based; negative for header)
            \param noReinterpret if true, obtain the cell's original value instead of the reinterpretation resulting from reinterpretCol()
            \returns whether icol/irow are valid for the table's geometry
            \note If a cell's value is missing() but irow/icol are valid, val() will return true */
        virtual bool val(sVariant & out, idx irow, idx icol, bool noReinterpret=false, bool blankIsNull=false) const;
        virtual idx dimTopHeader() const { return _index.topHeader(); }
        virtual idx dimLeftHeader() const { return _index.leftHeader(); }
        //! whether the table's data buffer is read-only
        virtual bool isReadonly() const
        {
            if (_tableBuf != _tableFil.ptr())
                return true;
            return _tableFil.flags & sMex::fReadonly;
        }
        //! remap the table's index and data to be read-only
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
        //! Add a new cell to a writable table.
        /*! \param val String value to add. Must already be already escaped for
                        the table's data format (e.g. CSV).
            \param len Length of val in bytes; if 0, will be calculated from val
                        using strlen()
            \param typeHint Hint for the cell data type: sVariant::eType value if
                            known, or -1 to guess automatically */
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

    //! A table of variant values, for use by query language
    class sVariantTbl : public sTabular {
    protected:
        mutable sVec<sVariant> _cells; // mutable to allow asString()
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

#endif // sLib_utils_tbl_hpp
