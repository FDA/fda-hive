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

/**********************************************
* color functions
**********************************************/


function randColor()
{
    return '#'+Math.floor(Math.random()*16777215).toString(16);
}


function pastelColor(color, opacity )
{
    if(opacity === undefined ) {
        opacity =  0.5;
    }
    return shadeColor(color, opacity, "#7f7f7f" );
}

function shadeColor(color, shade, base )
{
    var baseInt=base ? parseInt(base.substring(1),16) : 0x000000;
    var baseR = (baseInt & 0xFF0000) >> 16;
    var baseG = (baseInt & 0x00FF00) >> 8;
    var baseB = (baseInt & 0x0000FF) >> 0;

    var colorInt = parseInt(color.substring(1),16);

    var R = (colorInt & 0xFF0000) >> 16;
    var G = (colorInt & 0x00FF00) >> 8;
    var B = (colorInt & 0x0000FF) >> 0;

    R = baseR+Math.floor((R*shade));
    G = baseG+Math.floor((G*shade));
    B = baseB+Math.floor((B*shade));


    var newColorInt = (1<<24) + (R<<16) + (G<<8) + (B);
    newColorInt = newColorInt.toString(16).slice(1);
    var newColorStr = "#"+newColorInt;
    return newColorStr;
}

function lightenColor(color, factor)
{
    if (typeof(color) == "string")
        color = new vjColor(color);

    if (!factor)
        factor = 0.5;

    return vjHCL(color.hue(), color.chroma(), factor + (1-factor) * color.lightness()).hex();
}

function darkenColor(color, factor)
{
    if (typeof(color) == "string")
        color = new vjColor(color);

    if (!factor)
        factor = 0.5;

    return vjHCL(color.hue(), color.chroma(), (1-factor) * color.lightness()).hex();
}

function hueshiftColor(color,shift)
{
    if (typeof(color) == "string")
        color = new vjColor(color);
    var  h = color.hue() + shift;
    while (h>=360.0) {
        h-=360.0;
    }
    while (h<0.0) {
        h+=360.0;
    }
    return vjHCL(h,color.chroma(),color.lightness()).hex();
}

function oppositeColor(color)
{
    return hueshiftColor(color,180);
}

var gClrTable2 = new Array ("#74c493","#51574a","#447c69","#8e8c6d","#e4bf80","#e9d78e","#e16552","#c94a53","#e2975d","#be5168","#a34974","#f19670","#993767","#65387d","#4e2472","#9163b6","#e279a3","#e0598b","#7c9fb0","#5698c4","#9abf88");
var gClrTable = new Array (
    "#000000",
    "#7f7f7f",
    "#40B000",
    "#0000FF",
    "#A52A2A",
    "#D2691E",
    "#00008B",
    "#008B8B",
    "#B8860B",
    "#A9A9A9",
    "#006400",
    "#BDB76B",
    "#8B008B",
    "#556B2F",
    "#FF8C00",
    "#9932CC",
    "#8B0000",
    "#E9967A",
    "#8FBC8F",
    "#483D8B",
    "#2F4F4F",
    "#00CED1",
    "#9400D3",
    "#808080",
    "#0000CD",
    "#BA55D3",
    "#9370DB",
    "#3CB371",
    "#7B68EE",
    "#00FA9A",
    "#48D1CC",
    "#C71585",
    "#191970",
    "#000080",
    "#FFA500",
    "#FF4500",
    "#FF0000",
    "#4169E1",
    "#FF6347",
    "#40E0D0",
    "#EE82EE",
    "#F5DEB3",
    "#FFFF00",
    "#9ACD32",
    "#F0F8FF",
    "#FAEBD7",
    "#00FFFF",
    "#7FFFD4",
    "#F0FFFF",
    "#F5F5DC",
    "#FFE4C4",
    "#000000",
    "#FFEBCD",
    "#0000FF",
    "#8A2BE2",
    "#A52A2A",
    "#DEB887",
    "#5F9EA0",
    "#7FFF00",
    "#D2691E",
    "#FF7F50",
    "#6495ED",
    "#FFF8DC",
    "#DC143C",
    "#00FFFF",
    "#00008B",
    "#008B8B",
    "#B8860B",
    "#A9A9A9",
    "#006400",
    "#BDB76B",
    "#8B008B",
    "#556B2F",
    "#FF8C00",
    "#9932CC",
    "#8B0000",
    "#E9967A",
    "#8FBC8F",
    "#483D8B",
    "#2F4F4F",
    "#00CED1",
    "#9400D3",
    "#FF1493",
    "#00BFFF",
    "#696969",
    "#1E90FF",
    "#B22222",
    "#FFFAF0",
    "#228B22",
    "#FF00FF",
    "#DCDCDC",
    "#F8F8FF",
    "#FFD700",
    "#DAA520",
    "#808080",
    "#008000",
    "#ADFF2F",
    "#F0FFF0",
    "#FF69B4",
    "#CD5C5C",
    "#4B0082",
    "#FFFFF0",
    "#F0E68C",
    "#E6E6FA",
    "#FFF0F5",
    "#7CFC00",
    "#FFFACD",
    "#ADD8E6",
    "#F08080",
    "#E0FFFF",
    "#FAFAD2",
    "#90EE90",
    "#D3D3D3",
    "#FFB6C1",
    "#FFA07A",
    "#20B2AA",
    "#87CEFA",
    "#778899",
    "#B0C4DE",
    "#FFFFE0",
    "#00FF00",
    "#32CD32",
    "#FAF0E6",
    "#FF00FF",
    "#800000",
    "#66CDAA",
    "#0000CD",
    "#BA55D3",
    "#9370DB",
    "#3CB371",
    "#7B68EE",
    "#00FA9A",
    "#48D1CC",
    "#C71585",
    "#191970",
    "#F5FFFA",
    "#FFE4E1",
    "#FFE4B5",
    "#FFDEAD",
    "#000080",
    "#FDF5E6",
    "#808000",
    "#6B8E23",
    "#FFA500",
    "#FF4500",
    "#DA70D6",
    "#EEE8AA",
    "#98FB98",
    "#AFEEEE",
    "#DB7093",
    "#FFEFD5",
    "#FFDAB9",
    "#CD853F",
    "#FFC0CB",
    "#DDA0DD",
    "#B0E0E6",
    "#800080",
    "#FF0000",
    "#BC8F8F",
    "#4169E1",
    "#8B4513",
    "#FA8072",
    "#F4A460",
    "#2E8B57",
    "#FFF5EE",
    "#A0522D",
    "#C0C0C0",
    "#87CEEB",
    "#6A5ACD",
    "#708090",
    "#FFFAFA",
    "#00FF7F",
    "#4682B4",
    "#D2B48C",
    "#008080",
    "#D8BFD8",
    "#FF6347",
    "#40E0D0",
    "#EE82EE",
    "#F5DEB3",
    "#FFFFFF",
    "#F5F5F5",
    "#FFFF00",
    "#9ACD32"
    );

