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

#include <slib/std/string.hpp>
#include <slib/core/var.hpp>
#include <fcntl.h>
#include <slib/std/file.hpp>
#include <slib/std/http.hpp>

using namespace slib;
const char * sHtml::headerStartMarker="__headersStart";

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/                                          _/
_/  HTTP/CGI BUFFER MANIPULATION            _/
_/                                          _/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

enum EHtmlCleanEscapesState {
    eDelim,
    eName,
    eNameEquals,
    eNullEquals,
    eNameEqualsValue,
    eNullEqualsValue
};

static inline void resetPos(sMex::Pos & pos)
{
    pos.pos = -1;
    pos.size = 0;
}

static void writeNameValue(sStr * dst, sVec <sMex::Pos> * ofs, sMex::Pos & name_pos, sMex::Pos & value_pos, const EHtmlCleanEscapesState state, const char sep)
{
    switch( state ) {
        case eDelim:
        case eNullEquals:
            // nothing to do: both name and value are missing
            resetPos(name_pos);
            resetPos(value_pos);
            return;
        case eName:
            // bare "name" is interpreted as "name=1"
            dst->add(&sep, 1);
            value_pos.pos = dst->length();
            value_pos.size = 1;
            dst->add("1", 1);
            dst->add(&sep, 1);
            break;
        case eNameEquals:
            // "name="
            resetPos(value_pos);
            break;
        case eNameEqualsValue:
            // "name=val"
            break;
        case eNullEqualsValue:
            // "=val"
            resetPos(name_pos);
            break;
    }

    sMex::Pos * ppos = ofs->add(2);
    *ppos++ = name_pos;
    *ppos++ = value_pos;

    resetPos(name_pos);
    resetPos(value_pos);
}

/* cleans escape symbols from CGI */
char * sHtml::cleanEscapes(sStr * dst, sVec <sMex::Pos> * ofs, const char * src, idx len, bool issplit, bool is_http_header)
{
    if(!len)len=sLen(src);
    if(!len)return dst->ptr();

    EHtmlCleanEscapesState state = eDelim;

    sMex::Pos name_pos, value_pos;
    name_pos.pos = -1;
    name_pos.size = 0;
    value_pos.pos = -1;
    value_pos.size = 0;

    const char sep = issplit ? (char)0 : ' ';

    for(idx i = 0; i < len; ++i) {
        bool in_name_or_value = true;
        char ch = src[i];

        const EHtmlCleanEscapesState prev_state = state;

        switch( ch ) {
            case '+':
                ch = ' ';
                break;
            case '&':
                if( !is_http_header ) {
                    // delimits var/val pair in url query strings
                    state = eDelim;
                    in_name_or_value = false;
                    ch = sep;
                }
                break;
            case ';':
                if( is_http_header && src[i+1] == ' ') {
                    // delimits var/val pair in HTTP headers
                    state = eDelim;
                    in_name_or_value = false;
                    ch = sep;
                    i++;
                }
                break;
            case '=':
                switch( state ) {
                    case eDelim:
                        state = eNullEquals;
                        in_name_or_value = false;
                        ch = sep;
                        break;
                    case eName:
                        state = eNameEquals;
                        in_name_or_value = false;
                        ch = sep;
                        break;
                    default:
                        // treat as normal character, fall through
                        break;
                }
                break;
            case '%':
                // interpret the following two caracters as the hexadecimal symbol number
                ch = 0;
                if( i + 2 < len ) {
                    if( src[i + 1] >= 'A' && src[i + 1] <= 'F' ) {
                        ch = (char) (src[i + 1] - 'A' + 10);
                    } else if( src[i + 1] >= 'a' && src[i + 1] <= 'f' ) {
                        ch = (char) (src[i + 1] - 'a' + 10);
                    } else if( src[i + 1] >= '0' && src[i + 1] <= '9' ) {
                        ch = (char) (src[i + 1] - '0');
                    }
                    ch *= 16;
                    if( src[i + 2] >= 'A' && src[i + 2] <= 'F' ) {
                        ch = (char) (ch + src[i + 2] - 'A' + 10);
                    } else if( src[i + 2] >= 'a' && src[i + 2] <= 'f' ) {
                        ch = (char) (ch + src[i + 2] - 'a' + 10);
                    } else if( src[i + 2] >= '0' && src[i + 2] <= '9' ) {
                        ch = (char) (ch + src[i + 2] - '0');
                    }
                }
                i += 2;
                break;
            default:
                // normal character, fall through
                break;
        }

        if( in_name_or_value ) {
            // so is it a name or a value?
            switch( prev_state ) {
                case eDelim:
                    // start of name string
                    state = eName;
                    name_pos.pos = dst->length();
                    name_pos.size = 1;
                    break;
                case eName:
                    // middle of name string
                    state = prev_state;
                    name_pos.size++;
                    break;
                case eNameEquals:
                    // start of value string
                    state = eNameEqualsValue;
                    value_pos.pos = dst->length();
                    value_pos.size = 1;
                    break;
                case eNullEquals:
                    // start of value string
                    state = eNullEqualsValue;
                    value_pos.pos = dst->length();
                    value_pos.size = 1;
                    break;
                case eNameEqualsValue:
                case eNullEqualsValue:
                    // middle of value string
                    state = prev_state;
                    value_pos.size++;
                    break;
            }
        } else if( state == eDelim ) {
            writeNameValue(dst, ofs, name_pos, value_pos, prev_state, sep);
        }

        dst->add(&ch, 1);
    }

    // write out final name & value
    writeNameValue(dst, ofs, name_pos, value_pos, state, sep);

    dst->add(__,2);
    return dst->ptr();
}

