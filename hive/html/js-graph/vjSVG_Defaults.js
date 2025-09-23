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

var DefaultBrush = {
    "fill" : 'black',
    "fill-rule" : 'nonzero',
    "fill-opacity" : 0
};

var DefaultPen = {
    "stroke" : 'black',
    "stroke-width" : 0.6,
    "stroke-opacity" : 1,
    "stroke-linecap" : 'butt',
    "stroke-linejoin" : 'miter',
    "stroke-miterlimit" : 4,
    "stroke-dasharray" : 'none',
    "stroke-dashoffset" : 0
};

var DefaultTextBrush = {
    "fill" : 'black',
    "fill-opacity" : 1
};

var DefaultTextPen = {
    "stroke-opacity" : 0
};

var DefaultFont = {
    "font-family" : 'Arial',
    "font-variant" : 'normal',
    "font-weight" : 'normal',
    "font-strech" : 'normal',
    "font-size" : 'medium',
    "font-size-adjust" : 'none',
    "kerning" : "auto",
    "letter-spacing" : 'normal',
    "word-spacing" : 'normal',
    "text-decoration" : 'none',
    "writting-mode" : 'lr-tb',
    "glyph-orientation-verical" : 'auto',
    "glyph-orientation-horizontal" : '0deg',
    "direction" : 'ltr',
    "unicode-bidi" : 'normal',
    "text-anchor" : 'start',
    "alignment-baseline" : 'auto',
    "dominant-baseline" : 'auto',
    "baseline-shift" : 'baseline'
};

var DefaultFilters = {
    family : 'Arial',
    size : 10,
    width : 0,
    style : 'italic',
    weight : 'bold'
};

var DefaultCallback = {
    onerror : 'Oops! somthing went wrong with SVG'
};

var DefaultSVGID =  "svg";

var DefaultsizeXYZ = {
    x : 100,
    y : 100,
    z : 100
};
