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

#include <slib/std/regexp.hpp>
#include <slib/std/string.hpp>

#include <ctype.h>
#include <sys/types.h>

#ifdef HAS_PCRE
#include <pcre.h>
#else
#include <regex.h>
#endif

using namespace slib;

bool sRegExp::use_jit = true;

struct sRegExp::Priv {
    idx nmatches;
#ifdef HAS_PCRE
    pcre * re;
    pcre_extra * extra;
    int matches_head[3];
    typedef int Tmatch;
#else
    regex_t re;
    bool initialized;
    regmatch_t matches_head[1];
    typedef regmatch_t Tmatch;
#endif

    bool ok() const
    {
#ifdef HAS_PCRE
        return re;
#else
        return initialized;
#endif
    }

    void destroy()
    {
#ifdef HAS_PCRE
        if( re ) {
            pcre_free(re);
            re = 0;
        }
        if( extra ) {
            pcre_free_study(extra);
            extra = 0;
        }
#else
        if( initialized ) {
            regfree(&re);
            initialized = false;
        }
#endif
    }

    static idx sizeWithSubmatches(idx submatches)
    {
#ifdef HAS_PCRE
        return sizeof(Priv) + 3 * submatches * sizeof(int);
#else
        return sizeof(Priv) + submatches * sizeof(regmatch_t);
#endif
    }

    static Priv * ensure(sMex & mex, idx submatches = 0)
    {
        if( mex.pos() ) {
            mex.resize(sizeWithSubmatches(submatches));
        } else {
            mex.resize(sizeWithSubmatches(submatches));
            memset(mex.ptr(), 0, sizeof(Priv));
        }
        Priv * priv = static_cast<Priv*>(mex.ptr());
        priv->nmatches = submatches + 1;
        return priv;
    }

    idx posMatch(idx imatch)
    {
#ifdef HAS_PCRE
        return matches_head[imatch * 2];
#else
        return matches_head[imatch].rm_so;
#endif
    }

    idx sizeMatch(idx imatch)
    {
#ifdef HAS_PCRE
        return matches_head[imatch * 2 + 1] - matches_head[imatch * 2];
#else
        return matches_head[imatch].rm_eo - matches_head[imatch].rm_so;
#endif
    }

    Tmatch * matchesList() { return matches_head; }
    idx matchesListDim(idx max_levels = -1)
    {
        if( max_levels < 0 ) {
            max_levels = sIdxMax;
        }
        max_levels = sMin<idx>(nmatches, max_levels);
#ifdef HAS_PCRE
        return max_levels * 3;
#else
        return max_levels;
#endif
    }

    bool regexec(const char * str, idx pos, idx len, idx max_levels = -1)
    {
        int rc = 0;
        bool matched = false;
        idx matches_list_dim = matchesListDim(max_levels);
#ifdef HAS_PCRE
        rc = pcre_exec(re, extra, str, len, pos, 0, matchesList(), matches_list_dim);
        matched = (rc >= 0);
#else
        rc = ::regexec(&re, str + pos, matches_list_dim, matchesList(), pos ? REG_NOTBOL : 0);
        matched = (rc == 0);
        if( matched ) {
            for(idx i=0; i<matches_list_dim; i++) {
                regmatch_t & m = matchesList()[i];
                if( m.rm_so >= 0 ) {
                    m.rm_so += pos;
                    m.rm_eo += pos;
                }
            }
        }
#endif
        return matched;
    }
};

//static
bool sRegExp::isPCRE()
{
#ifdef HAS_PCRE
    return true;
#else
    return false;
#endif
}

bool sRegExp::ok() const
{
    return _mex.pos() ? static_cast<const Priv*>(_mex.ptr())->ok() : 0;
}

sRegExp::Priv * sRegExp::getPriv()
{
    return static_cast<Priv*>(_mex.ptr());
}