// this function parses direct POST data and splits into pieces.
idx sHtml::readCGIPost(sVec < sMex::Pos> * ofs, char * post, idx len)
{
    sMex::Pos * ppos = 0;
    //char * post=(char *)postM->ptr();
    //idx len=postM->pos();
    if(!len)len=sLen(post);
    const char * dispos="Content-Disposition: form-data; name=\"";
    idx lendisp=sLen(dispos);

    //read the first line : the separator
    char * blockstart=post, * blockend=strchr(blockstart,'\n');
    if(!blockend)return 0;
    --blockend; if(*(blockend-1)=='\r')blockend--;
    idx lenblock=(idx)(blockend-blockstart);
    char * valpos=0, *valend=0;
    for(char * ptr=blockend+1 ; ptr < post+len;  ++ptr ) {
        if( !strncmp(ptr,dispos,lendisp) ) {


            char * elename=sString::searchSubstring(ptr,len,"name=\""__,1,"\n"__,1);
            if(elename ) {
                elename+=6;
                char * eleend=strpbrk(elename,"\"\r\n");
                if(eleend) {
                    *eleend=0;ptr=eleend+1;
                    ppos=ofs->add(1);
                    ppos->pos=(idx )(elename-post);
                    ppos->size=(idx )(eleend-elename);
                }

                char * filename=sString::searchSubstring(ptr,len,"filename=\""__,1,"\n"__,1);
                if(filename) {
                    char * filenameend=strpbrk(filename+10,"\"\r\n");
                    if(filenameend) {
                        sMex::Pos namepos=*ppos; // rememeber the pos elemenet for the name

                        // push the word "filename"
                        *(filename+9)=0;
                        ppos->pos=filename-post;
                        ppos->size=9;

                        filename+=10;
                        // push the filename itself
                        *filenameend=0;ptr=filenameend+1;
                        ppos=ofs->add(1);
                        ppos->pos=(idx )(filename-post);
                        ppos->size=(idx )(filenameend-filename);

                        ppos=ofs->add(1);
                        *ppos=namepos;
                    }
                }

            }

            // position to the empty line
            ptr=sString::searchSubstring(ptr,0,"\n\r\n"_"\n\n"__,1,0,0);

            for(idx inew=0;inew<2 && ptr && ptr<post+len && (*ptr=='\n' || *ptr=='\r') ;){
                if(*ptr=='\n')++inew;
                ++ptr; // skip the empty line
            }
            valpos=ptr;
        }

        else if( !strncmp(ptr,blockstart,lenblock) ) {
            valend=ptr;
            if(*(ptr-1)=='\n')valend--;
            if(*(ptr-2)=='\r')valend--;
            *valend=0;
            ++ptr;
            //cmd->add(valpos,(idx )(valend-valpos)+1);
            ppos=ofs->add();
            ppos->pos=(idx )(valpos-post);
            ppos->size=(idx )(valend-valpos);
//            valpos=0;
        }

    }

    return ofs->dim();
}

