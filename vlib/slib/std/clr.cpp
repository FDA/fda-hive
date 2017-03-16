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

#include <slib/std/clr.hpp>
#include <stdio.h>

using namespace slib;

sClr sClr::none((idx)0,(idx)0,(idx)0,(idx)0);

// Standard SVG list of color keywords, see
// http://www.w3.org/TR/css3-color/#svg-color
const sClr::Pigment sClr::clrTable[] = {
    { "aliceblue", 0xF0F8FF },
    { "antiquewhite", 0xFAEBD7 },
    { "aqua", 0x00FFFF },
    { "aquamarine", 0x7FFFD4 },
    { "azure", 0xF0FFFF },
    { "beige", 0xF5F5DC },
    { "bisque", 0xFFE4C4 },
    { "black", 0x000000 },
    { "blanchedalmond", 0xFFEBCD },
    { "blue", 0x0000FF },
    { "blueviolet", 0x8A2BE2 },
    { "brown", 0xA52A2A },
    { "burlywood", 0xDEB887 },
    { "cadetblue", 0x5F9EA0 },
    { "chartreuse", 0x7FFF00 },
    { "chocolate", 0xD2691E },
    { "coral", 0xFF7F50 },
    { "cornflowerblue", 0x6495ED },
    { "cornsilk", 0xFFF8DC },
    { "crimson", 0xDC143C },
    { "cyan", 0x00FFFF },
    { "darkblue", 0x00008B },
    { "darkcyan", 0x008B8B },
    { "darkgoldenrod", 0xB8860B },
    { "darkgray", 0xA9A9A9 },
    { "darkgreen", 0x006400 },
    { "darkgrey", 0xA9A9A9 },
    { "darkkhaki", 0xBDB76B },
    { "darkmagenta", 0x8B008B },
    { "darkolivegreen", 0x556B2F },
    { "darkorange", 0xFF8C00 },
    { "darkorchid", 0x9932CC },
    { "darkred", 0x8B0000 },
    { "darksalmon", 0xE9967A },
    { "darkseagreen", 0x8FBC8F },
    { "darkslateblue", 0x483D8B },
    { "darkslategray", 0x2F4F4F },
    { "darkslategrey", 0x2F4F4F },
    { "darkturquoise", 0x00CED1 },
    { "darkviolet", 0x9400D3 },
    { "deeppink", 0xFF1493 },
    { "deepskyblue", 0x00BFFF },
    { "dimgray", 0x696969 },
    { "dimgrey", 0x696969 },
    { "dodgerblue", 0x1E90FF },
    { "firebrick", 0xB22222 },
    { "floralwhite", 0xFFFAF0 },
    { "forestgreen", 0x228B22 },
    { "fuchsia", 0xFF00FF },
    { "gainsboro", 0xDCDCDC },
    { "ghostwhite", 0xF8F8FF },
    { "gold", 0xFFD700 },
    { "goldenrod", 0xDAA520 },
    { "gray", 0x808080 },
    { "green", 0x008000 },
    { "greenyellow", 0xADFF2F },
    { "grey", 0x808080 },
    { "honeydew", 0xF0FFF0 },
    { "hotpink", 0xFF69B4 },
    { "indianred", 0xCD5C5C },
    { "indigo", 0x4B0082 },
    { "ivory", 0xFFFFF0 },
    { "khaki", 0xF0E68C },
    { "lavender", 0xE6E6FA },
    { "lavenderblush", 0xFFF0F5 },
    { "lawngreen", 0x7CFC00 },
    { "lemonchiffon", 0xFFFACD },
    { "lightblue", 0xADD8E6 },
    { "lightcoral", 0xF08080 },
    { "lightcyan", 0xE0FFFF },
    { "lightgoldenrodyellow", 0xFAFAD2 },
    { "lightgray", 0xD3D3D3 },
    { "lightgreen", 0x90EE90 },
    { "lightgrey", 0xD3D3D3 },
    { "lightpink", 0xFFB6C1 },
    { "lightsalmon", 0xFFA07A },
    { "lightseagreen", 0x20B2AA },
    { "lightskyblue", 0x87CEFA },
    { "lightslategray", 0x778899 },
    { "lightslategrey", 0x778899 },
    { "lightsteelblue", 0xB0C4DE },
    { "lightyellow", 0xFFFFE0 },
    { "lime", 0x00FF00 },
    { "limegreen", 0x32CD32 },
    { "linen", 0xFAF0E6 },
    { "magenta", 0xFF00FF },
    { "maroon", 0x800000 },
    { "mediumaquamarine", 0x66CDAA },
    { "mediumblue", 0x0000CD },
    { "mediumorchid", 0xBA55D3 },
    { "mediumpurple", 0x9370DB },
    { "mediumseagreen", 0x3CB371 },
    { "mediumslateblue", 0x7B68EE },
    { "mediumspringgreen", 0x00FA9A },
    { "mediumturquoise", 0x48D1CC },
    { "mediumvioletred", 0xC71585 },
    { "midnightblue", 0x191970 },
    { "mintcream", 0xF5FFFA },
    { "mistyrose", 0xFFE4E1 },
    { "moccasin", 0xFFE4B5 },
    { "navajowhite", 0xFFDEAD },
    { "navy", 0x000080 },
    { "oldlace", 0xFDF5E6 },
    { "olive", 0x808000 },
    { "olivedrab", 0x6B8E23 },
    { "orange", 0xFFA500 },
    { "orangered", 0xFF4500 },
    { "orchid", 0xDA70D6 },
    { "palegoldenrod", 0xEEE8AA },
    { "palegreen", 0x98FB98 },
    { "paleturquoise", 0xAFEEEE },
    { "palevioletred", 0xDB7093 },
    { "papayawhip", 0xFFEFD5 },
    { "peachpuff", 0xFFDAB9 },
    { "peru", 0xCD853F },
    { "pink", 0xFFC0CB },
    { "plum", 0xDDA0DD },
    { "powderblue", 0xB0E0E6 },
    { "purple", 0x800080 },
    { "red", 0xFF0000 },
    { "rosybrown", 0xBC8F8F },
    { "royalblue", 0x4169E1 },
    { "saddlebrown", 0x8B4513 },
    { "salmon", 0xFA8072 },
    { "sandybrown", 0xF4A460 },
    { "seagreen", 0x2E8B57 },
    { "seashell", 0xFFF5EE },
    { "sienna", 0xA0522D },
    { "silver", 0xC0C0C0 },
    { "skyblue", 0x87CEEB },
    { "slateblue", 0x6A5ACD },
    { "slategray", 0x708090 },
    { "slategrey", 0x708090 },
    { "snow", 0xFFFAFA },
    { "springgreen", 0x00FF7F },
    { "steelblue", 0x4682B4 },
    { "tan", 0xD2B48C },
    { "teal", 0x008080 },
    { "thistle", 0xD8BFD8 },
    { "tomato", 0xFF6347 },
    { "turquoise", 0x40E0D0 },
    { "violet", 0xEE82EE },
    { "wheat", 0xF5DEB3 },
    { "white", 0xFFFFFF },
    { "whitesmoke", 0xF5F5F5 },
    { "yellow", 0xFFFF00 },
    { "yellowgreen", 0x9ACD32 }
};

