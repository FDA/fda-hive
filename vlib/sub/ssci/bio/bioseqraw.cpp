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
#include <ssci/bio/bioseqraw.hpp>

#include <zlib.h>

using namespace slib;


idx checkIUPACBuf(const char * inBuf, idx lenChar, sStr & outBuf){
    idx slen=0; outBuf.cut(0);
    for (idx is=0; is<lenChar; ++is) {
        if( inBuf[is]!=0 && strchr("ACGTRYSWKMBDHVN.-",inBuf[is])!=0 ) {
            ++slen;
            outBuf.add(inBuf+is,1);
        }
    }
    return slen;
}

idx sBioseqRaw::processFastqGzRaw(processorCallbackType procCallback,void * param){
    if (!m_gz) {
        errorMsg.printf("Error: Failed to open file.");
        return 1;
    }
    idx CHUNK_SIZE = 200;
    idx MAX_LINE_LENGTH = 512;
    char buffer[CHUNK_SIZE + 1];
    char line[MAX_LINE_LENGTH];

    idx num_read, read_size = sizeof(buffer);
    bool found = false;
    while ((num_read = gzread(m_gz, buffer, read_size)) > 0) {
        char * ptr = buffer;
        while (ptr && (ptr - buffer) < read_size ) {
            if (*ptr!='@') ++ptr;
            else {found=true; break;}
        }
       if (found) {
            if (gzseek(m_gz, - (num_read-(ptr-buffer)), SEEK_CUR) == -1) {
                errorMsg.printf("Error: Failed to seek in gzip file.\n");
                return 1;
            }
            break;
       }
    }
   
    idx state = 0;
    idx idNum =0;
    sStr ids, seq, plus, qua;
    while (gzgets(m_gz, buffer, sizeof(buffer))) {
        char *ptr = buffer;
        while (*ptr) {
            char *newline = strchr(ptr, '\n');
            if (newline) {
                idx len = newline - ptr;
                strncpy(line, ptr, len);
                line[len] = '\0';
                ptr = newline + 1;
            } else {
                strncpy(line, ptr, MAX_LINE_LENGTH - 1);
                line[MAX_LINE_LENGTH - 1] = '\0';
                ptr += strlen(ptr);
            }


            switch (state) {
                case 0:
                    ids.printf("%s",line);
                    if (newline) state = 1;
                    break;
                case 1:
                    seq.printf("%s",line);
                    if (newline) state = 2;
                    break;
                case 2:
                    plus.printf("%s",line);
                    if (newline) state = 3;
                    break;
                case 3:
                    qua.printf("%s",line);
                    if (!newline) continue;
                    if (seq.length() != qua.length()) {
                        errorMsg.printf( "Error: Sequence and quality length mismatch for %s\n", ids.ptr());
                        return 1;
                    }
                    if (idNum % 50000 ==0) {
                        printf("ID: %s\nSequence: %s\nPlus: %s\nQuality: %s\n\n", ids.ptr(), seq.ptr(), "+", qua.ptr());
                    }
                    if (procCallback) {
                        procCallback(param, ids.ptr(), 0, seq.length(), seq.ptr(),  0, 1);
                    }
                    ids.cut(0); seq.cut(0); qua.cut(0); plus.cut(0);
                    state = 0;
                    idNum+=1;
                    break;
            }
        }
    }
    if (ids.length()) {
        if (procCallback) {
            procCallback(param, ids.ptr(), 0, seq.length(), seq.ptr(),  0, 1);
        }
    }

    return 0;
}

idx sBioseqRaw::processGzRaw(processorCallbackType procCallback,void *param, idx chunkSize) {
    if (!m_gz) {
        errorMsg.printf("Error: Failed to open file.");
        return 1;
    }

    char buffer[chunkSize];
    idx toRemove, cleanLen, idNum=-1;
    
    sStr tmpBuf, cleanBuf, seqWithPrefix, ids;
    bool still_header = false;

    while (gzgets(m_gz, buffer, sizeof(buffer)) != 0) {
        tmpBuf.printf(0,"%s",buffer);
        toRemove=0;
        if (tmpBuf.length() && tmpBuf.ptr(tmpBuf.length()-1)[0] == '\n') {
            toRemove+=1;
        }
        if (tmpBuf.length() && tmpBuf.ptr(tmpBuf.length()-1)[0] == '\r') {
            toRemove+=1;
        }
        if (toRemove) {
            tmpBuf.cut(tmpBuf.length()-toRemove);
        } 
        if (tmpBuf.length() && tmpBuf.ptr(0)[0] == '>' ) {
            idNum+=1;
            seqWithPrefix.cut(0);
            ids.cut(0);ids.addString(tmpBuf.ptr(), tmpBuf.length());
            if (toRemove==0) {
                still_header = true;
            } else { still_header = false;}
        } else if (tmpBuf.length() && still_header) {
            ids.addString(tmpBuf.ptr(), tmpBuf.length());
            if (toRemove==0) {
                still_header = true;
            } else { still_header = false;}
        }else {
            cleanLen = checkIUPACBuf(tmpBuf.ptr(),tmpBuf.length(),cleanBuf);
            idx rollback = 0;
            if (cleanLen) {
                seqWithPrefix.addString(cleanBuf.ptr(),cleanLen);
                cleanLen = seqWithPrefix.length();
                if (cleanLen  && procCallback) {
                    rollback = procCallback(param, ids.ptr(), idNum, cleanLen, seqWithPrefix.ptr(),  0, 1);
                }
                seqWithPrefix.cut(0);
                seqWithPrefix.addString(seqWithPrefix.ptr(cleanLen-rollback),rollback);
            } 
        }

    }
    return 0;
}