idx sHtml::inputCGI(sStr * bfr, sVec <sMex::Pos> * ofs, FILE * fp, idx argc, const char * * argv,sVar * variables, const char * mangleNameChar, bool isCookies, const char * method, bool no_env)
{
    if(!method && !no_env)method=getenv("REQUEST_METHOD");
    const char * data=0;
//    if(!method)method="GET";
    bool ispost=false;
    sStr bfrDa;
    if( fp && method && !strcmp(method, "POST") ) {
        ispost=true;
        ioModeBin(fp);
        idx bfrL=bfr->length();
        bfr->readIO(fp); // we stream it with the stream-accumulator, not to consume too much memory
        if( bfr->pos() ) {
            if( !readCGIPost(ofs, bfr->ptr(), bfr->length())) {
                ispost=false; // not multipart
                method="GET";
                bfrDa.add(bfr->ptr(0),bfr->length());
                bfrDa.add0();
                data=bfrDa.ptr(0);
                bfr->cut(bfrL);
            }
        }
    }

    if( !ispost ) {
        idx argstart = 0;
        if( method && !strcmp(method, "GET") ) {
            if(!data && !no_env)
                data = getenv("QUERY_STRING");
            if( data ) {
                argc = 1;
                argv = &data;
            } else {
                argstart = 1;
            }
        } else {
            argstart = 1;
        }
        for(idx i = argstart; i < (idx) argc && argv; ++i) {
            sHtml::cleanEscapes(bfr, ofs, argv[i], 0, true, false);
            bfr->add(__, 2);
        }
    }
    idx cntRealFormParameters=ofs->dim();

    //add cookies
    struct {
            const char* src;
            const char* dst;
    } env[] = {
        { isCookies ? "HTTP_COOKIE" : 0, 0 },
        { "HTTP_ACCEPT_ENCODING", "ENCODING" },
        { "HTTP_ACCEPT_LANGUAGE", "LANGUAGE" },
        { "HTTP_IF_NONE_MATCH", "IF_NONE_MATCH" },
        { "HTTP_REFERER", "REFERER" },
        { "HTTP_HOST", "HOST" },
        { "HTTP_USER_AGENT", "USER_AGENT" },
        { "HTTPS", "HTTPS" },
        { "REMOTE_ADDR", "ADDR" },
        { "REQUEST_URI", "URI" },
        { "SERVER_PORT", "PORT" },
        { "SCRIPT_NAME", "SCRIPT_NAME" }
    };
    for(idx i = 0; i < sDim(env) && !no_env; ++i) {
        if(env[i].src && (data = getenv(env[i].src)) != 0) {
            if( !env[i].dst ) {
                sHtml::cleanEscapes(bfr, ofs, data, 0, true, true);
            } else {
                sMex::Pos * ppos = ofs->add(2);
                ppos->pos = bfr->length();
                bfr->add(env[i].dst);
                ppos->size = bfr->length() - ppos->pos;

                ppos++;
                ppos->pos = bfr->length();
                bfr->add(data);
                ppos->size = bfr->length() - ppos->pos;
            }
            bfr->add(__, 2);
        }
    }


    if(variables) {
        sStr filenameVar;
        sMex::Pos * valOFileName=0;

        for(idx i=0; i+1<ofs->dim(); i+=2) {
            sMex::Pos * varO=ofs->ptr(i);
            sMex::Pos * valO=ofs->ptr(i+1);
            const char * var = 0;
            const char * var0 = varO->pos >= 0 ? bfr->ptr(varO->pos) : sStr::zero;
            if(mangleNameChar)var=strrchr(var0,*mangleNameChar);
            if(!var)var=var0;else ++var;
            //var=var0;
            const char * val = valO->pos >= 0 ? bfr->ptr(valO->pos) : sStr::zero;

            if(strcmp(var,"filename=")==0 ) {
                valOFileName=valO;
                continue;
            }

            if(variables->out(var))continue; // we never reset the same variable

            if( i==cntRealFormParameters ) {
                variables->inp(headerStartMarker,"1",2);
            }

            variables->inp(var,val, valO->size + (valOFileName ? 0 : 1));
            if(valOFileName) {
                variables->inp(filenameVar.printf(0,"%s-filename",var), bfr->ptr(valOFileName->pos), valOFileName->size+1);
                valOFileName=0;
            }
            //const char * pop = variables->value("imgStructure");
        }
#if 0
        fprintf(stderr, "sHtml::inputCGI() result:\n");
        for(idx i=0; i<variables->dim(); i++) {
            const char * key = static_cast<const char*>(variables->id(i));
            idx value_size = 0;
            const char * value = variables->value(key, 0, &value_size);
            fprintf(stderr, "%s=%s (val strlen = %"DEC", size = %"DEC")\n", key, value, sLen(value), value_size);
        }
#endif
    }
    //const char * lop= variables->value("imgStructure");
    return ofs->dim();
}





