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
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <slib/std/variant.hpp>
#include <slib/std/string.hpp>

using namespace slib;

static const char *sVariantNames[] = {
    "null",
    "integer",
    "unsigned",
    "real",
    "string",
    "list",
    "dic",
    "object ID",
    "date-time",
    "date",
    "time",
    "other data",
    "bool",
    NULL
};

struct DefaultWhitespaces
{
    sVariant::Whitespace csv, json;

    DefaultWhitespaces()
    {
        static const char sp1[] = " ";
        static const char sp4[] = "    ";
        static const char nl[] = "\n";

        csv.space = csv.indent = csv.newline = sStr::zero;
        json.space = sp1;
        json.indent = sp4;
        json.newline = nl;
    }
};
static DefaultWhitespaces default_whitespaces;

const sVariant::Whitespace * sVariant::getDefaultWhitespace(sVariant::ePrintMode mode)
{
    return mode == sVariant::eCSV ? &default_whitespaces.csv : &default_whitespaces.json;
}

struct ShortStrings
{
    char bytes[256 * 2];
    sDic<idx> interns;
    sStr buf;

    ShortStrings()
    {
        for (idx i=0; i<256; i++) {
            bytes[2 * i] = static_cast<char>(i);
            bytes[2 * i + 1] = 0;
        }
    }

    const char * getString(idx i) const
    {
        if (i >= 256) {
            return static_cast<const char *>(interns.id(i - 256));
        } else if (i > 0) {
            return bytes + 2 * i;
        } else {
            return sStr::zero;
        }
    }

    inline const char * getChar(char c) const { return c ? bytes + 2 * static_cast<unsigned char>(c) : sStr::zero; }

    void intern(const char *s, idx len)
    {
        if (!s || len < 2 || !s[0] || !s[1])
            return;

        interns.setString(s, len);
    }

    idx getIndex(const char *s, idx len) const
    {
        if (!s || !len || !s[0]) {
            return 0;
        }

        if (len == 1 || !s[1]) {
            return s[0];
        }

        idx result = interns.find(s, len) - 1;
        if (result >= 0) {
            return result + 256;
        }

        return 0;
    }
} shortStrings;

#define AS_STRING_CONST (_str.length() ? _str.ptr() : shortStrings.getString(_val.i))
#define RHS_AS_STRING_CONST (rhs._str.length() ? rhs._str.ptr() : shortStrings.getString(rhs._val.i))

void sVariant::empty()
{
    switch (_type) {
    case value_LIST:
        _val.list->decrementRefCount();
        if (!_val.list->hasRefs())
            delete _val.list;
        _val.list = NULL;
        break;
    case value_DIC:
        _val.dic->decrementRefCount();
        if (!_val.dic->hasRefs())
            delete _val.dic;
        _val.dic = NULL;
        break;
    case value_DATA:
        _val.data->decrementRefCount();
        if (!_val.data->hasRefs())
            delete _val.data;
        _val.data = NULL;
        break;
    default:
        break;
    }
    _type = value_NULL;
    _val.i = 0;
}

sVariant::sVariant(const sVariant &rhs): _type(rhs._type), _val(rhs._val), _str(sMex::fExactSize)
{
    switch (_type) {
    case value_STRING:
        setString(RHS_AS_STRING_CONST);
        break;
    case value_LIST:
        _val.list->incrementRefCount();
        break;
    case value_DIC:
        _val.dic->incrementRefCount();
        break;
    case value_DATA:
        _val.data = rhs._val.data->clone();
        break;
    default:
        break;
    }
}

const char* sVariant::getTypeName() const
{
    if (_type == value_DATA)
        return _val.data->getTypeName();

    return sVariantNames[_type];
}

const char* sVariant::getTypeName(idx type)
{
    if (unlikely(type < 0 || type > (idx)(sizeof(sVariantNames)/sizeof(const char *))))
        return 0;

    return sVariantNames[type];
}

sVariant::eType sVariant::parseTypeName(const char * type_name)
{
    for (int i = 0; i < sDim(sVariantNames) - 1; i++) {
        if (sIs(type_name, sVariantNames[i])) {
            return (eType)i;
        }
    }
    return value_NULL;
}

sVariant& sVariant::operator=(const sVariant &rhs)
{
    if (unlikely(this == &rhs))
        return *this;

    switch (rhs._type) {
    case value_NULL:
    case value_BOOL:
    case value_INT:
    case value_UINT:
    case value_REAL:
    case value_HIVE_ID:
    case value_DATE_TIME:
    case value_DATE:
    case value_TIME:
        empty();
        _val = rhs._val;
        break;
    case value_STRING:
        setString(RHS_AS_STRING_CONST);
        break;
    case value_LIST:
        if (likely(_type != rhs._type || _val.list != rhs._val.list)) {
            empty();
            _val.list = rhs._val.list;
            _val.list->incrementRefCount();
        }
        break;
    case value_DIC:
        if (likely(_type != rhs._type || _val.dic != rhs._val.dic)) {
            empty();
            _val.dic = rhs._val.dic;
            _val.dic->incrementRefCount();
        }
        break;
    case value_DATA:
        if (_type == value_DATA) {
            _val.data = rhs._val.data->cloneInto(_val.data);
        } else {
            empty();
            _val.data = rhs._val.data->clone();
        }

        assert (_val.data);
        break;
    }
    _type = rhs._type;
    return *this;
}

sVariant & sVariant::clone(const sVariant & rhs)
{
    if( unlikely(this == &rhs) ) {
        return *this;
    }
    switch(rhs._type) {
        case value_NULL:
        case value_BOOL:
        case value_INT:
        case value_UINT:
        case value_REAL:
        case value_HIVE_ID:
        case value_DATE_TIME:
        case value_DATE:
        case value_TIME:
            empty();
            _val = rhs._val;
            break;
        case value_STRING:
            setString(RHS_AS_STRING_CONST);
            break;
        case value_LIST:
            setList();
            _val.list->clone(*rhs._val.list);
            break;
        case value_DIC:
            setDic();
            _val.dic->clone(*rhs._val.dic);
            break;
        case value_DATA:
            if( _type == value_DATA ) {
                _val.data = rhs._val.data->cloneInto(_val.data);
            } else {
                empty();
                _val.data = rhs._val.data->clone();
            }
            assert(_val.data);
            break;
    }
    _type = rhs._type;
    return *this;
}


void sVariant::setBool(bool b)
{
    empty();
    _type = value_BOOL;
    _val.i = b ? 1 : 0;
}

void sVariant::setInt(idx i)
{
    empty();
    _type = value_INT;
    _val.i = i;
}

void sVariant::setUInt(udx u)
{
    empty();
    _type = value_UINT;
    _val.u = u;
}

void sVariant::setReal(real r)
{
    empty();
    _type = value_REAL;
    _val.r = r;
}

void sVariant::setString(const char *s, idx len)
{
    empty();
    _type = value_STRING;
    _str.cut(0);
    if (!len && s && *s) {
        len = strlen(s);
    }
    _val.i = shortStrings.getIndex(s, len);
    if (!_val.i && s && *s) {
        _str.add(s, len);
        _str.add0cut();
    }
}

void sVariant::setVSprintf(const char *fmt, va_list marker)
{
    empty();
    _type = value_STRING;
    _str.cut(0);
    _str.vprintf(fmt, marker);
    _val.i = shortStrings.getIndex(_str.ptr(), _str.length());
    if (_val.i || !_str.length()) {
        _str.cut(0);
    }
}

void sVariant::setSprintf(const char *fmt, ...)
{
    sCallVarg(setVSprintf, fmt);
}

void sVariant::setList()
{
    empty();
    _type = value_LIST;
    _val.list = new sVariantList();
}

void sVariant::setList(sVariant *list, idx listdim)
{
    setList();
    _val.list->resize(listdim);
    for (idx i=0; i<listdim; i++)
        _val.list->set(i, list[i]);
}

void sVariant::setDic()
{
    empty();
    _type = value_DIC;
    _val.dic = new sVariantDic();
}

void sVariant::setHiveId(const sHiveId & id)
{
    empty();
    _type = value_HIVE_ID;
    *_val.id() = id;
}

void sVariant::setHiveId(const char * domain_id, udx obj_id, udx ion_id)
{
    empty();
    _type = value_HIVE_ID;
    _val.id()->set(domain_id, obj_id, ion_id);
}

