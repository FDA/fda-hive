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
#ifndef sHiveannot_hpp
#define sHiveannot_hpp

#include <slib/std.hpp>
#include <ssci/bio.hpp>
#include <ulib/ulib.hpp>

#include <ssci/bio/bioseqsnp.hpp>
#include "hiveutils.hpp"
#include "hiveion.hpp"


namespace sviolin
{
    class sHiveannot
    {
        public:
            struct sequenceInfo{
                sDic <idx> idList;
                sDic <const char *> sequenceList;
            };

            static idx outInfo(sStr & output, const char * inputRangeTable, sVec< sVioAnnot > & anotList);


        public:

            static void InitAnnotList( sUsr * user, sVec < sVioAnnot > & annotList,sVec < sHiveId > * annotIDListToUse=0, bool getAll=false);
            static void getAnnotListFromIdAndIdType(sUsr * user, const char * idTypeToUse, const char * idToUse, sVec < sVioAnnot > * annotListOut=0, sStr * tableOut=0);

            sHiveannot(sUsr * usr, const char * annotation_source, const char * annotation_enrichment, sVec < sHiveId > * infoSrcList=0, sStr * srcNames=0, sDic < idx > * columnIds=0);
            sHiveIon hionAnnot;
            sVec < sHiveUtils::TBLINFO > infoSet;
            sStrT gene_name,lBuf,Chromosome;
            const char * mapAnnotInfo(void){
                sIonWander * wander=hionAnnot.wanderList.get("mapGenePos");
                return (wander && wander->traverseBuf.length()) ? wander->traverseBuf.ptr() : 0;
            }
            const char * gene(){return gene_name.length() ? gene_name.ptr() : "";};
            const char * chromosome(){return Chromosome.length() ? Chromosome.ptr() : "";};
            idx mapPosToGeneInfo(sVar * var , const char * refid, idx idlen,  idx start ,idx end, idx varStart, idx varEnd);
            const char * value(const char * var, sStr * dst=0, bool cleanBuf=true);


    };

}
#endif