idx sHtml::outFieldList(sStr * str, sVar * form, const char * fieldlist00, idx doflags, const char * autoQuote, const char * separSymb, const char * quoteSymb, bool doValueClipEnds, const char * excludefields00)
{

    idx cntout = 0, j;
    const char * p;
    for(j = 0, p = fieldlist00; p; p = sString::next00(p), ++j) {
        if( excludefields00 && sString::compareChoice(p, excludefields00, 0, false, 0) != -1 )
            continue;
        const char * val = form->value(p);

        if( (doflags & fDoFieldValRequired) && !val )
            continue;

        if( cntout ) {
            if( separSymb )
                str->addString(separSymb);
            else
                str->add0();
        }
        ++cntout;

        if( (doflags & fDoFieldName) ) {
            if( doflags & fDoFieldNameQuote )
                str->addString(quoteSymb);
            str->addString(p);
            if( doflags & fDoFieldNameQuote )
                str->addString(quoteSymb);
        }
        if( (doflags & fDoFieldName) && (doflags & fDoFieldVal) )
            str->addString("=");
        if( doflags & fDoFieldVal ) {
            if( (doflags & fDoFieldValQuote) || (autoQuote && autoQuote[j] == '1') )
                str->addString(quoteSymb);
            sStr tmp;
            if( doValueClipEnds ) {
                sString::cleanEnds(&tmp, val ? val : "", 0, sString_symbolsBlank, true);
                val = tmp.ptr();
            }
            str->addString(val);
            if( (doflags & fDoFieldValQuote) || (autoQuote && autoQuote[j] == '1') )
                str->addString(quoteSymb);
        }
    }
    //str->add0();
    return cntout;
}