void sVariant::setHiveId(const sVariant & rhs)
{
    sHiveId tmp;
    rhs.asHiveId(&tmp);
    setHiveId(tmp);
}

void sVariant::setHiveId(const char *s)
{
    empty();
    _type = value_HIVE_ID;
    _val.id()->parse(s);
}

void sVariant::setDateTime(idx unix_time)
{
    empty();
    _type = value_DATE_TIME;
    _val.date.timestamp = unix_time;
    _val.date.utc_offset = -sIdxMax;

#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
    struct tm tm;
    time_t t = unix_time;
    memset(&tm, 0, sizeof(struct tm));
    if (localtime_r(&t, &tm) && tm.tm_zone) {
        _val.date.utc_offset = tm.tm_gmtoff;
    }
#endif
}

void sVariant::setDateTime(const struct tm * tm)
{
    empty();
    _type = value_DATE_TIME;
    _val.date.timestamp = -sIdxMax;
    _val.date.utc_offset = -sIdxMax;
    if (!tm) {
        return;
    }

    struct tm tm_copy;
    memcpy(&tm_copy, tm, sizeof(struct tm));
    tm_copy.tm_isdst = -1;

#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
    if (tm->tm_zone) {
        _val.date.utc_offset = tm->tm_gmtoff;
        tm_copy.tm_sec -= tm->tm_gmtoff;
        tm_copy.tm_gmtoff = 0;
        tm_copy.tm_zone = "GMT";
        _val.date.timestamp = static_cast<idx>(timegm(&tm_copy));
    }
#endif
    if (_val.date.timestamp == -sIdxMax) {
        _val.date.timestamp = static_cast<idx>(mktime(&tm_copy));
    }

    if (_val.date.timestamp == -1) {
        _val.date.timestamp = -sIdxMax;
    }
}

void sVariant::setDateTime(const sVariant & rhs)
{
    switch(rhs._type) {
    case value_NULL:
    case value_LIST:
    case value_DIC:
        setDateTime((struct tm *)0);
        break;
    case value_REAL:
        if (isnan(rhs._val.r)) {
            setDateTime((struct tm *)0);
        } else {
            setDateTime(rhs.asInt());
        }
        break;
    case value_DATE_TIME:
        *this = rhs;
        break;
    case value_DATE:
    case value_TIME:
        *this = rhs;
        _type = value_DATE_TIME;
        break;
    case value_STRING:
        parseDateTime(RHS_AS_STRING_CONST);
        break;
    case value_DATA:
        {
            sStr buf;
            parseDateTime(rhs.print(buf));
        }
        break;
    default:
        setDateTime(rhs.asInt());
        break;
    }
}

static idx removeTimeComponent(idx unix_time, idx utc_offset)
{
    if (unix_time == -sIdxMax) {
        return -sIdxMax;
    }
    struct tm tm;
    if (utc_offset == -sIdxMax) {
        time_t t = unix_time;
        if (!localtime_r(&t, &tm)) {
            return -sIdxMax;
        }
    } else {
        time_t t = unix_time - utc_offset;
        if (!gmtime_r(&t, &tm)) {
            return -sIdxMax;
        }
    }
    tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
    tm.tm_isdst = -1;
    idx ret = static_cast<idx>(mktime(&tm));
    return ret == -1 ? -sIdxMax : ret;
}

static idx extractTimeComponent(idx unix_time, idx utc_offset)
{
    if (unix_time == -sIdxMax) {
        return 0;
    }
    struct tm tm;
    if (utc_offset == -sIdxMax) {
        time_t t = unix_time;
        if (!localtime_r(&t, &tm)) {
            return 0;
        }
    } else {
        time_t t = unix_time + utc_offset;
        if (!gmtime_r(&t, &tm)) {
            return 0;
        }
    }
    return 60 * (60 * (idx)tm.tm_hour + (idx)tm.tm_min) + (idx)tm.tm_sec;
}

static idx makeLocalizedDateTime(struct tm * out, idx unix_time, idx utc_offset)
{
    if (unix_time == -sIdxMax) {
        return false;
    }
    if (utc_offset == -sIdxMax) {
        time_t t = unix_time;
        if (!localtime_r(&t, out)) {
            return -sIdxMax;
        }
    } else {
        time_t t = unix_time + utc_offset;
        if (gmtime_r(&t, out)) {
#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
            out->tm_gmtoff = utc_offset;
            out->tm_zone = "???";
#endif
        } else {
            return -sIdxMax;
        }
    }

    return unix_time;
}

void sVariant::setDate(idx unix_time)
{
    empty();
    _type = value_DATE;
    _val.date.timestamp = removeTimeComponent(unix_time, -sIdxMax);
    _val.date.utc_offset = -sIdxMax;
}

void sVariant::setDate(const struct tm * tm)
{
    setDateTime(tm);
    _type = value_DATE;
    _val.date.timestamp = removeTimeComponent(_val.date.timestamp, _val.date.utc_offset);
    _val.date.utc_offset = -sIdxMax;
}

void sVariant::setDate(const sVariant & rhs)
{
    switch(rhs._type) {
    case value_NULL:
    case value_LIST:
    case value_DIC:
        setDate((struct tm *)0);
        break;
    case value_REAL:
        if (isnan(rhs._val.r)) {
            setDate((struct tm *)0);
        } else {
            setDate(rhs.asInt());
        }
        break;
    case value_DATE_TIME:
        *this = rhs;
        _type = value_DATE;
        _val.date.timestamp = removeTimeComponent(_val.date.timestamp, _val.date.utc_offset);
        _val.date.utc_offset = 0;
        break;
    case value_DATE:
        *this = rhs;
        break;
    case value_TIME:
        setDate(time(0));
        break;
    case value_STRING:
        parseDate(RHS_AS_STRING_CONST);
        break;
    case value_DATA:
        {
            sStr buf;
            parseDate(rhs.print(buf));
        }
        break;
    default:
        setDate(rhs.asInt());
        break;
    }
}

static inline bool timeInRange(idx seconds)
{
    return seconds >= 0 && seconds <= 24 * 60 * 60 + 1;
}

void sVariant::setTime(idx seconds, idx utc_offset)
{
    empty();
    _type = value_TIME;
    _val.date.timestamp = sClamp<idx>(seconds, 0, 24 * 60 * 60 + 1);
    _val.date.utc_offset = utc_offset;
}

void sVariant::setTime(const struct tm * tm)
{
    empty();
    _type = value_TIME;
    _val.date.utc_offset = -sIdxMax;
    if (!tm) {
        _val.date.timestamp = 0;
        return;
    }
    _val.date.timestamp = 60 * (60 * (idx)tm->tm_hour + (idx)tm->tm_min) + (idx)tm->tm_sec;
#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
    if (tm->tm_zone) {
        _val.date.utc_offset = tm->tm_gmtoff;
    }
#endif
}

void sVariant::setTime(const sVariant & rhs)
{
    switch(rhs._type) {
    case value_LIST:
    case value_DIC:
        setTime((idx)0);
        break;
    case value_DATE_TIME:
        setTime(extractTimeComponent(rhs._val.date.timestamp, rhs._val.date.utc_offset), rhs._val.date.utc_offset);
        break;
    case value_DATE:
        setTime((idx)0);
        break;
    case value_TIME:
        *this = rhs;
        break;
    case value_STRING:
        parseTime(RHS_AS_STRING_CONST);
        break;
    case value_DATA:
        {
            sStr buf;
            rhs.print(buf);
            parseTime(buf.ptr());
        }
        break;
    default:
        setTime(rhs.asInt());
        break;
    }
}

void sVariant::setData(sVariantData &data)
{
    if (_type == value_DATA) {
        _val.data = data.cloneInto(_val.data);
    } else {
        empty();
        _type = value_DATA;
        _val.data = data.clone();
    }
}

static real stringToReal(const char *s, bool *perr=0)
{
    real val = 0;
    bool err = false;
    char * endptr = 0;

    if (!s)
        goto RET;

    val = strtod(s, &endptr);
    while (*endptr) {
        if (!isspace(*endptr++)) {
            err = true;
            goto RET;
        }
    }

  RET:
    if (unlikely(err))
        val = NAN;

    if (perr)
        *perr = err;

    return val;
}

