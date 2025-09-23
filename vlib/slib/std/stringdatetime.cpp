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

#include <ctype.h>
#include <time.h>

#include <slib/std/string.hpp>

using namespace slib;

inline static idx makeAM(idx hour)
{
    if( hour == 12 ) {
        return 0;
    } else if( hour > 12 ) {
        return -sIdxMax;
    } else {
        return hour;
    }
}

inline static idx makePM(idx hour)
{
    if( hour == 12 ) {
        return 12;
    } else if( hour < 12 ) {
        return hour + 12;
    } else {
        return -sIdxMax;
    }
}

idx sString::parseTime(const char * s, idx * pLenParsed, idx * pExplicitUtcOffset)
{
    const char * s_initial = s;
    char * end = 0;

    if( pLenParsed )
        *pLenParsed = 0;

    if( !s )
        return -sIdxMax;

    for( ; isspace(*s); s++ );
    idx hour = strtoidx(s, &end, 10);
    if( hour < 0 || hour > 24 || *end != ':' )
        return -sIdxMax;
    s = end + 1;
    idx minute = strtoidx(s, &end, 10);
    if( minute < 0 || minute > 60 )
        return -sIdxMax;
    idx second = 0;
    if( *end == ':' ) {
        s = end + 1;
        second = strtoidx(s, &end, 10);
        if( second < 0 || second > 61 )
            return -sIdxMax;
        s = end;
        if( *s == '.' ) {
            for( s++; isdigit(*s) || isspace(*s); s++ );
        }
    } else {
        for( s = end + 1; isspace(*s); s++ );
    }

    idx ampm = -1;
    static const char * ampm00 = "am" _ "a.m." _ "pm" _ "p.m." __;
    idx ampm_len = sString::compareChoice(s, ampm00, &ampm, true, 0, false);
    if( ampm_len > 0 ) {
        if( ampm == 0 || ampm == 1 ) {
            hour = makeAM(hour);
        } else {
            hour = makePM(hour);
        }
        if( hour < 0 )
            return -sIdxMax;

        for( s += ampm_len; isspace(*s); s++ );
    }

    for( ; isspace(*s); s++ );
    idx utc_offset = -sIdxMax;
    if( *s == 'z' || *s == 'Z' ) {
        utc_offset = 0;
        s++;
    } else if( *s == '+' || *s == '-' ) {
        bool negate = *s == '-';
        s++;
        idx utc_hr_offset = strtoidx(s, &end, 10);
        idx utc_min_offset = 0;
        if( end - s == 2 ) {
            if( *end == ':' ) {
                s = end + 1;
                utc_min_offset = strtoidx(s, &end, 10);
                if( end - s != 2 )
                    return -sIdxMax;
            } else {
                utc_min_offset = 0;
            }
        } else if( end - s == 4 ) {
            utc_min_offset = utc_hr_offset % 100;
            utc_hr_offset /= 100;
        } else {
            return -sIdxMax;
        }

        if( utc_hr_offset < 0 || utc_min_offset < 0 )
            return -sIdxMax;

        utc_offset = utc_hr_offset * 60 * 60 + utc_min_offset * 60;

        if( negate ) {
            utc_offset *= -1;
        }
    }

    if( pLenParsed ) {
        *pLenParsed = sMax<const char *>(end, s) - s_initial;
    }
    if( pExplicitUtcOffset ) {
        *pExplicitUtcOffset = utc_offset;
    }

    return 60 * (60 * hour + minute) + second;
}

idx sString::parseTime(struct tm * result, const char * s, idx * pLenParsed, idx * pExplicitUtcOffset)
{
    idx utc_offset = -sIdxMax;
    idx len_parsed = 0;

    idx seconds = sString::parseTime(s, &len_parsed, &utc_offset);

    if( seconds == -sIdxMax ) {
        return -sIdxMax;
    }

    result->tm_hour = 0;
    result->tm_min = 0;
    result->tm_sec = seconds;

    idx ret = -sIdxMax;

    if( utc_offset != -sIdxMax ) {
        time_t timestamp = -1;
#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || defined(timegm)
        timestamp = timegm(result);
#else
        sStr saved_tz;
        saved_tz.addString(getenv("TZ"));
        setenv("TZ", "", 1);
        tzset();
        timestamp = mktime(result);
        if( saved_tz.length() )
            setenv("TZ", saved_tz.ptr(), 1);
        else
            unsetenv("TZ");
        tzset();
#endif
        if( timestamp == -1 )
            return -sIdxMax;
        timestamp -= utc_offset;
        result->tm_isdst = -1;
        if( !localtime_r(&timestamp, result) )
            return -sIdxMax;
        ret = static_cast<idx>(timestamp);
    } else {
        result->tm_isdst = -1;
        ret = static_cast<idx>(mktime(result));
        if( ret == -1 ) {
            return -sIdxMax;
        }
    }

    if( ret != -sIdxMax ) {
        if( pLenParsed ) {
            *pLenParsed = len_parsed;
        }
        if( pExplicitUtcOffset ) {
            *pExplicitUtcOffset = utc_offset;
        }
    }

    return ret;
}

