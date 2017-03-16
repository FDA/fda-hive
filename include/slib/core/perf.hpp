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
#ifndef sLib_core_perf_h
#define sLib_core_perf_h

#include <slib/core/dic.hpp>
#include <slib/core/tim.hpp>

namespace slib
{

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ class sPerf
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    class sPerf : public sDic < idx, 8 >
    {
        struct Element {idx microseconds, counter;} ;
        sVec < idx > clkOn;
        sDic < Element > clkDic;

        sStr nodL;

    public: // constructor/destructor
        sPerf( ) { }
    public:
        inline void start (const char * node, bool closenext=false) {
            if(closenext)end();
            idx * pclk=clkOn.add(1);
            nodL.printf("/%s",node);
            *pclk=sTime::sysclock(gPerfWallClock);
        }

        inline void end(void) {
            idx tm=sTime::sysclock(gPerfWallClock);
            idx cnt=clkOn.dim()-1;
            idx * pclk=clkOn.ptr(cnt);
            idx diff=tm-(*pclk);
            clkOn.cut(cnt);

            char * nodeSlash=strrchr(nodL.ptr(),'/'), *node;
            if(nodeSlash) node=nodeSlash+1; else node=nodL.ptr();
            Element * el=clkDic.set(node);
            el->microseconds+=diff;
            ++el->counter;
            if(nodeSlash) {*nodeSlash=0; nodL.cut(nodeSlash-nodL.ptr());}
        }


       public: // serialization
        const char * printf(sStr * out=0){
            if(!clkDic.dim() ) return 0;
            idx sum=0;
            for ( idx i =0 ; i< clkDic.dim() ; ++i ){
                Element * el=clkDic.ptr(i);
                const char * id=(const char  *)clkDic.id(i);
                if(out)out->printf("%"DEC",%s,%.2lf/%"DEC"/%.2lf\n",i,id,el->microseconds*(1.e-6),el->counter,el->microseconds*1./el->counter);
                else ::printf("%"DEC",%s,%.2lf/%"DEC"/%.2lf\n",i,id,el->microseconds*(1.e-6),el->counter,el->microseconds*1./el->counter);
                sum+=el->microseconds;
            }
            time_t tt;
            time(&tt);
            struct tm & t = *localtime(&tt);
            if(out)out->printf("\n%"DEC"/%"DEC"/%"DEC" %"DEC":%"DEC":%"DEC" \n", (idx)t.tm_mday, (idx)t.tm_mon + 1,    (idx)t.tm_year + 1900, (idx)t.tm_hour, (idx)t.tm_min, (idx)t.tm_sec);
            else ::printf("\n%"DEC"/%"DEC"/%"DEC" %"DEC":%"DEC":%"DEC" \n", (idx)t.tm_mday, (idx)t.tm_mon + 1,    (idx)t.tm_year + 1900, (idx)t.tm_hour, (idx)t.tm_min, (idx)t.tm_sec);

            if(out)out->printf("\n%.2lf total\n\n",sum*(1.e-6));
            else ::printf("\n%.2lf total\n\n",sum*(1.e-6));
            if(out)return out->ptr(0);
            else return 0;
        }

        static sPerf gPerf;
        static idx gDebugStart, gDebugEnd, gDebugCurrent;
        static bool gPerfWallClock; // true if gPerf should measure wall clock time; false if gPerf should measure CPU time used by process
    };
    //sOutClass(sVec)

    #ifdef SLIB_PERF
        #define PERF_START(_v_nm) sPerf::gPerf.start((_v_nm))
        #define PERF_NEXT(_v_nm) sPerf::gPerf.start((_v_nm),true)
        #define PERF_END() sPerf::gPerf.end()
        #define PERF_PRINT() sPerf::gPerf.printf()
        #define DEBUG_START(_v_cond)    if((_v_cond) && sPerf::gDebugStart<=sPerf::gDebugCurrent && sPerf::gDebugCurrent<=sPerf::gDebugEnd){
        #define DEBUG_END()            }
    #else
        #define PERF_START(_v_nm)
        #define PERF_NEXT(_v_nm)
        #define PERF_END()
        #define PERF_PRINT()
        #define DEBUG_START(_v_cond) if(0){
        #define DEBUG_END()         }
    #endif

    /*
    #else
        #define PERF_START(_v_nm) ::printf("PERF-%s\n",(_v_nm))
        #define PERF_NEXT(_v_nm) ::printf("PERF-%s\n",(_v_nm))
        #define PERF_END()
        #define PERF_PRINT()
        #define DEBUG_START(_v_cond) if(0){
        #define DEBUG_END()         }
    #endif
*/

}


#endif // sLib_core_perf_h


