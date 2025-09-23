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
#include <assert.h>
#include <ctype.h>
#include <math.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <slib/utils/tbl.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>

using namespace slib;

#define SYM(_v_i,_v_ch)  dst->add((_v_ch),1)
#define ZER(_v_i)  if(isIN)((char*)dst)[(_v_i)]=0; else dst->add(__,2)

char * sTbl::parse(char * src, idx len, idx flags,  const char * separ, idx maxCount , idx rowStart, idx colStart, idx rowCnt, idx colCnt , const char * endl)
{
    idx i;
    idx newcell=1,howmany=0;
    char  inquote=0;
    idx curRow=0, curCol=0, maxCntCols=0;

    if(!src || !src[0])return sString::nonconst(src);
    if(!len)len=sIdxMax;

    sVec < Element > geom;
    Element * pCurGeom=0;
    idx newCellPos=-1;

    for(i=0,curRow=0;i<len && howmany<maxCount ;i++){
        newCellPos++;
        if(flags&fPreserveQuotes){
            if(!inquote && (src[i]=='\"' || src[i]=='\'') ){
                if(newCellPos==0)
                    inquote=src[i];
            }
            else if(src[i]==inquote && (src[i-1])!='\\')inquote=0;
        }
        idx symsepar=0;
        if(!inquote) {
                idx sc;
                for(sc=0;src[i] && endl[sc] && src[i]!=endl[sc];sc++);
                if(endl[sc]){
                    idx endlSize = 1;
                    if(i<len-1 && endl[sc]=='\r' && src[i+1]=='\n') endlSize++;
                    ++curRow;
                    curCol=0;
                    if(pCurGeom){
                        if(flags&fDoZero) {
                            for (idx j=i; j<i+endlSize; j++)
                                src[j]=0;
                        }
                        pCurGeom->len=i-pCurGeom->start;
                        if (flags&fPreserveQuotes) {
                            char ch=src[pCurGeom->start];
                            if ( ( ch=='\'' || ch=='\"') && src[pCurGeom->start+pCurGeom->len-1]==ch) {
                                ++pCurGeom->start;
                                pCurGeom->len-=2;
                            }
                        }
                    }

                    newcell=1;
                    i += endlSize-1;
                    if(curRow+rowStart>=rowCnt)
                        break;
                    continue;
                }

            idx found=0,lenMatch=0;
            if( flags&fSeparatorIsListStrings){
                lenMatch=sString::compareChoice( src+i, separ,0,0, 0, false);
                if( lenMatch!=-1){
                    found=1;
                }else lenMatch=0;
            } else {
                for(sc=0;src[i] && separ[sc] && src[i]!=separ[sc];sc++);
                found=separ[sc];
                if(found)
                    symsepar=1;
            }
            if(found){
                    if(flags&fDoZero)
                            src[i]=0;
                    if(pCurGeom){
                        if(pCurGeom->len==0){
                            pCurGeom->len=i-pCurGeom->start;
                            if (flags&fPreserveQuotes) {
                                char ch=src[pCurGeom->start];
                                if ( ( ch=='\'' || ch=='\"') && src[pCurGeom->start+pCurGeom->len-1]==ch) {
                                    ++pCurGeom->start;
                                    pCurGeom->len-=2;
                                }
                            }
                        }
                    }
                howmany++;
                newcell=1;
                newCellPos=-1;
                ++curCol;
                if(curCol>maxCntCols)maxCntCols=curCol;

                if(!(flags&fAllowEmptyCells)) {
                    if(lenMatch)i+=lenMatch;
                    continue;
                }
            }
            if(lenMatch)i+=lenMatch;
        }

        if( curCol>=colStart && curCol<colStart+colCnt && curRow>=rowStart && curRow<rowStart+rowCnt) {
                if(newcell){
                    pCurGeom=geom.add();

                    pCurGeom->start=i+symsepar;
                    pCurGeom->icol=curCol;
                    pCurGeom->irow=curRow;
                    pCurGeom->len=0;
                    newcell=0;
                    if(flags&fAllowEmptyCells) {
                        if(flags&fSeparatorIsListStrings) {
                            if(sString::compareChoice( src+i, separ,0,0, 0, false)!=-1)
                                i--;
                        }
                    }
                }
        }

    }

    if(pCurGeom )
        pCurGeom->len=i-pCurGeom->start;
    ++curRow;
    ++maxCntCols;

    maxCntCols-=colStart;
    curRow-=rowStart;

    Hdr * hdr=(Hdr *) fil.add((sizeof(Hdr)/sizeof(idx))+maxCntCols*curRow*2+maxCntCols);
    idx * tv=(idx *)(hdr+1);
    idx * d=(tv+maxCntCols);
    hdr->cols=maxCntCols;
    hdr->rows=curRow-1;
    strncpy(hdr->separators,separ,sizeof(hdr->separators)-1);

    Element * pg=geom.ptr();

    for ( idx i=0; i<geom.dim() ; ++i , ++pg){
            idx index=pg->irow*hdr->cols+pg->icol;
            d[2*index+0]=pg->start;
            d[2*index+1]=pg->len;

            if( (flags&fHdrCols) && pg->irow==0) continue;
            if( (flags&fHdrRows) && pg->icol==0) continue;

            const char * c=src+pg->start;
            for ( idx is=0; is< pg->len; ++is ){
                if(isdigit(c[is]) )
                    tv[pg->icol]|=0x2;
                else if(tv[pg->icol]==0x02 && ( c[is]=='e' ||  c[is]=='e' ||  c[is]=='.' ||  c[is]=='+' ||  c[is]=='-' ) )
                    tv[pg->icol]|=0x2;
                else
                    tv[pg->icol]|=0x1;
            }
    }

    for ( idx ic=0; ic<hdr->cols; ++ic){
            if(tv[ic]==0x2)tv[ic]=1;
            else tv[ic]=0;
    }

    data=src;


    return sString::nonconst(src+i);
}

const char * sTabular::printCSV(sStr & out, idx irowStart, idx icolStart, idx numRows, idx numCols, bool withHeaders) const
{
    idx startOut = out.length();

    numRows = sMin<idx>(numRows, rows() - irowStart);
    numCols = sMin<idx>(numCols, cols() - icolStart);

    if (withHeaders) {
        for (idx ir=0; ir<dimTopHeader(); ir++) {
            bool needComma = false;
            for (idx ic=0; ic<dimLeftHeader(); ic++) {
                if (needComma)
                    out.addString(",", 1);
                printTopHeader(out, ic-dimLeftHeader(), ir, sIdxMax, 0, fForCSV);
                needComma = true;
            }
            for (idx ic=icolStart; ic-icolStart < numCols; ic++) {
                if (needComma)
                    out.addString(",", 1);
                printTopHeader(out, ic, ir, sIdxMax, 0, fForCSV);
                needComma = true;
            }
            out.addString("\r\n", 2);
        }
    }

    for (idx ir=irowStart; ir-irowStart < numRows; ir++) {
        bool needComma = false;
        if (withHeaders) {
            for (idx ic=0; ic<dimLeftHeader(); ic++) {
                if (needComma)
                    out.addString(",", 1);
                printLeftHeader(out, ir, ic, sIdxMax, 0, fForCSV);
                needComma = true;
            }
        }
        for (idx ic=icolStart; ic-icolStart < numCols; ic++) {
            if (needComma)
                out.addString(",", 1);
            printCell(out, ir, ic, sIdxMax, 0, fForCSV);
            needComma = true;
        }
        out.addString("\r\n", 2);
    }

    return out.ptr(startOut);
}

real sTabular::rdiff(idx irow1, idx icol1, idx irow2, idx icol2) const
{
    idx abscol1 = icol1 + dimLeftHeader();
    if (icol1 == icol2 && abscol1 >= 0 && abscol1 < _colReinterp.dim() && _colReinterp[abscol1].dim()) {
        if (irow1 >= 0 && irow1 < _colReinterp[abscol1].dim() && irow2 >= 0 && irow2 < _colReinterp[abscol1].dim() && _colReinterp[abscol1][irow1].numeric && _colReinterp[abscol1][irow2].numeric) {
            real d = _colReinterp[abscol1][irow1].r - _colReinterp[abscol1][irow2].r;
            return isnan(d) ? 0 : d;
        }
        real r1 = rval(irow1, icol1);
        real r2 = rval(irow2, icol2);

        if (r1 == r2 || isnan(r1) || isnan(r2)) {
            return 0;
        } else if (r1 > r2) {
            return 1;
        } else {
            return -1;
        }
    }

    val(_conversion_val, irow1, icol1);
    val(_conversion_val2, irow2, icol2);

    if (_conversion_val.isNumeric() && _conversion_val2.isNumeric()) {
        real d = _conversion_val.asReal() - _conversion_val2.asReal();
        return isnan(d) ? 0 : d;
    }

    if (_conversion_val == _conversion_val2) {
        return 0;
    } else if (_conversion_val > _conversion_val2) {
        return 1;
    } else {
        return -1;
    }
}

real reinterpretMatch(const char * s, idx len, sTabular::eReinterpretType type, idx flags, sDic<real> & matches, sStr & buf, bool * numeric)
{
    if (numeric) {
        *numeric = false;
    }

    if (!(flags & sTabular::fCaseSensitive)) {
        buf.cut(0);
        if (len)
            sString::changeCase(&buf, s, len, sString::eCaseLo);
        s = buf.ptr();
    }
    if (!len) {
        s = "";
    }

    real ret, * pret = matches.get(s, len);
    if (pret) {
        return *pret;
    }

    if (!(flags & sTabular::fNumbersAsStrings) && numeric) {
        char * end = 0;
        ret = strtod(s, &end);
        for (; *end && isspace(*end); end++);
        if (!*end) {
            *numeric = true;
            return ret;
        }
    }

    switch (type) {
    case sTabular::eUnique:
        pret = matches.set(s, len);
        *pret = matches.dim() - 1;
        ret = *pret;
        break;
    case sTabular::eBool:
        if (matches.dim()) {
            ret = 1;
        } else {
            ret = sString::parseBool(s);
        }
        break;
    default:
        ret = NAN;
    }
    return ret;
}