static idx stringToInt(const char *s, bool *perr=0)
{
    idx val = 0;
    bool err = false;
    char * endptr = 0;

    if (!s)
        goto RET;

    val = strtoidx(s, &endptr, 10);
    while (*endptr) {
        if (!isspace(*endptr++)) {
            err = true;
            goto RET;
        }
    }

  RET:
    if (unlikely(err)) {
        val = idxround(stringToReal(s, &err));
    }

    if (unlikely(err))
        val = 0;

    if (perr)
        *perr = err;

    return val;
}

static udx stringToUInt(const char *s, bool *perr=0)
{
    udx val = 0;
    bool err = false;
    char * endptr = 0;

    if (!s)
        goto RET;

    val = strtoudx(s, &endptr, 10);
    while (*endptr) {
        if (!isspace(*endptr++)) {
            err = true;
            goto RET;
        }
    }

  RET:
    if (unlikely(err)) {
        val = idxround(stringToReal(s, &err));
    }

    if (unlikely(err))
        val = 0;

    if (perr)
        *perr = err;

    return val;
}

void sVariant::parseInt(const char *s)
{
    setInt(stringToInt(s));
}

void sVariant::parseUInt(const char *s)
{
    setUInt(stringToUInt(s));
}

void sVariant::parseBool(const char *s)
{
    setBool(sString::parseBool(s));
}

void sVariant::parseHiveId(const char *s)
{
    empty();
    _type = value_HIVE_ID;
    _val.id()->parse(s);
}

void sVariant::parseReal(const char *s)
{
    setReal(stringToReal(s));
}

void sVariant::parseDateTime(const char *s)
{
    struct tm tm;
    idx utc_offset = -sIdxMax;
    idx parsedEnd = 0;
    idx t_datetime = sString::parseDateTime(&tm, s, &parsedEnd, &utc_offset);
    setDateTime(parsedEnd == sLen(s) ? t_datetime : -sIdxMax);
    _val.date.utc_offset = utc_offset;
}

void sVariant::parseDate(const char *s)
{
    parseDateTime(s);
    _type = value_DATE;
    _val.date.timestamp = removeTimeComponent(_val.date.timestamp, _val.date.utc_offset);
    _val.date.utc_offset = -sIdxMax;
}

void sVariant::parseTime(const char *s)
{
    empty();
    _type = value_TIME;
    idx parsedEnd = 0;
    _val.date.timestamp = sString::parseTime(s, &parsedEnd, &_val.date.utc_offset);
    if (_val.date.timestamp == -sIdxMax || parsedEnd != sLen(s)) {
        setTime(stringToInt(s), 0);
    }
}

void sVariant::parseNumList(const char *s, idx type)
{
    setList();
    if (!s)
        return;

    sStr s00;
    sString::searchAndReplaceSymbols(&s00, s, 0, ",; \t\r\n", 0, 0, true, true, true);
    s00.add0(2);
    for (const char *t = s00.ptr(); t && *t; t = sString::next00(t)) {
        if (type == value_INT)
            _val.list->add().parseInt(t);
        else if (type == value_HIVE_ID)
            _val.list->add().parseHiveId(t);
    }
}

void sVariant::parseIntList(const char *s)
{
    parseNumList(s, value_INT);
}

void sVariant::parseHiveIdList(const char *s)
{
    parseNumList(s, value_HIVE_ID);
}

void sVariant::parseScalarType(const char * s, sVariant::eType type)
{
    switch(type) {
    case value_NULL:
        if( s && s[0] ) {
            type = guessScalarType(s);
        }
        if( type == value_NULL ) {
            setNull();
        } else {
            parseScalarType(s, type);
            if( !asBool() ) {
                setNull();
            }
        }
        break;
    case value_BOOL:
        parseBool(s);
        break;
    case value_INT:
        parseInt(s);
        break;
    case value_UINT:
        parseUInt(s);
        break;
    case value_REAL:
        parseReal(s);
        break;
    case value_HIVE_ID:
        parseHiveId(s);
        break;
    case value_DATE_TIME:
        parseDateTime(s);
        break;
    case value_DATE:
        parseDate(s);
        break;
    case value_TIME:
        parseTime(s);
        break;
    case value_STRING:
    default:
        setString(s);
        break;
    }
}

#define GUESS_TYPE_BUF_LEN 128
sVariant::eType sVariant::guessScalarType(const char * buf, idx len, sVariant::eType prevGuess)
{
    if( prevGuess == value_STRING || len >= GUESS_TYPE_BUF_LEN )
        return sVariant::value_STRING;

    if( len <= 0 )
        return prevGuess;

    char tmpbuf[GUESS_TYPE_BUF_LEN];
    memcpy(tmpbuf, buf, len);
    tmpbuf[len] = 0;
    return guessScalarType(tmpbuf, prevGuess);
}

sVariant::eType sVariant::guessScalarType(const char * s, sVariant::eType prevGuess)
{
    if( prevGuess == sVariant::value_STRING || !s || !s[0] )
        return prevGuess;

    bool isBool = false, isInt = false, isReal = false, isDate = false, isTime = false, isDateTime = false;

    char * endptr = 0;
    idx ival = strtoidx(s, &endptr, 10);
    if( *endptr ) {
        if( sIsExactly(s, "false") || sIsExactly(s, "off") || sIsExactly(s, "true") || sIsExactly(s, "on") ) {
            isBool = true;
        } else {
            strtod(s, &endptr);
            if( !*endptr ) {
                isReal = true;
            }
        }
    } else {
        isInt = true;
        isReal = true;
        if( ival == 0 || ival == 1 ) {
            isBool = true;
        }
    }

    if( !isBool && !isInt && !isReal ) {
        sVariant date_time;
        date_time.parseDateTime(s);
        if( date_time.asBool() ) {
            isDateTime = true;
            sVariant date_only;
            date_only.parseDate(s);
            if( date_time == date_only ) {
                isDate = true;
            }
        } else {
            struct tm tm;
            isTime = sString::parseTime(&tm, s) != -sIdxMax;
        }
    }

    bool checkForHiveId = false;

    switch (prevGuess) {
        case sVariant::value_NULL:
            if( isInt ) {
                return sVariant::value_INT;
            } else if( isReal ) {
                return sVariant::value_REAL;
            } else if( isBool ) {
                return sVariant::value_BOOL;
            } else if( isDate ) {
                return sVariant::value_DATE;
            } else if( isDateTime ) {
                return sVariant::value_DATE_TIME;
            } else if( isTime ) {
                return sVariant::value_TIME;
            } else {
                checkForHiveId = true;
                break;
            }
        case sVariant::value_INT:
        case sVariant::value_UINT:
            if( isInt ) {
                return sVariant::value_INT;
            } else if( isReal ) {
                return sVariant::value_REAL;
            } else if( isDate ) {
                return sVariant::value_DATE;
            } else if( isDateTime ) {
                return sVariant::value_DATE_TIME;
            } else if( isTime ) {
                return sVariant::value_TIME;
            } else {
                checkForHiveId = true;
                break;
            }
        case sVariant::value_REAL:
            if( isReal ) {
                return sVariant::value_REAL;
            } else {
                checkForHiveId = true;
                break;
            }
        case sVariant::value_BOOL:
            if( isBool ) {
                return sVariant::value_BOOL;
            } else {
                return sVariant::value_STRING;
            }
        case sVariant::value_DATE:
            if( isInt || isDate ) {
                return sVariant::value_DATE;
            } else if( isDateTime ) {
                return sVariant::value_DATE_TIME;
            } else {
                return sVariant::value_STRING;
            }
        case sVariant::value_DATE_TIME:
            if( isInt || isDate || isDateTime ) {
                return sVariant::value_DATE_TIME;
            } else {
                return sVariant::value_STRING;
            }
        case sVariant::value_TIME:
            if( isInt || isTime ) {
                return sVariant::value_TIME;
            } else {
                return sVariant::value_STRING;
            }
        case sVariant::value_HIVE_ID:
            checkForHiveId = true;
            break;
        default:
            break;
    }

    if( checkForHiveId ) {
        idx start = 0;
        for(; start < GUESS_TYPE_BUF_LEN && s[start] && isspace(s[start]); start++);
        sHiveId tmp_id;
        idx end = start + tmp_id.parse(s + start);
        if( end > start ) {
            for(; end < GUESS_TYPE_BUF_LEN && s[end] && isspace(s[end]); end++);
            if( end >= GUESS_TYPE_BUF_LEN || !s[end] ) {
                return value_HIVE_ID;
            }
        }
    }

    return sVariant::value_STRING;
}

