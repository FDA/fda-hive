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
#ifndef sLib_utils_cron_h
#define sLib_utils_cron_h

#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>

#include <time.h>

namespace slib {
    class sCronTime {
        public:
            sCronTime(const char * s = 0)
            {
                _parsed = false;
                parse(s);
            }

            void reset()
            {
                _ary.empty();
                sSetArray(_poses);
                _parsed = false;
            }

            bool parse(const char * s, idx * plen_parsed = 0);
            bool parsed() const
            {
                return _parsed;
            }

            const char * print(sStr & buf) const;
            const char * print() const;

            bool matches(const struct tm * tim) const;
            bool matches(time_t t) const;

            time_t nextMatch(struct tm * tim) const;
            time_t nextMatch(time_t t) const;

            time_t prevMatch(struct tm * tim) const;
            time_t prevMatch(time_t t) const;

        private:
            sVec<idx> _ary;
            sMex::Pos _poses[5];
            bool _parsed;
            enum EPos {
                eMinutes,
                eHours,
                eDaysOfMonth,
                eMonths,
                eDaysOfWeek
            };
            static const int min_values[5], max_values[5];

            idx findNextAryIndex(EPos kind, idx qry) const;
            idx findPrevAryIndex(EPos kind, idx qry) const;
    };
};

#endif