const idx sClr::clrTableDim = sizeof(sClr::clrTable) / sizeof(sClr::Pigment);

real sClr::hue360(void) const
{
    if (r() > g() && g() >= b()) {
        // red domain: r max, b min, g ascending
        return 60 * (real)(g() - b()) / (r() - b());
    } else if (g() >= r() && r() > b()) {
        // yellow domain: g max, b min, r descending
        return 60 * (1 + (real)(g() - r()) / (g() - b()));
    } else if (g() > b() && b() >= r()) {
        // green domain: g max, r min, b ascending
        return 60 * (2 + (real)(b() - r()) / (g() - r()));
    } else if (b() >= g() && g() > r()) {
        // cyan domain: b max, r min, g descending
        return 60 * (3 + (real)(b() - g()) / (b() - r()));
    } else if (b() > r() && r() >= g()) {
        // blue domain": b max, g min, r ascending
        return 60 * (4 + (real)(r() - g()) / (b() - g()));
    } else if (r() >= b() && b() > g()) {
        // magenta domain: r max, g min, b descending
        return 60 * (5 + (real)(r() - b()) / (r() - g()));
    }

    // black, white, or grey
    return 0;
}

real sClr::luminance1(sClr::eLumaMode m /*= eRec709 */) const
{
    switch (m) {
    case sClr::eRec601 :
        return 0.299 * r1() + 0.587 * g1() + 0.114 * b1();
    case sClr::eRec709 :
        return 0.2126 * r1() + 0.7152 * g1() + 0.0722 * b1();
    }

    return 0;
}