// http://www.w3.org/TR/css3-color/#svg-color
var vjStandardColors = {
    aliceblue: "#f0f8ff",
    antiquewhite: "#faebd7",
    aqua: "#00ffff",
    aquamarine: "#7fffd4",
    azure: "#f0ffff",
    beige: "#f5f5dc",
    bisque: "#ffe4c4",
    black: "#000000",
    blanchedalmond: "#ffebcd",
    blue: "#0000ff",
    blueviolet: "#8a2be2",
    brown: "#a52a2a",
    burlywood: "#deb887",
    cadetblue: "#5f9ea0",
    chartreuse: "#7fff00",
    chocolate: "#d2691e",
    coral: "#ff7f50",
    cornflowerblue: "#6495ed",
    cornsilk: "#fff8dc",
    crimson: "#dc143c",
    cyan: "#00ffff",
    darkblue: "#00008b",
    darkcyan: "#008b8b",
    darkgoldenrod: "#b8860b",
    darkgray: "#a9a9a9",
    darkgreen: "#006400",
    darkgrey: "#a9a9a9",
    darkkhaki: "#bdb76b",
    darkmagenta: "#8b008b",
    darkolivegreen: "#556b2f",
    darkorange: "#ff8c00",
    darkorchid: "#9932cc",
    darkred: "#8b0000",
    darksalmon: "#e9967a",
    darkseagreen: "#8fbc8f",
    darkslateblue: "#483d8b",
    darkslategray: "#2f4f4f",
    darkslategrey: "#2f4f4f",
    darkturquoise: "#00ced1",
    darkviolet: "#9400d3",
    deeppink: "#ff1493",
    deepskyblue: "#00bfff",
    dimgray: "#696969",
    dimgrey: "#696969",
    dodgerblue: "#1e90ff",
    firebrick: "#b22222",
    floralwhite: "#fffaf0",
    forestgreen: "#228b22",
    fuchsia: "#ff00ff",
    gainsboro: "#dcdcdc",
    ghostwhite: "#f8f8ff",
    gold: "#ffd700",
    goldenrod: "#daa520",
    gray: "#808080",
    green: "#008000",
    greenyellow: "#adff2f",
    grey: "#808080",
    honeydew: "#f0fff0",
    hotpink: "#ff69b4",
    indianred: "#cd5c5c",
    indigo: "#4b0082",
    ivory: "#fffff0",
    khaki: "#f0e68c",
    lavender: "#e6e6fa",
    lavenderblush: "#fff0f5",
    lawngreen: "#7cfc00",
    lemonchiffon: "#fffacd",
    lightblue: "#add8e6",
    lightcoral: "#f08080",
    lightcyan: "#e0ffff",
    lightgoldenrodyellow: "#fafad2",
    lightgray: "#d3d3d3",
    lightgreen: "#90ee90",
    lightgrey: "#d3d3d3",
    lightpink: "#ffb6c1",
    lightsalmon: "#ffa07a",
    lightseagreen: "#20b2aa",
    lightskyblue: "#87cefa",
    lightslategray: "#778899",
    lightslategrey: "#778899",
    lightsteelblue: "#b0c4de",
    lightyellow: "#ffffe0",
    lime: "#00ff00",
    limegreen: "#32cd32",
    linen: "#faf0e6",
    magenta: "#ff00ff",
    maroon: "#800000",
    mediumaquamarine: "#66cdaa",
    mediumblue: "#0000cd",
    mediumorchid: "#ba55d3",
    mediumpurple: "#9370db",
    mediumseagreen: "#3cb371",
    mediumslateblue: "#7b68ee",
    mediumspringgreen: "#00fa9a",
    mediumturquoise: "#48d1cc",
    mediumvioletred: "#c71585",
    midnightblue: "#191970",
    mintcream: "#f5fffa",
    mistyrose: "#ffe4e1",
    moccasin: "#ffe4b5",
    navajowhite: "#ffdead",
    navy: "#000080",
    oldlace: "#fdf5e6",
    olive: "#808000",
    olivedrab: "#6b8e23",
    orange: "#ffa500",
    orangered: "#ff4500",
    orchid: "#da70d6",
    palegoldenrod: "#eee8aa",
    palegreen: "#98fb98",
    paleturquoise: "#afeeee",
    palevioletred: "#db7093",
    papayawhip: "#ffefd5",
    peachpuff: "#ffdab9",
    peru: "#cd853f",
    pink: "#ffc0cb",
    plum: "#dda0dd",
    powderblue: "#b0e0e6",
    purple: "#800080",
    red: "#ff0000",
    rosybrown: "#bc8f8f",
    royalblue: "#4169e1",
    saddlebrown: "#8b4513",
    salmon: "#fa8072",
    sandybrown: "#f4a460",
    seagreen: "#2e8b57",
    seashell: "#fff5ee",
    sienna: "#a0522d",
    silver: "#c0c0c0",
    skyblue: "#87ceeb",
    slateblue: "#6a5acd",
    slategray: "#708090",
    slategrey: "#708090",
    snow: "#fffafa",
    springgreen: "#00ff7f",
    steelblue: "#4682b4",
    tan: "#d2b48c",
    teal: "#008080",
    thistle: "#d8bfd8",
    tomato: "#ff6347",
    turquoise: "#40e0d0",
    violet: "#ee82ee",
    wheat: "#f5deb3",
    white: "#ffffff",
    whitesmoke: "#f5f5f5",
    yellow: "#ffff00",
    yellowgreen: "#9acd32"
};