bool sVariant::asBool() const
{
    switch(_type) {
    case value_NULL:
        return false;
    case value_BOOL:
    case value_INT:
        return _val.i;
    case value_REAL:
        if (likely(isfinite(_val.r)))
            return _val.r != 0;
        if (isinf(_val.r))
            return true;
        return false;
    case value_STRING:
        return sString::parseBool(AS_STRING_CONST);
    case value_LIST:
        return _val.list->dim();
    case value_DIC:
        return _val.dic->dim();
    case value_HIVE_ID:
        return _val.id()->valid();
    case value_UINT:
        return _val.u;
    case value_DATE_TIME:
    case value_DATE:
        return _val.date.timestamp != -sIdxMax;
    case value_TIME:
        return _val.date.timestamp;
    case value_DATA:
        return _val.data->asBool();
    }
    return false;
}

bool sVariant::isNullish() const
{
    switch(_type) {
    case value_NULL:
        return true;
    case value_BOOL:
    case value_INT:
        return _val.i == 0;
    case value_REAL:
        return _val.r == 0.0;
    case value_STRING:
        return _str.length() == 0;
    case value_LIST:
        return _val.list->dim() == 0;
    case value_DIC:
        return _val.dic->dim() == 0;
    case value_HIVE_ID:
        return *_val.id() == sHiveId::zero;
    case value_UINT:
        return _val.u == 0;
    case value_DATE_TIME:
    case value_DATE:
        return _val.date.timestamp == -sIdxMax;
    case value_TIME:
        return _val.date.timestamp == 0;
    case value_DATA:
        return _val.data->isNullish();
    }
    return false;
}

idx sVariant::asInt() const
{
    switch(_type) {
    case value_NULL:
        return 0;
    case value_BOOL:
        return _val.i ? 1 : 0;
    case value_INT:
        return _val.i;
    case value_REAL:
        if (likely(isfinite(_val.r)))
            return idxrint(_val.r);
        if (isinf(_val.r)) {
            if (signbit(_val.r))
                return -sIdxMax;
            return sIdxMax;
        }
        return 0;
    case value_STRING:
        return stringToInt(AS_STRING_CONST);
    case value_LIST:
        return _val.list->dim();
    case value_DIC:
        return _val.dic->dim();
    case value_UINT:
    case value_HIVE_ID:
        return (idx)(_val.id()->objId());
    case value_DATE_TIME:
    case value_DATE:
    case value_TIME:
        return _val.date.timestamp;
    case value_DATA:
        return _val.data->asInt();
    }
    return 0;
}

udx sVariant::asUInt() const
{
    switch(_type) {
    case value_UINT:
        return _val.u;
    case value_HIVE_ID:
        return _val.id()->objId();
    case value_DATE_TIME:
    case value_TIME:
        return _val.date.timestamp == -sIdxMax ? 0 : asInt();
    default:
        return asInt();
    }
}

real sVariant::asReal() const
{
    switch(_type) {
    case value_NULL:
        return 0;
    case value_BOOL:
        return _val.i ? 1 : 0;
    case value_INT:
        return _val.i;
    case value_UINT:
        return _val.u;
    case value_REAL:
        return _val.r;
    case value_STRING:
        return stringToReal(AS_STRING_CONST);
    case value_LIST:
        return _val.list->dim();
    case value_DIC:
        return _val.dic->dim();
    case value_HIVE_ID:
        return _val.id()->objId();
    case value_DATE_TIME:
    case value_DATE:
        return _val.date.timestamp == -sIdxMax ? NAN : _val.date.timestamp;
    case value_TIME:
        return _val.date.timestamp;
    case value_DATA:
        return _val.data->asReal();
    default:
        break;
    }
    return NAN;
}

const sHiveId * sVariant::asHiveId(sHiveId * out) const
{
    if (!out)
        return _type == value_HIVE_ID ? _val.id() : 0;

    switch(_type) {
    case value_HIVE_ID:
        *out = *_val.id();
        break;
    case value_STRING:
        out->parse(AS_STRING_CONST);
        break;
    default:
        out->set((const char *)0, asUInt(), 0);
    }

    return out;
}

idx sVariant::asHiveIds(sVec<sHiveId> & out) const
{
    idx start_dim = out.dim();
    switch(_type) {
        case value_NULL:
            return 0;
        case value_STRING:
            sHiveId::parseRangeSet(out, AS_STRING_CONST);
            break;
        case value_LIST:
            out.add(dim());
            for(idx i=0; i<dim(); i++) {
                getListElt(i)->asHiveId(out.ptr(i + start_dim));
            }
            break;
        default:
            asHiveId(out.add(1));
    }
    return out.dim() - start_dim;
}

static inline idx asDateTimeFromUnix(struct tm * out, idx unix_time)
{
    time_t t = unix_time;
    return localtime_r(&t, out) ? unix_time : -sIdxMax;
}

idx sVariant::asDateTime(struct tm * out) const
{
    struct tm * tm = out, tmp;
    if (!tm) {
        time_t t = 0;
        localtime_r(&t, &tmp);
        tm = &tmp;
    }
    switch(_type) {
    case value_NULL:
    case value_LIST:
    case value_DIC:
        return -sIdxMax;
    case value_REAL:
        return isnan(_val.r) ? asDateTimeFromUnix(tm, asInt()) : -sIdxMax;
    case value_DATE_TIME:
        return makeLocalizedDateTime(tm, _val.date.timestamp, _val.date.utc_offset);
    case value_DATE:
        return makeLocalizedDateTime(tm, _val.date.timestamp, -sIdxMax);
    case value_TIME:
        if (out) {
            out->tm_hour = out->tm_min = out->tm_sec = 0;
            idx unix_time = mktime(out) + _val.date.timestamp;
            return makeLocalizedDateTime(out, unix_time, _val.date.utc_offset);
        } else {
            return _val.date.timestamp;
        }
    case value_STRING:
        return sString::parseDateTime(tm, AS_STRING_CONST);
    case value_DATA:
        {
            sStr buf;
            print(buf);
            return sString::parseDateTime(tm, buf.ptr());
        }
    default:
        return asDateTimeFromUnix(tm, asInt());
    }
}

void sVariant::getNumericValue(sVariant& numeric) const
{
    switch(_type) {
    case value_NULL:
        numeric.setInt(0);
        break;
    case value_BOOL:
        numeric.setInt(_val.i ? 1 : 0);
        break;
    case value_INT:
        numeric.setInt(_val.i);
        break;
    case value_REAL:
        numeric.setReal(_val.r);
        break;
    case value_STRING:
        {
            bool err;
            idx ival = stringToInt(AS_STRING_CONST, &err);
            if (err)
                numeric.setReal(stringToReal(AS_STRING_CONST));
            else
                numeric.setInt(ival);
        }
        break;
    case value_LIST:
    case value_DIC:
        numeric.setInt(asInt());
        break;
    case value_UINT:
        numeric.setUInt((idx)(_val.u));
        break;
    case value_HIVE_ID:
        numeric.setUInt(asUInt());
        break;
    case value_DATE_TIME:
    case value_DATE:
    case value_TIME:
        if (_val.date.timestamp == -sIdxMax) {
            numeric.setReal(NAN);
        } else {
            numeric.setInt(_val.date.timestamp);
        }
        break;
    case value_DATA:
        _val.data->getNumericValue(numeric);
        break;
    }
}

static void indent(sStr & s, const sVariant::Whitespace * w, idx indent_len, idx newline_len, idx level)
{
    if( newline_len ) {
        s.add(w->newline, newline_len);
    }
    if( indent_len ) {
        for (idx i=0; i<level; i++)
            s.add(w->indent, indent_len);
    }
}

static void printPrettyListSep(sStr & s, const sVariant::Whitespace * w, idx indent_len, idx newline_len, idx level, bool is_multiline, idx irow)
{
    if (is_multiline) {
        if (irow)
            s.add(",", 1);
        indent(s, w, indent_len, newline_len, level + 1);
    } else if (irow) {
        s.add(",", 1);
        s.addString(w->space);
    }
}