bool sTabular::reinterpretCol(idx icol, sTabular::eReinterpretType type, idx flags, const char * ref, idx reflen)
{
    if (unlikely(icol < -dimLeftHeader() || icol >= cols()))
        return false;

    idx abscol = icol + dimLeftHeader();

    if (type == sTabular::eNone) {
        if (_colReinterp.dim() > abscol) {
            _colReinterp[abscol].cut(0);
        }
        return true;
    }

    _colReinterp.resize(abscol + 1);
    _colReinterp[abscol].resize(rows());

    if (ref && !reflen)
        reflen = sLen(ref);

    sDic<real> matchDic;
    sStr canonical_buf, cell_buf;
    if (ref) {
        reinterpretMatch(ref, reflen, type, flags, matchDic, canonical_buf, 0);
    }

    for (idx irow=0; irow<rows(); irow++) {
        if (missing(irow, icol) && flags & fMissingAsNaN) {
            _colReinterp[abscol][irow].r = NAN;
            _colReinterp[abscol][irow].numeric = false;
            continue;
        }

        cell_buf.cut(0);
        printCell(cell_buf, irow, icol);
        _colReinterp[abscol][irow].r = reinterpretMatch(cell_buf, cell_buf.length(), type, flags, matchDic, canonical_buf, &(_colReinterp[abscol][irow].numeric));
    }

    return true;
}

template<typename T>
inline idx read_num_incr_pos(const sMex * mex, idx * pos)
{
    idx ret = *static_cast<const T*>(mex->ptr(*pos));
    *pos += sizeof(T);
    return ret;
}

template idx read_num_incr_pos<uint8_t>(const sMex *, idx *);
template idx read_num_incr_pos<uint16_t>(const sMex *, idx *);
template idx read_num_incr_pos<int32_t>(const sMex *, idx *);
template idx read_num_incr_pos<int64_t>(const sMex *, idx *);

template<typename T>
inline idx read_num(const sMex * mex, idx pos)
{
    return *static_cast<const T*>(mex->ptr(pos));
}

template idx read_num<uint8_t>(const sMex *, idx);
template idx read_num<uint16_t>(const sMex *, idx);
template idx read_num<int32_t>(const sMex *, idx);
template idx read_num<int64_t>(const sMex *, idx);

idx read_num_incr_pos(const sMex * mex, idx * pos, idx size)
{
    switch (size) {
    case 1:
        return read_num_incr_pos<uint8_t>(mex, pos);
    case 2:
        return read_num_incr_pos<uint16_t>(mex, pos);
    case 4:
        return read_num_incr_pos<int32_t>(mex, pos);
    }
    return read_num_incr_pos<int64_t>(mex, pos);
}

idx read_num(const sMex * mex, idx pos, idx size)
{
    switch (size) {
    case 1:
        return read_num<uint8_t>(mex, pos);
    case 2:
        return read_num<uint16_t>(mex, pos);
    case 4:
        return read_num<int32_t>(mex, pos);
    }
    return read_num<int64_t>(mex, pos);
}

template<typename T>
inline idx write_num(sMex * mex, T src, idx pos)
{
    mex->resize(pos + sizeof(T));
    *static_cast<T*>(mex->ptr(pos)) = src;
    return sizeof(T);
}

template idx write_num<uint8_t>(sMex *, unsigned char, idx);
template idx write_num<uint16_t>(sMex *, uint16_t, idx);
template idx write_num<int32_t>(sMex *, int32_t, idx);
template idx write_num<int64_t>(sMex *, int64_t, idx);

idx write_num(sMex * mex, idx src, idx pos, idx size)
{
    switch (size) {
    case 1:
        return write_num<uint8_t>(mex, static_cast<uint8_t>(src), pos);
    case 2:
        return write_num<uint16_t>(mex, static_cast<uint16_t>(src), pos);
    case 4:
        return write_num<int32_t>(mex, static_cast<int32_t>(src), pos);
    }
    return write_num<int64_t>(mex, src, pos);
}

idx write_charbuf(sMex * mex, const char * src, idx pos, idx len)
{
    mex->resize(pos + len);
    memcpy(mex->ptr(pos), src, len);
    return len;
}

idx align_and_clear(sMex * mex, idx pos, idx align)
{
    idx len = sAlign(pos, align) - pos;
    mex->resize(pos + len);
    if (len) {
        memset(mex->ptr(pos), 0, len);
    }
    return len;
}

void sTblIndex::Header::init(idx total_cols, idx flags_, idx dimTopHeader, idx dimLeftHeader)
{
    sSet(this, 0);
    strncpy(version, "sTblIndex2.2", 16);
    topHeader = sMax<idx>(0, dimTopHeader);
    leftHeader = sMax<idx>(0, dimLeftHeader);
    totalCols = total_cols;
    flags = flags_;
    segSize = sTblIndex::SegInfo::size();
}

idx sTblIndex::Header::read(sMex * mex, idx pos)
{
    if (unlikely(pos + size() > mex->pos()))
        return -1;

    memcpy(version, mex->ptr(pos), 16);
    version[16] = 0;
    pos += 16;
    if (unlikely(strcmp(version, "sTblIndex2.2") != 0))
        return -1;

    sourceTime = read_num_incr_pos<int64_t>(mex, &pos);
    sourceSize = read_num_incr_pos<int64_t>(mex, &pos);
    indexRangeStart = read_num_incr_pos<int64_t>(mex, &pos);
    indexRangeLast = read_num_incr_pos<int64_t>(mex, &pos);
    totalCols = read_num_incr_pos<int64_t>(mex, &pos);
    totalRows = read_num_incr_pos<int64_t>(mex, &pos);
    topHeader = read_num_incr_pos<uint8_t>(mex, &pos);
    leftHeader = read_num_incr_pos<uint8_t>(mex, &pos);
    segments = read_num_incr_pos<int64_t>(mex, &pos);
    flags = read_num_incr_pos<int64_t>(mex, &pos);
    colsepPos = read_num_incr_pos<int64_t>(mex, &pos);
    rowsepPos = read_num_incr_pos<int64_t>(mex, &pos);
    quotesPos = read_num_incr_pos<int64_t>(mex, &pos);
    colTypesPos = read_num_incr_pos<int64_t>(mex, &pos);
    segInfoPos = read_num_incr_pos<int64_t>(mex, &pos);
    segSize = read_num_incr_pos<int64_t>(mex, &pos);

    return pos;
}

idx sTblIndex::Header::write(sMex * mex, idx pos, bool finished) const
{
    mex->resize(pos + size());

    if (finished) {
        memcpy(mex->ptr(pos), version, 16);
    } else {
        static const char * unfinished = "Unfinished" _ _ _ _ _ _;
        memcpy(mex->ptr(pos), unfinished, 16);
    }
    pos += 16;
    pos += write_num<int64_t>(mex, sourceTime, pos);
    pos += write_num<int64_t>(mex, sourceSize, pos);
    pos += write_num<int64_t>(mex, indexRangeStart, pos);
    pos += write_num<int64_t>(mex, indexRangeLast, pos);
    pos += write_num<int64_t>(mex, totalCols, pos);
    pos += write_num<int64_t>(mex, totalRows, pos);
    pos += write_num<unsigned char>(mex, topHeader, pos);
    pos += write_num<unsigned char>(mex, leftHeader, pos);
    pos += write_num<int64_t>(mex, segments, pos);
    pos += write_num<int64_t>(mex, flags, pos);
    pos += write_num<int64_t>(mex, colsepPos, pos);
    pos += write_num<int64_t>(mex, rowsepPos, pos);
    pos += write_num<int64_t>(mex, quotesPos, pos);
    pos += write_num<int64_t>(mex, colTypesPos, pos);
    pos += write_num<int64_t>(mex, segInfoPos, pos);
    pos += write_num<int64_t>(mex, segSize, pos);

    return pos;
}

idx sTblIndex::Header::size() const
{
    return 16 + 6 * sizeof(int64_t) + 2 + 8 * sizeof(int64_t);
}

idx sTblIndex::SegInfo::read(sMex * mex, idx pos)
{
    if (unlikely(pos + size() > mex->pos()))
        return -1;

    init();
    segPos = read_num_incr_pos<int64_t>(mex, &pos);
    index = read_num_incr_pos<int64_t>(mex, &pos);
    firstAbsCol = read_num_incr_pos<int64_t>(mex, &pos);
    firstAbsRow = read_num_incr_pos<int64_t>(mex, &pos);
    lastAbsCol = read_num_incr_pos<int64_t>(mex, &pos);
    lastAbsRow = read_num_incr_pos<int64_t>(mex, &pos);
    maxAbsCol = read_num_incr_pos<int64_t>(mex, &pos);
    flags = read_num_incr_pos<int64_t>(mex, &pos);

    updateSegSize();

    return pos;
}

idx sTblIndex::SegInfo::write(sMex * mex, idx pos) const
{
    mex->resize(pos + size());

    pos += write_num<int64_t>(mex, segPos, pos);
    pos += write_num<int64_t>(mex, index, pos);
    pos += write_num<int64_t>(mex, firstAbsCol, pos);
    pos += write_num<int64_t>(mex, firstAbsRow, pos);
    pos += write_num<int64_t>(mex, lastAbsCol, pos);
    pos += write_num<int64_t>(mex, lastAbsRow, pos);
    pos += write_num<int64_t>(mex, maxAbsCol, pos);
    pos += write_num<int64_t>(mex, flags, pos);

    return pos;
}

idx sTblIndex::SegInfo::size()
{
    return 8 * sizeof(int64_t);
}

void sTblIndex::SegInfo::updateSegSize()
{
    if (flags & fUInt8)
        colSize = 1;
    else if (flags & fUInt16)
        colSize = 2;
    else
        colSize = 4;

    if (flags & fRowMaxAbsCol)
        rowMaxAbsColSize = flags & fCompressRowMaxAbsCol ? colSize : sizeof(int64_t);
    else
        rowMaxAbsColSize = 0;

    rowSize = sizeof(int32_t) + rowMaxAbsColSize + maxAbsCol * colSize;
}

idx sTblIndex::SegInfo::segSize() const
{
    return rowSize * (lastAbsRow + 1 - firstAbsRow);
}

idx sTblIndex::SegInfo::maxAllowedRowMaxAbsCol() const
{
    if (flags & fRowMaxAbsCol)
        return flags & fCompressRowMaxAbsCol ? maxAllowedColIndex() : INT64_MAX;

    return 0;
}

idx sTblIndex::SegInfo::maxAllowedColIndex() const
{
    if (flags & fUInt8)
        return UINT8_MAX;
    else if (flags & fUInt16)
        return UINT16_MAX;
    return INT32_MAX;
}