function vjColor(source) {
    if (typeof source == "string") {
        if (source[0] != '#') {
            for (var c in vjStandardColors) {
                if (source.toLowerCase() == c) {
                    source = vjStandardColors[c];
                    break;
                }
            }
        }

        if (source.match(/^#([0-9a-fA-F]{6})/)) {
            source = {
                r: parseInt("0x" + source.substr(1,2)),
                g: parseInt("0x" + source.substr(3,2)),
                b: parseInt("0x" + source.substr(5,2))
            };
        } else if (source.match(/^#([0-9a-fA-F]{3})/)) {
            source = {
                r: parseInt("0x" + source[1]),
                g: parseInt("0x" + source[2]),
                b: parseInt("0x" + source[3])
            };
        } else {
            source = {r:0, g:0, b:0};
        }
    }

    for (var i in source)
        this[i] = source[i];

    var fields = ["r", "g", "b"];
    for (var i = 0; i < fields.length; i++)
        if (this[fields[i]] === null || this[fields[i]] === undefined || this[fields[i]] < 0 || this[fields[i]] > 255)
            this[fields[i]] = 0;

    this.hex = function() {
        var ret = "#";

        var colors = [Math.round(this.r), Math.round(this.g), Math.round(this.b)];

        for (var i = 0; i < colors.length; i++) {
            if (colors[i] < 16)
                ret += "0" + colors[i].toString(16);
            else
                ret += colors[i].toString(16);
        }

        return ret;
    };

    this.hue = function() {
        if (this.r > this.g && this.g >= this.b) {
            // red domain: r max, b min, g ascending
            return 60 * (this.g - this.b) / (this.r - this.b);
        } else if (this.g >= this.r && this.r > this.b) {
            // yellow domain: g max, b min, r descending
            return 60 * (1 + (this.g - this.r) / (this.g - this.b));
        } else if (this.g > this.b && this.b >= this.r) {
            // green domain: g max, r min, b ascending
            return 60 * (2 + (this.b - this.r) / (this.g - this.r));
        } else if (this.b >= this.g && this.g > this.r) {
            // cyan domain: b max, r min, g descending
            return 60 * (3 + (this.b - this.g) / (this.b - this.r));
        } else if (this.b > this.r && this.r >= this.g) {
            // blue domain": b max, g min, r ascending
            return 60 * (4 + (this.r - this.g) / (this.b - this.g));
        } else if (this.r >= this.b && this.b > this.g) {
            // magenta domain: r max, g min, b descending
            return 60 * (5 + (this.r - this.b) / (this.r - this.g));
        }
        // black, white, or grey
        return 0;
    };

    this.chroma = function() {
        return (Math.max(this.r, this.g, this.b) - Math.min(this.r, this.g, this.b))/255;
    };

    this.saturation = function() {
        return this.chroma() / (1 - Math.abs(2 * this.lightness() - 1));
    };

    this.lightness = function() {
        return (Math.max(this.r, this.g, this.b) + Math.min(this.r, this.g, this.b))/(2*255);
    };
}

// red, green, and blue are in [0, 255]
function vjRGB(red, green, blue)
{
    return new vjColor({r:red, g:green, b:blue});
}

// hue is in [0, 360); chroma is in [0, 1]; lightness is in [0, 1] (where 0 is black, 1 is white)
function vjHCL(hue, chroma, lightness)
{
    // clamp lightness in [0, 1]
    lightness = Math.max(0, Math.min(lightness, 1));

    // clamp chroma to a valid range for the HCL bicone
    if (lightness >= 0.5)
        chroma = Math.min(chroma, 2 - 2*lightness);
    else
        chroma = Math.min(chroma, 2*lightness);

    var h60 = (hue % 360)/60;
    var x = chroma * (1 - Math.abs(h60 % 2 - 1));
    var f = [0, 0, 0];
    if (h60 >= 0) {
        if (h60 < 1)
            f = [chroma, x, 0];
        else if (h60 < 2)
            f = [x, chroma, 0];
        else if (h60 < 3)
            f = [0, chroma, x];
        else if (h60 < 4)
            f = [0, x, chroma];
        else if (h60 < 5)
            f = [x, 0, chroma];
        else if (h60 < 6)
            f = [chroma, 0, x];
    }

    var m = lightness - chroma/2;

    return new vjColor({r: 255 * (f[0] + m), g: 255 * (f[1] + m), b: 255 * (f[2] + m), h: hue, c: chroma, l: lightness});
}

function randRangeColor(lowth, hith) {
    if (lowth===undefined){
        lowth = 50;
    }
    if (hith===undefined){
        hith = 210;
    }
    var randColor;
    var hsp = 0;
    var r,b,g;
    while (hsp < lowth || hsp > hith) {
        r = (Math.random() * 0xFF << 0);
        b = (Math.random() * 0xFF << 0);
        g = (Math.random() * 0xFF << 0);
        hsp = Math.sqrt(0.299 * (r * r) + 0.587 * (g * g) + 0.114 * (b * b));
    }
    randColor = '#' + (r.toString(16).length < 2 ? '0': '') + r.toString(16)  + (b.toString(16).length < 2 ? '0': '') + b.toString(16) + (g.toString(16).length < 2 ? '0': '') + g.toString(16); 
    return randColor;
}

// hue is in [0, 360); saturation is in [0, 1]; lightness is in [0, 1] (where 0
// is black, 1 is white)
function vjHSL(hue, saturation, lightness) {
    var ret = vjHCL(hue, (1 - Math.abs(2 * lightness - 1)) * saturation, lightness);
    ret.s = saturation;
    return ret;
}

var valgoGenomeColors = new Array();
for (var it = 0; it < gClrTable.length; ++it) valgoGenomeColors[it] = pastelColor(gClrTable[(it + 1) % (gClrTable.length)]); // make colors for those

var vjGenomeColors = new Array();
for (var it = 0; it < gClrTable.length; ++it) vjGenomeColors[it] = pastelColor(gClrTable[(it + 1) % (gClrTable.length)],0.6); // make colors for those

//# sourceURL = getBaseUrl() + "/js/colors.js"