static bool validUnquotedKey(const char * s)
{
    if( !s || !*s ) {
        return false;
    }

    if( !((s[0] >= 'a' && s[0] <= 'z') || (s[0] >= 'A' && s[0] <= 'Z') || s[0] == '_') ) {
        return false;
    }

    for(idx i=1; s[i]; i++) {
        if( !((s[0] >= 'a' && s[0] <= 'z') || (s[0] >= 'A' && s[0] <= 'Z') || (s[0] >= '0' && s[0] <= '9') || s[0] == '_') ) {
                return false;
            }
    }
    return true;
}

const char * sVariant::print(sStr &s, ePrintMode mode, const Whitespace * whitespace, idx indent_level) const
{
    idx start_pos = s.length();
    sStr tmp;

    if (!whitespace)
        whitespace = getDefaultWhitespace(mode);

    switch(_type) {
    case value_NULL:
        switch (mode) {
        case eDefault:
        case eUnquoted:
            return s.addString("0", 1);
        case eCSV:
            return s.add0cut();
        case eJSON:
            return s.addString("null", 4);
        }
        break;
    case value_BOOL:
        switch (mode) {
        case eCSV:
            return s.printf(_val.i ? "1" : "0");
        case eDefault:
        case eUnquoted:
        case eJSON:
            return s.printf(_val.i ? "true" : "false");
        }
        break;
    case value_INT:
        return s.printf("%" DEC, _val.i);
    case value_UINT:
        return s.printf("%" UDEC, _val.u);
    case value_REAL:
        if( mode == eJSON ) {
            switch( fpclassify(_val.r) ) {
                case FP_NAN:
                    return s.addString("null", 4);
                case FP_INFINITE:
                    return _val.r > 0 ? s.addString("\"Infinity\"") : s.addString("\"-Infinity\"");
                default:
                    break;
            }
        }
        return s.printf("%g", _val.r);
    case value_STRING:
        switch (mode) {
        case eDefault:
            return sString::escapeForC(s, AS_STRING_CONST);
        case eUnquoted:
            return s.addString(AS_STRING_CONST);
        case eCSV:
            return sString::escapeForCSV(s, AS_STRING_CONST);
        case eJSON:
            return sString::escapeForJSON(s, AS_STRING_CONST);
        }
        break;
    case value_LIST:
        switch (mode) {
        case eCSV:
            if (_val.list->dim()) {
                ePrintMode item_mode = eDefault;
                for (idx i=0; i<_val.list->dim(); i++) {
                    if (!_val.list->get(i).isScalar()) {
                        item_mode = eJSON;
                        break;
                    }
                }
                if (item_mode == eJSON) {
                    tmp.add("[", 1);
                }
                for (idx i=0; i<_val.list->dim(); i++) {
                    if (i) {
                        tmp.add(",", 1);
                        tmp.addString(whitespace->space);
                    }
                    _val.list->get(i).print(tmp, item_mode, whitespace, indent_level + 1);
                }
                if (item_mode == eJSON) {
                    tmp.addString("]", 1);
                }
                sString::escapeForCSV(s, tmp.ptr(), tmp.length());
            } else {
                s.add0cut();
            }
            break;
        default:
            {
                idx indent_len = sLen(whitespace->indent);
                idx newline_len = sLen(whitespace->newline);
                bool is_multiline = false;
                if (indent_len && newline_len) {
                    for (idx i=0; i<_val.list->dim(); i++) {
                        if (!_val.list->get(i).isScalar() && _val.list->get(i).dim()) {
                            is_multiline = true;
                            break;
                        }
                    }
                }
                s.add("[", 1);
                for (idx i=0; i<_val.list->dim(); i++) {
                    printPrettyListSep(s, whitespace, indent_len, newline_len, indent_level, is_multiline, i);
                    _val.list->get(i).print(s, mode, whitespace, indent_level + 1);
                }
                if (is_multiline)
                    indent(s, whitespace, indent_len, newline_len, indent_level);
                s.addString("]", 1);
                break;
            }
        }
        break;
    case value_DIC:
        {
            idx indent_len = sLen(whitespace->indent);
            idx newline_len = sLen(whitespace->newline);
            bool is_multiline = false;
            if (mode != eCSV) {
                if (_val.dic->dim() == 1) {
                    is_multiline = !_val.dic->getPtr(0)->isScalar() && _val.dic->getPtr(0)->dim();
                } else {
                    is_multiline = _val.dic->dim() > 1;
                }
            }

            switch (mode) {
            case eDefault:
            case eUnquoted:
                s.add("{", 1);
                for (idx i=0; i<_val.dic->dim(); i++) {
                    printPrettyListSep(s, whitespace, indent_len, newline_len, indent_level, is_multiline, i);
                    const char * key = _val.dic->key(i);
                    if (mode == eUnquoted && validUnquotedKey(key)) {
                        s.addString(key);
                    } else {
                        sString::escapeForC(s, key);
                    }
                    s.add(":", 1);
                    s.addString(whitespace->space);
                    _val.dic->getPtr(i)->print(s, eDefault, whitespace, indent_level + 1);
                }
                if (is_multiline) {
                    indent(s, whitespace, indent_len, newline_len, indent_level);
                }
                s.addString("}", 1);
                break;
            case eCSV:
                if (_val.dic->dim()) {
                    tmp.add("{", 1);
                    for (idx i=0; i<_val.dic->dim(); i++) {
                        printPrettyListSep(tmp, whitespace, indent_len, newline_len, indent_level, is_multiline, i);
                        sString::escapeForJSON(tmp, _val.dic->key(i));
                        tmp.add(":", 1);
                        tmp.addString(whitespace->space);
                        _val.dic->getPtr(i)->print(tmp, eJSON, whitespace, indent_level + 1);
                    }
                    if (is_multiline)
                        indent(tmp, whitespace, indent_len, newline_len, indent_level);
                    tmp.addString("}", 1);
                    sString::escapeForCSV(s, tmp.ptr(), tmp.length());
                } else {
                    s.add0cut();
                }
                break;
            case eJSON:
                s.add("{", 1);
                for (idx i=0; i<_val.dic->dim(); i++) {
                    printPrettyListSep(s, whitespace, indent_len, newline_len, indent_level, is_multiline, i);
                    sString::escapeForJSON(s, _val.dic->key(i));
                    s.add(":", 1);
                    s.addString(whitespace->space);
                    _val.dic->getPtr(i)->print(s, mode, whitespace, indent_level + 1);
                }
                if (is_multiline)
                    indent(s, whitespace, indent_len, newline_len, indent_level);
                s.addString("}", 1);
                break;
            }
        }
        break;
    case value_HIVE_ID:
        switch (mode) {
        case eCSV:
        case eUnquoted:
            _val.id()->print(s);
            break;
        default:
            s.addString("\"", 1);
            _val.id()->print(s);
            s.addString("\"", 1);
        }
        return s.ptr(start_pos);
    case value_DATE_TIME:
        if (_val.date.timestamp == -sIdxMax) {
            if( mode == eJSON ) {
                s.addString("null");
            } else {
                s.add0cut();
            }
        } else {
            idx print_date_time_flags = mode == eJSON ? sString::fISO8601 : 0;
            if (_val.date.utc_offset == -sIdxMax) {
                if (mode != eUnquoted && mode != eCSV) {
                    s.addString("\"");
                }
                sString::printDateTime(s, _val.date.timestamp, print_date_time_flags);
                if (mode != eUnquoted && mode != eCSV) {
                    s.addString("\"");
                }
            } else {
                struct tm tm;
                if (makeLocalizedDateTime(&tm, _val.date.timestamp, _val.date.utc_offset) == -sIdxMax) {
                    s.add0cut();
                } else {
                    if (mode != eUnquoted && mode != eCSV) {
                        s.addString("\"");
                    }
                    sString::printDateTime(s, &tm, print_date_time_flags);
                    if (mode != eUnquoted && mode != eCSV) {
                        s.addString("\"");
                    }
                }
            }
        }
        break;
    case value_DATE:
        if (_val.date.timestamp == -sIdxMax) {
            if( mode == eJSON ) {
                s.addString("null");
            } else {
                s.add0cut();
            }
        } else {
            if (mode != eUnquoted && mode != eCSV) {
                s.addString("\"");
            }
            sString::printDateTime(s, _val.date.timestamp, sString::fNoTime);
            if (mode != eUnquoted && mode != eCSV) {
                s.addString("\"");
            }
        }
        break;
    case value_TIME:
        {
            if (mode != eUnquoted && mode != eCSV) {
                s.addString("\"");
            }
            idx hour = _val.date.timestamp / 3600;
            idx minute = _val.date.timestamp / 60 - hour * 60;
            idx second = _val.date.timestamp - 60 * (60 * hour + minute);
            s.printf("%02" DEC ":%02" DEC ":%02" DEC, hour, minute, second);
            if (_val.date.utc_offset != -sIdxMax) {
                if (_val.date.utc_offset) {
                    idx offset_hour = sAbs(_val.date.utc_offset) / 3600;
                    idx offset_minute = sAbs(_val.date.utc_offset) / 60 - offset_hour * 60;
                    s.printf("%c%02" DEC ":%02" DEC, _val.date.utc_offset > 0 ? '+' : '-', offset_hour, offset_minute);
                } else {
                    s.addString("Z");
                }
            }
            if (mode != eUnquoted && mode != eCSV) {
                s.addString("\"");
            }
        }
        break;
    case value_DATA:
        _val.data->print(s, mode);
        break;
    }
    return s.ptr(start_pos);
}