idx sTblIndex::SegInfo::getSegMaxAbsCol(const sMex * mex, idx absrow) const
{
    if (flags & fRowMaxAbsCol)
        return read_num(mex, segRowPos(absrow) + sizeof(int32_t), rowMaxAbsColSize);

    return maxAbsCol;
}

idx sTblIndex::SegInfo::getSegIndex(const sMex * mex, idx absrow, idx abscol, idx * nextIndexInRow) const
{
    idx pos = segRowPos(absrow);
    idx rowindex = read_num_incr_pos<int32_t>(mex, &pos);
    idx colindex = 0;

    if (likely(abscol || nextIndexInRow)) {
        idx rowMaxAbsCol = flags & fRowMaxAbsCol ? read_num_incr_pos(mex, &pos, rowMaxAbsColSize) : maxAbsCol;

        if (unlikely(abscol > rowMaxAbsCol))
            return -1;

        pos += (abscol - 1) * colSize;
        if (likely(abscol))
            colindex = read_num(mex, pos, colSize);

        pos += colSize;
        if (likely(nextIndexInRow && abscol + 1 <= rowMaxAbsCol))
            *nextIndexInRow = index + rowindex + read_num(mex, pos, colSize);
    }
    return index + rowindex + colindex;
}

idx sTblIndex::SegInfo::setSegIndexRow(sMex * mex, idx absrow, const idx * indices, idx indices_first_abscol, idx indices_last_abscol)
{
    idx rowpos = segRowPos(absrow);
    idx pos = rowpos;
    idx rowindex = indices[0] - index;
    if (unlikely(rowindex > INT32_MAX))
        return 0;

    assert (indices_first_abscol >= 0);
    assert (absrow == firstAbsRow || (absrow <= lastAbsRow + 1 && indices_first_abscol == 0));
    assert (indices_last_abscol <= maxAbsCol);

    lastAbsRow = sMax<int64_t>(lastAbsRow, absrow);
    if (absrow == lastAbsRow)
        lastAbsCol = indices_last_abscol;

    mex->resize(pos + rowSize);
    pos += write_num<int32_t>(mex, rowindex, pos);
    if (flags & fRowMaxAbsCol) {
        assert (indices_last_abscol <= maxAllowedRowMaxAbsCol());
        minRowMaxAbsCol = sMin<idx>(minRowMaxAbsCol, indices_last_abscol);
        maxRowMaxAbsCol = sMax<idx>(maxRowMaxAbsCol, indices_last_abscol);
        pos += write_num(mex, indices_last_abscol, pos, rowMaxAbsColSize);
    }

    if (indices_first_abscol > 1) {
        idx skiplen = colSize * (indices_first_abscol - 1);
        memset(mex->ptr(pos), 0, skiplen);
        pos += skiplen;
    }

    idx indices_written = 1;
    for (idx i=1; i<=indices_last_abscol - indices_first_abscol; i++) {
        idx colindex = indices[i] - index - rowindex;
        if (colindex > maxAllowedColIndex())
            break;

        maxColIndex = sMax<idx>(maxColIndex, colindex);
        pos += write_num(mex, colindex, pos, colSize);
        indices_written++;
    }

    if (pos < rowpos + rowSize)
        memset(mex->ptr(pos), 0, rowpos + rowSize - pos);

    return indices_written;
}

void sTblIndex::SegInfo::segCompress(sMex * mex)
{
    bool compressed = true;
    int64_t newflags = flags & ~(int64_t)(fUInt8|fUInt16|fRowMaxAbsCol|fCompressRowMaxAbsCol);

    if (flags & fUInt8) {
        newflags |= fUInt8;
    } else {
        if (maxColIndex <= UINT8_MAX) {
            compressed = false;
            newflags |= fUInt8;
        } else {
            if (flags & fUInt16) {
                newflags |= fUInt16;
            } else if (maxColIndex <= UINT16_MAX) {
                compressed = false;
                newflags |= fUInt16;
            }
        }
    }

    if (flags & fRowMaxAbsCol) {
        if (minRowMaxAbsCol == maxAbsCol && maxRowMaxAbsCol == maxAbsCol) {
            compressed = false;
        } else {
            newflags |= fRowMaxAbsCol;
            if (!(flags & fCompressRowMaxAbsCol) &&
                ( (newflags & fUInt8 && maxRowMaxAbsCol <= UINT8_MAX) ||
                  (newflags & fUInt16 && maxRowMaxAbsCol <= UINT16_MAX) ) )
            {
                compressed = false;
                newflags |= fCompressRowMaxAbsCol;
            }
        }
    }

    if (compressed)
        return;

    SegInfo old(*this);

    flags = newflags;
    updateSegSize();

    for (idx absrow = firstAbsRow; absrow <= lastAbsRow; absrow++) {
        idx rowpos = segRowPos(absrow);
        idx oldrowpos = old.segRowPos(absrow);
        idx pos = rowpos, oldpos = oldrowpos;

        int32_t rowindex = read_num_incr_pos<int32_t>(mex, &oldpos);
        pos += write_num<int32_t>(mex, rowindex, pos);

        idx rowmax = old.flags & fRowMaxAbsCol ? read_num_incr_pos(mex, &oldpos, old.rowMaxAbsColSize): old.maxAbsCol;
        if (flags & fRowMaxAbsCol)
            pos += write_num(mex, rowmax, pos, rowMaxAbsColSize);

        for (idx abscol=1; abscol<=rowmax; abscol++) {
            idx colindex = read_num_incr_pos(mex, &oldpos, old.colSize);
            pos += write_num(mex, colindex, pos, colSize);
        }

        if (pos < rowpos + rowSize)
            memset(mex->ptr(pos), 0, rowpos + rowSize - pos);
    }
}

void sTblIndex::SegInfo::segExpand(sMex * mex, idx newMaxAbsCol, bool raggedize)
{
    bool expanded = true;
    if ((flags & fUInt8) || (flags & fUInt16))
        expanded = false;

    if (newMaxAbsCol > maxAbsCol)
        expanded = false;

    if (raggedize && !(flags & fRowMaxAbsCol))
        expanded = false;

    if ((flags & fRowMaxAbsCol) && (flags & fCompressRowMaxAbsCol))
        expanded = false;

    if (expanded)
        return;

    SegInfo old(*this);

    flags &= ~(int64_t)(fUInt8|fUInt16|fCompressRowMaxAbsCol);

    if (raggedize)
        flags |= fRowMaxAbsCol;

    maxAbsCol = sMax<int64_t>(newMaxAbsCol, maxAbsCol);
    minRowMaxAbsCol = sIdxMax;
    maxRowMaxAbsCol = 0;

    updateSegSize();
    mex->resize(segPos + segSize());

    for (idx absrow = lastAbsRow; absrow >= firstAbsRow; absrow--) {
        idx rowpos = segRowPos(absrow);
        idx oldrowpos = old.segRowPos(absrow);

        idx oldpos = oldrowpos;
        int32_t rowindex = read_num_incr_pos<int32_t>(mex, &oldpos);
        idx rowmax = old.flags & fRowMaxAbsCol ? read_num_incr_pos(mex, &oldpos, old.rowMaxAbsColSize) : old.maxAbsCol;
        idx pos = rowpos + sizeof(int32_t) + rowMaxAbsColSize;

        for (idx abscol = old.maxAbsCol; abscol > 0; abscol--) {
            idx colindex = read_num(mex, oldpos + (abscol - 1) * old.colSize, old.colSize);
            write_num(mex, colindex, pos + (abscol - 1) * colSize, colSize);
        }

        if (maxAbsCol > old.maxAbsCol)
            memset(mex->ptr(pos + old.maxAbsCol * colSize), 0, (maxAbsCol - old.maxAbsCol) * colSize);

        write_num<int32_t>(mex, rowindex, rowpos);

        if (flags & fRowMaxAbsCol) {
            minRowMaxAbsCol = sMin<idx>(minRowMaxAbsCol, rowmax);
            maxRowMaxAbsCol = sMax<idx>(maxRowMaxAbsCol, rowmax);
            write_num(mex, rowmax, rowpos + sizeof(int32_t), rowMaxAbsColSize);
        }
    }
}

sTblIndex::sTblIndex()
{
    _curSeg = 0;
    _segMaxOffset = INT32_MAX;
}

bool sTblIndex::readHeader()
{
    idx pos = 0, size_read = 0;

    if (unlikely(_hdr.read(&_mex, 0) <= 0))
        goto FAIL;

    _coltypes.resize(_hdr.totalCols);
    pos = _hdr.colTypesPos;
    for (idx i=0; i<_hdr.totalCols; i++) {
        if (unlikely(pos >= _mex.pos()))
            goto FAIL;

        _coltypes[i] = (sVariant::eType)(*static_cast<const int64_t*>(_mex.ptr(pos)));
        pos += sizeof(int64_t);
    }

    _segs.resize(_hdr.segments);
    pos = _hdr.segInfoPos;
    for (idx i=0; i<_hdr.segments; i++) {
        if (unlikely(pos >= _mex.pos()))
            goto FAIL;

        size_read = _segs[i].read(&_mex, pos) - pos;
        if (unlikely(size_read <= 0 || size_read > _hdr.segSize))
            goto FAIL;

        pos += _hdr.segSize;
    }

    _colsepLen.cut(0);
    if (colsep00()) {
        for (const char * c = colsep(); *c; c++) {
            idx len = strlen(c);
            c += len;
            _colsepLen.vadd(1, len);
        }
        if (!_colsepLen.dim())
            _colsepLen.vadd(1, 0);
    } else {
        _colsepLen.vadd(1, sLen(colsep()));
    }

    return true;

  FAIL:

    _hdr.init(0, 0);
    _coltypes.cut(0);
    _colsepLen.cut(0);
    _segs.cut(0);

    return false;
}

int sTblIndex::cmpCellSeg(idx absrow, idx abscol, idx iseg) const
{
    if (unlikely(absrow < _segs[iseg].firstAbsRow))
        return -1;

    if (unlikely(absrow > _segs[iseg].lastAbsRow))
        return 1;

    if (unlikely(absrow == _segs[iseg].firstAbsRow && abscol < _segs[iseg].firstAbsCol))
        return -1;

    if (unlikely(absrow == _segs[iseg].lastAbsRow && abscol > _segs[iseg].lastAbsCol && iseg < _hdr.segments - 1 && _segs[iseg+1].firstAbsRow == absrow))
        return 1;

    return 0;
}

