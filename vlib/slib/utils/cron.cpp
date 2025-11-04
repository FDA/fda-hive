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

#include <slib/utils/cron.hpp>
#include <slib/utils/sort.hpp>

#include <string.h>

using namespace slib;

const int sCronTime::min_values[5] = { 0, 0, 1, 1, 0 };
const int sCronTime::max_values[5] = { 59, 23, 31, 12, 6 };

idx sCronTime::findNextAryIndex(EPos kind, idx qry) const
{
    idx left = _poses[kind].pos, right = _poses[kind].pos + _poses[kind].size;
    while( left < right ) {
        idx mid = (left + right) / 2;
        if( qry <= _ary[mid] ) {
            if( mid == _poses[kind].pos || qry == _ary[mid] || qry > _ary[mid - 1] ) {
                return mid;
            }
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    return -sIdxMax;
}

idx sCronTime::findPrevAryIndex(EPos kind, idx qry) const
{
    idx left = _poses[kind].pos, right = _poses[kind].pos + _poses[kind].size;
    while( left < right ) {
        idx mid = (left + right) / 2;
        if( qry >= _ary[mid] ) {
            if( mid + 1 == _poses[kind].pos + _poses[kind].size || qry == _ary[mid] || qry < _ary[mid + 1] ) {
                return mid;
            }
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return -sIdxMax;
}

bool sCronTime::parse(const char * s, idx * plen_parsed)
{
    reset();

    if( !s ) {
        return false;
    }

    const char * s_initial = s;
    idx force_len_parsed = 0;

    if( strncmp(s, "@yearly", 7) == 0 ) {
        s = "0 0 1 1 *";
        force_len_parsed = 7;
    } else if( strncmp(s, "@annually", 9) == 0 ) {
        s = "0 0 1 1 *";
        force_len_parsed = 9;
    } else if( strncmp(s, "@monthly", 8) == 0 ) {
        s = "0 0 1 * *";
        force_len_parsed = 8;
    } else if( strncmp(s, "@weekly", 7) == 0 ) {
        s = "0 0 * * 0";
        force_len_parsed = 7;
    } else if( strncmp(s, "@daily", 6) == 0 ) {
        s = "0 0 * * *";
        force_len_parsed = 6;
    } else if( strncmp(s, "@midnight", 9) == 0 ) {
        s = "0 0 * * *";
        force_len_parsed = 9;
    } else if( strncmp(s, "@hourly", 7) == 0 ) {
        s = "0 * * * *";
        force_len_parsed = 7;
    }

    sVec<idx> to_sort;
    for(idx i = 0; i < sDim(_poses); i++) {
        to_sort.cut(0);
        _poses[i].pos = _ary.dim();
        _poses[i].size = 0;

        if( i && *s != ' ' ) {
            return false;
        }
        for(; *s == ' '; s++);
        if( s[0] == '*' ) {
            if( s[1] == '/' ) {
                char * endptr = 0;
                idx every = strtoidx(s + 2, &endptr, 10);
                if( every <= 0 ) {
                    return false;
                }
                for(idx val = 0; val <= max_values[i]; val += every) {
                    if( val >= min_values[i] ) {
                        *to_sort.add(1) = val;
                    }
                }
                s = endptr;
            } else {
                s++;
            }
        } else {
            while( 1 ) {
                char * endptr = 0;
                idx val = strtoidx(s, &endptr, 10);
                if( endptr == s || val < min_values[i] || val > max_values[i] ) {
                    return false;
                }
                *to_sort.add(1) = val;
                s = endptr;
                if( *s == '-' ) {
                    s++;
                    idx val2 = strtoidx(s, &endptr, 10);
                    if( endptr == s || val2 < val || val2 < min_values[i] || val2 > max_values[i] ) {
                        return false;
                    }
                    for(val++; val <= val2; val++) {
                        *to_sort.add(1) = val;
                    }
                    s = endptr;
                }

                if( *s == ',' ) {
                    s++;
                } else {
                    break;
                }
            }
        }

        if( to_sort.dim() ) {
            sSort::sort(to_sort.dim(), to_sort.ptr());
            for(idx j = 0, prev = -sIdxMax; j < to_sort.dim(); j++) {
                if( to_sort[j] != prev ) {
                    *_ary.add(1) = prev = to_sort[j];
                    _poses[i].size++;
                }
            }
        }
    }

    _parsed = true;
    if( plen_parsed ) {
        *plen_parsed = force_len_parsed ? force_len_parsed : s - s_initial;
    }
    return true;
}

const char * sCronTime::print(sStr & buf) const
{
    if( !parsed() ) {
        return 0;
    }

    idx buf_start = buf.length();

    for(idx i = 0; i < sDim(_poses); i++) {
        if( i ) {
            buf.addString(" ");
        }
        if( _poses[i].size == 0 ) {
            buf.addString("*");
        } else {
            for(idx j = 0; j < _poses[i].size; j++) {
                if( j ) {
                    buf.addString(",");
                }
                buf.addNum(_ary[_poses[i].pos + j]);
            }
        }
    }

    return buf.ptr(buf_start);
}

const char * sCronTime::print() const
{
    static sStr static_buf;
    static_buf.cut0cut(0);
    return print(static_buf);
}

bool sCronTime::matches(const struct tm * tim) const
{
    if( !_parsed ) {
        return false;
    }
    if( tim->tm_sec ) {
        return false;
    }
    idx timvals[5] = { tim->tm_min, tim->tm_hour, tim->tm_mday, tim->tm_mon + 1, tim->tm_wday };
    for(idx i = 0; i < sDim(_poses); i++) {
        if( !_poses[i].size ) {
            continue;
        }
        idx ary_index = findNextAryIndex((EPos)i, timvals[i]);
        if( ary_index < 0 || _ary[ary_index] != timvals[i] ) {
            return false;
        }
    }
    return true;
}

bool sCronTime::matches(time_t t) const
{
    struct tm tim;
    sSet(&tim);
    localtime_r(&t, &tim);
    return matches(&tim);
}

time_t sCronTime::nextMatch(struct tm * tim) const
{
    if( !parsed() ) {
        return 0;
    }

    tim->tm_sec = 0;
    tim->tm_min++;
    mktime(tim);

    if( _poses[eMinutes].size ) {
        idx ary_index = findNextAryIndex(eMinutes, tim->tm_min);
        if( ary_index >= 0 ) {
            tim->tm_min = _ary[ary_index];
        } else {
            tim->tm_min = _ary[_poses[eMinutes].pos];
            tim->tm_hour++;
            if( tim->tm_hour >= 24 ) {
                mktime(tim);
            }
        }
    }

    if( _poses[eHours].size ) {
        idx ary_index = findNextAryIndex(eHours, tim->tm_hour);
        if( ary_index >= 0 ) {
            tim->tm_hour = _ary[ary_index];
        } else {
            tim->tm_hour = _ary[_poses[eHours].pos];
            tim->tm_mday++;
            mktime(tim);
        }
    }

    int saved_tm_min = tim->tm_min;
    int saved_tm_hour = tim->tm_hour;

    if( _poses[eDaysOfMonth].size || _poses[eMonths].size || _poses[eDaysOfWeek].size ) {
        const bool want_mday = _poses[eDaysOfMonth].size;
        const bool want_month = _poses[eMonths].size;
        const bool want_wday = _poses[eDaysOfWeek].size;
        const bool want_day = want_mday || want_wday;

        while( 1 ) {
            bool matched_day = false, matched_month = false;
            if( want_mday ) {
                idx mday_index = findNextAryIndex(eDaysOfMonth, tim->tm_mday);
                if( mday_index >= 0 && _ary[mday_index] == tim->tm_mday ) {
                    matched_day = true;
                }
            }

            if( want_wday ) {
                idx wday_index = findNextAryIndex(eDaysOfWeek, tim->tm_wday);
                if( wday_index >= 0 && _ary[wday_index] == tim->tm_wday ) {
                    matched_day = true;
                }
            }

            if( want_month ) {
                idx month_index = findNextAryIndex(eMonths, tim->tm_mon + 1);
                if( month_index >= 0 && _ary[month_index] == tim->tm_mon + 1 ) {
                    matched_month = true;
                }
            }

            if( (matched_day || !want_day) && (matched_month || !want_month) ) {
                break;
            }

            tim->tm_mday++;
            mktime(tim);
        }
    }

    tim->tm_min = saved_tm_min;
    tim->tm_hour = saved_tm_hour;

    return mktime(tim);
}

time_t sCronTime::nextMatch(time_t t) const
{
    struct tm tim;
    sSet(&tim);
    localtime_r(&t, &tim);
    return nextMatch(&tim);
}

time_t sCronTime::prevMatch(struct tm * tim) const
{
    if( !parsed() ) {
        return 0;
    }

    if( tim->tm_sec ) {
        tim->tm_sec = 0;
    } else {
        tim->tm_min--;
    }
    mktime(tim);

    if( _poses[eMinutes].size ) {
        idx ary_index = findPrevAryIndex(eMinutes, tim->tm_min);
        if( ary_index >= 0 ) {
            tim->tm_min = _ary[ary_index];
        } else {
            tim->tm_min = _ary[_poses[eMinutes].pos + _poses[eMinutes].size - 1];
            tim->tm_hour--;
            if( tim->tm_hour < 0 ) {
                mktime(tim);
            }
        }
    }

    if( _poses[eHours].size ) {
        idx ary_index = findPrevAryIndex(eHours, tim->tm_hour);
        if( ary_index >= 0 ) {
            tim->tm_hour = _ary[ary_index];
        } else {
            tim->tm_hour = _ary[_poses[eHours].pos + _poses[eHours].size - 1];
            tim->tm_mday--;
            mktime(tim);
        }
    }

    int saved_tm_min = tim->tm_min;
    int saved_tm_hour = tim->tm_hour;

    if( _poses[eDaysOfMonth].size || _poses[eMonths].size || _poses[eDaysOfWeek].size ) {
        const bool want_mday = _poses[eDaysOfMonth].size;
        const bool want_month = _poses[eMonths].size;
        const bool want_wday = _poses[eDaysOfWeek].size;
        const bool want_day = want_mday || want_wday;

        while( 1 ) {
            bool matched_day = false, matched_month = false;
            if( want_mday ) {
                idx mday_index = findPrevAryIndex(eDaysOfMonth, tim->tm_mday);
                if( mday_index >= 0 && _ary[mday_index] == tim->tm_mday ) {
                    matched_day = true;
                }
            }

            if( want_wday ) {
                idx wday_index = findPrevAryIndex(eDaysOfWeek, tim->tm_wday);
                if( wday_index >= 0 && _ary[wday_index] == tim->tm_wday ) {
                    matched_day = true;
                }
            }

            if( want_month ) {
                idx month_index = findPrevAryIndex(eMonths, tim->tm_mon + 1);
                if( month_index >= 0 && _ary[month_index] == tim->tm_mon + 1 ) {
                    matched_month = true;
                }
            }

            if( (matched_day || !want_day) && (matched_month || !want_month) ) {
                break;
            }

            tim->tm_mday--;
            mktime(tim);
        }
    }

    tim->tm_min = saved_tm_min;
    tim->tm_hour = saved_tm_hour;

    return mktime(tim);
}

time_t sCronTime::prevMatch(time_t t) const
{
    struct tm tim;
    sSet(&tim);
    localtime_r(&t, &tim);
    return prevMatch(&tim);
}