const char* sVariant::asString()
{
    switch (_type) {
    case value_NULL:
        return sStr::zero;
    case value_STRING:
        return AS_STRING_CONST;
    case value_BOOL:
        return _val.i ? "true" : "false";
    case value_INT:
        if (_val.i >= 0 && _val.i < 10)
            return shortStrings.getChar('0' + _val.i);
        break;
    case value_UINT:
        if (_val.u < 10)
            return shortStrings.getChar('0' + _val.u);
        break;
    default:
        break;
    }
    _str.cut(0);
    print(_str);
    return _str.length() ? _str.ptr() : sStr::zero;
}

idx sVariant::dim() const
{
    switch(_type) {
    case value_STRING:
        return _str.length() ? _str.length() : sLen(AS_STRING_CONST);
    case value_LIST:
        return _val.list->dim();
    case value_DIC:
        return _val.dic->dim();
    default:
        break;
    }
    return 0;
}

const sVariant* sVariant::getListElt(idx i) const
{
    if (likely(isList() && _val.list->has(i)))
        return &(_val.list->get(i));
    return NULL;
}

sVariant* sVariant::getListElt(idx i)
{
    if (likely(isList() && _val.list->has(i)))
        return &(_val.list->get(i));
    return NULL;
}

const sVariant* sVariant::getDicElt(const char *key) const
{
    if (likely(isDic() && key))
        return _val.dic->getPtr(key);
    return NULL;
}

sVariant* sVariant::getDicElt(const char *key)
{
    if (likely(isDic() && key))
        return _val.dic->getPtr(key);
    return NULL;
}

sVariant* sVariant::getDicElt(idx i)
{
    if (unlikely(!isDic() || i < 0 || i >= _val.dic->dim())) {
        return NULL;
    }

    return _val.dic->getPtr(i);
}

const char * sVariant::getDicKeyVal(idx i, sVariant &outValue) const
{
    if (unlikely(!isDic() || i < 0 || i >= _val.dic->dim())) {
        outValue.setNull();
        return NULL;
    }

    outValue = *(_val.dic->getPtr(i));
    return _val.dic->key(i);
}