void sClr::setHCL(real hue360, real chroma1, real lightness1)
{
    // clamp lightness in [0, 1]
    lightness1 = sClamp<real>(lightness1, 0, 1);

    // clamp chroma in valid range for HCL bicone
    if (lightness1 >= 0.5) {
        chroma1 = sMin<real>(chroma1, 2. - 2 * lightness1);
    } else {
        chroma1 = sMin<real>(chroma1, 2 * lightness1);
    }

    real domain = fmod(hue360, 360) / 60.;

    real min_value = lightness1 - chroma1 / 2.;
    real mid_value = min_value + chroma1 * (1 - fabs(fmod(domain, 2) - 1));
    real max_value = min_value + chroma1;

    real red1 = 0, green1 = 0, blue1 = 0;
    if (domain < 1) {
        // red domain: r max, g ascends */
        red1 = max_value;
        green1 = mid_value;
        blue1 = min_value;
    } else if (domain < 2) {
        // yellow domain: g max, r descends
        red1 = mid_value;
        green1 = max_value;
        blue1 = min_value;
    } else if (domain < 3) {
        // green domain: g max, b ascends
        red1 = min_value;
        green1 = max_value;
        blue1 = mid_value;
    } else if (domain < 4) {
        // cyan domain: b max, g descends
        red1 = min_value;
        green1 = mid_value;
        blue1 = max_value;
    } else if (domain < 5) {
        // blue domain: b max, r ascends
        red1 = mid_value;
        green1 = min_value;
        blue1 = max_value;
    } else if (domain < 6) {
        // magenta domain: r max, b descends
        red1 = max_value;
        green1 = min_value;
        blue1 = mid_value;
    }

    set(red1, green1, blue1, 1.0);
}

const char * sClr::printHex(sStr & out, bool forceAlpha/*=false*/) const
{
    idx start = out.length();

    out.printf("#%02X%02X%02X", static_cast<unsigned int>(r()), static_cast<unsigned int>(g()), static_cast<unsigned int>(b()));
    if (forceAlpha || a() < 255) {
        out.printf("%02X", static_cast<unsigned int>(a()));
    }

    return out.ptr(start);
}

// static
int sClr::Pigment::compareName(const void * name, const void * pig)
{
    return strcasecmp(static_cast<const char *>(name), static_cast<const Pigment *>(pig)->name);
}

void sClr::parse(const char * s)
{
    if (!s) {
        set((idx)0);
        return;
    }

    if (s[0] == '#') {
        // hex string
        s++;
        idx digit[8];
        idx len = 0;
        for (; len < 8; len++) {
            char c = s[len];
            if (c >= '0' && c <= '9') {
                digit[len] = c - '0';
            } else if (c >= 'a' && c <= 'f') {
                digit[len] = 10 + c - 'a';
            } else if (c >= 'A' && c <= 'F') {
                digit[len] = 10 + c - 'A';
            } else {
                break;
            }
        }

        switch (len) {
        case 3:
            // #RGB
            set((digit[0] << 4) + digit[0], (digit[1] << 4) + digit[1], (digit[2] << 4) + digit[2], (idx)255);
            break;
        case 6:
            // #RRGGBB
            set((digit[0] << 4) + digit[1], (digit[2] << 4) + digit[3], (digit[4] << 4) + digit[5], (idx)255);
            break;
        case 8:
            // #RRGGBBAA
            set((digit[0] << 4) + digit[1], (digit[2] << 4) + digit[3], (digit[4] << 4) + digit[5], (digit[6] << 4) + digit[7]);
            break;
        default:
            set((idx)0);
        }
    } else {
        // color name
        const Pigment * pig = static_cast<const Pigment *>(bsearch(s, clrTable, clrTableDim, sizeof(Pigment), Pigment::compareName));
        set(pig ? pig->rgb : (idx)0);
    }
}

