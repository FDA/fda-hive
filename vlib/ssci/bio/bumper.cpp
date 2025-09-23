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
#include <ssci/bio/bumper.hpp>

using namespace slib;

idx Bumper::loopAnnots (idx current_layer, idx thisChunk) {


    idx _vs = annotationToInsert.start / nucleotideChunkSize;
    idx _ve = annotationToInsert.end / nucleotideChunkSize;

    annotationToInsert.virtualStart = _vs * nucleotideChunkSize;
    annotationToInsert.virtualEnd =  (_ve * nucleotideChunkSize)-1;

    if (annotationToInsert.end % nucleotideChunkSize != 0) annotationToInsert.virtualEnd += nucleotideChunkSize;

    for( idx layer = current_layer; layer < annotations[thisChunk].dim(); ++layer) {


            annot & _annotation = annotations[thisChunk][layer];



            if((_annotation.virtualStart > annotationToInsert.virtualEnd) || (_annotation.virtualEnd < annotationToInsert.virtualStart)) {
                continue;
            } else {
                layer++;



                loopAnnots(layer, thisChunk);
                return 0;
       }


    }

       annot * pNewAnnot = annotations[thisChunk].add(1);

       pNewAnnot->end = annotationToInsert.end;
       pNewAnnot->start = annotationToInsert.start;
       pNewAnnot->virtualStart = annotationToInsert.virtualStart;
       pNewAnnot->virtualEnd = annotationToInsert.virtualEnd;
       pNewAnnot->source.printf(0,"%s",annotationToInsert.source.ptr());
       pNewAnnot->seqID.printf(0,"%s",annotationToInsert.seqID.ptr());
       pNewAnnot->idTypeId.printf(0,"%s",annotationToInsert.idTypeId.ptr());

    return 0;
}

idx Bumper::add(idx _annotationStart, idx _annotationEnd, sStr & _source, sStr & _seqID, sStr & _idTypeId){


    annotationToInsert.start = _annotationStart;
    annotationToInsert.end = _annotationEnd;
    annotationToInsert.virtualStart = _annotationStart;
    annotationToInsert.virtualEnd = _annotationEnd;


    annotationToInsert.source.printf(0,"%s",_source.ptr());
    annotationToInsert.seqID.printf(0,"%s",_seqID.ptr());
    annotationToInsert.idTypeId.printf(0,"%s",_idTypeId.ptr());
    idx thisChunk = -1;

    for (idx i = 0; i < annotationChunks.dim(); i++) {
        if (annotationChunks[i].lookFor(annotationToInsert.start)) {
            thisChunk = i;
            break;
        }
    }

    if (thisChunk==-1) {
        ::printf("ERROR - no chunk exists to hold this annotation");
        return 2;
    }

    if (annotations.dim() <= 0) {
        ::printf("ERROR - no default layer!");
        return 3;
    }


    if (annotationChunks[thisChunk].annotationsAdded.find(_source.ptr()) && annotationChunks[thisChunk].annotationsAdded[_source.ptr()] >= annotationDensity)
        return 1;
    annotationChunks[thisChunk].annotationsAdded[_source.ptr()] +=1;
    idx current_layer = 0;
    loopAnnots(current_layer, thisChunk);

    return 0;
}

idx Bumper::print(sStr & output, bool printHeader) {
    output.cut(0);
    if (printHeader){
        output.printf("seqID,start,end,virtual_start,virtual_end,source, idType-Id\n");
    }

    for (idx chunk = 0; chunk < annotations.dim(); chunk++) {
        for (idx layer = 0; layer < annotations[chunk].dim(); ++layer){
            output.printf("%s,%" DEC ",%" DEC ",%" DEC ",%" DEC ",%s,%s\n",annotations[chunk][layer].seqID.ptr() ,annotations[chunk][layer].start ,annotations[chunk][layer].end, annotations[chunk][layer].virtualStart, annotations[chunk][layer].virtualEnd, annotations[chunk][layer].source.ptr(), annotations[chunk][layer].idTypeId.ptr());
        }
    }

    return 0;
}
