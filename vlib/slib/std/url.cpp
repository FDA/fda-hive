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

#include <slib/std/url.hpp>
#include <string>
#include <list>
#include <assert.h>

namespace slib {

using namespace std;

const udx NPOS = static_cast<udx>(-1);

static
int HexChar(char ch)
{
    unsigned int rc = ch - '0';
    if (rc <= 9) {
        return rc;
    } else {
        rc = (ch | ' ') - 'a';
        return rc <= 5 ? int(rc + 10) : -1;
    }
}

static const char s_Encode[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "+",   "!",   "%22", "%23", "$",   "%25", "%26", "'",
    "(",   ")",   "*",   "%2B", ",",   "-",   ".",   "%2F",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

static const char s_EncodeMarkChars[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "+",   "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2A", "%2B", "%2C", "%2D", "%2E", "%2F",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "%5F",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

static const char s_EncodePercentOnly[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2A", "%2B", "%2C", "%2D", "%2E", "%2F",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "%5F",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

static const char s_EncodePath[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "+",   "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2A", "%2B", "%2C", "%2D", ".",   "/",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

// RFC-2396:
// scheme        = alpha *( alpha | digit | "+" | "-" | "." )
static const char s_EncodeURIScheme[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2A", "+",   "%2C", "-",   ".",   "%2F",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "%5F",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

// RFC-2396:
// userinfo      = *( unreserved | escaped |
//                   ";" | ":" | "&" | "=" | "+" | "$" | "," )
// unreserved    = alphanum | mark
// mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
static const char s_EncodeURIUserinfo[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "!",   "%22", "%23", "$",   "%25", "&",   "'",
    "(",   ")",   "*",   "+",   ",",   "-",   ".",   "%2F",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   ":",   ";",   "%3C", "=",   "%3E", "%3F",
    "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

// RFC-2396:
// host          = hostname | IPv4address
// hostname      = *( domainlabel "." ) toplabel [ "." ]
// domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
// toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
// IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit
static const char s_EncodeURIHost[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2A", "%2B", "%2C", "-",   ".",   "%2F",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "%5F",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "%7E", "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

// RFC-2396:
// path_segments = segment *( "/" segment )
// segment       = *pchar *( ";" param )
// param         = *pchar
// pchar         = unreserved | escaped |
//                 ":" | "@" | "&" | "=" | "+" | "$" | ","
// unreserved    = alphanum | mark
// mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
static const char s_EncodeURIPath[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "!",   "%22", "%23", "$",   "%25", "&",   "'",
    "(",   ")",   "*",   "+",   ",",   "-",   ".",   "/",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   ":",   ";",   "%3C", "=",   "%3E", "%3F",
    "@",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

static const char s_EncodeURIQueryName[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "!",   "%22", "%23", "$",   "%25", "%26", "'",
    "(",   ")",   "%2A", "%2B", "%2C", "-",   ".",   "/",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   ":",   "%3B", "%3C", "%3D", "%3E", "?",
    "@",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

static const char s_EncodeURIQueryNameEscapeQuotes[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "!",   "%22", "%23", "$",   "%25", "%26", "%27",
    "(",   ")",   "%2A", "%2B", "%2C", "-",   ".",   "/",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   ":",   "%3B", "%3C", "%3D", "%3E", "?",
    "@",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

static const char s_EncodeURIQueryValue[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "!",   "%22", "%23", "$",   "%25", "%26", "'",
    "(",   ")",   "%2A", "%2B", "%2C", "-",   ".",   "/",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   ":",   "%3B", "%3C", "%3D", "%3E", "?",
    "@",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

static const char s_EncodeURIQueryValueEscapeQuotes[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "!",   "%22", "%23", "$",   "%25", "%26", "%27",
    "(",   ")",   "%2A", "%2B", "%2C", "-",   ".",   "/",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   ":",   "%3B", "%3C", "%3D", "%3E", "?",
    "@",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

// RFC-2396:
// fragment      = *uric
// uric          = reserved | unreserved | escaped
// reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
// unreserved    = alphanum | mark
// mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
static const char s_EncodeURIFragment[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "!",   "%22", "%23", "$",   "%25", "&",   "'",
    "(",   ")",   "*",   "+",   ",",   "-",   ".",   "/",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   ":",   ";",   "%3C", "=",   "%3E", "?",
    "@",   "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

// RFC-5987, RFC-2231, RFC-2045
// extended-initial-value := [charset] "'" [language] "'" extended-other-values
// extended-other-values := *(ext-octet / attribute-char)
// ext-octet := "%" 2(DIGIT / "A" / "B" / "C" / "D" / "E" / "F")
// attribute-char := <any (US-ASCII) CHAR except SPACE, CTLs, "*", "'", "%", or tspecials>
// tspecials :=  "(" / ")" / "<" / ">" / "@" / "," / ";" / ":" / "\" / <"> "/" / "[" / "]" / "?" / "="
static const char s_EncodeExtValue[256][4] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
    "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
    "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
    "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
    "%20", "!",   "%22", "#",   "$",   "%25", "&",   "%27",
    "%28", "%29", "%2A", "+",   "%2C", "-",   ".",   "/",
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",
    "8",   "9",   "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
    "%40", "A",   "B",   "C",   "D",   "E",   "F",   "G",
    "H",   "I",   "J",   "K",   "L",   "M",   "N",   "O",
    "P",   "Q",   "R",   "S",   "T",   "U",   "V",   "W",
    "X",   "Y",   "Z",   "%5B", "%5C", "%5D", "%5E", "_",
    "%60", "a",   "b",   "c",   "d",   "e",   "f",   "g",
    "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
    "p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",
    "x",   "y",   "z",   "%7B", "%7C", "%7D", "~",   "%7F",
    "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
    "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
    "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
    "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
    "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
    "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
    "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
    "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
    "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
    "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
    "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
    "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
    "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
    "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
    "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
    "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

const char* URLEncode(const char* str, sStr& dst, EUrlEncode flag)
{
    const idx len = sLen(str);
    if(!len) {
        return dst.add0cut();
    }
    const char (*encode_table)[4];
    switch(flag) {
        case eUrlEncode_SkipMarkChars:
            encode_table = s_Encode;
            break;
        case eUrlEncode_ProcessMarkChars:
            encode_table = s_EncodeMarkChars;
            break;
        case eUrlEncode_PercentOnly:
            encode_table = s_EncodePercentOnly;
            break;
        case eUrlEncode_Path:
            encode_table = s_EncodePath;
            break;
        case eUrlEncode_URIScheme:
            encode_table = s_EncodeURIScheme;
            break;
        case eUrlEncode_URIUserinfo:
            encode_table = s_EncodeURIUserinfo;
            break;
        case eUrlEncode_URIHost:
            encode_table = s_EncodeURIHost;
            break;
        case eUrlEncode_URIPath:
            encode_table = s_EncodeURIPath;
            break;
        case eUrlEncode_URIQueryName:
            encode_table = s_EncodeURIQueryName;
            break;
        case eUrlEncode_URIQueryNameEscapeQuotes:
            encode_table = s_EncodeURIQueryNameEscapeQuotes;
            break;
        case eUrlEncode_URIQueryValue:
            encode_table = s_EncodeURIQueryValue;
            break;
        case eUrlEncode_URIQueryValueEscapeQuotes:
            encode_table = s_EncodeURIQueryValueEscapeQuotes;
            break;
        case eUrlEncode_URIFragment:
            encode_table = s_EncodeURIFragment;
            break;
        case eUrlEncode_ExtValue:
            encode_table = s_EncodeExtValue;
            break;
        case eUrlEncode_None:
            return dst.printf("%s", str);
        default:
            return dst.add0cut();
            break;
    }

    string d("");
    if(encode_table) {
        idx pos;
        idx dst_len = len;
        const unsigned char* cstr = (const unsigned char*) str;
        for(pos = 0; pos < len; pos++) {
            if(encode_table[cstr[pos]][0] == '%')
                dst_len += 2;
        }
        d.resize(dst_len);

        idx p = 0;
        for(pos = 0; pos < len; pos++, p++) {
            const char* subst = encode_table[cstr[pos]];
            if(*subst != '%') {
                d[p] = *subst;
            } else {
                d[p] = '%';
                d[++p] = *(++subst);
                d[++p] = *(++subst);
            }
        }
        assert(p == dst_len);
    }
    return dst.printf("%s", d.c_str());
}

bool NeedsURLEncoding(const char* str, EUrlEncode flag)
{
    const idx len = sLen(str);
    if(!len) {
        return false;
    }
    const char (*encode_table)[4];
    switch(flag) {
        case eUrlEncode_SkipMarkChars:
            encode_table = s_Encode;
            break;
        case eUrlEncode_ProcessMarkChars:
            encode_table = s_EncodeMarkChars;
            break;
        case eUrlEncode_PercentOnly:
            encode_table = s_EncodePercentOnly;
            break;
        case eUrlEncode_Path:
            encode_table = s_EncodePath;
            break;
        case eUrlEncode_None:
            return false;
        default:
            // To keep off compiler warning
            encode_table = 0;
            break;
    }
    const unsigned char* cstr = (const unsigned char*) str;

    for(idx pos = 0; encode_table && pos < len; pos++) {
        const char* subst = encode_table[cstr[pos]];
        if(*subst != cstr[pos]) {
            return true;
        }
    }
    return false;
}

static
void s_URLDecode(const string& src, string& dst, EUrlDecode flag)
{
    idx len = src.length();
    if(!len) {
        dst.clear();
        return;
    }
    if(dst.length() < src.length()) {
        dst.resize(len);
    }

    idx pdst = 0;
    for(idx psrc = 0; psrc < len; pdst++) {
        switch(src[psrc]) {
            case '%': {
                // Accordingly RFC 1738 the '%' character is unsafe
                // and should be always encoded, but sometimes it is
                // not really encoded...
                if(psrc + 2 > len) {
                    dst[pdst] = src[psrc++];
                } else {
                    int n1 = HexChar(src[psrc + 1]);
                    int n2 = HexChar(src[psrc + 2]);
                    if(n1 < 0 || n1 > 15 || n2 < 0 || n2 > 15) {
                        dst[pdst] = src[psrc++];
                    } else {
                        dst[pdst] = (char)((n1 << 4) | n2);
                        psrc += 3;
                    }
                }
                break;
            }
            case '+': {
                dst[pdst] = (flag == eUrlDecode_All) ? ' ' : '+';
                psrc++;
                break;
            }
            default:
                dst[pdst] = src[psrc++];
                break;
        }
    }
    if(pdst < len) {
        dst.resize(pdst);
    }
}

const char* URLDecode(const char* str, sStr& dst, EUrlDecode flag)
{
    string src(str), d;
    s_URLDecode(src, d, flag);
    return dst.add(d.c_str());
}

/////////////////////////////////////////////////////////////////////////////
///
/// CUrlArgs_Parser::
///
/// Base class for arguments parsers.
///
class CUrlArgs_Parser
{
    public:
        CUrlArgs_Parser(void)
                : m_SemicolonIsNotArgDelimiter(false)
        {
        }
        virtual ~CUrlArgs_Parser(void)
        {
        }

        /// Parse query string, call AddArgument() to store each value.
        bool SetQueryString(const char* query, EUrlEncode encode);
        /// Parse query string, call AddArgument() to store each value.
        bool SetQueryString(const char* query, const IUrlEncoder* encoder = 0);

        /// Treat semicolon as query string argument separator
        void SetSemicolonIsNotArgDelimiter(bool enable = true)
        {
            m_SemicolonIsNotArgDelimiter = enable;
        }

    protected:
        /// Query type flag
        enum EArgType
        {
            eArg_Value, ///< Query contains name=value pairs
            eArg_Index
        ///< Query contains a list of names: name1+name2+name3
        };

        /// Process next query argument. Must be overriden to process and store
        /// the arguments.
        /// @param position
        ///   1-based index of the argument in the query.
        /// @param name
        ///   Name of the argument.
        /// @param value
        ///   Contains argument value if query type is eArg_Value or
        ///   empty string for eArg_Index.
        /// @param arg_type
        ///   Query type flag.
        virtual void AddArgument(unsigned int position, const string& name, const string& value, EArgType arg_type = eArg_Index) = 0;
    private:
        bool x_SetIndexString(const string& query, const IUrlEncoder& encoder);

        bool m_SemicolonIsNotArgDelimiter;
};

bool CUrlArgs_Parser::SetQueryString(const char* query, EUrlEncode encode)
{
    CDefaultUrlEncoder encoder(encode);
    return SetQueryString(query, &encoder);
}

bool CUrlArgs_Parser::x_SetIndexString(const string& query, const IUrlEncoder& encoder)
{
    size_t len = query.size();
    assert(len);

    // No '=' and spaces must be present in the parsed string
    assert(query.find_first_of("= \t\r\n") == NPOS);

    // Parse into indexes
    unsigned int position = 1;
    for(udx beg = 0; beg < len;) {
        udx end = query.find('+', beg);
        // Skip leading '+' (empty value).
        if(end == beg) {
            beg++;
            continue;
        }
        if(end == NPOS) {
            end = len;
        }
        sStr in, out;
        in.replace( query.substr(beg, end - beg).c_str());
        AddArgument(position++, encoder.DecodeArgName(in, out), "", eArg_Index);
        beg = end + 1;
    }
    return true;
}

bool CUrlArgs_Parser::SetQueryString(const char* query, const IUrlEncoder* encoder)
{
    if(!encoder) {
        encoder = CUrl::GetDefaultEncoder();
    }
    // Parse and decode query string
    if(!query) {
        return true;
    }
    string qstr(query);
    udx len = qstr.length();
    if(!query) {
        return true;
    }
    {
        {
            // No spaces are allowed in the parsed string
            udx err_pos = qstr.find_first_of(" \t\r\n");
            if(err_pos != NPOS) {
                return false;
                //throw runtime_error("Space character in URL arguments: \"" + qstr + "\"", err_pos + 1);
            }
        }
    }

    // If no '=' present in the parsed string then try to parse it as ISINDEX
    // RFC3875
    if(qstr.find("=") == NPOS) {
        return x_SetIndexString(qstr, *encoder);
    }

    // Parse into entries
    unsigned int position = 1;
    for(udx beg = 0; beg < len;) {
        // ignore ampersand and "&amp;"
        if(qstr[beg] == '&') {
            ++beg;
            if(beg < len && strncasecmp(&qstr[beg], "amp;", 4) == 0) {
                beg += 4;
            }
            continue;
        }
        // Alternative separator - ';'
        else if(!m_SemicolonIsNotArgDelimiter && qstr[beg] == ';') {
            ++beg;
            continue;
        }

        // parse and URL-decode name
        string mid_seps = "=&";
        string end_seps = "&";
        if(!m_SemicolonIsNotArgDelimiter) {
            mid_seps += ';';
            end_seps += ';';
        }

        udx mid = qstr.find_first_of(mid_seps, beg);
        // '=' is the first char (empty name)? Skip to the next separator.
        if(mid == beg) {
            beg = qstr.find_first_of(end_seps, beg);
            if(beg == NPOS)
                break;
            continue;
        }
        if(mid == NPOS) {
            mid = len;
        }

        sStr out;
        string name(encoder->DecodeArgName(qstr.substr(beg, mid - beg).c_str(), out));

        // parse and URL-decode value(if any)
        string value;
        if(qstr[mid] == '=') { // has a value
            mid++;
            udx end = qstr.find_first_of(end_seps, mid);
            if(end == NPOS) {
                end = len;
            }
            value = encoder->DecodeArgValue(qstr.substr(mid, end - mid).c_str(), out);

            beg = end;
        } else { // has no value
            beg = mid;
        }

        // store the name-value pair
        AddArgument(position++, name, value, eArg_Value);
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////////
///
/// CUrlArgs::
///
/// URL arguments list.
///
class CUrlArgs: public CUrlArgs_Parser
{
    public:
        /// Create an empty arguments set.
        CUrlArgs(void);
        /// Parse the query string, store the arguments.
        CUrlArgs(const char* query, EUrlEncode decode);
        /// Parse the query string, store the arguments.
        CUrlArgs(const char* query, const IUrlEncoder* encoder = 0);

        /// Construct and return complete query string. Use selected amp
        /// and name/value encodings.
        string GetQueryString(EAmpEncoding amp_enc, EUrlEncode encode) const;
        /// Construct and return complete query string. Use selected amp
        /// and name/value encodings.
        string GetQueryString(EAmpEncoding amp_enc, const IUrlEncoder* encoder = 0) const;

        /// Name-value pair.
        struct SUrlArg
        {
                SUrlArg(const string& aname, const string& avalue)
                        : name(aname), value(avalue)
                {
                }
                string name;
                string value;
        };
        typedef SUrlArg TArg;
        typedef list<TArg> TArgs;
        typedef TArgs::iterator iterator;
        typedef TArgs::const_iterator const_iterator;

        /// Check if an argument with the given name exists.
        bool IsSetValue(const string& name) const
        {
            return FindFirst(name) != m_Args.end();
        }

        /// Get value for the given name. finds first of the arguments with the
        /// given name. If the name does not exist, is_found is set to false.
        /// If is_found is null, CUrlArgsException is thrown.
        const string* GetValue(const string& name) const;

        /// Set new value for the first argument with the given name or
        /// add a new argument.
        void SetValue(const string& name, const string& value);

        /// Get the const list of arguments.
        const TArgs& GetArgs(void) const
        {
            return m_Args;
        }

        /// Get the list of arguments.
        TArgs& GetArgs(void)
        {
            return m_Args;
        }

        /// Find the first argument with the given name. If not found, return
        /// GetArgs().end().
        iterator FindFirst(const string& name)
        {
            return x_Find(name, m_Args.begin());
        }

        /// Take argument name from the iterator, find next argument with the same
        /// name, return GetArgs().end() if not found.
        iterator FindNext(const iterator& iter)
        {
            return x_Find(iter->name, iter);
        }

        /// Find the first argument with the given name. If not found, return
        /// GetArgs().end().
        const_iterator FindFirst(const string& name) const
        {
            return x_Find(name, m_Args.begin());
        }

        /// Take argument name from the iterator, find next argument with the same
        /// name, return GetArgs().end() if not found.
        const_iterator FindNext(const const_iterator& iter) const
        {
            return x_Find(iter->name, iter);
        }

        /// Select case sensitivity of arguments' names.
        void SetCase(bool is_name_case_sensitivite = true)
        {
            m_name_case_sensitivite = is_name_case_sensitivite;
        }

    protected:
        virtual void AddArgument(unsigned int position, const string& name, const string& value, EArgType arg_type);
    private:
        iterator x_Find(const string& name, const iterator& start);
        const_iterator x_Find(const string& name, const const_iterator& start) const;

        bool m_name_case_sensitivite;
        bool m_IsIndex;
        TArgs m_Args;
};

CUrlArgs::CUrlArgs(void)
        : m_name_case_sensitivite(false), m_IsIndex(false)
{
    return;
}

CUrlArgs::CUrlArgs(const char* query, EUrlEncode decode)
        : m_name_case_sensitivite(false), m_IsIndex(false)
{
    SetQueryString(query, decode);
}

CUrlArgs::CUrlArgs(const char* query, const IUrlEncoder* encoder)
        : m_name_case_sensitivite(false), m_IsIndex(false)
{
    SetQueryString(query, encoder);
}

void CUrlArgs::AddArgument(unsigned int /* position */, const string& name, const string& value, EArgType arg_type)
{
    if(arg_type == eArg_Index) {
        m_IsIndex = true;
    } else {
        assert(!m_IsIndex);
    }
    m_Args.push_back(TArg(name, value));
}

string CUrlArgs::GetQueryString(EAmpEncoding amp_enc, EUrlEncode encode) const
{
    CDefaultUrlEncoder encoder(encode);
    return GetQueryString(amp_enc, &encoder);
}

string CUrlArgs::GetQueryString(EAmpEncoding amp_enc, const IUrlEncoder* encoder) const
{
    if(!encoder) {
        encoder = CUrl::GetDefaultEncoder();
    }
    // Encode and construct query string
    string query;
    const char* amp = (amp_enc == eAmp_Char) ? "&" : "&amp;";
    for(TArgs::const_iterator arg = m_Args.begin(), arg_end = m_Args.end(); arg != arg_end; ++arg) {
        if(!query.empty()) {
            query += m_IsIndex ? "+" : amp;
        }
        sStr out;
        query += encoder->EncodeArgName(arg->name.c_str(), out);
        if(!m_IsIndex) {
            query += "=";
            query += encoder->EncodeArgValue(arg->value.c_str(), out);
        }
    }
    return query;
}

const string* CUrlArgs::GetValue(const string& name) const
{
    const_iterator iter = FindFirst(name);
    if(iter != m_Args.end()) {
        return &iter->value;
    }
    return 0;
}

void CUrlArgs::SetValue(const string& name, const string& value)
{
    m_IsIndex = false;
    iterator it = FindFirst(name);
    if(it != m_Args.end()) {
        it->value = value;
    } else {
        m_Args.push_back(TArg(name, value));
    }
}

CUrlArgs::iterator CUrlArgs::x_Find(const string& name, const iterator& start)
{
    for(iterator it = start; it != m_Args.end(); ++it) {
        if( m_name_case_sensitivite && strcmp(it->name.c_str(), name.c_str()) == 0 ) {
            return it;
        } else if( strcasecmp(it->name.c_str(), name.c_str()) == 0 ) {
            return it;
        }
    }
    return m_Args.end();
}

CUrlArgs::const_iterator CUrlArgs::x_Find(const string& name, const const_iterator& start) const
{
    for(const_iterator it = start; it != m_Args.end(); ++it) {
        if( m_name_case_sensitivite && strcmp(it->name.c_str(), name.c_str()) == 0 ) {
            return it;
        } else if( strcasecmp(it->name.c_str(), name.c_str()) == 0 ) {
            return it;
        }
    }
    return m_Args.end();
}

//////////////////////////////////////////////////////////////////////////////
//
// CUrl
//

CUrl::CUrl(void)
        : m_IsGeneric(false)
{
    return;
}

CUrl::CUrl(const char* url, const IUrlEncoder* encoder)
        : m_IsGeneric(false)
{
    if( url ) {
        SetUrl(url, encoder);
    }
}

CUrl::CUrl(const CUrl& url)
{
    *this = url;
}

CUrl::~CUrl()
{
    m_ArgsList.reset(0);
}

CUrl& CUrl::operator=(const CUrl& url)
{
    if(this != &url) {
        m_Scheme.replace(url.m_Scheme);
        m_IsGeneric = url.m_IsGeneric;
        m_User.replace(url.m_User);
        m_Password.replace(url.m_Password);
        m_Host.replace(url.m_Host);
        m_Port.replace(url.m_Port);
        m_Path.replace(url.m_Path);
        m_Fragment.replace(url.m_Fragment);
        m_OrigArgs.replace(url.m_OrigArgs);
        if(url.m_ArgsList.get()) {
            m_ArgsList.reset(new CUrlArgs(*url.m_ArgsList));
        }
    }
    return *this;
}

bool CUrl::SetUrl(const char* orig_url, const IUrlEncoder* encoder)
{
    m_Scheme.empty();
    m_IsGeneric = false;
    m_User.empty();
    m_Password.empty();
    m_Host.empty();
    m_Port.empty();
    m_Path.empty();
    m_Fragment.empty();
    m_OrigArgs.empty();
    m_ArgsList.reset();

    string url;
    string orig(orig_url);

    if(!encoder) {
        encoder = GetDefaultEncoder();
    }

    udx frag_pos = orig.find_last_of("#");
    if(frag_pos != NPOS) {
        x_SetFragment(orig.substr(frag_pos + 1, orig.size()).c_str(), *encoder);
        url = orig.substr(0, frag_pos);
    } else {
        url = orig;
    }

    bool skip_host = false;
    bool skip_path = false;
    udx beg = 0;
    udx pos = url.find_first_of(":@/?[");

    while(beg < url.size()) {
        if(pos == NPOS) {
            if(!skip_host) {
                x_SetHost(url.substr(beg, url.size()).c_str(), *encoder);
            } else if(!skip_path) {
                x_SetPath(url.substr(beg, url.size()).c_str(), *encoder);
            } else {
                x_SetArgs(url.substr(beg, url.size()).c_str(), *encoder);
            }
            break;
        }
        switch(url[pos]) {
            case '[': // IPv6 address
            {
                udx closing = url.find(']', pos);
                if(closing == NPOS) {
                    return false;
                    //NCBI_THROW2(CUrlParserException, eFormat, "Unmatched '[' in the URL: \"" + url + "\"", pos);
                }
                beg = pos;
                pos = url.find_first_of(":/?", closing);
                break;
            }
            case ':': // scheme: || user:password || host:port
            {
                if(url.substr(pos, 3) == "://") {
                    // scheme://
                    x_SetScheme(url.substr(beg, pos - beg).c_str(), *encoder);
                    beg = pos + 3;
                    m_IsGeneric = true;
                    if( strcmp(m_Scheme.ptr(), "file") == 0 ) {
                        // Special case - no further parsing, use the whole
                        // string as path.
                        x_SetPath(url.substr(beg).c_str(), *encoder);
                        return true;
                    }
                    pos = url.find_first_of(":@/?[", beg);
                    break;
                }
                // user:password@ || host:port...
                udx next = url.find_first_of("@/?[", pos + 1);
                if(m_IsGeneric && next != NPOS && url[next] == '@') {
                    // user:password@
                    x_SetUser(url.substr(beg, pos - beg).c_str(), *encoder);
                    beg = pos + 1;
                    x_SetPassword(url.substr(beg, next - beg).c_str(), *encoder);
                    beg = next + 1;
                    pos = url.find_first_of(":/?[", beg);
                    break;
                }
                // host:port || host:port/path || host:port?args
                string host = url.substr(beg, pos - beg);
                beg = pos + 1;
                if(next == NPOS) {
                    next = url.size();
                }
                x_SetPort(url.substr(beg, next - beg).c_str(), *encoder);
                if(!skip_host) {
                    x_SetHost(host.c_str(), *encoder);
                }
                skip_host = true;
                beg = next;
                if(next < url.size() && url[next] == '/') {
                    pos = url.find_first_of("?", beg);
                } else {
                    skip_path = true;
                    pos = next;
                }
                break;
            }
            case '@': // username@host
            {
                x_SetUser(url.substr(beg, pos - beg).c_str(), *encoder);
                beg = pos + 1;
                pos = url.find_first_of(":/?[", beg);
                break;
            }
            case '/': // host/path
            {
                if(!skip_host) {
                    x_SetHost(url.substr(beg, pos - beg).c_str(), *encoder);
                    skip_host = true;
                }
                beg = pos;
                pos = url.find_first_of("?", beg);
                break;
            }
            case '?': {
                if(!skip_host) {
                    x_SetHost(url.substr(beg, pos - beg).c_str(), *encoder);
                    skip_host = true;
                } else {
                    x_SetPath(url.substr(beg, pos - beg).c_str(), *encoder);
                    skip_path = true;
                }
                beg = pos + 1;
                x_SetArgs(url.substr(beg, url.size()).c_str(), *encoder);
                beg = url.size();
                pos = NPOS;
                break;
            }
        }
    }
    return true;
}

const char* CUrl::ComposeUrl(EAmpEncoding amp_enc, sStr& dst, const IUrlEncoder* encoder) const
{
    if( !encoder ) {
        encoder = GetDefaultEncoder();
    }
    if( m_Scheme ) {
        dst.printf("%s", m_Scheme.ptr());
        dst.printf(m_IsGeneric ? "://" : ":");
    }
    if( m_User ) {
        encoder->EncodeUser(m_User, dst);
        if( m_Password ) {
            dst.printf(":");
            encoder->EncodePassword(m_Password, dst);
        }
        dst.printf("@");
    }
    dst.printf("%s", m_Host.ptr());
    if( m_Port ) {
        dst.printf(":%s", m_Port.ptr());
    }
    encoder->EncodePath(m_Path, dst);
    if( HaveArgs() ) {
        string a(m_ArgsList->GetQueryString(amp_enc, encoder));
        dst.printf("?%s", a.c_str());
    }
    if( m_Fragment ) {
        dst.printf("#");
        encoder->EncodeFragment(m_Fragment, dst);
    }
    return dst;
}

const char* CUrl::GetQueryString(EAmpEncoding amp_enc, sStr& dst, EUrlEncode encode) const
{
    string x(GetArgs()->GetQueryString(amp_enc, encode));
    return dst.add(x.c_str());
}

const char* CUrl::GetQueryString(EAmpEncoding amp_enc, sStr& dst, const IUrlEncoder* encoder) const
{
    string x(GetArgs()->GetQueryString(amp_enc, encoder));
    return dst.add(x.c_str());
}

bool CUrl::IsSetValue(const char* name) const
{
    return GetArgs()->IsSetValue(name);
}

const char* CUrl::GetValue(const char* name) const
{
    const string* p = GetArgs()->GetValue(name);
    return p ? p->c_str() : 0;
}

void CUrl::SetValue(const char* name, const char* value)
{
    GetArgs()->SetValue(name, value);
}

bool CUrl::HaveArgs(void) const
{
    return m_ArgsList.get() != 0 && !m_ArgsList->GetArgs().empty();
}

const CUrlArgs* CUrl::GetArgs(void) const
{
    if(!m_ArgsList.get()) {
        return 0;
        //NCBI_THROW(CUrlException, eNoArgs, "The URL has no arguments");
    }
    return m_ArgsList.get();
}

CUrlArgs* CUrl::GetArgs(void)
{
    if(!m_ArgsList.get()) {
        x_SetArgs(0, *GetDefaultEncoder());
    }
    return m_ArgsList.get();
}

//////////////////////////////////////////////////////////////////////////////
//
// Url encode/decode
//

IUrlEncoder* CUrl::GetDefaultEncoder(void)
{
    static CDefaultUrlEncoder s_DefaultEncoder;
    return &s_DefaultEncoder;
}

void CUrl::x_SetArgs(const char* args, const IUrlEncoder& encoder)
{
    m_OrigArgs.replace(args);
    m_ArgsList.reset(new CUrlArgs(m_OrigArgs.ptr(), &encoder));
}

};
