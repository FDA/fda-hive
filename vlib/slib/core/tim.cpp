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

#include <pthread.h>
#include <unistd.h>

#include <slib/core/def.hpp>
#include <slib/core/atomic.hpp>
#include <slib/core/tim.hpp>

using namespace slib;

idx sTime::_timeCoarse = -sIdxMax;
static idx threadTimeCoarseRunning = 0;
static pthread_t threadTimeCoarse;

static void * updateTimeCoarse(void * param)
{
    idx * ptimeCoarse = static_cast<idx*>(param);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    sAtomicStore(&threadTimeCoarseRunning, 1);

    while( true ) {
        sAtomicStore(ptimeCoarse, sTime::systime());
        sleepMS(1000);
    }
    return 0;
}

//static
void sTime::initTimeCoarse(void)
{
    if( threadTimeCoarseRunning ) {
        return;
    }

    _timeCoarse = systime();
    if( pthread_create(&threadTimeCoarse, 0, updateTimeCoarse, &_timeCoarse) == 0 ) {
        sAtomicStore(&threadTimeCoarseRunning, 1);
    }
}

//static
bool sTime::cancelTimeCoarse(void)
{
    if( threadTimeCoarseRunning && pthread_cancel(threadTimeCoarse) == 0 ) {
        threadTimeCoarseRunning = 0;
        _timeCoarse = -sIdxMax;
        return true;
    }
    return false;
}
