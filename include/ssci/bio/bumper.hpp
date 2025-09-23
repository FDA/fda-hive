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
#ifndef BUMPER_H_
#define BUMPER_H_

#include <slib/core.hpp>

namespace slib {
    class Bumper
    {
        public:
            Bumper(idx _referenceStart, idx _referenceEnd, idx _width, idx _resolution, idx _annotationDensity, idx _maxLayers)
            {
                referenceStart = _referenceStart;
                referenceEnd = _referenceEnd ;
                width = _width;
                resolution = _resolution ;
                if (resolution < 1) resolution = 1;
                annotationDensity = _annotationDensity ;
                maxLayers = _maxLayers ;

                nucleotideChunkSize = (referenceEnd - referenceStart) / resolution;
                pixelPerChunk = width / resolution;
                idx _start = _referenceStart;
                idx _end = _referenceStart + (nucleotideChunkSize - 1);
                for(idx i = 0; i < resolution; i++) {
                    chunk * _chunk = annotationChunks.add();
                    _chunk->chunkStart = _start;
                    _chunk->chunkEnd = _end;
                    _start = _end + 1;
                    _end += nucleotideChunkSize;

                }

                annotations.add(resolution);

            } ;
            ~Bumper() {} ;

            idx add(idx _annotationStart, idx _annotationEnd, sStr & _source, sStr & _seqID, sStr & idTypeId);
            idx print(sStr & output,bool printHeader=false);

            struct annot
            {
                    sStr seqID;
                    sStr source;
                    sStr idTypeId;
                    idx start;
                    idx end;
                    idx virtualStart;
                    idx virtualEnd;
            };

            struct chunk
            {
                    idx chunkStart;
                    idx chunkEnd;
                    sDic <idx> annotationsAdded;
                    bool lookFor(idx start)
                    {
                        if( start >= chunkStart && start <= chunkEnd )
                            return true;
                        else
                            return false;
                    };
            };

        protected:
            annot annotationToInsert;
            idx referenceStart;
            idx referenceEnd;
            idx width;
            idx resolution;
            idx annotationDensity;
            idx nucleotideChunkSize;
            idx pixelPerChunk;
            idx maxLayers;

            sVec<chunk> annotationChunks;
             sVec <sVec < annot> >  annotations;

            idx loopAnnots(idx current_layer, idx thisChunk);

    };
}
#endif
