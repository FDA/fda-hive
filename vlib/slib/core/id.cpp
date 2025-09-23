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

#include <slib/core/id.hpp>
#include <slib/core/dic.hpp>
#include <assert.h>
#include <regex.h>
#include <ctype.h>

using namespace slib;

sDic<udx> sHiveId::_urlToDomainId;
sDic<sStr> sHiveId::_domainIdToUrl;
sDic<sHiveId> sHiveId::_domainIdToId;
bool sHiveId::_printDomainAsUrl = false;
const sHiveId sHiveId::zero;

static inline bool validDomainIdStart(char c)
{
    return (c >= 'a' && c <= 'z') || c == '_';
}

static inline bool validDomainIdMid(char c)
{
    return validDomainIdStart(c) || isdigit(c);
}

bool sHiveId::setDomainId(const char * dom)
{
    if( dom ) {
        if( validDomainId(dom, 0) ) {
            _domain_id = encodeDomainId(dom);
            return true;
        } else if( const udx * pdom = (_urlToDomainId.get(dom)) ) {
            _domain_id = *pdom;
            return true;
        }
    }
    _domain_id = 0;
    return false;
}

bool sHiveId::setDomainId(udx dom_id)
{
    if( dom_id ) {
        char dombuf[sizeof(_domain_id) + 1];
        if( !decodeDomainId(dombuf, dom_id) ) {
            return false;
        }
        _domain_id = encodeDomainId(dombuf);
        return true;
    }
    _domain_id = 0;
    return true;
}

udx sHiveId::encodeDomainId(const char * dom, idx len)
{
    if( !dom || !*dom ) {
        return 0;
    }
    if( !len ) {
        len = sIdxMax;
    }
    udx out = 0;
    for(idx i = 0; i < (idx)sizeof(_domain_id) && i < len && dom[i]; ++i) {
        char c = tolower(dom[i]);
        if( i > (idx)sizeof(_domain_id) - 1 ) {
            return 0;
        }
        if( i ) {
            if( unlikely(!validDomainIdMid(c)) ) {
                return 0;
            }
            out <<= 8;
        } else {
            if( unlikely(!validDomainIdStart(c)) ) {
                return 0;
            }
        }
        out |= ((udx) c);
    }
    return out;
}

bool sHiveId::decodeDomainId(char * outbuf, udx dom)
{
    sSet(outbuf, 0, sizeof(_domain_id) + 1);
    bool saw_nonzero = false;
    for(udx i = 0, len = 0; i < sizeof(_domain_id); ++i) {
        char c = (dom >> ((sizeof(_domain_id) - 1 - i) * 8)) & 0xFF;
        if( c ) {
            outbuf[len++] = c;
            saw_nonzero = true;
        } else if( unlikely(saw_nonzero) ) {
            outbuf[0] = 0;
            return false;
        }
    }
    if( !validDomainId(outbuf, sizeof(_domain_id)) ) {
        outbuf[0] = 0;
        return false;
    }
    return true;
}

bool sHiveId::validDomainId(const char * dom, idx len)
{
    if( !dom || !dom[0] ) {
        return true;
    }
    if( len == 0 ) {
        len = sIdxMax;
    } else if( len < 0 || len > (idx)sizeof(_domain_id) ) {
        return false;
    }
    if( !validDomainIdStart(tolower(dom[0])) ) {
        return false;
    }
    for(idx i = 1; dom[i] && i < len; i++) {
        if( !validDomainIdMid(tolower(dom[i])) ) {
            return false;
        }
    }
    return true;
}

static idx parse_obj_ion_id(sHiveId & id, const char * s, idx pos, idx s_len)
{
    idx len_parsed = 0;
    udx obj_id = 0, ion_id = 0;
    for(; pos < s_len && isdigit(s[pos]); pos++) {
        udx next_obj_id = 10 * obj_id + s[pos] - '0';
        if( next_obj_id > obj_id || obj_id == 0 ) {
            obj_id = next_obj_id;
            len_parsed++;
        } else {
            len_parsed = 0;
            obj_id = 0;
            break;
        }
    }

    if( len_parsed > 0 && pos + 1 < s_len && s[pos] == '.' && isdigit(s[pos + 1]) ) {
        for(pos++, len_parsed++; pos < s_len && isdigit(s[pos]); pos++) {
            udx next_ion_id = 10 * ion_id + s[pos] - '0';
            if( next_ion_id > ion_id || ion_id == 0 ) {
                ion_id = next_ion_id;
                len_parsed++;
            } else {
                len_parsed = 0;
                ion_id = 0;
                break;
            }
        }
    }

    id.setObjId(obj_id);
    id.setIonId(ion_id);

    return len_parsed;
}

