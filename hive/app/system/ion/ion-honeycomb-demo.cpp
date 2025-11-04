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

#include "ion-tools.hpp"
#include <ulib/ulib.hpp>

extern sIonBirel * RION;

#define  sIonBirel_INDEX(_v_i)  sIonBirel::Link_INDEX,(_v_i)
#define  sIonBirel_STRING(_v_s)  (_v_s), sLen(_v_s)
#define  sIonBirel_AUTOVAL  0, sIonBirel::Link_AUTOVAL
#define  sIonBirel_AUTOARR  0, sIonBirel::Link_AUTOARR

void demoRegisterNewType1(void)
{
    sIonBirel::LinkType lnk[10];




    RION->link(&lnk[1], sIonBirel_INDEX(RION->root), sIonBirel_STRING("hc_demo1"), sIonBirel_AUTOVAL );
    RION->link(&lnk[2], sIonBirel_INDEX(lnk[1].ixes.val), sIonBirel_STRING("_type"), sIonBirel_STRING("type") );
    RION->link(&lnk[3], sIonBirel_INDEX(lnk[1].ixes.val), sIonBirel_STRING("_attribute"), sIonBirel_AUTOVAL );
        RION->link(&lnk[5], sIonBirel_INDEX(lnk[3].ixes.val), sIonBirel_STRING("filename"), sIonBirel_AUTOVAL );
            RION->link(&lnk[0], sIonBirel_INDEX(lnk[5].ixes.val), sIonBirel_STRING("_type"), sIonBirel_STRING("string") );
        RION->link(&lnk[4], sIonBirel_INDEX(lnk[1].ixes.val), sIonBirel_STRING("_inherit"), sIonBirel_AUTOARR );
            RION->link(&lnk[0], sIonBirel_INDEX(lnk[4].ixes.val), sIonBirel_STRING("0"), sIonBirel_STRING("hc_user_base") );
            RION->link(&lnk[0], sIonBirel_INDEX(lnk[4].ixes.val), sIonBirel_STRING("1"), sIonBirel_STRING("hc_file") );
}

void demoRegisterNewType2(void)
{

    const char * t="{ \n"
        "\"hc_demo2\" : { \n"
            "\"_type\" : \"type\",  \n"
            "\"_fields\" : {  \n"
                "\"filename\" : {  \n"
                    "\"_type\" : \"string\"  \n"
                "}  \n"
            "},  \n"
            "\"_inherit\" : [  \n"
                "\"hc_user_base\",  \n"
                "\"hc_file\"  \n"
            "]  \n"
        "}\n"
    "} \n";

    sJax vs(0,t,sLen(t));

    RION->parse(&vs);

}

idx __on_hc_demo(sIonTools * iap, const char * cmd, const char * args, const char * ,sVar * pForm)
{


    if(sIs(cmd,"-hcDemoRegisterType") ) {
        demoRegisterNewType1();
        demoRegisterNewType2();
    }

    return 0;
}

