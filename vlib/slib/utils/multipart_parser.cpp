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
#include <slib/utils/multipart_parser.hpp>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

using namespace slib;

#define DEBUG_MULTIPART

#if _DEBUG_off
#define multipart_log(fmt, ...) ::fprintf(stderr, "%s:%d: "fmt"\n", __FILE__, __LINE__, __VA_ARGS__)
#else
#define multipart_log(fmt, ...)
#endif

#define NOTIFY_CB(FOR) \
    do { \
        if( !on_##FOR() ) { \
            return false; \
        } \
    } while(false) \

#define EMIT_DATA_CB(FOR, ptr, len) \
    do { \
        if( !on_##FOR(ptr, len) ) { \
            return false; \
        } \
    } while(false) \

bool sMultipartParser::parse(const char * boundary, udx boundaryLen /* = 0 */)
{
    sStr bnd;
    if( boundary ) {
        if( !boundaryLen ) {
            boundaryLen = sLen(boundary);
        }
        bnd.printf("--%.*s", (int) boundaryLen, boundary);
        boundaryLen += 2;
    }
    if( !bnd ) {
        multipart_log("Invalid boundary '%s'", boundary ? boundary : "(null)");
        return false;
    }
    bnd.add0(boundaryLen + 10);
    char * lookbehind = bnd.ptr(boundaryLen + 1);

    udx i = 0, mark = 0, index = 0;
    bool isLast = false;
    char c, cl;
    const char LF = 10, CR = 13;
    const char * buf = 0;
    udx len = 0;

    enum state
    {
        s_uninitialized = 1,
        s_start,
        s_start_boundary,
        s_header_field_start,
        s_header_field,
        s_header_value_start,
        s_header_value,
        s_header_value_almost_done,
        s_headers_almost_done,
        s_part_data_start,
        s_part_data,
        s_part_data_almost_boundary,
        s_part_data_boundary,
        s_part_data_almost_end,
        s_part_data_end,
        s_part_data_final_hyphen,
        s_end
    } state = s_start;

    EMIT_DATA_CB(next_chunk, &buf, len);
    if( len == 0 ) {
        return false;
    }
    while( i < len ) {
        c = buf[i];
        isLast = (i == (len - 1));
        switch(state) {
            case s_start:
                multipart_log("%s", "s_start");
                index = 0;
                state = s_start_boundary;
                /* no break */

            case s_start_boundary:
                if( index == boundaryLen ) {
                    if( c != CR ) {
                        return false;
                    }
                    index++;
                    break;
                } else if( index == boundaryLen + 1 ) {
                    if( c != LF ) {
                        return false;
                    }
                    index = 0;
                    NOTIFY_CB(part_data_begin);
                    state = s_header_field_start;
                    multipart_log("%s", "s_header_field_start");
                    break;
                }
                if( c != bnd.ptr(index)[0] ) {
                    return false;
                }
                index++;
                break;

            case s_header_field_start:
                mark = i;
                state = s_header_field;
                /* no break */

            case s_header_field:
                if( c == CR ) {
                    state = s_headers_almost_done;
                    break;
                }
                if( c == '-' ) {
                    break;
                }
                if( c == ':' ) {
                    multipart_log("header '%.*s'", (int)(i - mark), buf + mark);
                    EMIT_DATA_CB(header_field, buf + mark, i - mark);
                    state = s_header_value_start;
                    break;
                }
                cl = tolower(c);
                if( cl < 'a' || cl > 'z' ) {
                    multipart_log("invalid character in header name '%c'", cl);
                    return false;
                }
                if( isLast ) {
                    buf = &buf[mark]; // shift buff to start of file name
                    len -= mark;
                    EMIT_DATA_CB(next_chunk, &buf, len);
                    mark = i = 0;
                }
                break;

            case s_header_value_start:
                if( c == ' ' ) {
                    break;
                }
                mark = i;
                state = s_header_value;
                multipart_log("%s", "s_header_value_start");
                /* no break */

            case s_header_value:
                //multipart_log("%s", "s_header_value");
                if( c == CR ) { // final piece
                    multipart_log("header value '%.*s'", (int)(i - mark), buf + mark);
                    EMIT_DATA_CB(header_value, buf + mark, i - mark);
                    state = s_header_value_almost_done;
                }
                if( isLast ) { // intermediate piece
                    multipart_log("header value (last) '%.*s'", (int)(i - mark + 1), buf + mark);
                    EMIT_DATA_CB(header_value, buf + mark, (i - mark) + 1);
                }
                break;

            case s_header_value_almost_done:
                multipart_log("%s", "s_header_value_almost_done");
                if( c != LF ) {
                    return false;
                }
                state = s_header_field_start;
                break;

            case s_headers_almost_done:
                multipart_log("%s", "s_headers_almost_done");
                if( c != LF ) {
                    return false;
                }
                state = s_part_data_start;
                break;

            case s_part_data_start:
                multipart_log("%s", "s_part_data_start");
                NOTIFY_CB(headers_complete);
                mark = i;
                state = s_part_data;
                /* no break */

            case s_part_data:
                if( c == CR ) {
                    multipart_log("%s %" UDEC ":%" UDEC "=%" UDEC "%s", "s_part_data", mark, i - 1, i - mark, isLast ? " last" : "");
                    EMIT_DATA_CB(part_data, buf + mark, i - mark);
                    mark = i;
                    state = s_part_data_almost_boundary;
                    lookbehind[0] = CR;
                    break;
                }
                if( isLast ) {
                    multipart_log("%s %" UDEC ":%" UDEC "=%" UDEC " last", "s_part_data", mark, i + 1, i - mark + 1);
                    EMIT_DATA_CB(part_data, buf + mark, i - mark + 1);
                }
                break;

            case s_part_data_almost_boundary:
                if( c == LF ) {
                    multipart_log("%s", "s_part_data_almost_boundary LF");
                    state = s_part_data_boundary;
                    lookbehind[1] = LF;
                    index = 0;
                    break;
                }
                multipart_log("%s 1 0x%02X%s", "s_part_data_almost_boundary", (int)(lookbehind[0]), isLast ? " last" : "");
                EMIT_DATA_CB(part_data, lookbehind, 1);
                if( isLast ) {
                    multipart_log("%s %" UDEC ":%" UDEC "=0x%02X last", "s_part_data_almost_boundary", i, i, (int)buf[i]);
                    EMIT_DATA_CB(part_data, buf + i, 1);
                }
                state = s_part_data;
                mark = i--;
                break;


            case s_part_data_boundary:
                if( bnd.ptr(index)[0] != c ) {
                    multipart_log("%s %" UDEC, "s_part_data_boundary", 2 + index);
                    EMIT_DATA_CB(part_data, lookbehind, 2 + index);
                    if( isLast ) {
                        multipart_log("%s %" UDEC ":%" UDEC "=0x%02X last", "s_part_data_boundary", i, i, (int)buf[i]);
                        EMIT_DATA_CB(part_data, buf + i, 1);
                    }
                    state = s_part_data;
                    mark = i--;
                    break;
                }
                multipart_log("%s", "s_part_data_boundary");
                lookbehind[2 + index] = c;
                if( (++index) == boundaryLen ) {
                    NOTIFY_CB(part_data_end);
                    state = s_part_data_almost_end;
                }
                break;

            case s_part_data_almost_end:
                multipart_log("%s", "s_part_data_almost_end");
                if( c == '-' ) {
                    state = s_part_data_final_hyphen;
                    break;
                }
                if( c == CR ) {
                    state = s_part_data_end;
                    break;
                }
                return false;

            case s_part_data_final_hyphen:
                multipart_log("%s", "s_part_data_final_hyphen");
                if( c == '-' ) {
                    NOTIFY_CB(body_end);
                    state = s_end;
                    break;
                }
                return false;

            case s_part_data_end:
                multipart_log("%s", "s_part_data_end");
                if( c == LF ) {
                    state = s_header_field_start;
                    NOTIFY_CB(part_data_begin);
                    break;
                }
                return false;

            case s_end:
                multipart_log("s_end: %02X", (int) c);
                break;

            default:
                multipart_log("%s", "Multipart parser unrecoverable error");
                return false;
        }
        ++i;
        if( isLast ) {
            len = 0;
            EMIT_DATA_CB(next_chunk, &buf, len);
            mark = i = 0;
            if( len == 0 && state != s_end ) {
                return false;
            }
        }
    }
    return i == len;
}
