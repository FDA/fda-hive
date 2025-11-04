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
#ifndef QPSVC_DNAALIGNMENTREMAPPER_hpp
#define QPSVC_DNAALIGNMENTREMAPPER_hpp

#include <qpsvc/qpsvc.hpp>

class QPSvcDnaAlignmentRemapper: public sQPSvc
{
        typedef sQPSvc TParent;

    public:

        QPSvcDnaAlignmentRemapper(sQPride & qp, const char *alId, const sHiveId &msaId, const char * resultsFilename, const char * serviceToBeLaunched, udx alCnt , udx chnkSize)
            : TParent(qp), m_totSize(alCnt), m_maxChunkSize(chnkSize)
        {
            setAlignmentId(alId);
            setMultipleAlignmentId(msaId);
            setResultsFilename(resultsFilename);
            setServiceToBeLaunched(serviceToBeLaunched);

            setVar("splitType","alignments");
            setVar("splitSize","200000");
            setVar("splitField","parent_proc_ids");
        }

        virtual ~QPSvcDnaAlignmentRemapper()
        {
        }

        void setAlignmentId(const sHiveId & objId)
        {
            setVar("alignment", "%s", objId.print());
        }
        void setMultipleAlignmentId(const sHiveId & qryId)
        {
            setVar("multipleAlignment", "%s", qryId.print());
        }
        void setResultsFilename(const char * file)
        {
            setVar("resultsFileName", "%s", file);
        }
        void setServiceToBeLaunched(const char * serviceToBeLaunched)
        {
            setVar("serviceToBeLaunched", "%s", serviceToBeLaunched);
        }
        void setObjDestination(const sHiveId & objId)
        {
            setVar("obj", "%s", objId.print());
        }
        virtual const char* getSvcName() const
        {
            return "dna-alignment-remapper";
        }

        udx m_totSize;
        udx m_maxChunkSize;
};

#endif
