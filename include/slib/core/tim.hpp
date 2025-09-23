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
#ifndef sLib_core_tim_h
#define sLib_core_tim_h

#include <slib/core/def.hpp>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

namespace slib {
    class sTime
    {
        public:
            sTime(idx tm = 0)
            {
                _lastValue = tm;
            }

            double clock(sTime * tmrel = 0, idx * tm = 0, bool wallclock = false)
            {
                idx curT;
                double elapsed_time;

                if( !tmrel ) {
                    tmrel = this;
                }
                if( !tm ) {
                    tm = &curT;
                }
                *tm = sysclock(wallclock);
                elapsed_time = (double) ((*tm) - (tmrel->_lastValue));
                _lastValue = *tm;
                return elapsed_time / CLOCKS_PER_SEC;
            }

            idx time(sTime * tmrel = 0, idx * tm = 0, bool coarse = false)
            {
                idx curT;
                idx elapsed_time;

                if( !tmrel ) {
                    tmrel = this;
                }
                if( !tm ) {
                    tm = &curT;
                }
                *tm = coarse ? systimeCoarse() : systime();
                elapsed_time = (*tm) - (tmrel->_lastValue);
                _lastValue = *tm;
                return elapsed_time;
            }

            bool operator !(void)
            {
                return _lastValue ? false : true;
            }

            operator bool(void)
            {
                return _lastValue ? true : false;
            }

        public:
            static inline idx sysclock(bool wallclock = false)
            {
                if( !wallclock ) {
                    return (idx) ::clock();
                }
                struct timeval tv;
                gettimeofday(&tv, NULL);
                return (idx) (CLOCKS_PER_SEC) * tv.tv_sec + tv.tv_usec;
            }
            static inline idx systime(void)
            {
                return (idx) ::time(0);
            }
            static inline idx systimeCoarse(void)
            {
                if( unlikely(_timeCoarse == -sIdxMax) ) {
                    initTimeCoarse();
                }
                return _timeCoarse;
            }

            static inline void setSystimeCoarse(const idx time_secs = 0)
            {
                _timeCoarse = time_secs ? time_secs : systime();
            }

            static bool cancelTimeCoarse(void);

            static inline void randomSleep(idx useconds_max = 500000, idx useconds_min = 0)
            {
                usleep(useconds_min + (real) (rand()) / RAND_MAX * useconds_max);
            }

        private:
            idx _lastValue;
            static idx _timeCoarse;

            static void initTimeCoarse(void);
    };
}

#endif 

