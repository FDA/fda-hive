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
#ifndef QPSVC_DnaHexagon_hpp
#define QPSVC_DnaHexagon_hpp

#include <qpsvc/qpsvc.hpp>

class QPSvcDnaHexagon: public sQPSvc
{
        typedef sQPSvc TParent;
    public:

        QPSvcDnaHexagon(sQPride& qp): TParent(qp){}
        virtual ~QPSvcDnaHexagon(){}

        virtual const char* getSvcName() const
        {
            return "dna-hexagon";
        }
        static const char * getQuery00(sUsrObj & obj, sStr & query, const char * alSeparator = 0) {
            if(!alSeparator)alSeparator=";";
            idx pos = query.length();
            if( obj.propGet00("query",&query,alSeparator) ) {
                query.shrink00();
                query.add(alSeparator,sLen(alSeparator));
                if(!obj.propGet00("query_paired",&query,alSeparator))
                    query.shrink00(alSeparator,2);
            }
            return query.ptr(pos);
        }
        static const char * getSubject00(sUsrObj & obj, sStr & subject, const char * alSeparator = 0){
            if(!alSeparator)alSeparator=";";
            return obj.propGet00("subject", &subject, alSeparator);
        }
};

#endif 