const char * sHtml::contentTypeByExt(const char * flnm)
{
    static struct {
        const char * extention;
        const char * mime;
    } types[] = {
        {"","application/octet-stream"},
        {"323","text/h323"},
        {"acx","application/internet-property-stream"},
        {"ai","application/postscript"},
        {"aif","audio/x-aiff"},
        {"aifc","audio/x-aiff"},
        {"aiff","audio/x-aiff"},
        {"asf","video/x-ms-asf"},
        {"asr","video/x-ms-asf"},
        {"asx","video/x-ms-asf"},
        {"au","audio/basic"},
        {"avi","video/x-msvideo"},
        {"axs","application/olescript"},
        {"bas","text/plain"},
        {"bcpio","application/x-bcpio"},
        {"bin","application/octet-stream"},
        {"bmp","image/bmp"},
        {"c","text/plain"},
        {"cat","application/vnd.ms-pkiseccat"},
        {"cdf","application/x-cdf"},
        {"cer","application/x-x509-ca-cert"},
        {"class","application/octet-stream"},
        {"clp","application/x-msclip"},
        {"cmx","image/x-cmx"},
        {"cod","image/cis-cod"},
        {"cpio","application/x-cpio"},
        {"crd","application/x-mscardfile"},
        {"crl","application/pkix-crl"},
        {"crt","application/x-x509-ca-cert"},
        {"csh","application/x-csh"},
        {"css","text/css"},
        {"csv","text/csv"},
        {"dcr","application/x-director"},
        {"der","application/x-x509-ca-cert"},
        {"dir","application/x-director"},
        {"dll","application/x-msdownload"},
        {"dms","application/octet-stream"},
        {"doc","application/msword"},
        {"dot","application/msword"},
        {"dvi","application/x-dvi"},
        {"dxr","application/x-director"},
        {"eps","application/postscript"},
        {"etx","text/x-setext"},
        {"evy","application/envoy"},
        {"exe","application/octet-stream"},
        {"fif","application/fractals"},
        {"flr","x-world/x-vrml"},
        {"gif","image/gif"},
        {"png","image/png"},
        {"gtar","application/x-gtar"},
        {"gz","application/x-gzip"},
        {"h","text/plain"},
        {"hdf","application/x-hdf"},
        {"hlp","application/winhlp"},
        {"hqx","application/mac-binhex40"},
        {"hta","application/hta"},
        {"htc","text/x-component"},
        {"htm","text/html"},
        {"html","text/html"},
        {"htt","text/webviewhtml"},
        {"ico","image/x-icon"},
        {"ief","image/ief"},
        {"iii","application/x-iphone"},
        {"ins","application/x-internet-signup"},
        {"isp","application/x-internet-signup"},
        {"jfif","image/pipeg"},
        {"jpe","image/jpeg"},
        {"jpeg","image/jpeg"},
        {"jpg","image/jpeg"},
        {"js","application/x-javascript"},
        {"json", "application/json"},
        {"latex","application/x-latex"},
        {"lha","application/octet-stream"},
        {"lsf","video/x-la-asf"},
        {"lsx","video/x-la-asf"},
        {"lzh","application/octet-stream"},
        {"m13","application/x-msmediaview"},
        {"m14","application/x-msmediaview"},
        {"m3u","audio/x-mpegurl"},
        {"man","application/x-troff-man"},
        {"mdb","application/x-msaccess"},
        {"me","application/x-troff-me"},
        {"mht","message/rfc822"},
        {"mhtml","message/rfc822"},
        {"mid","audio/mid"},
        {"mny","application/x-msmoney"},
        {"mov","video/quicktime"},
        {"movie","video/x-sgi-movie"},
        {"mp2","video/mpeg"},
        {"mp3","audio/mpeg"},
        {"mpa","video/mpeg"},
        {"mpe","video/mpeg"},
        {"mpeg","video/mpeg"},
        {"mpg","video/mpeg"},
        {"mpp","application/vnd.ms-project"},
        {"mpv2","video/mpeg"},
        {"ms","application/x-troff-ms"},
        {"mvb","application/x-msmediaview"},
        {"nws","message/rfc822"},
        {"oda","application/oda"},
        {"p10","application/pkcs10"},
        {"p12","application/x-pkcs12"},
        {"p7b","application/x-pkcs7-certificates"},
        {"p7c","application/x-pkcs7-mime"},
        {"p7m","application/x-pkcs7-mime"},
        {"p7r","application/x-pkcs7-certreqresp"},
        {"p7s","application/x-pkcs7-signature"},
        {"pbm","image/x-portable-bitmap"},
        {"pdf","application/pdf"},
        {"pfx","application/x-pkcs12"},
        {"pgm","image/x-portable-graymap"},
        {"pko","application/ynd.ms-pkipko"},
        {"pma","application/x-perfmon"},
        {"pmc","application/x-perfmon"},
        {"pml","application/x-perfmon"},
        {"pmr","application/x-perfmon"},
        {"pmw","application/x-perfmon"},
        {"pnm","image/x-portable-anymap"},
        {"pot","application/vnd.ms-powerpoint"},
        {"ppm","image/x-portable-pixmap"},
        {"pps","application/vnd.ms-powerpoint"},
        {"ppt","application/vnd.ms-powerpoint"},
        {"prf","application/pics-rules"},
        {"ps","application/postscript"},
        {"pub","application/x-mspublisher"},
        {"qt","video/quicktime"},
        {"ra","audio/x-pn-realaudio"},
        {"ram","audio/x-pn-realaudio"},
        {"ras","image/x-cmu-raster"},
        {"rgb","image/x-rgb"},
        {"rmi","audio/mid"},
        {"roff","application/x-troff"},
        {"rtf","application/rtf"},
        {"rtx","text/richtext"},
        {"scd","application/x-msschedule"},
        {"sct","text/scriptlet"},
        {"setpay","application/set-payment-initiation"},
        {"setreg","application/set-registration-initiation"},
        {"sh","application/x-sh"},
        {"shar","application/x-shar"},
        {"sit","application/x-stuffit"},
        {"snd","audio/basic"},
        {"spc","application/x-pkcs7-certificates"},
        {"spl","application/futuresplash"},
        {"src","application/x-wais-source"},
        {"sst","application/vnd.ms-pkicertstore"},
        {"stl","application/vnd.ms-pkistl"},
        {"stm","text/html"},
        {"svg","image/svg+xml"},
        {"sv4cpio","application/x-sv4cpio"},
        {"sv4crc","application/x-sv4crc"},
        {"swf","application/x-shockwave-flash"},
        {"t","application/x-troff"},
        {"tar","application/x-tar"},
        {"tcl","application/x-tcl"},
        {"tex","application/x-tex"},
        {"texi","application/x-texinfo"},
        {"texinfo","application/x-texinfo"},
        {"tgz","application/x-compressed"},
        {"tif","image/tiff"},
        {"tiff","image/tiff"},
        {"tr","application/x-troff"},
        {"trm","application/x-msterminal"},
        {"tsv","text/tab-separated-values"},
        {"txt","text/plain"},
        {"fasta","text/plain"},
        {"uls","text/iuls"},
        {"ustar","application/x-ustar"},
        {"vcf","text/x-vcard"},
        {"vrml","x-world/x-vrml"},
        {"wav","audio/x-wav"},
        {"wcm","application/vnd.ms-works"},
        {"wdb","application/vnd.ms-works"},
        {"wks","application/vnd.ms-works"},
        {"wmf","application/x-msmetafile"},
        {"wps","application/vnd.ms-works"},
        {"wri","application/x-mswrite"},
        {"wrl","x-world/x-vrml"},
        {"wrz","x-world/x-vrml"},
        {"xaf","x-world/x-vrml"},
        {"xbm","image/x-xbitmap"},
        {"xla","application/vnd.ms-excel"},
        {"xlc","application/vnd.ms-excel"},
        {"xlm","application/vnd.ms-excel"},
        {"xls","application/vnd.ms-excel"},
        {"xlt","application/vnd.ms-excel"},
        {"xlw","application/vnd.ms-excel"},
        {"xof","x-world/x-vrml"},
        {"xpm","image/x-xpixmap"},
        {"xwd","image/x-xwindowdump"},
        {"z","application/x-compress"},
        {"zip","application/zip"}
    };
    if( flnm && flnm[0] ) {
        const char * ext = strrchr(flnm, '.');
        if( !ext ) {
            ext = flnm;
        } else {
            ++ext;
        }
        for(idx id = 0; id < sDim(types); ++id) {
            if( strcmp(types[id].extention, ext) == 0 ) {
                return types[id].mime;
            }
        }
    }
    return types[0].mime;
}