static idx parseDateTimeRFC2822(struct tm * result, const char * s, idx * pLenParsed, idx * pExplicitUtcOffset)
{
    const char * s_initial = s;
    char * end = 0;

    if( isalpha(*s) ) {
        static const char * wdays00 = "Sun," _ "Mon," _ "Tue," _ "Wed," _ "Thu," _ "Fri," _ "Sat," __;
        idx wday = -1;
        sString::compareChoice(s, wdays00, &wday, true, 0, false);
        if( wday < 0 || wday > 6 )
            return -sIdxMax;
        result->tm_wday = static_cast<int>(wday);
        for( s += 4; isspace(*s); s++ );
    }

    idx day = strtoidx(s, &end, 0);
    if( day < 1 || day > 31 )
        return -sIdxMax;
    result->tm_mday = day;
    for( s = end; isspace(*s); s++ );

    static const char * months00 = "Jan" _ "Feb" _ "Mar" _ "Apr" _ "May" _ "Jun" _ "Jul" _ "Aug" _ "Sep" _ "Oct" _ "Nov" _ "Dec" __;
    idx month = -1;
    sString::compareChoice(s, months00, &month, true, 0, false);
    if( month < 0 || month > 11 )
        return -sIdxMax;
    result->tm_mon = static_cast<int>(month);
    for( s +=3; isspace(*s); s++ );

    idx year = strtoidx(s, &end, 0);
    if( end - s != 4 )
        return -sIdxMax;
    result->tm_year = year - 1900;

    idx timestamp_len = 0;
    idx ret = -sIdxMax;
    if( isspace(*end) ) {
        ret = sString::parseTime(result, end, &timestamp_len, pExplicitUtcOffset);
    } else {
        ret = static_cast<idx>(mktime(result));
        if( ret == -1 ) {
            return -sIdxMax;
        }
    }

    if( ret != -sIdxMax ) {
        if( pLenParsed ) {
            *pLenParsed = sMax<const char *>(end, s) + timestamp_len - s_initial;
        }
        if( pExplicitUtcOffset && !timestamp_len ) {
            *pExplicitUtcOffset = -sIdxMax;
        }
    }

    return ret;
}

static inline bool isDateSep(char c)
{
    return c == '-' || c == '/';
}

static inline bool isEnd(char c)
{
    return !c || isspace(c);
}