idx sTblIndex::findSeg(idx absrow, idx abscol, idx iseg) const
{
    idx abscell = absrow * _hdr.totalCols + abscol;

    int cmpRes ;
    while ( (cmpRes = cmpCellSeg(absrow, abscol, iseg))!=0 ) {
        idx startAbsCell, endAbsCell;
        idx startSeg, endSeg;
        if (cmpRes < 0) {
            assert (iseg > 0);
            startAbsCell = 0;
            endAbsCell = _hdr.totalCols * _segs[iseg].firstAbsRow + _segs[iseg].firstAbsCol - 1;
            startSeg = 0;
            endSeg = iseg - 1;
        } else {
            assert (iseg < _hdr.segments - 1);
            startAbsCell = _hdr.totalCols * _segs[iseg].lastAbsRow + _segs[iseg].lastAbsCol + 1;
            endAbsCell = _hdr.totalCols * _hdr.totalRows;
            startSeg = iseg + 1;
            endSeg = _hdr.segments - 1;
        }

        iseg = idxrint((real)startSeg + (real)(endSeg - startSeg) * (real)(abscell - startAbsCell) / (real)(endAbsCell - startAbsCell));

        if (iseg < startSeg)
            iseg = startSeg;
        if (iseg > endSeg)
            iseg = endSeg;
    }

    return iseg;
}

idx sTblIndex::index(idx absrow, idx abscol, idx * nextIndexInRow) const
{
    if (unlikely(absrow >= totalRows() || absrow < 0 || abscol >= totalCols() || abscol < 0))
        return -1;

    _curSeg = findSeg(absrow, abscol, _curSeg);
    return _segs[_curSeg].getSegIndex(&_mex, absrow, abscol, nextIndexInRow);
}

idx sTblIndex::totalColsInRow(idx absrow) const
{
    if (unlikely(absrow >= totalRows() || absrow < 0))
        return 0;

    if (_hdr.flags & fRaggedEdge) {
        _curSeg = findSeg(absrow, _hdr.totalCols - 1, _curSeg);
        return 1 + _segs[_curSeg].getSegMaxAbsCol(&_mex, absrow);
    }

    return totalCols();
}

bool sTblIndex::isStandardCSV() const
{
    if( colsepCanRepeat() ) {
        return false;
    }

    const char * col_sep = colsep();
    if( strcmp(col_sep, ",") != 0 ) {
        return false;
    }
    if( colsep00() && col_sep[2] ) {
        return false;
    }

    const char * row_sep = rowsep();
    if( strcmp(row_sep, "\r\n") != 0 && strcmp(row_sep, "\n") != 0 ) {
        return false;
    }

    if( strcmp(quotes(), "\"") != 0 ) {
        return false;
    }

    if( puntQuotes() ) {
        return false;
    }

    return true;
}

void sTblIndex::initHeader(idx totalCols, idx flags, const char * colsep, const char * rowsep, const char * quotes_, idx dimTopHeader, idx dimLeftHeader)
{
    assert (totalCols >= 0);
    if (!colsep) colsep = "";
    if (!rowsep) rowsep = "";
    if (!quotes_) quotes_ = "";

    _mex.cut(0);
    _hdr.init(totalCols, flags, dimTopHeader, dimLeftHeader);
    idx pos = _hdr.write(&_mex, 0, false);

    _hdr.colsepPos = pos;
    _colsepLen.cut(0);
    if (colsep00()) {
        idx total = 0;

        for (const char * c = colsep; *c; c++) {
            idx len = 0;
            while (*c) {
                c++;
                len++;
            }
            _colsepLen.vadd(1, len);
            total += len + 1;
        }

        if (!_colsepLen.dim())
            _colsepLen.vadd(1, 0);

        pos += write_charbuf(&_mex, colsep, pos, 1+total);
    } else {
        idx len = sLen(colsep);
        _colsepLen.vadd(1, len);
        pos += write_charbuf(&_mex, colsep, pos, 1+len);
    }

    _hdr.rowsepPos = pos;
    pos += write_charbuf(&_mex, rowsep, pos, 1 + strlen(rowsep));

    _hdr.quotesPos = pos;
    pos += write_charbuf(&_mex, quotes_, pos, 1 + strlen(quotes_));

    pos += align_and_clear(&_mex, pos, sizeof(int64_t));

    _coltypes.resize(_hdr.totalCols);
    for (idx i=0; i<_hdr.totalCols; i++)
        _coltypes[i] = sVariant::value_NULL;

    _hdr.segments = 1;
    _segs.resize(1);
    _segs[0].maxAbsCol = sMax<idx>(0, totalCols - 1);
    if (flags & fRaggedEdge)
        _segs[0].flags |= SegInfo::fRowMaxAbsCol;
    _segs[0].segPos = pos;
    _segs[0].updateSegSize();
    _mex.resize(pos + _segs[0].rowSize);
    memset(_mex.ptr(pos), 0, _segs[0].rowSize);
}

void sTblIndex::compressLastSeg()
{
    lastSeg().segCompress(&_mex);
    _mex.cut(lastSeg().segPos + lastSeg().segSize());
}

void sTblIndex::addRow(const idx * indices, idx numIndices)
{
    assert (numIndices > 0);

    if (numIndices != _hdr.totalCols) {
        _hdr.totalCols = sMax<int64_t>(_hdr.totalCols, numIndices);
        _hdr.flags |= fRaggedEdge;
    }

    idx abscol = 0;
    while (abscol < numIndices) {
        if (abscol == 0 && _hdr.totalRows == 0) {
            lastSeg().index = indices[abscol];
            _hdr.indexRangeStart = indices[0];
        } else if (indices[abscol] - lastSeg().index > _segMaxOffset) {
            compressLastSeg();
            int64_t newPos = lastSeg().segPos + lastSeg().segSize();
            _hdr.segments++;
            _segs.resize(_hdr.segments);

            lastSeg().segPos = newPos + align_and_clear(&_mex, newPos, sizeof(int64_t));
            if (_hdr.flags & fRaggedEdge)
                lastSeg().flags |= SegInfo::fRowMaxAbsCol;
            lastSeg().index = indices[abscol];
            lastSeg().firstAbsRow = _hdr.totalRows;
            lastSeg().firstAbsCol = abscol;
            lastSeg().updateSegSize();
        }

        if ( ((_hdr.flags & fRaggedEdge) && !(lastSeg().flags & SegInfo::fRowMaxAbsCol)) ||
             (lastSeg().maxAbsCol < numIndices - 1) ||
             (indices[numIndices - 1] - indices[abscol] > lastSeg().maxAllowedColIndex())
           )
        {
            lastSeg().segExpand(&_mex, numIndices- 1, _hdr.flags & fRaggedEdge);
        }

        idx colsWritten = lastSeg().setSegIndexRow(&_mex, _hdr.totalRows, indices + abscol, abscol, numIndices - abscol - 1);
        abscol += colsWritten;
    }

    _hdr.totalRows++;
    _hdr.indexRangeLast = indices[numIndices - 1];
}

void sTblIndex::finish()
{
    compressLastSeg();
    idx pos = lastSeg().segPos + lastSeg().segSize();
    pos += align_and_clear(&_mex, pos, sizeof(int64_t));
    _hdr.colTypesPos = pos;
    idx size_written;
    for (idx i=0; i<_hdr.totalCols; i++) {
        _mex.resize(pos + sizeof(int64_t));
        size_written = write_num<int64_t>(&_mex, (int64_t)(_coltypes[i]), pos);
        pos += size_written;
    }

    _hdr.segInfoPos = pos;
    for (idx i=0; i<_hdr.segments; i++) {
        size_written = _segs[i].write(&_mex, pos) - pos;
        pos += _hdr.segSize;
    }

    _hdr.write(&_mex, 0, true);
}

const sTxtTbl::ParseOptions sTxtTbl::_default_parse_options;

sTxtTbl::sTxtTbl(const char * indexPath, bool indexReadOnly) : _indexPath(sMex::fExactSize)
{
    _tableBuf = 0;
    _tableBufLen = 0;
    _tableBufTime = sIdxMax;
    _addedCellsDim = 0;
    _addedRows = 0;

    if (indexPath) {
        _indexPath.addString(indexPath);
        if (sFile::size(indexPath)) {
            _index.mex()->init(indexPath, sMex::fReadonly);
            if (likely(_index.readHeader()))
                return;

            _index.mex()->destroy();
            _index.mex()->flags = 0;
            if( !indexReadOnly ) {
                sFile::remove(indexPath);
            }
        }

        if( !indexReadOnly ) {
            ensureIndexWritable();
        }
    }
}

sTxtTbl::~sTxtTbl()
{
    finish();
    if( _indexWritablePath ) {
        if( isParsed() ) {
            sFile::rename(_indexWritablePath.ptr(), _indexPath.ptr());
        } else {
            sFile::remove(_indexWritablePath.ptr());
        }
        _indexWritablePath.cut0cut();
        _index.mex()->destroy();
    }
}

void sTxtTbl::setBufInternal(const char * buf, idx len, idx time)
{
    if (!_tableFil.ptr() || buf < _tableFil.ptr() || buf > _tableFil.last())
        _tableFil.destroy();

    _tableBuf = buf;
    _tableBufLen = len;
    _tableBufTime = time;

    updateColIds();
}

void sTxtTbl::updateColIds()
{
    _colIds.empty();
    if (unlikely(isReadonly() && !isParsed()))
        return;

    sStr header_buf;

    for (idx icol=-dimLeftHeader(); icol<cols(); icol++) {
        header_buf.cut(0);
        printTopHeader(header_buf, icol);
        if (header_buf.length()) {
            *_colIds.setString(header_buf.ptr())->add(1) = icol;
        }
    }
}

void sTxtTbl::setFile(const char * path, idx mexflags)
{
    _tableFil.init(path, mexflags);
    setBufInternal(_tableFil.ptr(), _tableFil.length(), _tableFil.mtime());
    setTableMetadata("name", path);
}

bool sTxtTbl::isParsed() const
{
    return _tableBuf && _index.mex()->pos() && _index.sourceTime() == _tableBufTime && _index.sourceSize() == _tableBufLen;
}

static const char * bufFind00(const char * buf, idx buflen, const char * needle00)
{
    for (const char * needle = needle00; *needle; needle++) {
        idx i=0;
        while (i<buflen && needle[i] && buf[i] && needle[i] == buf[i])
            i++;

        if (needle[i]) {
            needle += i;
            while (*needle)
                needle++;
        } else {
            assert (i>0);
            return buf + i;
        }
    }
    return 0;
}