static idx parse_short_domain_id_dot(sHiveId & id, const char * s, idx pos, idx s_len)
{
    idx start_pos = pos;
    idx len_parsed = 0;
    udx domain_id = 0;
    if( likely(pos + 1 < s_len && validDomainIdStart(tolower(s[pos]))) ) {
        idx max_dom_len = sMin<idx>(s_len, sizeof(domain_id));
        for(pos++, len_parsed++; pos < max_dom_len && validDomainIdMid(tolower(s[pos])); pos++) {
            len_parsed++;
        }
        if( pos < s_len && s[pos] == '.' ) {
            domain_id = sHiveId::encodeDomainId(s + start_pos, len_parsed);
            if( domain_id ) {
                len_parsed++;
            } else {
                len_parsed = 0;
            }
        } else {
            len_parsed = 0;
        }
    }
    id.setDomainId(domain_id);
    return len_parsed;
}

static idx len_http_schema(const char * s, idx pos, idx s_len)
{
    if( s_len - pos < 8 ) {
        return 0;
    }
    if( strncasecmp(s + pos, "http://", 7) == 0 ) {
        return 7;
    } else if( strncasecmp(s + pos, "https://", 8) == 0 ) {
        return 8;
    }
    return 0;
}

idx sHiveId::parse(const char * s, idx len)
{
    reset();

    idx dequote = 0;
    if( !len ) {
        len = sLen(s);
    }
    if( !s || len < 0 || !s[0] ) {
        return 0;
    }
    if( (s[0] == '"' && s[len - 1] == '"') || (s[0] == '\'' && s[len - 1] == '\'') ) {
        ++s;  dequote = 2; len -= dequote;
    }
    idx len_parsed = 0;
    if( likely(isdigit(s[0])) ) {
        len_parsed = parse_obj_ion_id(*this, s, 0, len);
    } else if( (len_parsed = parse_short_domain_id_dot(*this, s, 0, len)) > 0 ) {
        idx len_obj_ion_parsed = parse_obj_ion_id(*this, s, len_parsed, len);
        if( len_obj_ion_parsed ) {
            len_parsed += len_obj_ion_parsed;
        } else {
            reset();
            len_parsed = 0;
        }
    } else if( len_http_schema(s, 0, len) ) {
        for(idx i = 0; i < _urlToDomainId.dim(); i++) {
            idx url_len = 0;
            const char * url = static_cast<const char *>(_urlToDomainId.id(i, &url_len));
            if( url_len > 0 && url_len < len && strncmp(url, s, url_len) == 0 ) {
                idx pos_slash = url[url_len - 1] == '/' ? url_len - 1 : url_len;
                idx len_obj_ion_parsed = 0;
                if( pos_slash + 1 < len && s[pos_slash] == '/' && isdigit(s[pos_slash + 1]) ) {
                    len_obj_ion_parsed = parse_obj_ion_id(*this, s, pos_slash + 1, len);
                }
                if( len_obj_ion_parsed ) {
                    setDomainId(*_urlToDomainId.ptr(i));
                    len_parsed = pos_slash + 1 + len_obj_ion_parsed;
                } else {
                    reset();
                    len_parsed = 0;
                }
            }
        }
    }
    return len_parsed + (len_parsed ? dequote : 0);
}

idx sHiveId::parseRangeSet(sVec<sHiveId>& out, const char * s, idx len, idx * pLenParsed)
{
    if( !s || !*s ) {
        return 0;
    }
    bool canRangeSpec = false;
    bool inRangeSpec = false;

    idx i=0;
    while( (!len || i < len) && s[i] ) {
        if( s[i] == '-' ) {
            if( !canRangeSpec ) {
                break;
            }
            canRangeSpec = false;
            inRangeSpec = true;
            i++;
            continue;
        } else if( strchr(" \t\n,;/", s[i]) ) {
            if( inRangeSpec ) {
                break;
            }
            canRangeSpec = true;
            inRangeSpec = false;
            i++;
            continue;
        }
        sHiveId cur;
        idx curParseLen = cur.parse(s + i, len ? len - i : 0);
        if( !curParseLen ) {
            break;
        }
        i += curParseLen;

        if( inRangeSpec ) {
            inRangeSpec = false;
            canRangeSpec = false;
            assert (out.dim());
            idx iprev = out.dim() - 1;
            if( cur.domainId() != out[iprev].domainId() ) {
                break;
            }
            if( pLenParsed ) {
                *pLenParsed = i;
            }
            if( cur.objId() == out[iprev].objId() ) {
                idx step = cur.ionId() >= out[iprev].ionId() ? 1 : -1;
                for( udx ion_id = out[iprev].ionId() + step; step > 0 ? ion_id <= cur.ionId() : ion_id >= cur.ionId(); ion_id += step ) {
                    sHiveId * next = out.add(1);
                    *next = out[iprev];
                    next->setIonId(ion_id);
                }
            } else {
                idx step = cur.objId() >= out[iprev].objId() ? 1 : -1;
                for( udx obj_id = out[iprev].objId() + step; step > 0 ? obj_id <= cur.objId() : obj_id >= cur.objId(); obj_id += step ) {
                    sHiveId * next = out.add(1);
                    *next = out[iprev];
                    next->setObjId(obj_id);
                }
            }
        } else {
            canRangeSpec = true;
            *out.add(1) = cur;
            if( pLenParsed ) {
                *pLenParsed = i;
            }
        }
    }
    return out.dim();
}