void sClr::setHSB(real hue1, real saturation1, real brightness1)
{
    real  red,green,blue;
    real  domainOffset;         /* hue mod 1/6 */

    if (brightness1 == 0.0)/* safety short circuit again */
    {   
        set((idx)0,(idx)0,(idx)0);
    }

    if (saturation1 == 0.0)      /* grey */
    {   
        set((idx)brightness1,(idx)brightness1,(idx)brightness1);
    }

    
    if (hue1 < 1.0/6)        /* red domain; green ascends */
    {   
        domainOffset = hue1;
        red   = (brightness1); // (idx)
        blue  = (brightness1 * (1.0 - saturation1)); //(idx)
        green = (blue + (brightness1 - blue) * domainOffset * 6); // (idx)
    }
    else if (hue1 < 2.0/6)       /* yellow domain; red descends */
    { 
        domainOffset = hue1 - 1.0/6;
        green = (brightness1);
        blue  = (brightness1 * (1.0 - saturation1));
        red   = (green - (brightness1 - blue) * domainOffset * 6);
    }
    else if (hue1 < 3.0/6) /* green domain; blue ascends */
    { 
        domainOffset = hue1 - 2.0/6;
        green = (brightness1);
        red   = (brightness1 * (1.0 - saturation1));
        blue  = (red + (brightness1 - red) * domainOffset * 6);
    }
    else if (hue1 < 4.0/6)   /* cyan domain; green descends */
    { 
        domainOffset = hue1 - 3.0/6;
        blue  = (brightness1);
        red   = (brightness1 * (1.0 - saturation1));
        green = (blue - (brightness1 - red) * domainOffset * 6);
    }
    else if (hue1 < 5.0/6)/* blue domain; red ascends */
    { 
        domainOffset = hue1 - 4.0/6;
        blue  = (brightness1);
        green = (brightness1 * (1.0 - saturation1));
        red   = (green + (brightness1 - green) * domainOffset * 6);
    }
    else    /* magenta domain; blue descends */
    { 
        domainOffset = hue1 - 5.0/6;
        red   = (brightness1);
        green = (brightness1 * (1.0 - saturation1));
        blue  = (red - (brightness1 - green) * domainOffset * 6);
    }

    //set((idx)(red*255),(idx)(green*255),(idx)(255*blue), 0);
    set(red,green,blue, 1.);
    //return rgb; 
}




void sClr::getHSB(real * hue1,real * saturation1,real * brightness1) const
{
    idx     red=r(_rgb);
    idx     green=g(_rgb);
    idx     blue=b(_rgb);
    real  bri,domainBase,oneSixth,domainOffset;
    idx     desaturator,midigator;

    bri = (real)sMax(red, green);
    bri= (real)sMax((idx)bri, blue);

    if (bri == 0)/* short-circuit now and avoid division by zero problems later */
    {   
        *brightness1 = 0.0;
        *saturation1 = 0.0;
        *hue1        = 0.0;
        return;
    }

    *brightness1 = bri / 255.0;
    desaturator =sMin(red,green);
    desaturator =sMin(desaturator ,blue);

    if (bri == desaturator)/* we're grey (and still have division by zero issues to bypass) */
    {   
        *saturation1 = 0.0;
        *hue1        = 0.0;
        return;
    }

    *saturation1 = ((*brightness1) - (desaturator / 255.0)) / (*brightness1);
    midigator = (red+green+blue)/3;
        
    /* "domains" are 60 degrees of red, yellow, green, cyan, blue, or magenta */
    /* compute how far we are from a domain base */
    //domainBase;
    oneSixth = 1.0f / 6.0;
    domainOffset = (midigator - desaturator) /
                         (real)(bri - desaturator) / 6.0;
    if (red == bri)
    {
        if (midigator == green) /* green is ascending */
        {   
            domainBase = 0 / 6.0;       /* red domain */
        }
        else    /* blue is descending */
        { 
            domainBase = 5 / 6.0;       /* magenta domain */
            domainOffset = oneSixth - domainOffset;
        }
    }
    else if (green == bri)
    {
        if (midigator == blue)  /* blue is ascending */
        {   
            domainBase = 2 / 6.0;       /* green domain */
        }
        else    /* red is descending */
        { 
            domainBase = 1 / 6.0;       /* yellow domain */
            domainOffset = oneSixth - domainOffset;
        }
    }
    else
    {
        if (midigator == red)   /* red is ascending */
        {   
            domainBase = 4 / 6.0;       /* blue domain */
        }
        else    /* green is descending */
        { 
            domainBase = 3 / 6.0;       /* cyan domain */
            domainOffset = oneSixth - domainOffset;
        }
    }

    *hue1 = domainBase + domainOffset;
    return;
}

///sClr::Pigment * sClr::rndPigment(void){return pigment(rand());};
//
sClr sClr::pigment(idx iclr, idx a ){
    if(iclr==-1)iclr=rand(); 
    iclr=clrTable[iclr % clrTableDim].rgb; 
    return sClr(sClr::r(iclr),sClr::g(iclr),sClr::b(iclr),(a&0xFF));
}
