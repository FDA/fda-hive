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
#ifndef sLib_uperm_h
#define sLib_uperm_h

namespace slib {

    enum ePerm
    {
        ePermNone           = 0x00000000,
        ePermCanBrowse      = 0x00000001,
        ePermCanRead        = 0x00000002,
        ePermCanWrite       = 0x00000004,
        ePermCanExecute     = 0x00000008,
        ePermCanDelete      = 0x00000010,
        ePermCanAdmin       = 0x00000020,
        ePermCanShare       = 0x00000040,
        ePermCanDownload    = 0x00000080,
        ePermMask           = 0x000000FF,
        ePermCompleteAccess = 0xFFFFFFFF
    };

    enum eFlags
    {
        eFlagNone        = 0x00000000,
        eFlagRestrictive = 0x00000001,
        eFlagInheritDown = 0x00000002,
        eFlagInheritUp   = 0x00000004,
        eFlagOnHold      = 0x10000000,
        eFlagRevoked     = 0x20000000,
        eFlagDefault = eFlagInheritDown
    };

    udx permPermParse(const char * src, idx len=0);

    udx permFlagParse(const char * src, idx len=0);

    char * permPrettyPrint(sStr &dst, const udx perm, const udx flags);

    void permPretty2JSON(sJSONPrinter & printer, udx num_group, const char * pretty_group, const char * perm_pretty_print, udx perm_perm = 0, udx perm_flags = 0);
};

#endif 