const char * sHiveId::print(sStr & out, bool domain_id_as_url) const
{
    idx startLen = out.length();
    if( _domain_id || _printDomainAsUrl ) {
        if( domain_id_as_url || _printDomainAsUrl ) {
            const char * url = domainIdAsUrl();
            idx url_len = sLen(url);
            if( url_len ) {
                out.add(url, url_len);
                if( url[url_len - 1] != '/' ) {
                    out.add("/", 1);
                }
            }
        } else {
            char dombuf[sizeof(_domain_id) + 1];
            decodeDomainId(dombuf, _domain_id);
            out.printf("%s.", dombuf);
        }
    }
    out.printf("%" UDEC, _obj_id);
    if( _ion_id ) {
        out.printf(".%" UDEC, _ion_id);
    }
    return out.ptr(startLen);
}

idx sHiveId::print(char * out, bool domain_id_as_url) const
{
    idx ret = 0;
    if( _domain_id || _printDomainAsUrl ) {
        if( domain_id_as_url || _printDomainAsUrl ) {
            const char * url = domainIdAsUrl();
            idx url_len = sLen(url);
            if( url_len ) {
                memcpy(out, url, url_len);
                ret += url_len;
                if( url[url_len - 1] != '/' ) {
                    out[url_len] = '/';
                    ret++;
                }
            }
            out += ret;
        } else {
            decodeDomainId(out, _domain_id);
            ret = strlen(out);
            out[ret] = '.';
            ret++;
            out += ret;
        }
    }
    if( _ion_id ) {
        ret += sprintf(out, "%" UDEC ".%" UDEC, _obj_id, _ion_id);
    } else {
        ret += sprintf(out, "%" UDEC, _obj_id);
    }
    return ret;
}

const char * sHiveId::printSQL(sStr & out, const char * prefix, bool no_ion) const
{
    const char * p1 = prefix ? prefix : "";
    const char * p2 = prefix ? "." : "";
    idx pos = out.pos();
    out.printf("(%s%sdomainID=%" UDEC " AND %s%sobjID=%" UDEC ")", p1, p2, _domain_id, p1, p2, _obj_id);
    return out.ptr(pos);
}

const char * sHiveId::printVec(sStr & out, const sVec<sHiveId> & vec, const char * separ, bool domain_id_as_url)
{
    idx startLen = out.length();
    for( idx i=0; i<vec.dim(); i++ ) {
        if( i ) {
            out.printf("%s", separ);
        }
        vec[i].print(out, domain_id_as_url);
    }
    return out.ptr(startLen);
}

bool sHiveId::mapDomainIdUrl(const udx domain_id, const char * url, const sHiveId & id)
{
    if( url && url[0] ) {
        char dombuf[sizeof(_domain_id) + 1];
        decodeDomainId(dombuf, domain_id);
        if( validDomainId(dombuf, sizeof(dombuf) - 1) ) {
            sStr * purl = _domainIdToUrl.get(&domain_id, sizeof(domain_id));
            udx * pdom = _urlToDomainId.get(url);
            sHiveId * pid = _domainIdToId.get(&domain_id, sizeof(domain_id));
            if( purl && strcasecmp(url, purl->ptr()) == 0 && pdom && domain_id == *pdom && pid && id == *pid ) {
                return true;
            } else if( purl || pdom || pid ) {
                return false;
            }
            purl = static_cast<sStr *>(_domainIdToUrl.set(&domain_id, sizeof(domain_id)));
            pdom = static_cast<udx *>(_urlToDomainId.setString(url));
            pid = static_cast<sHiveId *>(_domainIdToId.set(&domain_id, sizeof(domain_id)));
            if( purl && pdom && pid ) {
                purl->printf(0, "%s", url);
                *pdom = domain_id;
                *pid = id;
                return true;
            }
        }
    }
    return false;
}

bool sHiveId::mapDomainIdUrl(const char * domain_id, const char * url, const sHiveId & id)
{
    if( url && url[0] ) {
        const idx len = sLen(domain_id);
        if( validDomainId(domain_id, len) ) {
            udx dom = encodeDomainId(domain_id, len);
            return mapDomainIdUrl(dom, url, id);
        }
    }
    return false;
}

void sHiveId::mapDomainReset()
{
    _urlToDomainId.empty();
    _domainIdToUrl.empty();
    _domainIdToId.empty();
}