bool sRegExp::init(const char * pat, idx flags)
{
    if( !pat ) {
        if( Priv * priv = getPriv() ) {
            priv->destroy();
        }
        _flags = 0;
        return false;
    }

    Priv * priv = Priv::ensure(_mex);
    priv->destroy();
    _flags = flags;

    static bool try_getenv_regexp_jit = true;
    if( try_getenv_regexp_jit ) {
         const char * s = getenv("REGEXP_JIT");
         sRegExp::use_jit = s ? sString::parseBool(s) : true;
         try_getenv_regexp_jit = false;
    }

#ifdef HAS_PCRE
    int cflags = PCRE_NEWLINE_ANYCRLF;
    if( flags & fIgnoreCase ) {
        cflags |= PCRE_CASELESS;
    }
    if( flags & fMultiline ) {
        cflags |= PCRE_MULTILINE;
    }
    const char * err_str = 0;
    int err_offset = 0;
    priv->re = pcre_compile(pat, cflags, &err_str, &err_offset, 0);
    if( priv->re ) {
        if( sRegExp::use_jit ) {
            priv->extra = pcre_study(priv->re, PCRE_STUDY_JIT_COMPILE, &err_str);
        }
    } else {
        _err.printf(0, "Pattern '%s': char %d: %s", pat, err_offset, err_str);
    }
    int submatches = 0;
    if( pcre_fullinfo(priv->re, priv->extra, PCRE_INFO_CAPTURECOUNT, &submatches) == 0 ) {
        priv = Priv::ensure(_mex, submatches);
    }
#else
    int cflags = REG_EXTENDED;
    if( flags & fIgnoreCase ) {
        cflags |= REG_ICASE;
    }
    if( flags & fMultiline ) {
        // FIXME - not quite right semantics. POSIX regexp makes '.' match newline unless REG_NEWLINE, but
        // we want to '.' never match a newline
        cflags |= REG_NEWLINE;
    }

    if( int errcode = regcomp(&priv->re, pat, cflags) ) {
        _err.resize(regerror(errcode, &priv->re, 0, 0));
        _err.cut0cut(regerror(errcode, &priv->re, _err.ptr(), _err.total()));
        priv->destroy();
        return false;
    }

    priv->initialized = true;
    idx submatches = 0;
    for(const char * paren = strchr(pat, '('); paren; paren = strchr(paren + 1, '('), submatches++);
    priv = Priv::ensure(_mex, submatches);
#endif

    return priv->ok();
}

void sRegExp::destroy()
{
    if( _mex.pos() ) {
        Priv::ensure(_mex)->destroy();
        _mex.cut(0);
    }
}

const char * sRegExp::search(const char * str, idx len, idx * out_len_matched)
{
    if( !ok() || !str ) {
        return 0;
    }
    Priv * priv = getPriv();
    if( len ) {
#ifndef HAS_PCRE
        // POSIX regexp API requires 0-terminated strings
        _err.cutAddString(0, str, len);
        str = _err.ptr();
        _err.cut(0);
#endif
    } else {
        len = sLen(str);
    }
    if( priv->regexec(str, 0, len, 1) ) {
        if( out_len_matched ) {
            *out_len_matched = priv->sizeMatch(0);
        }
        return str + priv->posMatch(0);
    } else {
        if( out_len_matched ) {
            *out_len_matched = -1;
        }
        return 0;
    }
}

static idx nextUnicodeSym(const char * str, idx pos, idx len, bool multiline)
{
    if( multiline && pos + 1 < len && str[pos] == '\r' && str[pos + 1] == '\n' ) {
        pos += 2;
    } else {
        pos++;
        while(pos < len) {
            unsigned char u = (unsigned char)str[pos];
            if( (u & 0x80) && !(u & 0x40) ) {
                pos++;
            } else {
                break;
            }
        }
    }
    return pos;
}

idx sRegExp::exec(sVec<sMex::Pos> & res, const char * str, idx len)
{
    if( !ok() || !str ) {
        return 0;
    }
    Priv * priv = getPriv();
    idx nmatches = 0;

    if( len ) {
#ifndef HAS_PCRE
        // POSIX regexp API requires 0-terminated strings
        _err.cutAddString(0, str, len);
        str = _err.ptr();
        _err.cut(0);
#endif
    } else {
        len = sLen(str);
    }

    idx pos = 0;
    bool matched = false;
    do {
        matched = priv->regexec(str, pos, len, _flags & fGlobal ? 1 : -1);
        if( matched ) {
            idx cur_nmatches;
            if( _flags & fGlobal ) {
                cur_nmatches = 1;
            } else {
                cur_nmatches = priv->nmatches;
                while(cur_nmatches > 1 && priv->posMatch(cur_nmatches - 1) < 0) {
                    cur_nmatches--;
                }
            }
            nmatches += cur_nmatches;
            idx res_start = res.dim();
            res.addM(cur_nmatches);
            for(idx i = 0; i < cur_nmatches; i++) {
                res[res_start + i].pos = priv->posMatch(i);
                res[res_start + i].size = priv->sizeMatch(i);
            }
            pos = res[res_start].pos + res[res_start].size;
            if( res[res_start].size == 0 ) {
                pos = nextUnicodeSym(str, pos, len, _flags & fMultiline);
            }
        }
    } while( matched && (_flags & fGlobal) && len > 0 && pos < len );

    return nmatches;
}

namespace {
    struct Replacement {
        enum EType {
            eRawString = 0,
            eRawDollar,
            eMatch,
            eSubmatch,
            eBefore,
            eAfter
        };

        struct Elt {
            idx start;
            idx len;
            idx isubmatch;
            EType type;
        };