void sHtml::outFormData(sVar * form, const char * formflnm, bool truncate) // this is to debug the list of variable value pairs in HTML
{
    if(!formflnm)formflnm="/tmp/__form.html";
    if(truncate) sFile::remove(formflnm);

    sFil frm(formflnm);
    if(frm.length()) frm.printf("<html><body>");

    frm.printf("######################### SERVICE #############\n");
    frm.printf("<table border=1>\n");
    for ( idx i=0; i<form->dim(); ++i) {
        const char * variable=(const char *)form->id(i);
        const char * value=form->value(variable,"(null)") ;

        frm.printf("<tr><td>%"DEC"</td><td>'%s'</td><td>'%s'</td></tr>\n",i,variable, value);
    }
    frm.printf("</table>\n");

    frm.printf("######################### TERMINE #############\n"); // </body></html>
}

FILE * sHtml::grabInData(FILE * readfrom, const char * * envp, const char  * postFile) // this is to debug the list of variable value pairs in HTML
{
    char * method=getenv("REQUEST_METHOD");
    //const char  * data=getenv("QUERY_STRING");
    /*::printf("Content-type: text/plain\r\n\r\n");
    ::printf("method=<%s>\n",method);
*/
    if(method && strcasecmp(method,"POST")==0) {
        if(!postFile)postFile="/tmp/__post.in";
        sFile::remove(postFile);
        sMex pst;pst.init(postFile); pst.readIO(stdin);
        //::printf("postFileFlags=%"DEC"\n",(pst.flags>>(sMex_FileHandleShift)) & 0xFFFF); // this is where file handles are kept);
        readfrom=fopen(postFile,"r");
        //::printf("postFile= %s\n--------------------\n%.*s\n-------------------\n",postFile,(int)pst.pos(),pst.ptr());
    } /*else {
        ::printf("NOT post\n");
    }*/
#if 0 // _DEBUG
    if(envp){
        const char * envFile="/tmp/__env.in";
        sFile::remove(envFile);
        sFil fEnv(envFile);
        for ( idx ie=0; envp[ie]; ++ie){
            fEnv.printf("%"DEC" %s\n",ie, envp[ie]);
            ::fprintf(stderr, "%"DEC" %s\n",ie, envp[ie]);
        }
    }
#endif
    //exit(0);

    //else if( data ) {
        //sFil pst(postFile); pst.addString(data);
    //}

    return readfrom;
}