static const char * revFind00(const char * buf, idx buflen, const char * needle00, const idx * pneedleLens)
{
    if (unlikely(!needle00))
        return 0;

    const char * bufend = buf + buflen;
    const char * needle = needle00;
    while (*needle) {
        idx nlen = *pneedleLens;

        if (nlen <= buflen && !strncmp(needle, bufend-nlen, nlen))
            return bufend-nlen;

        needle += nlen + 1;
        pneedleLens++;
    }
    return 0;
}

#define VALID_BUF_PTR(c) ((c) < buf + buflen && *(c))

static const char * nextCell(const char * buf, idx buflen, idx flags, const char * colsep, const char * rowsep, const char * quotes, idx * pcellLen, bool * pfoundRowsep, bool * pquoted)
{
    idx seplen = 0;
    *pfoundRowsep = false;
    char quote = 0;

    const char *c = buf;

    if (strchr(quotes, *buf)) {
        quote = *buf;
        *pquoted = true;
        c++;
    } else {
        *pquoted = false;
    }

    while (VALID_BUF_PTR(c)) {
        if (quote) {
            while (VALID_BUF_PTR(c+1) && c[0] == '"' && c[1] == '"')
                c += 2;

            if (unlikely(!VALID_BUF_PTR(c)))
                goto FAIL;

            if (*c == quote) {
                c++;
                if (!VALID_BUF_PTR(c))
                    break;

            } else {
                c++;
                continue;
            }
        }

        if (flags & sTblIndex::fColsep00) {
            idx curseplen = 0;
            do {
                const char * cursepstart = c + seplen;
                curseplen = bufFind00(cursepstart, buflen - (cursepstart-buf), colsep) - cursepstart;
                if (curseplen > 0)
                    seplen += curseplen;
                else
                    break;
            } while (flags & sTblIndex::fColsepCanRepeat);

            if (seplen > 0)
                break;

        } else {
            if (strchr(colsep, *c)) {
                seplen = 1;
                if (flags & sTblIndex::fColsepCanRepeat) {
                    while(VALID_BUF_PTR(c + seplen) && strchr(colsep, c[seplen]))
                        seplen++;
                }
                break;
            }
        }

        if (strchr(rowsep, *c)) {
            *pfoundRowsep = true;
            for (seplen=1; VALID_BUF_PTR(c + seplen) && strchr(rowsep, c[seplen]); seplen++);
            break;
        }

        if (unlikely(quote && VALID_BUF_PTR(c)))
            goto FAIL;

        c++;
    }

    *pcellLen = c - buf;
    return VALID_BUF_PTR(c + seplen) ? c + seplen : 0;

  FAIL:
    *pcellLen = 0;
    return 0;
}

static sVariant::eType guessType(sVariant::eType curType, const char * val, idx len, bool quoted)
{
    if (quoted) {
        val++;
        len -= 2;
    }

    sVariant::eType ret = sVariant::guessScalarType(val, len, curType);

    if(ret == sVariant::value_HIVE_ID)
        return sVariant::value_STRING;

    return ret;
}

bool sTxtTbl::ensureIndexWritable(bool truncate_index)
{
    if( _index.mex()->ok() && !(_index.mex()->flags & sMex::fReadonly) )
        return true;

    if (_indexPath) {
        if( _indexWritablePath ) {
            return false;
        } else {
            const char * index_filename = sFilePath::nextToSlash(_indexPath.ptr());

            sStr mktemp_dir;
            if( index_filename > _indexPath.ptr() ) {
                mktemp_dir.addString(_indexPath.ptr(), index_filename - _indexPath.ptr());
            } else {
                mktemp_dir.addString(".", 1);
                mktemp_dir.addString(&sDir::sysSep, 1);
            }

            sStr mktemp_pattern;
            mktemp_pattern.addString(index_filename);
            mktemp_pattern.addString(DEFAULT_MKTEMP_PATTERN);

            if( unlikely(!sFile::mktemp(_indexWritablePath, mktemp_dir, 0, mktemp_pattern)) ) {
                return false;
            }
            sFil new_mex(_indexWritablePath.ptr(), sMex::fBlockDoubling);
            if( likely(new_mex.ok()) ) {
                if( _index.mex()->ok() && !truncate_index ) {
                    new_mex.add(static_cast<const char *>(_index.mex()->ptr(0)), _index.mex()->pos());
                }
                _index.mex()->destroy();
                _index.mex()->borrow(new_mex.mex());
            } else {
                _indexWritablePath.cut0cut();
                return false;
            }
        }
    } else {
        bool was_ok = _index.mex()->ok();
        bool remapped = _index.mex()->remap(0, sMex::fReadonly);
        if( unlikely(was_ok && !remapped) ) {
            fprintf(stderr, "%s:%u: ERROR: failed to remap table index memory allocation as read-write\n", __FILE__, __LINE__);
            return false;
        }
        if( truncate_index ) {
            _index.mex()->cut(0);
        }
    }
    return true;
}

void sTxtTbl::resetParseOptions(ParseOptions & p)
{
    static const char default_colsep[] = ",";
    static const char default_rowsep[] = "\r\n";
    static const char default_quotes[] = "\"";

    p.flags = sTblIndex::fTopHeader | sTblIndex::fSaveRowEnds;
    p.colsep = default_colsep;
    p.rowsep = default_rowsep;
    p.quotes = default_quotes;
    p.comment = 0;
    p.absrowStart = p.abscolStart = 0;
    p.absrowCnt = p.abscolCnt = sIdxMax;
    p.dimTopHeader = p.dimLeftHeader = 1;
    p.initialOffset = 0;
    p.headerOffset = -1;
    p.maxLen = sIdxMax;
    p.progressCallback = 0;
    p.progressParam = 0;
}

#define PTR_IF_NONEMPTY(s) ((s).length() ? (s).ptr() : 0)

static idx skipInitialComments(const char * buf, idx offset, idx len, const char * comment)
{
    idx comLen = sLen(comment);

    if (!comLen || !len || !buf)
        return offset;

    while (offset + comLen <= len && strncmp(buf + offset, comment, comLen) == 0) {
        offset += comLen;
        while (offset < len && buf[offset] != '\r' && buf[offset] != '\n')
            offset++;
        while (offset < len && (buf[offset] == '\r' || buf[offset] == '\n'))
            offset++;
    }

    return offset;
}

#define MAX_ROWS_GUESS_TYPE 2000

const char * sTxtTbl::parse()
{
    if (!_tableBuf)
        return 0;

    if (parseOptions().initialOffset >= _tableBufLen)
        return 0;

    if (parseOptions().abscolCnt < sIdxMax)
        parseOptions().flags = parseOptions().flags | sTblIndex::fSaveRowEnds;

    if (!ensureIndexWritable(true))
        return 0;

    const ParseOptions & p = parseOptions();
    if (p.progressCallback) {
        p.progressCallback(p.progressParam, 0, sNotIdx, _tableBufLen);
    }

    if (p.abscolCnt > _tableBufLen || p.absrowCnt > _tableBufLen) {
        _index.mex()->resize(_tableBufLen);
    }
    _index.mex()->cut(0);

    sVec<idx> indices;
    sVec<sVariant::eType> types;

    idx absrow = 0;
    idx abscol = 0;

    idx table_buf_len = _tableBufLen;
    if( p.maxLen < sIdxMax && p.maxLen + p.initialOffset < table_buf_len ) {
        table_buf_len = p.maxLen + p.initialOffset;
    }

    const char * first_data_cell = _tableBuf + skipInitialComments(_tableBuf, p.initialOffset, table_buf_len, p.comment);
    const char * cell = first_data_cell;
    idx skip_to_data = -1;
    if( p.headerOffset >= 0 && (p.flags & sTblIndex::fTopHeader) && p.dimTopHeader ) {
        cell = _tableBuf + skipInitialComments(_tableBuf, p.headerOffset, _tableBufLen, p.comment);
        skip_to_data = p.dimTopHeader;
    }
    const char * ret = 0;
    bool initializedHeader = false;
    bool savedRowEnds = false;
    idx headerRows = p.dimTopHeader;

    do {
        bool foundRowSep = false;
        bool quoted = false;
        idx cellLen = 0;
        const char * next = nextCell(cell, table_buf_len - (cell-_tableBuf), p.flags, p.colsep, p.rowsep, p.quotes, &cellLen, &foundRowSep, &quoted);
        idx index = cell - _tableBuf;
        if (p.progressCallback) {
            p.progressCallback(p.progressParam, index, sNotIdx, table_buf_len);
        }

        if (quoted && (p.flags & sTblIndex::fPuntQuotes)) {
            index++;
            cellLen--;
        }

        assert (index < table_buf_len);
        assert (cellLen >= 0);

        if (absrow >= p.absrowStart) {
            idx abscolOffset = abscol - p.abscolStart;
            idx absrowOffset = absrow - p.absrowStart;

            if (abscolOffset >= 0 && abscolOffset < p.abscolCnt) {
                *indices(abscolOffset) = index;
                if (types.dim() < abscolOffset + 1) {
                    idx prev_dim = types.dim();
                    types.resize(abscolOffset + 1);
                    for (idx i=prev_dim; i<abscolOffset + 1; i++) {
                        types[i] = sVariant::value_NULL;
                    }
                }

                if (absrow - p.absrowStart < MAX_ROWS_GUESS_TYPE && headerRows == 0)
                    types[abscolOffset] = guessType(types[abscolOffset], cell, cellLen, quoted && !(p.flags & sTblIndex::fPuntQuotes));
            }

            if (abscolOffset == p.abscolCnt - 1 && (p.flags & sTblIndex::fSaveRowEnds)) {
                *indices(abscolOffset + 1) = index + cellLen;
                savedRowEnds = true;
            }

            if (foundRowSep || !next) {
                idx dimIndices = abscolOffset + 1;
                if (p.flags & sTblIndex::fSaveRowEnds) {
                    if (!savedRowEnds) {
                        *indices(dimIndices) = index + cellLen;
                        savedRowEnds = true;
                    }
                    dimIndices++;
                }

                if (absrow == p.absrowStart) {
                    _index.initHeader(dimIndices, p.flags, p.colsep, p.rowsep, p.quotes, p.dimTopHeader, p.dimLeftHeader);
                    initializedHeader = true;
                }

                if (absrowOffset < p.absrowCnt) {
                    _index.addRow(indices.ptr(), dimIndices);
                    _index.setIndexRangeLast(index + cellLen);
                }

                indices.mex()->set(0);

                if( skip_to_data > 0 ) {
                    skip_to_data--;
                }
                headerRows--;
            }
        }

        if( skip_to_data == 0 ) {
            next = first_data_cell;
            skip_to_data = -1;
        }

        ret = next ? next : cell + cellLen;

        if (foundRowSep) {
            absrow++;
            abscol = 0;
            savedRowEnds = false;
        } else {
            abscol++;
        }
        cell = next;
    } while (cell && (absrow - p.absrowStart < p.absrowCnt - 1 || (absrow - p.absrowStart == p.absrowCnt - 1 && abscol - p.abscolStart < p.abscolCnt)));

    if (!initializedHeader)
        _index.initHeader(0, p.flags, p.colsep, p.rowsep, p.quotes, p.dimTopHeader, p.dimLeftHeader);

    for (idx i=0; i<_index.totalCols(); i++)
        _index.setColtype(i, i < types.dim() ? types[i] : sVariant::value_NULL);

    _index.setSourceInfo(_tableBufTime, _tableBufLen);

    _index.finish();

    if (p.progressCallback) {
        p.progressCallback(p.progressParam, table_buf_len, sNotIdx, table_buf_len);
    }

    updateColIds();

    return ret;
}

