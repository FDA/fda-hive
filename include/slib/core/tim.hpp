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
    //! Timestamps for logging and profiling
    class sTime
    {
        public:
            sTime(idx tm = 0)
            {
                _lastValue = tm;
            }

            //! Seconds of CPU time used by the program since the previous clock() call
            /*! \param tmrel if non-0, measure CPU time used since the previous \a tmrel->clock() call 
             *  \param[out] tm if non-0, retrieves current \ref sysclock "sTime::sysclock(wallclock)" value
             *  \param wallclock if true, will measure wallclock difference instead of CPU time usage */
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

            //! Seconds since the previous time() call
            /*! \param tmrel if non-0, measure time used since the previous \a tmrel->time() call 
             *  \param[out] tm if non-0, retrieves current sTime::systime() or sTime::systimeCoarse() value
             *  \param coarse if true, measure time using coarse low-overhead timer */
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
            //! Nanoseconds of total CPU time used by the program
            /*! \param wallclock if true, return current wallclock value in nanoseconds (can wrap around if idx is 32-bit) */
            static inline idx sysclock(bool wallclock = false)
            {
                if( !wallclock ) {
                    return (idx) ::clock();
                }
                struct timeval tv;
                gettimeofday(&tv, NULL);
                return (idx) (CLOCKS_PER_SEC) * tv.tv_sec + tv.tv_usec;
            }
            //! System time, i.e. seconds since Unix epoch
            static inline idx systime(void)
            {
                return (idx) ::time(0);
            }
            //! Coarse system time for low-overhead checks, updated once per second by a separate thread
            static inline idx systimeCoarse(void)
            {
                if( unlikely(_timeCoarse == -sIdxMax) ) {
                    initTimeCoarse();
                }
                return _timeCoarse;
            }

            //! Manually update coarse system time
            /*! \note Subsequent systimeCoarse() calls will not launch an updater thread (unless that thread is running already) */
            static inline void setSystimeCoarse(const idx time_secs = 0)
            {
                _timeCoarse = time_secs ? time_secs : systime();
            }

            //! Manually stop coarse system time updater thread; this is optional, the thread will stop automatically when the program stops.
            static bool cancelTimeCoarse(void);

            //! Sleep for random interval (in microseconds) from useconds_min to useconds_max
            static inline void randomSleep(idx useconds_max = 500000, idx useconds_min = 0)
            {
                usleep(useconds_min + (real) (rand()) / RAND_MAX * useconds_max);
            }

        private:
            idx _lastValue;
            //! lightweight global time designed to be set by only one thread and read by many w/o locks/atomics/system calls, etc
            static idx _timeCoarse;

            static void initTimeCoarse(void);
    };
}

#endif // sLib_core_tim_h

/*
 inline idx tclock(void) {
 struct timeval etv;
 struct timezone tz;
 gettimeofday(&etv, &tz);
 idx usec = (etv.tv_sec * 1000000 + etv.tv_usec);
 return usec;
 }

 */

