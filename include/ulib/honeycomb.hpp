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
#ifndef sHoneyComb_hpp
#define sHoneyComb_hpp

#include <ion/sIon.hpp>
#include <slib/utils.hpp>

namespace slib
{
    class sHoneyComb
    {
        public:
        struct Locator {
            idx bucket;
            idx indexIon;
            idx indexObjId;
            idx indexName;
            idx indexPath;
            idx indexValue;
            Locator () {
                bucket=sNotIdx;
                indexIon=0;
            }
        };

        public:
            sHoneyComb(const char * baseName=0, idx openMode=0){}
            ~sHoneyComb(){}

            virtual sHoneyComb * init(const char * baseName, idx openMode ){ return this;}
            virtual void destroy(){}
            virtual bool ok(){return false;}
            virtual const char * propGet(sIO * buf, idx objID, const char * name, const char * path=0){return 0;}
            virtual idx objListIterate(Locator * locator, bool isMult, const char * type, const char * name, const char * value , sIO * buf=0){return 0;}
            virtual const idx * getObjId(Locator * locator, idx * pSize=0) {return 0;}
            virtual const char * getName(Locator * locator, idx * pSize=0) {return 0;}
            virtual const char * getPath(Locator * locator, idx * pSize=0) {return 0;}
            virtual const char * getValue(Locator * locator, idx * pSize=0) {return 0;}
    };


    class sHoneyCombIon : public sHoneyComb
    {
        public:
            sIon * ion;
            sIon noi;

            idx objType,typeType,nameType,pathType,valueType;
            idx objToValueRelationshipIndex,objToTypeRelationshipIndex;


        public:

            sHoneyCombIon(sIon * lion)
            {
                ion=lion;
            }
            sHoneyCombIon(const char * baseName="honeycomb", idx openMode=0)
            {
                ion=&noi;
                ion->init(baseName, openMode);
            }
            virtual ~sHoneyCombIon(){}


            sHoneyCombIon * init(const char * baseName, idx openMode );
            void destroy(){ion->destroy();}
            bool ok(){return ion->ok();}


            idx createObject(idx objID, idx * pObjIndex=0);
            bool propSet(idx objID, const char * name, const char * path, const char * value);

            const char * propGet(sIO * buf, idx objID, const char * name, const char * path=0);
            idx objListIterate(Locator * iterator, bool isMult, const char * type, const char * name, const char * value , sIO * buf=0);
    };



    class sHoneyCombSet: private sHoneyComb
    {
            sDic <sHoneyCombIon> hcList;
        public:
            sHoneyCombSet(const char * baseNameList00="honeycomb", idx openMode=sMex::fReadonly): sHoneyComb(0,openMode)
                {init(baseNameList00, openMode);}
            ~sHoneyCombSet(){destroy();}

            virtual sHoneyCombSet * init(const char * baseNameList00="honeycomb", idx openMode=0);
            virtual void destroy(){hcList.empty();}
            virtual bool ok(){return hcList.dim()>0 && hcList[(idx)0].ok();}
            virtual const char * propGet(sIO * buf, idx objID, const char * name, const char * path=0);
            virtual idx objListIterate(sHoneyCombIon::Locator * locator, bool isMult, const char  * type, const char * name, const char * value ,sIO * buf=0);

            virtual const idx * getObjId(Locator * locator, idx * pSize=0) {return hcList[locator->indexIon].getObjId(locator, pSize);}
            virtual const char * getName(Locator * locator, idx * pSize=0) {return hcList[locator->indexIon].getName(locator, pSize);}
            virtual const char * getPath(Locator * locator, idx * pSize=0) {return hcList[locator->indexIon].getPath(locator, pSize);}
            virtual const char * getValue(Locator * locator, idx * pSize=0) {return hcList[locator->indexIon].getValue(locator, pSize);}

    };





}

#endif 