#define QUOTE_SCAN_MAX (1<<19)

#define DECREASE_LEN_TO_ROWSEP_FROM(initial_len) \
do { \
    for (len = (initial_len); len > 0 && !strchr(parseOptions->rowsep, buf[start + len - 1]); len--); \
    if (len <= 0) { \
        segments.cut(0); \
        return false; \
    } \
} while (0)

static bool matchCellSep(const char * buf, idx buflen, idx pos, const sTxtTbl::ParseOptions * parseOptions)
{
    if (pos == 0 || pos >= buflen)
        return true;

    if (strchr(parseOptions->rowsep, buf[pos]))
        return true;

    if (parseOptions->flags & sTblIndex::fColsep00) {
        if (bufFind00(buf, buflen, parseOptions->colsep)) {
            return true;
        }
    } else if (strchr(parseOptions->colsep, buf[pos])) {
        return true;
    }

    return false;
}

static bool revMatchCellSep(const char * buf, idx buflen, idx pos, const sTxtTbl::ParseOptions * parseOptions)
{
    if (pos == 0 || pos >= buflen)
        return true;

    if (strchr(parseOptions->rowsep, buf[pos]))
        return true;

    if (parseOptions->flags & sTblIndex::fColsep00) {
        sVec<idx> colsepLens;
        for (const char * colsep = parseOptions->colsep; colsep && *colsep; colsep = sString::next00(colsep)) {
            *colsepLens.add(1) = sLen(colsep);
        }
        if (revFind00(buf, pos + 1, parseOptions->colsep, colsepLens.ptr())) {
            return true;
        }
    } else if (strchr(parseOptions->colsep, buf[pos])) {
        return true;
    }

    return false;
}

bool sTxtTbl::segmentizeBuffer(sVec<sMex::Pos> & segments, const char * buf, idx buflen, const sTxtTbl::ParseOptions * parseOptions, idx maxSegSize)
{
    if (!buflen)
        buflen = sLen(buf);

    if (!parseOptions)
        parseOptions = &_default_parse_options;

    if (maxSegSize <= 0)
        maxSegSize = INT32_MAX;

    idx start = 0;
    idx len = maxSegSize;

    while (start + maxSegSize < buflen) {
        DECREASE_LEN_TO_ROWSEP_FROM(maxSegSize);

        idx quote_start = -1, quote_len = 0;
        for (idx qpos = start+len-1; qpos>start && qpos>(start+len-QUOTE_SCAN_MAX); qpos--) {
            if (strchr(parseOptions->quotes, buf[qpos])) {
                if (quote_start < 0 || quote_start > qpos) {
                    quote_start = qpos;
                }
                quote_len++;
            } else if (quote_len % 2) {
                break;
            } else {
                quote_start = -1;
                quote_len = 0;
            }
        }

        if (quote_len % 2) {
            bool has_inner_sep = matchCellSep(buf, buflen, quote_start + quote_len, parseOptions);
            bool has_outer_sep = revMatchCellSep(buf, buflen, quote_start - 1, parseOptions);
            if (has_outer_sep && !has_inner_sep) {
                DECREASE_LEN_TO_ROWSEP_FROM(quote_start - start);
            }
        }


        sMex::Pos * seg = segments.add(1);
        seg->pos = start;
        seg->size = len;

        start += len;
        len = maxSegSize;
    }

    if (start >= 0 && start < buflen) {
        sMex::Pos * seg = segments.add(1);
        seg->pos = start;
        seg->size = buflen - start;
    }

    return true;
}

const char * sTxtTbl::printCell(sStr & out, idx irow, idx icol, idx maxLen, const char * defValue, idx flags) const
{
    idx outStart = out.length();
    idx cellLen;

    if (maxLen <= 0)
        maxLen = sIdxMax;

    const char * s = cell(irow, icol, &cellLen);
    if (unlikely(!s)) {
        if (defValue) {
            if (flags & fForCSV)
                return sString::escapeForCSV(out, defValue);
            else
                return out.addString(defValue);
        }
        return 0;
    }
    if (unlikely(!cellLen)) {
        return out.add0cut();
    }

    _conversion_buf.cut0cut();

    if (flags & fForCSV) {
        if ( !_index.isStandardCSV() ) {
            return sString::escapeForCSV(out, s, sMin<idx>(cellLen, maxLen));
        }

        if (unlikely(cellLen > maxLen && *s == '"')) {
            sString::unescapeFromCSV(_conversion_buf, s, cellLen);
            return sString::escapeForCSV(out, _conversion_buf.ptr(), sMin<idx>(_conversion_buf.length(), maxLen));
        }
    } else if (!(flags & fNoUnescape) && *s == '"') {
        sString::unescapeFromCSV(_conversion_buf, s, cellLen);
        s = _conversion_buf.ptr();
        cellLen = _conversion_buf.length();
    }

    if (cellLen > 0) {
        out.add(s, sMin<idx>(maxLen, cellLen));
    }
    out.add0cut();

    return out.ptr(outStart);
}

const char * sTxtTbl::getRowBuf(idx irow, idx * len_out) const
{
    idx absrow = irow + dimTopHeader();

    idx first_cell_len = 0;
    const char * first_cell = cell(irow, -dimLeftHeader(), &first_cell_len);
    if( !first_cell ) {
        return 0;
    }

    idx row_ncols = _index.totalColsInRow(absrow) - (_index.saveRowEnds() ? 1 : 0) - dimLeftHeader();
    for(idx icol = row_ncols - 1; icol > dimLeftHeader(); icol--) {
        idx last_cell_len = 0;
        if( const char * last_cell = cell(irow, icol, &last_cell_len) ) {
            if( len_out ) {
                *len_out = last_cell_len + (last_cell - first_cell);
            }
            return first_cell;
        }
    }

    if( len_out ) {
        *len_out = first_cell_len;
    }
    return first_cell;
}

const char * sTxtTbl::getRowsBuf(idx irow, idx nrows, idx * len_out, idx * first_len_out) const
{
    idx first_row_len = 0, last_row_len = 0;
    const char * first_row = getRowBuf(irow, &first_row_len);
    const char * last_row = 0;

    for(idx irow2 = irow + nrows - 1; irow2 > irow; irow2--) {
        if( (last_row = getRowBuf(irow2, &last_row_len)) != 0 ) {
            break;
        }
    }

    if( !first_row ) {
        return 0;
    }

    if( first_len_out ) {
        *first_len_out = first_row_len;
    }
    if( len_out ) {
        *len_out = last_row ? (last_row_len + (last_row - first_row)) : first_row_len;
    }
    return first_row;
}

const char * sTxtTbl::printCSV(sStr & out, idx irowStart, idx icolStart, idx numRows, idx numCols, bool withHeaders) const
{
    if( !_index.isStandardCSV() || irowStart != 0 || icolStart != -dimLeftHeader() || numCols < cols() ) {
        return sTabular::printCSV(out, irowStart, icolStart, numRows, numCols, withHeaders);
    }

    idx startOut = out.length();

    numRows = sMin<idx>(numRows, rows() - irowStart);
    numCols = sMin<idx>(numCols, cols() - icolStart);

    idx header_buf_len = 0, header_buf_first_row_len = 0, data_buf_len = 0, data_buf_first_row_len = 0;
    const char * header_buf = withHeaders ? getRowsBuf(-dimTopHeader(), dimTopHeader(), &header_buf_len, &header_buf_first_row_len) : 0;
    const char * data_buf = getRowsBuf(irowStart, numRows, &data_buf_len, &data_buf_first_row_len);

    const char * newline = _index.rowsep();
    idx newline_len = sLen(newline);

    if( header_buf && dimTopHeader() > 1 ) {
        newline = header_buf + header_buf_first_row_len;
        newline_len = (strncmp(newline, "\r\n", 2) == 0) ? 2 : 1;
    } else if( data_buf && numRows > 1 ) {
        newline = data_buf + data_buf_first_row_len;
        newline_len = (strncmp(newline, "\r\n", 2) == 0) ? 2 : 1;
    }

    if( header_buf ) {
        if( header_buf_len ) {
            out.add(header_buf, header_buf_len);
        }
        out.addString(newline, newline_len);
    }

    if( data_buf ) {
        if( data_buf_len ) {
            out.add(data_buf, data_buf_len);
        }
        out.addString(newline, newline_len);
    }

    out.add0cut();

    return out.ptr(startOut);
}