bool sVariant::getElt(idx i, sVariant &out) const
{
    switch(_type) {
    case value_STRING:
        if (i >= 0 && i < dim()) {
            char s[2] = {0,0};
            s[0] = AS_STRING_CONST[i];
            out.setString(s);
            return true;
        }
        break;
    case value_LIST:
        if (_val.list->has(i)) {
            out = _val.list->get(i);
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

bool sVariant::getElt(const char *key, sVariant &out) const
{
    if (likely(_type == value_DIC)) {
        if (_val.dic->has(key)) {
            out = _val.dic->get(key);
            return true;
        }
    }
    return false;
}

sVariant* sVariant::setElt(idx i, sVariant &elt)
{
    if (likely(_type == value_LIST)) {
        if (unlikely(i < 0))
            return 0;

        _val.list->resize(i+1);
        return &(_val.list->set(i, elt));
    }
    return 0;
}

sVariant* sVariant::setElt(const char *key, sVariant &elt)
{
    if (likely(_type == value_DIC))
        return &(_val.dic->set(key, 0, elt));

    return 0;
}

sVariant* sVariant::setElt(const char *key, idx key_len, sVariant &elt)
{
    if (likely(_type == value_DIC))
        return &(_val.dic->set(key_len ? key : sStr::zero, key_len, elt));

    return 0;
}

sVariant* sVariant::push(sVariant &elt)
{
    if (likely(_type == value_LIST))
        return &(_val.list->add(elt));
    return 0;
}

bool sVariant::append(sVariant &rhs)
{
    if (_type == value_LIST && rhs._type == value_LIST) {
        idx oldDim = _val.list->dim();
        _val.list->resize(oldDim + rhs._val.list->dim());
        for (idx i=0; i<rhs._val.list->dim(); i++)
            _val.list->set(oldDim+i, rhs._val.list->get(i));
        return true;
    } else if (_type == value_STRING) {
        switch (rhs._type) {
        case value_NULL:
            break;
        case value_STRING:
            _str.addString(RHS_AS_STRING_CONST);
            break;
        default:
            rhs.print(_str);
        }
        return true;
    }
    return false;
}

bool sVariant::appendVSprintf(const char *fmt, va_list marker)
{
    if (unlikely(_type != value_STRING))
        return false;
    _str.vprintf(fmt, marker);
    return true;
}

bool sVariant::append(const char *fmt, ...)
{
    if (unlikely(_type != value_STRING))
        return false;
    sCallVarg(appendVSprintf, fmt);
    return true;
}

const char * sVariant::getDicKey(idx i) const
{
    if (unlikely(!isDic() || i < 0 || i >= _val.dic->dim())) {
        return NULL;
    }

    return _val.dic->key(i);
}

bool sVariant::getDicKeys(sVariant &out) const
{
    if (unlikely(_type != value_DIC))
        return false;

    sStr keys00;
    _val.dic->keys(keys00);
    out.setList();
    for (const char *k=keys00.ptr(); k && *k; k=sString::next00(k)) {
        sVariant elt;
        elt.setString(k);
        out.push(elt);
    }

    return true;
}

bool sVariant::getDicKeys(sStr &out00) const
{
    if (unlikely(_type != value_DIC))
        return false;

    _val.dic->keys(out00);
    return true;
}

#define SVARIANT_MATH_OP_IDX(op, var) \
do { \
    switch (_type) { \
    case value_NULL: \
        _type = value_INT; \
        _val.i = 0; \
        _val.i op var;  \
        return true; \
    case value_BOOL: \
        _type = value_INT; \
        _val.i = _val.i ? 1 : 0; \
        _val.i op var; \
        return true; \
    case value_INT: \
        _val.i op var; \
        return true; \
    case value_UINT: \
        _val.u op var; \
        return true; \
    case value_REAL: \
        _val.r op var; \
        return true; \
    case value_DATE_TIME: \
    case value_DATE: \
        _type = value_DATE_TIME; \
        if (_val.date.timestamp != -sIdxMax) { \
            _val.date.timestamp op var; \
        } \
        return true; \
    case value_TIME: \
        _val.date.timestamp op var; \
        if (!timeInRange(_val.date.timestamp)) { \
            setInt(_val.date.timestamp); \
        } \
        return true; \
    default:\
        break; \
    } \
} while(0)

bool sVariant::operator+=(idx i)
{
    SVARIANT_MATH_OP_IDX(+=, i);
    return false;
}

bool sVariant::operator-=(idx i)
{
    SVARIANT_MATH_OP_IDX(-=, i);
    return false;
}

bool sVariant::operator*=(idx i)
{
    SVARIANT_MATH_OP_IDX(*=, i);
    return false;
}

bool sVariant::operator/=(idx i)
{
    if( _type == value_NULL ) {
        _type = value_INT;
        _val.i = 0;
    } else if( _type == value_BOOL ) {
        _type = value_INT;
        _val.i = _val.i ? 1 : 0;
    }

    switch (_type) {
    case value_INT:
        if (i)
            _val.i /= i;
        else {
            _type = value_REAL;
            if ((_val.i >= 0 && i >= 0) || (_val.i < 0 && i < 0))
                _val.r = HUGE_VAL;
            else
                _val.r = -HUGE_VAL;
        }
        return true;
    case value_UINT:
        if (i)
            _val.u /= i;
        else {
            _type = value_REAL;
            _val.r = HUGE_VAL;
        }
        return true;
    case value_REAL:
        _val.r /= i;
        return true;
    case value_DATE_TIME:
    case value_DATE:
        _type = value_DATE_TIME;
        if (i) {
            if (_val.date.timestamp != -sIdxMax) {
                _val.date.timestamp /= i;
            }
        } else {
            _type = value_REAL;
            _val.r = HUGE_VAL;
        }
        return true;
    case value_TIME:
        if (i) {
            _val.date.timestamp /= i;
            if (!timeInRange(_val.date.timestamp)) {
                setInt(_val.date.timestamp);
            }
        } else {
            _type = value_REAL;
            _val.r = HUGE_VAL;
        }
        return true;
    default:
        break;
    }
    return false;
}

bool sVariant::operator%=(idx i)
{
    if( _type == value_NULL ) {
        _type = value_INT;
        _val.i = 0;
    } else if( _type == value_BOOL ) {
        _type = value_INT;
        _val.i = _val.i ? 1 : 0;
    }

    switch (_type) {
    case value_NULL:
    case value_INT:
        if (i)
            _val.i %= i;
        else {
            _type = value_REAL;
            _val.r = NAN;
        }
        return true;
    case value_UINT:
        if (i)
            _val.u %= i;
        else {
            _type = value_REAL;
            _val.r = NAN;
        }
        return true;
    case value_REAL:
        _val.r = remainder(_val.r, i);
        return true;
    case value_DATE_TIME:
    case value_DATE:
        _type = value_DATE_TIME;
        if (i) {
            if (_val.date.timestamp != -sIdxMax) {
                _val.date.timestamp %= i;
            }
        } else {
            _type = value_REAL;
            _val.r = NAN;
        }
        return true;
    case value_TIME:
        if (i) {
            _val.date.timestamp %= i;
            if (!timeInRange(_val.date.timestamp)) {
                setInt(_val.date.timestamp);
            }
        } else {
            _type = value_REAL;
            _val.r = NAN;
        }
        return true;
    default:
        break;
    }
    return false;
}

#define SVARIANT_MATH_OP_REAL(op, var) \
do { \
    switch (_type) { \
    case value_NULL: \
        _type = value_INT; \
        _val.i = 0; \
        _val.r op var; \
        return true; \
    case value_BOOL: \
        _type = value_INT; \
        _val.i = _val.i ? 1 : 0; \
        _val.r op var; \
        return true; \
    case value_INT: \
        _type = value_REAL; \
        _val.r = _val.i; \
        _val.r op var; \
        return true; \
    case value_UINT: \
        _type = value_REAL; \
        _val.r = _val.u; \
        _val.r op var; \
        return true; \
    case value_REAL: \
        _val.r op var; \
        return true; \
    case value_DATE_TIME: \
    case value_DATE: \
    case value_TIME: \
        _type = value_REAL; \
        if (_val.date.timestamp == -sIdxMax) { \
            _val.r = NAN; \
        } else { \
            _val.r = _val.date.timestamp; \
            _val.r op var; \
        } \
        return true; \
    default:\
        break; \
    } \
} while(0)

bool sVariant::operator+=(real r)
{
    SVARIANT_MATH_OP_REAL(+=, r);
    return false;
}

bool sVariant::operator-=(real r)
{
    SVARIANT_MATH_OP_REAL(-=, r);
    return false;
}

bool sVariant::operator*=(real r)
{
    SVARIANT_MATH_OP_REAL(*=, r);
    return false;
}

bool sVariant::operator/=(real r)
{
    SVARIANT_MATH_OP_REAL(/=, r);
    return false;
}

bool sVariant::operator%=(real r)
{
    if( _type == value_NULL ) {
        _type = value_INT;
        _val.i = 0;
    } else if( _type == value_BOOL ) {
        _type = value_INT;
        _val.i = _val.i ? 1 : 0;
    }

    switch (_type) {
    case value_INT:
        _type = value_REAL;
        _val.r = remainder(_val.i, r);
        return true;
    case value_UINT:
        _type = value_REAL;
        _val.r = remainder(_val.u, r);
        return true;
    case value_REAL:
        _val.r = remainder(_val.r, r);
        return true;
    case value_DATE_TIME:
    case value_DATE:
    case value_TIME:
        _type = value_REAL;
        if (_val.date.timestamp == -sIdxMax) {
            _val.r = NAN;
        } else {
            _val.r = remainder(_val.date.timestamp, r);
        }
        return true;
    default:
        break;
    }
    return false;
}

#define SVARIANT_MATH_OP_SVARIANT(op, rhs, op_r_u) \
do { \
    if( _type == value_NULL ) { \
        _type = value_INT; \
        _val.i = 0; \
    } else if( _type == value_BOOL ) { \
        _type = value_INT; \
        _val.i = _val.i ? 1 : 0; \
    } \
    switch(_type) { \
    case value_INT: \
        switch (rhs._type) { \
        case value_NULL: \
            return *this op (idx)0; \
        case value_BOOL: \
            return *this op rhs._val.i ? (idx)1 : (idx)0; \
        case value_INT: \
            return *this op rhs._val.i; \
        case value_UINT: \
            if (rhs._val.u == 0) \
                return *this op (idx)0; \
            _val.i op rhs._val.u; \
            return true; \
        case value_REAL: \
            _type = value_REAL; \
            _val.r = _val.i; \
            return *this op rhs._val.r; \
        case value_DATE_TIME: \
        case value_DATE: \
        case value_TIME: \
            if (rhs._val.date.timestamp == -sIdxMax) { \
                _type = value_REAL; \
                _val.r = NAN; \
                return true; \
            } else { \
                return *this op rhs._val.date.timestamp; \
            } \
        default:\
            break; \
        } \
        break; \
    case value_UINT: \
        switch (rhs._type) { \
        case value_NULL: \
            return *this op (idx)0; \
        case value_BOOL: \
            return *this op rhs._val.i ? (idx)1 : (idx)0; \
        case value_INT: \
            return *this op rhs._val.i; \
        case value_UINT: \
            if (rhs._val.u == 0) \
                return *this op (idx)0; \
            _val.u op rhs._val.u; \
            return true; \
        case value_REAL: \
            _type = value_REAL; \
            _val.r = _val.u; \
            return *this op rhs._val.r; \
        case value_DATE_TIME: \
        case value_DATE: \
        case value_TIME: \
            if (rhs._val.date.timestamp == -sIdxMax) { \
                _type = value_REAL; \
                _val.r = NAN; \
                return true; \
            } else { \
                return *this op rhs._val.date.timestamp; \
            } \
        default:\
            break; \
        } \
        break; \
    case value_REAL: \
        switch (rhs._type) { \
        case value_NULL: \
            return *this op (idx)0; \
        case value_BOOL: \
            return *this op rhs._val.i ? (idx)1 : (idx)0; \
        case value_INT: \
            return *this op rhs._val.i; \
        case value_UINT: \
            op_r_u; \
            return true; \
        case value_REAL: \
            return *this op rhs._val.r; \
        case value_DATE_TIME: \
        case value_DATE: \
        case value_TIME: \
            if (rhs._val.date.timestamp == -sIdxMax) { \
                _val.r = NAN; \
                return true; \
            } else { \
                return *this op rhs._val.date.timestamp; \
            } \
        default:\
            break; \
        } \
        break; \
    case value_DATE_TIME: \
    case value_DATE: \
    case value_TIME: \
        switch (rhs._type) { \
        case value_NULL: \
            return *this op (idx)0; \
        case value_BOOL: \
            return *this op rhs._val.i ? (idx)1 : (idx)0; \
        case value_INT: \
            return *this op rhs._val.i; \
        case value_UINT: \
            return *this op rhs.asInt(); \
        case value_REAL: \
            return *this op rhs._val.r; \
        case value_DATE_TIME: \
        case value_DATE: \
            if (rhs._val.date.timestamp == -sIdxMax) { \
                _type = value_DATE_TIME; \
                _val.date.timestamp = -sIdxMax; \
                return true; \
            } else if (_val.date.timestamp != -sIdxMax) { \
                setInt(_val.date.timestamp); \
                return *this op rhs._val.date.timestamp; \
            } else { \
                return true; \
            } \
        case value_TIME: \
            _type = value_DATE_TIME; \
            return *this op rhs._val.date.timestamp; \
        default:\
            break; \
        } \
        break; \
    default:\
        break; \
    } \
} while(0)

bool sVariant::operator+=(const sVariant &rhs)
{
    SVARIANT_MATH_OP_SVARIANT(+=, rhs, (_val.r += rhs._val.u));
    return false;
}

bool sVariant::operator-=(const sVariant &rhs)
{
    SVARIANT_MATH_OP_SVARIANT(-=, rhs, (_val.r -= rhs._val.u));
    return false;
}

bool sVariant::operator*=(const sVariant &rhs)
{
    SVARIANT_MATH_OP_SVARIANT(*=, rhs, (_val.r *= rhs._val.u));
    return false;
}

bool sVariant::operator/=(const sVariant &rhs)
{
    SVARIANT_MATH_OP_SVARIANT(/=, rhs, (_val.r /= rhs._val.u));
    return false;
}

bool sVariant::operator%=(const sVariant &rhs)
{
    SVARIANT_MATH_OP_SVARIANT(%=, rhs, (_val.r = remainder(_val.r, rhs._val.u)));
    return false;
}

bool sVariant::operator==(const sVariant &rhs) const
{
    if( isNull() ) {
        return rhs.isScalar() && rhs.isNullish();
    }

    if( rhs.isNull() ) {
        return isScalar() && isNullish();
    }

    if( isIntLike() && rhs.isIntLike() ) {
        return asInt() == rhs.asInt();
    }

    if( isUIntLike() && rhs.isUIntLike() ) {
        return asUInt() == rhs.asUInt();
    }

    if (isNumeric() && rhs.isNumeric())
        return asReal() == rhs.asReal();

    if (isString() && rhs.isString())
        return !strcmp(AS_STRING_CONST, RHS_AS_STRING_CONST);

    if (isList() && rhs.isList()) {
        if (_val.list->dim() != rhs._val.list->dim())
            return false;

        for (idx i=0; i<_val.list->dim(); i++) {
            if (_val.list->get(i) != rhs._val.list->get(i))
                return false;
        }
        return true;
    }

    if (isDic() && rhs.isDic()) {
        if (_val.dic->dim() != rhs._val.dic->dim())
            return false;

        for (idx i=0; i<_val.dic->dim(); i++) {
            const char * k = _val.dic->key(i);
            const sVariant * l = _val.dic->getPtr(i);
            const sVariant * r = rhs._val.dic->getPtr(k);
            if (!l || !r || *l != *r)
                return false;
        }
        for (idx i=0; i<rhs._val.dic->dim(); i++) {
            const char * k = rhs._val.dic->key(i);
            const sVariant * l = _val.dic->getPtr(k);
            const sVariant * r = rhs._val.dic->getPtr(i);
            if (!l || !r || *l != *r)
                return false;
        }
        return true;
    }

    if( isHiveId() && rhs.isHiveId() ) {
        return *_val.id() == *rhs._val.id();
    }

    if ((isDateTime() || isDate()) && (rhs.isDateTime() || rhs.isDate()))
        return _val.date.timestamp == rhs._val.date.timestamp;

    if (isTime() && rhs.isTime())
        return _val.date.timestamp == rhs._val.date.timestamp;

    if (isData() && rhs.isData())
        return *asData() == *(rhs.asData());

    return false;
}

#define SVARIANT_COMPARABLE_CMP(op, rhs) \
do { \
    if (isIntLike() && rhs.isIntLike()) \
        return asInt() op rhs.asInt(); \
\
    if (isUIntLike() && rhs.isUIntLike()) \
        return asUInt() op rhs.asUInt(); \
\
    if (isNumeric() && rhs.isNumeric()) \
        return asReal() op rhs.asReal(); \
\
    if (isNull() || rhs.isNull()) \
        return (int)!isNullish() op (int)!rhs.isNullish(); \
\
    if (isString() && rhs.isString()) \
        return strcmp(AS_STRING_CONST, RHS_AS_STRING_CONST) op 0; \
\
    if (isHiveId() && rhs.isHiveId()) \
        return *_val.id() op *rhs._val.id(); \
\
    if ((isDateTime() || isDate()) && (rhs.isDateTime() || rhs.isDate())) \
        return _val.date.timestamp op rhs._val.date.timestamp; \
\
    if (isTime() && rhs.isTime()) \
        return _val.date.timestamp op rhs._val.date.timestamp; \
\
    if (isData() && rhs.isData()) \
        return *asData() op *(rhs.asData()); \
\
    if (isList() && rhs.isList()) { \
        idx minDim = sMin<idx>(_val.list->dim(), rhs._val.list->dim()); \
        for (idx i=0; i<minDim; i++) { \
            if (!(_val.list->get(i) op rhs._val.list->get(i))) \
                return false; \
        } \
        return _val.list->dim() op rhs._val.list->dim(); \
    } \
} while(0)

bool sVariant::operator<(const sVariant &rhs) const
{
    SVARIANT_COMPARABLE_CMP(<, rhs);

    sStr lstr, rstr;
    print(lstr);
    rhs.print(rstr);
    return strcmp(lstr.ptr(), rstr.ptr()) < 0;
}

bool sVariant::operator>(const sVariant &rhs) const
{
    SVARIANT_COMPARABLE_CMP(>, rhs);

    sStr lstr, rstr;
    print(lstr);
    rhs.print(rstr);
    return strcmp(lstr.ptr(), rstr.ptr()) >= 0;
}

idx sVariant::cmp(const sVariant &rhs) const
{
    if (isIntLike() && rhs.isIntLike()) {
        return asInt() - rhs.asInt();
    } else if (isUIntLike() && rhs.isUIntLike()) {
        udx ulhs = asUInt();
        udx urhs = rhs.asUInt();
        if( ulhs < urhs ) {
            return -1;
        } else if( ulhs > urhs ) {
            return 1;
        } else {
            return 0;
        }
    } else if (isNumeric() && rhs.isNumeric()) {
        real diff = asReal() - rhs.asReal();
        return diff > 0 ? ceil(diff) : floor(diff);
    } else if (isNull() || rhs.isNull()) {
        return (idx)!isNullish() - (idx)!rhs.isNullish();
    } else if (isString() && rhs.isString()) {
        return strcmp(AS_STRING_CONST, RHS_AS_STRING_CONST);
    } else if (isHiveId() && rhs.isHiveId()) {
        if (*_val.id() == *rhs._val.id()) {
            return 0;
        } else if (*_val.id() > *rhs._val.id()) {
            return 1;
        } else {
            return -1;
        }
    } else if ((isDateTime() || isDate()) && (rhs.isDateTime() || rhs.isDate())) {
        return _val.date.timestamp - rhs._val.date.timestamp;
    } else if (isTime() && rhs.isTime()) {
        return _val.date.timestamp - rhs._val.date.timestamp;
    } else if (isData() && rhs.isData()) {
        if (*asData() == *rhs.asData()) {
            return 0;
        } else if (*asData() > *rhs.asData()) {
            return 1;
        } else {
            return -1;
        }
   } else if (isList() && rhs.isList()) {
        idx minDim = sMin<idx>(_val.list->dim(), rhs._val.list->dim());
        for (idx i=0; i<minDim; i++) {
            if (idx result = _val.list->get(i).cmp(rhs._val.list->get(i))) {
                return result;
            }
        }
        return _val.list->dim() - rhs._val.list->dim();
    }

    sStr lstr, rstr;
    print(lstr);
    rhs.print(rstr);
    return strcmp(lstr.ptr(), rstr.ptr());
}

void sVariant::internString(const char *s, idx len)
{
    if (!len && s && *s) {
        len = strlen(s);
    }
    shortStrings.intern(s, len);
}