        const char * string;
        idx len;
        sVec<Elt> elts;

        static EType nextDollarCode(const char * s, idx * len_before)
        {
            *len_before = 0;
            while( 1 ) {
                if( const char * dol = strchr(s + *len_before, '$') ) {
                    *len_before = dol - s;
                    switch(dol[1]) {
                        case '$':
                            return eRawDollar;
                        case '&':
                            return eMatch;
                        case '`':
                            return eBefore;
                        case '\'':
                            return eAfter;
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            return eSubmatch;
                        default:
                            *len_before += 2;
                    }
                } else {
                    *len_before += sLen(s + *len_before);
                    break;
                }
            }
            return eRawString;
        }

        Replacement(const char * repl_)
        {
            string = repl_ ? repl_ : sStr::zero;
            len = sLen(string);

            idx cur_start = 0;
            while( cur_start < len ) {
                idx raw_len = 0;
                EType next_type = nextDollarCode(string + cur_start, &raw_len);
                if( next_type == eRawString && !elts.dim() ) {
                    // no dollar codes found in repl
                    break;
                }
                if( raw_len ) {
                    Elt * e = elts.add();
                    e->start = cur_start;
                    e->len = raw_len;
                    e->isubmatch = -1;
                    e->type = eRawString;
                    cur_start += raw_len;
                }

                Elt * e = elts.add();
                e->start = cur_start;
                if( next_type == eSubmatch ) {
                    // submatch number is in range 1..99 (1 or 2 digits)
                    e->len = isdigit(string[cur_start + 2]) ? 3 : 2;
                    char numbuf[4];
                    strncpy(numbuf, string + cur_start + 1, e->len - 1);
                    numbuf[e->len - 1] = 0;
                    e->isubmatch = atoidx(numbuf);
                } else {
                    e->len = 2;
                    e->isubmatch = -1;
                }
                e->type = next_type;
                cur_start = e->start + e->len;
            }
        }
    };
}

idx sRegExp::replace(sStr & out, const char * str, idx str_len, const char * repl_str)
{
    if( !ok() || !str ) {
        return 0;
    }
    Priv * priv = getPriv();
    idx nmatches = 0;
    if( str_len ) {
#ifndef HAS_PCRE
        // POSIX regexp API requires 0-terminated strings
        _err.cutAddString(0, str, str_len);
        str = _err.ptr();
        _err.cut(0);
#endif
    } else {
        str_len = sLen(str);
    }

    Replacement repl(repl_str);

    idx pos = 0;
    do {
        if( !priv->regexec(str, pos, str_len, -1) ) {
            break;
        }
        // copy unmatched substring from str
        if( priv->posMatch(0) > pos ) {
            out.addString(str + pos, priv->posMatch(0) - pos);
        }

        // substitute
        if( repl.elts.dim() ) {
            for(idx i=0; i<repl.elts.dim(); i++) {
                idx isubmatch = repl.elts[i].isubmatch;
                switch(repl.elts[i].type) {
                    case Replacement::eRawString:
                        if( repl.elts[i].len ) {
                            out.addString(repl.string + repl.elts[i].start, repl.elts[i].len);
                        }
                        break;
                    case Replacement::eRawDollar:
                        out.addString("$", 1);
                        break;
                    case Replacement::eMatch:
                        if( priv->sizeMatch(0) > 0 ) {
                            out.addString(str + priv->posMatch(0), priv->sizeMatch(0));
                        }
                        break;
                    case Replacement::eSubmatch:
                        if( isubmatch > 0 && isubmatch < priv->nmatches && priv->sizeMatch(isubmatch) > 0 ) {
                            out.addString(str + priv->posMatch(isubmatch), priv->sizeMatch(isubmatch));
                        }
                        break;
                    case Replacement::eBefore:
                        if( priv->posMatch(0) > 0 ) {
                            out.addString(str, priv->posMatch(0));
                        }
                        break;
                    case Replacement::eAfter:
                        idx end_match = priv->posMatch(0) + priv->sizeMatch(0);
                        if( priv->posMatch(0) >= 0 && priv->sizeMatch(0) >= 0 && end_match < str_len ) {
                            out.addString(str + end_match, str_len - end_match);
                        }
                        break;
                }
            }
        } else {
            out.addString(repl.string);
        }

        pos = priv->posMatch(0) + priv->sizeMatch(0);
        // If previous match was 0 length, advance in str by 1 UTF-8 character
        if( !priv->sizeMatch(0) ) {
            pos = nextUnicodeSym(str, pos, str_len, _flags & fMultiline);
        }
        nmatches++;
    } while( (_flags & fGlobal) && pos < str_len );

    // copy unmatched tail from str
    if( pos < str_len ) {
        out.addString(str + pos, str_len - pos);
    }

    return nmatches;
}