const char * sTxtTbl::cell(idx irow, idx icol, idx * cellLen) const
{
    idx absrow = irow + dimTopHeader();
    idx abscol = icol + dimLeftHeader();
    idx nextPos = -1;
    idx pos = _index.index(absrow, abscol, &nextPos);
    if (unlikely(pos < 0)) {
        if (cellLen)
            *cellLen = 0;

        return 0;
    }
    const char * ret = _tableBuf + pos;

    if (likely(cellLen)) {
        char quote = 0;
        if (_index.puntQuotes() && pos > 0 && strchr(_index.quotes(), ret[-1]))
            quote = ret[-1];

        const char * end, * sep;
        bool sep00, sepCanRepeat, puntEndQuote;
        const idx * sep00len;

        if (likely(abscol + 1 < _index.totalColsInRow(absrow))) {
            if (likely(nextPos >= pos)) {
                end = _tableBuf + nextPos;
            } else {
                end = _tableBuf + _index.index(absrow, 1 + abscol);
            }
            sep = _index.colsep();
            sep00 = _index.colsep00();
            sep00len = _index.colsep00len();
            sepCanRepeat = _index.colsepCanRepeat();
            puntEndQuote = _index.puntQuotes() && (abscol + 2 < _index.totalColsInRow(absrow) || !_index.saveRowEnds());
        } else if (likely(absrow + 1 < _index.totalRows())) {
            end = _tableBuf + _index.index(1 + absrow, 0);
            sep = _index.rowsep();
            sep00 = false;
            sep00len = 0;
            sepCanRepeat = true;
            puntEndQuote = _index.puntQuotes();
        } else {
            end = _tableBuf + sMin<idx>(_tableBufLen, _index.indexRangeLast());
            sep = _index.rowsep();
            sep00 = false;
            sep00len = 0;
            sepCanRepeat = true;
            puntEndQuote = false;
        }

        assert (end >= ret);

        if (puntEndQuote && strchr(_index.quotes(), end[-1]))
            end--;

        do {
            if (unlikely(sep00)) {
                if (const char * sepStart = revFind00(ret, end - ret, sep, sep00len))
                    end = sepStart;
                else
                    break;
            } else {
                if (likely(sep[0] == end[-1] || strchr(sep, end[-1])))
                    end--;
                else
                    break;
            }
        } while (unlikely(sepCanRepeat && end>ret));

        if (quote && end>ret && end[-1] == quote)
            end--;

        *cellLen = sMax<idx>(0, end-ret);
    }
    return ret;
}

bool sTxtTbl::val(sVariant & out, idx irow, idx icol, bool noReinterpret, bool blankIsNull) const
{
    idx abscol = icol + dimLeftHeader();
    if (!noReinterpret && abscol >= 0 && abscol < _colReinterp.dim() && _colReinterp[abscol].dim() && icol >= 0) {
        if (likely(irow >= 0 && irow < _colReinterp[abscol].dim())) {
            out.setReal(_colReinterp[abscol][irow].r);
            return true;
        } else {
            out.setNull();
            return false;
        }
    }

    idx cellLen;
    const char * s = cell(irow, icol, &cellLen);
    if (unlikely(!s)) {
        out.setNull();
        return false;
    }

    const char * parse_buf = 0;
    if (cellLen) {
        _conversion_buf.cut(0);
        if (*s == '"') {
            sString::unescapeFromCSV(_conversion_buf, s, cellLen);
        } else {
            _conversion_buf.resize(cellLen + 1);
            memcpy(_conversion_buf.ptr(), s, cellLen);
            _conversion_buf[cellLen] = 0;
        }
        parse_buf = _conversion_buf.ptr();
    }

    if (blankIsNull) {
        if (cellLen == 0) {
            out.setNull();
            return true;
        }
        bool is_blank = true;
        for (idx i=0; parse_buf[i]; i++) {
            if (!isspace(parse_buf[i])) {
                is_blank = false;
                break;
            }
        }
        if (is_blank) {
            out.setNull();
            return true;
        }
    }

    out.parseScalarType(parse_buf, likely(irow >= 0) ? coltype(icol) : sVariant::value_STRING);
    return true;
}

void sTxtTbl::remapReadonly()
{
    if( _indexWritablePath ) {
        if( isParsed() ) {
            sFile::rename(_indexWritablePath.ptr(), _indexPath.ptr());
        } else {
            sFile::remove(_indexWritablePath.ptr());
        }
        _indexWritablePath.cut0cut();
        _index.mex()->destroy();
        _index.mex()->init(_indexPath.ptr(), sMex::fReadonly);
        if( !isParsed() ) {
            _index.mex()->destroy();
        }
    } else {
        _index.mex()->remap(sMex::fReadonly, 0, true);
    }

    if (_tableBuf == _tableFil.ptr()) {
        _tableBuf = _tableFil.remap(sMex::fReadonly, 0, true);
    }
}

bool sTxtTbl::initWritable(idx cols, idx flags, const char * colsep, const char * rowsep, const char * quotes, idx dimTopHeader, idx dimLeftHeader)
{
    if (_index.initialized())
        return false;

    if (!ensureIndexWritable())
        return false;

    if (flags & sTblIndex::fLeftHeader)
        cols += dimLeftHeader;

    _index.initHeader(cols, flags, colsep, rowsep, quotes, dimTopHeader, dimLeftHeader);
    return true;
}


sVariant::eType sTabular::mergeTypes(sVariant::eType t1, sVariant::eType t2)
{
    if (t1 == t2)
        return t1;

    switch (t1) {
    case sVariant::value_NULL:
        switch (t2) {
        case sVariant::value_NULL:
        case sVariant::value_INT:
        case sVariant::value_UINT:
        case sVariant::value_REAL:
        case sVariant::value_HIVE_ID:
        case sVariant::value_DATE_TIME:
        case sVariant::value_DATE:
        case sVariant::value_TIME:
            return t2;
        default:
            return sVariant::value_STRING;
        }
        break;
    case sVariant::value_INT:
        switch (t2) {
        case sVariant::value_NULL:
        case sVariant::value_INT:
        case sVariant::value_UINT:
            return sVariant::value_INT;
        case sVariant::value_HIVE_ID:
        case sVariant::value_REAL:
        case sVariant::value_DATE_TIME:
        case sVariant::value_DATE:
        case sVariant::value_TIME:
            return t2;
        default:
            return sVariant::value_STRING;
        }
        break;
    case sVariant::value_UINT:
        switch (t2) {
        case sVariant::value_NULL:
        case sVariant::value_UINT:
            return sVariant::value_UINT;
        case sVariant::value_INT:
        case sVariant::value_REAL:
        case sVariant::value_HIVE_ID:
        case sVariant::value_DATE_TIME:
        case sVariant::value_DATE:
        case sVariant::value_TIME:
            return t2;
        default:
            return sVariant::value_STRING;
        }
        break;
    case sVariant::value_REAL:
        switch (t2) {
        case sVariant::value_NULL:
        case sVariant::value_INT:
        case sVariant::value_UINT:
        case sVariant::value_REAL:
            return sVariant::value_REAL;
        case sVariant::value_HIVE_ID:
            return t2;
        default:
            return sVariant::value_STRING;
        }
        break;
    case sVariant::value_HIVE_ID:
        switch (t2) {
        case sVariant::value_NULL:
        case sVariant::value_INT:
        case sVariant::value_UINT:
        case sVariant::value_REAL:
        case sVariant::value_HIVE_ID:
            return sVariant::value_HIVE_ID;
        default:
            return sVariant::value_STRING;
        }
        break;
    case sVariant::value_DATE_TIME:
        switch (t2) {
        case sVariant::value_NULL:
        case sVariant::value_INT:
        case sVariant::value_UINT:
        case sVariant::value_DATE:
        case sVariant::value_DATE_TIME:
            return sVariant::value_DATE_TIME;
        default:
            return sVariant::value_STRING;
        }
        break;
    case sVariant::value_DATE:
        switch (t2) {
        case sVariant::value_NULL:
        case sVariant::value_INT:
        case sVariant::value_UINT:
        case sVariant::value_DATE:
            return sVariant::value_DATE;
        case sVariant::value_DATE_TIME:
            return sVariant::value_DATE_TIME;
        default:
            return sVariant::value_STRING;
        }
        break;
    case sVariant::value_TIME:
        switch (t2) {
        case sVariant::value_NULL:
        case sVariant::value_INT:
        case sVariant::value_UINT:
            return sVariant::value_TIME;
        default:
            return sVariant::value_STRING;
        }
        break;
    default:
        return sVariant::value_STRING;
    }
}

bool sTxtTbl::addCell(const char * val, idx len, idx typeHint)
{
    if (unlikely(isReadonly()))
        return false;

    if (unlikely(!ensureIndexWritable()))
        return false;

    assert (len >= 0);

    if (!len && val)
        len = sLen(val);

    if (!_addedRows && !_addedCellsDim && _tableFil.length() && !strchr(_index.rowsep(), _tableFil[_tableFil.length() - 1]))
        _tableFil.addString(_index.rowsep());

    if (_addedCellsDim)
        _tableFil.add(_index.colsep(), _index.colsep00() ? sLen(_index.colsep()) : 1);

    *_addedCellsIndices(_addedCellsDim) = _tableFil.length();

    if (len && val)
        _tableFil.add(val, len);

    _tableBuf = _tableFil.ptr();
    _tableBufLen = _tableFil.length();

    if (_index.totalRows() >= _index.topHeader()) {
        if (typeHint < 0) {
            if (_addedRows < MAX_ROWS_GUESS_TYPE) {
                bool is_val_quoted = val && len && strchr(_index.quotes(), val[0]);
                _index.setColtype(_addedCellsDim, guessType(_index.coltype(_addedCellsDim), val, len, is_val_quoted));
            }
        } else {
            _index.setColtype(_addedCellsDim, mergeTypes(_index.coltype(_addedCellsDim), (sVariant::eType)typeHint));
        }
    }

    *_addedCellsIndices(_addedCellsDim + 1) = _tableFil.length();
    _addedCellsDim++;
    return true;
}

bool sTxtTbl::addCell(const sVariant & val)
{
    _conversion_buf.cut(0);
    val.print(_conversion_buf, sVariant::eCSV);
    return addCell(_conversion_buf.ptr(), _conversion_buf.length(), val.getType());
}

bool sTxtTbl::addCell(const sHiveId & val)
{
    _conversion_buf.cut(0);
    val.print(_conversion_buf);
    return addCell(_conversion_buf.ptr(), _conversion_buf.length(), sVariant::value_HIVE_ID);
}

bool sTxtTbl::addICell(idx ival)
{
    _conversion_buf.printf(0, "%" DEC, ival);
    return addCell(_conversion_buf.ptr(), _conversion_buf.length(), sVariant::value_INT);
}

bool sTxtTbl::addUCell(udx uval)
{
    _conversion_buf.printf(0, "%" UDEC, uval);
    return addCell(_conversion_buf.ptr(), _conversion_buf.length(), sVariant::value_UINT);
}