idx sString::parseDateTime(struct tm * result, const char * s, idx * pLenParsed, idx * pExplicitUtcOffset)
{
    sSet(result, 0);
    result->tm_isdst = -1;

    if( pLenParsed )
        *pLenParsed = 0;

    if( !s )
        return -sIdxMax;

    const char * s_initial = s;
    idx timestamp_len = 0;
    idx ret = -sIdxMax;

    for( ; isspace(*s); s++ );

    char * end = 0;
    idx first_num = strtoidx(s, &end, 10);

    switch (end - s) {
    case 4:
        result->tm_year = first_num - 1900;
        if( isDateSep(*end) ) {
            s = end + 1;
            idx month = strtoidx(s, &end, 10);
            if( month < 1 || month > 12 )
                return -sIdxMax;
            result->tm_mon = month - 1;
            if( isDateSep(*end) ) {
                s = end + 1;
                idx day = strtoidx(s, &end, 10);
                if( day < 1 || day > 31 )
                    return -sIdxMax;
                result->tm_mday = day;
                bool expect_time = false;
                if( *end == 'T' || *end == 't' ) {
                    s = end + 1;
                    expect_time = true;
                } else {
                    for( s = end; isspace(*s); s++ );
                    expect_time = isdigit(*s);
                }
                if( expect_time ) {
                    ret = sString::parseTime(result, s, &timestamp_len, pExplicitUtcOffset);
                    if (ret == -sIdxMax) {
                        return -sIdxMax;
                    }
                }
            }
        }
        if (!result->tm_mday) {
            result->tm_mday = 1;
        }
        if( !timestamp_len ) {
            ret = static_cast<idx>(mktime(result));
            if( ret == -1 ) {
                return -sIdxMax;
            }
        }
        break;
    case 2:
    case 1:
        if( first_num == 0 && isEnd(*end) ) {
            const time_t t = first_num;
            if( !localtime_r(&t, result) ) {
                return -sIdxMax;
            }
        } else if( isDateSep(*end) ) {
            if( first_num < 1 || first_num > 12 )
                return -sIdxMax;
            result->tm_mon = first_num - 1;
            s = end + 1;
            idx second_num = strtoidx(s, &end, 10);
            if( end - s == 4 ) {
                result->tm_year = second_num - 1900;
            } else {
                if( second_num < 1 || second_num > 31 || !isDateSep(*end) )
                    return -sIdxMax;
                result->tm_mday = second_num;
                s = end + 1;
                idx year = strtoidx(s, &end, 10);
                if( end - s != 4 )
                    return -sIdxMax;
                result->tm_year = year - 1900;
                for( s=end; isspace(*s); s++ );
                if( isdigit(*s) ) {
                    ret = sString::parseTime(result, s, &timestamp_len, pExplicitUtcOffset);
                    if( ret == -sIdxMax ) {
                        return -sIdxMax;
                    }
                }
            }
            if (!result->tm_mday) {
                result->tm_mday = 1;
            }
            if( !timestamp_len ) {
                ret = static_cast<idx>(mktime(result));
                if( ret == -1 ) {
                    return -sIdxMax;
                }
            }
        } else {
            return parseDateTimeRFC2822(result, s_initial, pLenParsed, pExplicitUtcOffset);
        }
        break;
    case 0:
        return parseDateTimeRFC2822(result, s_initial, pLenParsed, pExplicitUtcOffset);
    default:
        if( isEnd(*end) ) {
            const time_t t = first_num;
            ret = localtime_r(&t, result) ? first_num : -sIdxMax;
        } else {
            return -sIdxMax;
        }
    }

    if( ret != -sIdxMax ) {
        if( pLenParsed ) {
            *pLenParsed = sMax<const char *>(end, s) + timestamp_len - s_initial;
        }
        if( pExplicitUtcOffset && !timestamp_len ) {
            *pExplicitUtcOffset = -sIdxMax;
        }
    }

    return ret;
}

const char * sString::printDateTime(sStr & out, const struct tm * tm, idx flags)
{
    idx start = out.length();
    if( !(flags & fNoDate) ) {
        out.printf("%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
        if( !(flags & fNoTime) ) {
            out.addString(flags & fISO8601 ? "T" : " ");
        }
    }
    if( !(flags & fNoTime) ) {
        out.printf("%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
        if( !(flags & fNoTimezone) ) {
            idx utc_offset = -sIdxMax;
#if defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
            if( tm->tm_zone ) {
                utc_offset = tm->tm_gmtoff;
            }
#endif
            if( utc_offset == -sIdxMax ) {
                struct tm tm_copy, tm_utc;
                memcpy(&tm_copy, tm, sizeof(struct tm));
                time_t unix_time = mktime(&tm_copy);
                gmtime_r(&unix_time, &tm_utc);
                tm_utc.tm_isdst = -1;
                utc_offset = unix_time - mktime(&tm_utc);
            }
            if( utc_offset ) {
                out.printf("%c%02" DEC ":%02" DEC, utc_offset > 0 ? '+' : '-', sAbs(utc_offset) / 3600, (sAbs(utc_offset) / 60) % 60);
            } else {
                out.addString("Z");
            }
        }
    }
    return out.ptr(start);
}

const char * sString::printDateTime(sStr & out, idx unix_time, idx flags)
{
    time_t t = unix_time;
    struct tm tm;
    sSet(&tm, 0);
    if( localtime_r(&t, &tm) ) {
        return printDateTime(out, &tm, flags);
    } else {
        return out.add0cut();
    }
}