bool sTxtTbl::addRCell(real rval)
{
    _conversion_buf.printf(0, "%g", rval);
    return addCell(_conversion_buf.ptr(), _conversion_buf.length(), sVariant::value_REAL);
}

bool sTxtTbl::addBoolCell(bool bval)
{
    return addCell(bval ? "1" : "0", 1, sVariant::value_INT);
}

bool sTxtTbl::addEndRow()
{
    if (unlikely(isReadonly()))
        return false;

    if (unlikely(!ensureIndexWritable()))
        return false;

    idx numIndices = _addedCellsDim;

    if (!_addedCellsDim) {
        *_addedCellsIndices(0) = _tableFil.length();
        numIndices = 1;
    } else if (_index.saveRowEnds()) {
        numIndices = _addedCellsDim + 1;
    }

    _index.addRow(_addedCellsIndices.ptr(), numIndices);

    _addedCellsDim = 0;
    _addedRows++;
    _tableFil.addString(_index.rowsep());
    _tableBuf = _tableFil.ptr();
    _tableBufLen = _tableFil.length();
    _index.setIndexRangeLast(_tableBufLen);

    if (rows() == 0)
        updateColIds();

    return true;
}

bool sTxtTbl::finish()
{
    if (unlikely(isReadonly()))
        return false;

    if (unlikely(!_index.initialized()))
        return true;

    if (unlikely(!ensureIndexWritable()))
        return false;

    _index.setSourceInfo(_tableFil.mtime(), _tableFil.length());
    _index.finish();
    return true;
}

void sVcfTbl::resetParseOptions(ParseOptions & p)
{
    static const char default_colsep[] = "\t";

    sTxtTbl::resetParseOptions(p);
    p.colsep = default_colsep;
}

const char * sVcfTbl::parse()
{
    idx offset = skipInitialComments(_tableBuf, parseOptions().initialOffset, _tableBufLen, "##");

    if (offset < _tableBufLen && _tableBuf[offset] == '#')
        offset++;

    parseOptions().initialOffset = offset;

    return sTxtTbl::parse();
}

sGtfTbl::sGtfTbl(const char * indexPath, bool indexReadOnly) : sTxtTbl(indexPath, indexReadOnly)
{
    resetParseOptions(parseOptions());
    _colname_buf.add("seqname");
    _colname_buf.add("source");
    _colname_buf.add("feature");
    _colname_buf.add("start");
    _colname_buf.add("end");
    _colname_buf.add("score");
    _colname_buf.add("strand");
    _colname_buf.add("frame");
    _colname_buf.add("attribute");
    _colname_buf.add0();
    for (const char * nm = _colname_buf.ptr(0); nm && *nm; nm = sString::next00(nm)) {
        *_colname_offsets.add() = nm - _colname_buf.ptr();
    }
    _colname_buf.cut(_colname_buf.length() - 1);
}

static const char gtf_colsep[] = "\t";

void sGtfTbl::resetParseOptions(ParseOptions & p)
{
    sTxtTbl::resetParseOptions(p);
    p.colsep = gtf_colsep;
    p.flags = sTblIndex::fSaveRowEnds;
}

const char * sGtfTbl::parse()
{
    parseOptions().colsep = gtf_colsep;
    return sTxtTbl::parse();
}

const char * sGtfTbl::cell(idx irow, idx icol, idx * cellLen) const
{
    if (irow == -1 && icol >= 0 && icol < _colname_offsets.dim()) {
        const char * ret = _colname_buf.ptr(_colname_offsets[icol]);
        if (cellLen) {
            if (icol < _colname_offsets.dim() - 1) {
                *cellLen = _colname_offsets[icol+1] - _colname_offsets[icol] - 1;
            } else {
                *cellLen = _colname_buf.length() - _colname_offsets[icol] - 1;
            }
        }
        return ret;
    }
    return sTxtTbl::cell(irow, icol, cellLen);
}

const char * sVariantTbl::printCell(sStr & out, idx irow, idx icol, idx maxLen, const char *defValue, idx flags) const
{
    sStr tmp;

    if (maxLen <= 0)
        maxLen = sIdxMax;

    if (unlikely(!inrange(irow, icol))) {
        if (defValue) {
            if (flags & fForCSV)
                return sString::escapeForCSV(out, defValue);
            else
                return out.addString(defValue);
        }
        return 0;
    }

    sVariant * v = _cells.ptr(cellsIndex(irow, icol));

    if (v->isNull()) {
        return out.add0cut();
    }

    if (maxLen < sIdxMax && flags & fForCSV) {
        sStr escbuf, unescbuf;
        v->print(escbuf, sVariant::eCSV);
        sString::unescapeFromCSV(unescbuf, escbuf, escbuf.length());
        return sString::escapeForCSV(out, unescbuf.ptr(), sMin<idx>(unescbuf.length(), maxLen));
    }

    return v->print(out, sVariant::eCSV);
}

const char * stringifyForCell(sVariant * v)
{
    if (v->isNull()) {
        return sStr::zero;
    }
    return v->asString();
}

const char * sVariantTbl::cell(idx irow, idx icol, idx * cellLen) const
{
    if (unlikely(!inrange(irow, icol))) {
        if (cellLen) {
            *cellLen = 0;
        }
        return 0;
    }

    const char * ret = stringifyForCell(_cells.ptr(cellsIndex(irow, icol)));
    if (likely(cellLen)) {
        *cellLen = sLen(ret);
    }
    return ret;
}

bool sVariantTbl::val(sVariant & out, idx irow, idx icol, bool noReinterpret, bool blankIsNull) const
{
    idx abscol = icol + dimLeftHeader();
    if (!noReinterpret && abscol >= 0 && abscol < _colReinterp.dim() && _colReinterp[abscol].dim() && icol >= 0) {
        if (irow >= 0 && irow < _colReinterp[abscol].dim()) {
            out.setReal(_colReinterp[abscol][irow].r);
            return true;
        } else {
            out.setNull();
            return false;
        }
    }

    if (unlikely(!inrange(irow, icol))) {
        out.setNull();
        return false;
    }

    out = _cells[cellsIndex(irow, icol)];
    if (blankIsNull && out.isString()) {
        const char * s = out.asString();
        bool is_blank = true;
        for (idx i=0; s[i]; i++) {
            if (!isspace(s[i])) {
                is_blank = false;
                break;
            }
        }
        if (is_blank) {
            out.setNull();
        }
    }
    return true;
}

bool sVariantTbl::setVal(idx irow, idx icol, sVariant & val)
{
    if (unlikely(icol < -_dim_left_header)) {
        return false;
    }

    if (icol >= cols()) {
        idx old_total_cols = cols() + _dim_left_header;
        idx cur_total_cols = icol + 1 + _dim_left_header;
        idx total_rows = rows() + 1;
        _coltypes.resize(cur_total_cols);
        _cells.resize(total_rows * cur_total_cols);
        for (idx ir=total_rows - 1; ir >= 0; ir--) {
            for (idx ic=cur_total_cols - 1; ic >= 0; ic--) {
                idx cur_index = ir * cur_total_cols + ic;
                if (ic < old_total_cols) {
                    idx old_index = ir * old_total_cols + ic;
                    _cells[cur_index] = _cells[old_index];
                } else {
                    _cells[cur_index].setNull();
                }
            }
        }
    }

    if (irow < 0) {
        const char * old_name = stringifyForCell(_cells.ptr(cellsIndex(irow, icol)));
        const char * cur_name = stringifyForCell(&val);

        if (strcmp(old_name, cur_name) != 0) {
            sVec<idx> * old_ids = _colIds.get(old_name);
            for (idx iold=0; old_ids && iold<old_ids->dim(); iold++) {
                if (*old_ids->ptr(iold) == icol) {
                    old_ids->del(iold);
                    break;
                }
            }
            sVec<idx> * cur_ids = _colIds.setString(cur_name);
            idx icur = 0;
            for (; icur < cur_ids->dim() && *cur_ids->ptr(icur) < icol; icur++);
            *cur_ids->insert(icur, 1) = icol;
        }
    } else {
        _coltypes[icol + _dim_left_header] = mergeTypes(_coltypes[icol + _dim_left_header], val.getType());
    }

    *_cells.ptrx(cellsIndex(irow, icol)) = val;

    return true;
}

static void indent(sStr & s, const sVariant::Whitespace * w, idx indent_len, idx newline_len, idx level)
{
    s.add(w->newline, newline_len);
    for (idx i=0; i<level; i++)
        s.add(w->indent, indent_len);
}

const char * sVariantTbl::printJSON(sStr & out, const sVariant::Whitespace * whitespace, idx indent_level) const
{
    idx len = out.length();
    if (!whitespace)
        whitespace = sVariant::getDefaultWhitespace(sVariant::eJSON);

    idx indent_len = sLen(whitespace->indent);
    idx newline_len = sLen(whitespace->newline);
    bool rows_multiline = indent_len && newline_len && dimTopHeader() + rows() && dimLeftHeader() + cols();

    out.add("[", 1);

    for (idx ir=-dimTopHeader(); ir<rows(); ir++) {
        if (rows_multiline) {
            if (ir > -dimTopHeader())
                out.add(",", 1);
            indent(out, whitespace, indent_len, newline_len, indent_level + 1);
        } else {
            if (ir > -dimTopHeader())
                out.add(", ", 2);
        }

        out.add("[", 1);
        bool cols_multiline = false;
        for (idx ic=-dimLeftHeader(); ic<cols(); ic++) {
            if (!_cells.ptr(cellsIndex(ir, ic))->isScalar() && _cells.ptr(cellsIndex(ir, ic))->dim()) {
                cols_multiline = true;
                break;
            }
        }
        for (idx ic=-dimLeftHeader(); ic<cols(); ic++) {
            if (ic > -dimLeftHeader()) {
                if (cols_multiline) {
                    out.add(",", 1);
                    indent(out, whitespace, indent_len, newline_len, indent_level + 2);
                } else {
                    out.add(", ", 2);
                }
            }

            _cells.ptr(cellsIndex(ir, ic))->print(out, sVariant::eJSON, whitespace, indent_level + 2);
        }
        out.add("]", 1);
    }

    if (rows_multiline)
        indent(out, whitespace, indent_len, newline_len, indent_level);

    out.add("]", 1);
    out.add0cut();
    return out.ptr(len);
}

const char * sVariantTblData::_typeName = "variant-table";
