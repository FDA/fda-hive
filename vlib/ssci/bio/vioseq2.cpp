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
#include <slib/core.hpp>
#include <slib/core/def.hpp>
#include <slib/std/file.hpp>
#include <ssci/bio/vioseq2.hpp>
#include <slib/utils.hpp>
#include <ssci/bio/filterseq.hpp>

using namespace slib;

typedef enum EParseItem_enum
{
    eNone = -1,
    eIdLine = 0,
    eSeq,
    eId2,
    eQual,
    eStop,
} EParseItem;

typedef struct
{
        real percent;
        udx count;
        udx chunks;
        sVioseq2::callbackType callback;
        void * data;
} TSequential_parse_callback_data;

sVioseq2::errParser sVioseq2::listErrors[]={
    {-1, "Error 1"},
    {-2, "Reached the limit number of sequences allowed to parse, get the full version !!"},
    {-3, "Process was killed by user or administrator"},
    {-4, "Format error, no sequence was found"},
    {-5, "File format error, a quality delimiter '+' was found in the FastA parser"},
    {-6, "Invalid length of the file"},
    {-7, "No valid sequences in the proper format"},
    {-8, "Source file is not accessible"},
    {-9, "Source file length is 0 bytes"},
    {-10, "FastQ format error, quality values must contain the same number of symbols as letters in the sequence"},
    {-11, "Error -11"},
    {0},
};

udx sVioseq2::getPartCount(const udx fileSize, udx maxChunkSize)
{
    udx numchunks = 1;
    if( fileSize ) {
        maxChunkSize = maxChunkSize ? maxChunkSize : fileSize;
        udx chunkCnt = (fileSize - 1) / maxChunkSize + 1;
        for(numchunks = 1; numchunks < chunkCnt; numchunks *= 4) {
        }
    }
    return numchunks;
}

udx sVioseq2::getPrefixLength(const udx partCount)
{
    udx prefixLength = 0;
    for(udx abit = partCount; 1 < abit; ++prefixLength) {
        abit = abit >> 2;
    }
    return prefixLength;
}

inline bool parallelizationfilter(const char * cpy, idx sizeComb, idx comb)
{
    idx base = 1;
    idx sum = 0;
    for(idx i = sizeComb - 1; 0 <= i; i--) {
        sum += sBioseqAlignment::_seqBits(cpy, i) * base;
        base *= 4;
    }
    return sum == comb;
}

idx sVioseq2::sequential_parse_callback(void * param, idx countDone, idx percentDone, idx percentMax)
{
    TSequential_parse_callback_data * data = (TSequential_parse_callback_data*) param;
    if( data ) {
        return data->callback(data->data, data->count + countDone, (data->percent + percentDone) / data->chunks, percentMax);
    }
    return 1;
}

#define PROGRESS(items,cur,max,perc) (m_callback && !m_callback(m_callbackParam, items, ((cur) * 1.0 / (max)) * perc, 100))

idx sVioseq2::parseSequenceFile(const char * outfile, const char * flnm, idx flags, idx maxChunkSize, const char * primers00, idx complexityWindow, real complexityEntropy)
{
    idx fsize = sFile::size(flnm);
    if( !fsize ) {
        return -8;
    }

    udx chunksForThisPrefix = getPartCount(fsize, maxChunkSize);
    udx sizeComb = getPrefixLength(chunksForThisPrefix);
#if _DEBUG
    ::printf("\nParallel Version: %" DEC " partitions, %" DEC " letters\n", chunksForThisPrefix, sizeComb);
#endif

    idx res = 0, ires;
    sFilePath firstfile(outfile, "%%pathx-0.vioseq2");
    sStr filenames;
    if( chunksForThisPrefix <= 1 ) {
        if( flags & sVioseq2::eCreateVioseqlist ) {
            res = parseSequenceFile(firstfile, flnm, flags, primers00, complexityWindow, complexityEntropy, 0, 0);
            sFilePath tmpfile(firstfile, "%%flnm");
            filenames.printf("file://%s, 0, %" DEC "\n", tmpfile.ptr(0), res);
        } else {
            res = parseSequenceFile(outfile, flnm, flags, primers00, complexityWindow, complexityEntropy, 0, 0);
        }
        if( res <= 0 ) {
            return res;
        }
    } else {
        sVec<sVec<Infopart> > partList;
        partList.resize(chunksForThisPrefix);
        sVec<idx> countRes;
        countRes.resize(chunksForThisPrefix);
        TSequential_parse_callback_data data;
        for(udx i = 0; i < chunksForThisPrefix; i++) {
            sFilePath newfile(outfile, "%%pathx-%" UDEC ".vioseq2", i);
            ires = parseSequenceFile(newfile, flnm, flags, primers00, complexityWindow, complexityEntropy, sizeComb, i, &partList[i]);
            countRes[i] = ires;
            if( ires == 0 && i != 0 ) {
                continue;
            }
            if( ires < 0 ) {
                res = ires;
                break;
            }
            res += ires;
            data.count += fsize;
            data.percent += 100;\
            if( flags & sVioseq2::eCreateVioseqlist ) {
                if( i != 0 ) {
                    filenames.printf("\n");
                }
                sFilePath tmpfile(newfile, "%%flnm");
                filenames.printf("file://%s,0,%" DEC, tmpfile.ptr(0), ires);

            } else {
                if( i != 0 ) {
                    filenames.printf(",");
                }
                filenames.printf("%s", newfile.ptr(0));
            }
#if _DEBUG
            ::printf("%s > %" DEC "\n", newfile.ptr(0), ires);
#endif
        }
        if( res <= 0 ) {
            return res;
        }
        {
            sVioDB dbi(firstfile, 0, 0, 0);
            fixAddRelation(&dbi, &partList, countRes);
        }
#if _DEBUG
        ::printf("%s\n", filenames.ptr(0));
#endif
        if( (flags & sVioseq2::eCreateVioseqlist) == 0 ) {
            sVioDB db;
            db.concatFiles(outfile, filenames, "vioseq2", m_isSingleFile, true);
        }
    }
    if( flags & sVioseq2::eCreateVioseqlist ) {
        sVioDB dbi(firstfile, 0, 0, 0);
        res = dbi.GetRecordCnt(eRecID_TYPE);
        sStr errmsg;
        sFil fp(outfile);
        fp.cut(0);
        if( fp.ok() ) {
            fp.printf("%s", filenames.ptr(0));
        } else {
            errmsg.printf("Can't write '%s'", outfile);
        }
        vioDB.init(firstfile, 0, 0, 0);
    } else if (!dim()){
        vioDB.init(outfile, 0, 0, 0);
    }
    return res;
}

idx sVioseq2::fixAddRelation(sVioDB *db, sVec<sVec<Infopart> > * partList, idx *countRes, const char * partFiles00, callbackType callback, void * callbackParam)
{
    idx nchunks = (partList && partList->dim()) ? partList->dim() : sString::cnt00(partFiles00);
    sVec<Infopart> * part;

    idx offset = 0;
    const char * partfile = partFiles00;

    for(idx i = 0; i < nchunks; i++) {
        Infopart * rpart;
        sVec<Infopart> localPart;
        if( (partList && partList->dim()) )
            part = partList->ptr(i);
        else {
            localPart.init(partfile, sMex::fReadonly);
            part = &localPart;
        }
        if (callback){
            callback(callbackParam, -1, 91, 100);
        }

        for(idx j = 0; j < part->dim(); j++) {
            rpart = part->ptr(j);
            idx relationCnt = 0;
            if( !rpart->origID ) {
                return -1;
            }
            idx * relPtr = db->GetRelationPtr(eRecID_TYPE, rpart->origID, 1, &relationCnt);
            if( !relPtr ) {
                return -2;
            }
            *relPtr = rpart->partID + offset;
            if (callback){
                callback(callbackParam, -1, 92, 100);
            }
        }
        offset += countRes[i];
        if( partfile )
            partfile = sString::next00(partfile);
    }
    return 0;
}


idx sVioseq2::parseSequenceFile(const char * outfile, const char * flnm, idx flags, const char * primers00, idx complexityWindow, real complexityEntropy, idx sizecomb, idx comb, sVec<Infopart> * iPart)
{

    sFil inf(0);
    if( strcmp(flnm, "stdin") == 0 )
        inf.readIO((FILE*) stdin);
    else
        inf.init(flnm, sMex::fReadonly);
    if( !inf.ok() ) {
        return -8;
    }
    if( !inf.length() )
        return -9;


    if( flags & eParseMultiVioDB ) {
        setSingleFile(true);
    } else {
        setSingleFile(false);
    }

    sFilePath baseFileName(outfile, "%%pathx.baseFile.tmp");
    idx res = 0;
    sVioDB db(outfile, "vioseq2", (idx) 4, (idx) 3);

    idx relationlistIDS[1] = {
        2 };
    idx relationlistREC[3] = {
        1,
        3,
        4 };
    idx relationlistSEQ[1] = {
        3 };
    idx relationlistQUA[1] = {
        4 };

    db.AddType(sVioDB::eOther, 1, relationlistIDS, "ids", 1);
    db.AddType(sVioDB::eOther, 3, relationlistREC, "rec", 2);
    db.AddType(sVioDB::eOther, 1, relationlistSEQ, "seq", 3);
    db.AddType(sVioDB::eOther, 1, relationlistQUA, "qua", 4);

    if( ((flags >> 16) & 0xF) == 0 ) {
        const sFilePath ext(flnm, "%%ext");
        if( (strcasecmp(ext, "fasta") == 0) || (strcasecmp(ext, "fa") == 0) ) {
            flags |= (eTreatAsFastA | eParseQuaBit);
        }
        if( (strcasecmp(ext, "fastq") == 0) || (strcasecmp(ext, "fq") == 0) ) {
            flags |= eTreatAsFastQ;
        } else if( strcasecmp(ext, "sam") == 0 ) {
            flags |= eTreatAsSAM;
        } else if( (strcasecmp(ext, "gb") == 0) || (strcasecmp(ext, "gbk") == 0) || (strcasecmp(ext, "gbff") == 0)) {
            flags |= eTreatAsAnnotation;
        } else if( strcasecmp(ext, "ma") == 0 ) {
            flags |= (eTreatAsMA | eParseQuaBit);
        }  else {
            flags |= (eTreatAsFastA | eParseQuaBit);
        }
    }
    {
        sFile::remove(baseFileName);
        sFil baseFile(baseFileName);
        baseFile.cut(0);

        initVariables(baseFile, 0, sizecomb, comb, flags, complexityWindow, complexityEntropy);
        if( flags & eTreatAsFastA ) {
            res = parseFasta(db, baseFile, inf.ptr(), inf.length(), flags, primers00, complexityWindow, complexityEntropy, sizecomb, comb, iPart);
        } else if( flags & eTreatAsFastQ ) {
            res = parseFastQ(db, baseFile, inf.ptr(), inf.length(), flags, primers00, complexityWindow, complexityEntropy, sizecomb, comb, iPart);
        } else if( flags & eTreatAsSAM ) {
            res = ParseSam(inf.ptr(), inf.length(), db, baseFile, 0, 0, 0, iPart);
        } else if( flags & eTreatAsAnnotation ) {
            res = ParseGb(inf.ptr(), inf.length(), db, baseFile, iPart);
        } else if( flags & eTreatAsMA ) {
            sBioseq::mapATGC[(idx) '-'] = 0xFF;
            res = parseFasta(db, baseFile, inf.ptr(), inf.length(), flags, primers00, complexityWindow, complexityEntropy, sizecomb, comb, iPart);
            sBioseq::mapATGC[(idx) '-'] = 4;
        } else {
            sFilePath quanm(flnm, "%%pathx%s", ".qual");
            sFil qua(quanm, sMex::fReadonly);
            if( qua ) {
                res = parseFasta(db, baseFile, inf.ptr(), inf.length(), flags | eParsedQualities, primers00, complexityWindow, complexityEntropy, sizecomb, comb, iPart);
            } else
                res = parseFasta(db, baseFile, inf.ptr(), inf.length(), flags, primers00, complexityWindow, complexityEntropy, sizecomb, comb, iPart);
        }
        freeVariables();
    }
    sFile::remove(baseFileName);
    setFlagsDB(db, flags);
    if( res <= -1 ) {
        db.deleteAllJobs();
    } else {
        init(outfile);
    }
    return res;

}

void sVioseq2::updateQualities(char *origQua, char *currQua, idx numSeq, idx lenSeq, idx rptcount)
{
    for(idx i = 0; i < lenSeq; i++) {
        origQua[i] = 0.5 + ((real)(origQua[i] * numSeq) + (currQua[i] * rptcount)) / (numSeq + rptcount);
    }
    return;

}

void sVioseq2::updateBitQualities(char *origQua, char *currQua, idx lenSeq)
{
    return;

}

idx sVioseq2::parseFasta(sVioDB & db, sFil & baseFile, const char * fasta, idx fastalen, idx flags, const char * primers00, idx complexityWindow, real complexityEntropy, idx sizecomb, idx comb, sVec<Infopart> * iPart)
{

    isParallelversion = 0;
    if( sizecomb != 0 ) {
        isParallelversion = 1;
    }

    idx idofs = 0, seqofs = -1, idnext = -1;

    const char * c0 = fasta, *c = c0, *e = c + fastalen, *p = "\n";
    EParseItem mode = eSeq, pmode = eNone;

    Rec rec;

    while( c0 < e && (*c0 == '\n' || *c0 == '\r') )
        ++c0;
    c = c0;
    if( c > e ) {
        return -6;
    }


    idx errCode = 0;
    sFil aux;
    sApp::err_location = 0;
    idx cntFound = 0;
    idx unique = 0;
    const char * separator = "\xFF";
    bool isBitQual;
    sStr qty(sMex::fSetZero);
    sStr prefix;
    prefix.add(0, 5*sizecomb);
    const idx p100pcnt = e - c0;
    for(udx iN = 0; c < e; p = c, ++c) {
        if( *p == '\n' || *p == '\r' || c == e - 1 ) {
            if(*p=='\r' && c<e-1 && *c=='\n'){++c;}
            if( mode == eIdLine && *c!='>') {
                mode = eSeq;
                seqofs = c - c0;
            } else if( mode == eSeq && (*c == '>' || *c == '@') ) {
                mode = eIdLine;
                idnext = c - c0;
            } else if( *c == '#' || c == e - 1 ) {
                idnext = c - c0;
                mode = eStop;
            }

            if( pmode == eSeq && mode != eSeq ) {
                if( PROGRESS(cntFound, c - c0, p100pcnt, 80) ) {
                    errCode = -3;
                    break;
                }
                if( m_recLimit && m_recLimit < iN ) {
                    errCode = -2;
                    break;
                }
                if( idofs > seqofs || seqofs > idnext ) {
                    return -4;
                }

                const char * id = c0 + idofs;
                const char * seq = c0 + seqofs;
                const char * nxt = c0 + idnext;

                if( id[0] == '>' ) {
                    ++id;
                }

                idx il, iddiclen = 0;
                const char *bb = "H#=";
                const char *b;
                idx a;
                idx idhiverpt = 0;
                idx rptcount = 1;
                for(il = 0; id + il <= nxt && id[il] != '\n' && id[il] != '\r'; ++il) {
                    if( !iddiclen && il > 2 && (id[il] == ' ' || id[il] == '\t') )
                        iddiclen = il;
                    if( id[il] != *bb ) {
                        continue;
                    }
                    b = bb;
                    a = il;
                    while( 1 ) {
                        if( *b == 0 ) {
                            idhiverpt = il;
                            rptcount = 0;
                            while( id[a] != '\n' ) {
                                rptcount = rptcount * 10 + (id[a++] - '0');
                            }
                            break;
                        }
                        if( id[a++] != *b++ ) {
                            break;
                        }
                    }
                }
                if( !iddiclen ) {
                    iddiclen = il;
                }
                if( rptcount < 0 ) {
                    rptcount = 1;
                }
                if( idhiverpt != 0 ) {
                    il = idhiverpt;
                }
                if( !il ) {
                    errCode = -1;
                    break;
                }
                bool isValid = true;
                char *cpr = prefix.ptr(0);
                sBioseq::compressATGC((sStr *) cpr, seq, ((c == e - 1) ? 1 : 0) + sizecomb, false, 0);
                if( (isParallelversion) && !parallelizationfilter(prefix.ptr(), sizecomb, comb) ) {
                    isValid = false;
                    if (!complexityWindow){ ++iNN;}
                }
                if( !complexityWindow && !comb) {
                    addRecID(db, id, il);
                    idNum++;
                }
                if( isValid || complexityWindow ) {
                    qty.cut(0);
                    ++cntFound;
                    rec.ofsSeq = baseFile.length();
                    while( mapATGC[(idx) (*seq)] == 0xFF && seq < nxt )
                        ++seq;
                    char * cpy = baseFile.add(0, (nxt - seq) / 4 + 1);
                    rec.lenSeq = sBioseq::compressATGC((sStr *) cpy, seq, ((c == e - 1) ? 1 : 0) + nxt - seq, false, &qty);
                    baseFile.cut(rec.ofsSeq + (rec.lenSeq - 1) / 4 + 1);

                    bool isok = (complexityWindow != 0) ? sFilterseq::complexityFilter(cpy, rec.lenSeq, complexityWindow, complexityEntropy) : true;

                    idx primershift = 0;
                    if( isok && primers00 ) {
                        if( primershift == sNotIdx )
                            isok = false;
                    }

                    idx lqua = 0;
                    isBitQual = (qty.length() != 0) ? true : false;

                    if( isok && rec.lenSeq && isBitQual ) {
                        if( (flags & eParseQuaBit) ) {

                            baseFile.add0();
                            lqua = 1 + (rec.lenSeq - 1) / 8 + 1;
                            baseFile.add(qty.ptr(), qty.length());
                        } else {
                            lqua = rec.lenSeq;
                            baseFile.add(0, rec.lenSeq);
                        }
                    } else {
                        lqua = 1;
                        baseFile.addSeparator(separator, 1);
                    }

                    if( isok == false ) {
                        baseFile.cut(rec.ofsSeq);
                    } else {
                        if( complexityWindow && !comb ) {
                            addRecID(db, id, il);
                            idNum++;
                        }
                        if( isValid ) {
                            unique = tree->addSequence(rec.ofsSeq, rec.lenSeq);
                            if( unique == -1 ) {
                                baseFile.cut(rec.ofsSeq + (rec.lenSeq - 1) / 4 + 1 + lqua);
                                if( (flags & eParseNoId) == 0 )
                                    baseFile.add(id, il);
                                baseFile.add0();
                                rec.countSeq = rptcount;
                                *vofs.add() = rec;
                                uniqueCount++;

                                ids.vadd(1, uniqueCount - 1);
                                ods.vadd(1, iNN);
                            } else {
                                Rec *origRec = vofs.ptr(unique);
                                char *origQua = baseFile.ptr(origRec->ofsSeq) + ((origRec->lenSeq - 1) / 4 + 1);
                                if( origQua[0] != separator[0] ) {
                                    if( isBitQual ) {
                                        origQua++;
                                        char * qa = (char *) qty.ptr(0);
                                        for(idx iqua = 0; iqua < (origRec->lenSeq - 1) / 8 + 1; ++iqua) {
                                            origQua[iqua] &= qa[iqua];
                                        }
                                    } else {
                                        origQua[0] = separator[0];
                                    }
                                }
                                origRec->countSeq += rptcount;

                                baseFile.cut(rec.ofsSeq);
                                removeCount++;

                                ids.vadd(1, unique);
                            }
                            idsN.vadd(1, iNN);
                        }
                        ++iNN;
                    }
                }
                idofs = idnext;
                ++iN;
                sApp::err_location++;
            }

            pmode = mode;
        }

    }
    if( errCode != 0 ) {
        return errCode;
    }
    return writeSortVioDB(baseFile, db, iPart, true);
}

idx sVioseq2::parseFastQ(sVioDB &db, sFil & baseFile, const char * fasta, idx fastalen, idx flags, const char * primers00, idx complexityWindow, real complexityEntropy, idx sizecomb, idx comb, sVec<Infopart> * iPart)
{

    isParallelversion = 0;
    if( sizecomb != 0 ) {
        isParallelversion = 1;
    }
    idx idofs = 0, seqofs = -1, idnext = -1, idsecofs = -1, qualofs = -1;
    const char * c0 = fasta, *c = c0, *e = c + fastalen, *p = "\n";
    EParseItem mode = eQual, pmode = eNone;
    while( c0 < e && (*c0 == '\n' || *c0 == '\r') ) {
        ++c0;
    }
    c = c0;
    if( c > e ) {
        return -6;
    }

    BioseqTree tree(&baseFile, 0);
    idx errCode = 0;
    sFil aux;
    idx cntFound = 0;
    sApp::err_location = 0;
    sStr qty(sMex::fSetZero);
    sStr prefix;
    prefix.add(0, 5 * sizecomb);
    const idx p100pcnt = e - c0;
    for(udx iN = 0; c < e; p = c, ++c) {
        if( *p == '\n' || *p == '\r' || c == e - 1 ) {
            if(*p=='\r' && c<e-1 && *c=='\n'){++c;}
            if( mode == eIdLine ) {
                mode = eSeq;
                seqofs = c - c0;
            } else if( mode == eSeq && *c == '+' ) {
                mode = eId2;
                idsecofs = c - c0;
            } else if( mode == eId2 ) {
                mode = eQual;
                qualofs = c - c0;
            } else if( mode == eQual && (*c == '>' || *c == '@')) {
                if ((idsecofs != -1) && (idsecofs - seqofs < c-c0-qualofs)){
                    return -10;
                }
                mode = eIdLine;
                idnext = c - c0;
            } else if( c == e - 1 ) {
                idnext = c - c0;
                mode = eStop;
            }

            if( pmode == eQual && mode != eQual ) {
                if( PROGRESS(cntFound, c - c0, p100pcnt,80) ) {
                    errCode = -3;
                    break;
                }
                if( m_recLimit && m_recLimit < iN ) {
                    errCode = -2;
                    break;
                }
                if( idofs > seqofs || seqofs > idsecofs || idsecofs > qualofs || qualofs > idnext ) {
                    return -4;
                }

                const char * id = c0 + idofs;
                const char * seq = c0 + seqofs;
                const char * idsec = c0 + idsecofs;
                const char * qua = c0 + qualofs;
                const char * nxt = c0 + idnext;

                if( id[0] == '@' || id[0] == '>') {
                    ++id;
                }

                idx il, iddiclen = 0;
                const char *bb = "H#=";
                const char *b;
                idx a;
                idx idhiverpt = 0;
                idx rptcount = 1;
                for(il = 0; id + il < nxt && id[il] != '\n' && id[il] != '\r'; ++il) {
                    if( !iddiclen && il > 2 && (id[il] == ' ' || id[il] == '\t') )
                        iddiclen = il;
                    if( id[il] != *bb ) {
                        continue;
                    }
                    b = bb;
                    a = il;
                    while( 1 ) {
                        if( *b == 0 ) {
                            idhiverpt = il;
                            rptcount = 0;
                            while( id[a] != '\n' ) {
                                rptcount = rptcount * 10 + (id[a++] - '0');
                            }
                            break;
                        }
                        if( id[a++] != *b++ ) {
                            break;
                        }
                    }
                    b = bb;
                }
                if( !iddiclen ) {
                    iddiclen = il;
                }
                if( rptcount < 0 ) {
                    rptcount = 1;
                }
                if( idhiverpt != 0 ) {
                    il = idhiverpt;
                }
                if( !il ) {
                    errCode = -1;
                    break;
                }

                cntFound += addNewSequence(baseFile, db, id, il, seq, qua, (idsec - seq));
                idofs = idnext;
                ++iN;
                sApp::err_location++;
            }
            pmode = mode;
        }

    }

    if( errCode != 0 ) {
        return errCode;
    }

    return writeSortVioDB(baseFile, db, iPart);

}

char * sVioseq2::scanNumUntilEOL(const char * ptr, idx * pVal, const char * lastpos)
{
    while( (*ptr < '0' || *ptr > '9') && (*ptr != '\r' && *ptr != '\n') && *ptr != '*' && ptr < lastpos )
        ++ptr;
    *pVal = sNotIdx;
    if( ptr == lastpos || *ptr == '\n' || *ptr == '*' )
        return (char *) (ptr + 1);

    *pVal = 0;
    while( *ptr >= '0' && *ptr <= '9' && ptr < lastpos ) {
        *pVal = *pVal * 10 + (*ptr - '0');
        ++ptr;
    }
    return (char *) ptr;
}

char * sVioseq2::skipBlanks(const char * ptr, const char * lastpos)
{
    while( (*ptr == ' ' || *ptr == '\t') && (*ptr != '\r' && *ptr != '\n') && ptr < lastpos )
        ++ptr;

    return (char *) ptr;
}

char * sVioseq2::skipUntilEOL(const char * ptr, const char * lastpos)
{
    while( (*ptr != '\r' && *ptr != '\n') && ptr < lastpos )
        ++ptr;

    while( (*ptr == '\n' || *ptr == '\r') && ( ptr < lastpos ))
        ++ptr;
    return (char *) ptr;
}

void sVioseq2::skipMismatchesAtTheLeftEnd(const char * ptr, const char * lastpos, idx * skipLeftPositions)
{
    while( (*ptr != 'D') && (*ptr != '\r' && *ptr != '\n') && ptr < lastpos ) {
        ++ptr;
        if( *ptr == 'D' && *(ptr - 1) == 'M' && ptr + 4 < lastpos ) {
            ptr = ptr + 4;

            while( (*ptr != ' ') && (*ptr != '\t') && (*ptr != '\r' && *ptr != '\n') && *ptr != '*' && ptr < lastpos ) {
                if( *ptr == '0' ) {
                    ++(*skipLeftPositions);
                }
                if( *ptr >= '1' && *ptr <= '9' ) {
                    return;
                }
                ++ptr;
            }

        }
    }
    return;
}

void sVioseq2::skipMismatchesAtTheRightEnd(const char * ptr, const char * lastpos, idx * skipRightPositions)
{
    while( (*ptr != 'D') && (*ptr != '\r' && *ptr != '\n') && ptr < lastpos ) {
        ++ptr;
        if( *ptr == 'D' && *(ptr - 1) == 'M' && ptr + 4 < lastpos ) {
            ptr = ptr + 4;

            while( (*ptr != ' ') && (*ptr != '\t') && (*ptr != '\r' && *ptr != '\n') && ptr < lastpos ) {
                ++ptr;
            }
            if( (*ptr == ' ') || (*ptr == '\t') || (*ptr == '\n') || (*ptr == '\r') ) {
                --ptr;
                while( (*ptr != ' ') && (*ptr != '\t') && (*ptr != '\r' && *ptr != '\n') && *ptr != 'D' && ptr < lastpos ) {
                    if( *ptr == '0' ) {
                        ++(*skipRightPositions);
                    }
                    if( *ptr >= '1' && *ptr <= '9' ) {
                        return;
                    }
                    --ptr;
                }
            }

        }
    }
    return;
}

char * sVioseq2::scanUntilLetterOrSpace(const char * ptr, idx * pVal, const char * lastpos)
{
    while( (*ptr < '0' || *ptr > '9') && (*ptr != '\r' && *ptr != '\n') && *ptr != '*' && ptr < lastpos )
        ++ptr;
    *pVal = sNotIdx;
    if( ptr == lastpos || *ptr == '\r' || *ptr == '\n' || *ptr == '*' || *ptr == ' ' || *ptr == '\t' )
        return (char *) (ptr + 1);

    *pVal = 0;
    while( *ptr >= '0' && *ptr <= '9' && ptr < lastpos ) {
        *pVal = *pVal * 10 + (*ptr - '0');
        ++ptr;
    }
    return (char *) ptr;
}

char * sVioseq2::scanAllUntilSpace(const char * ptr, sStr * strVal, const char * lastpos, bool allowSpaces)
{
    idx p = 0;
    while( (ptr[p] != '\r' && ptr[p] != '\n') && ptr + p < lastpos && ((ptr[p] == '\t') || (allowSpaces && ptr[p] == ' ')) )
        ++p;

    const char * start = ptr + p;

    while( ptr + p < lastpos && ptr[p] != '\n' && ptr[p] != '\r' && (!allowSpaces || ptr[p] != ' ') && ptr[p] != '\t' )
        ++p;

    ptr = ptr + p;
    if( ptr > start && strVal ) {
        strVal->cut(0);
        strVal->add(start, ptr - start);
        strVal->add0();
    }
    return (char *) ptr;
}

char * sVioseq2::cigar_parser(const char * ptr, const char * lastpos, sVec<idx> * alignOut, idx * lenalign, idx * qryStart, idx PositionsToSkip, idx *matches)
{
    sStr full_str;
    *lenalign = 0;
    idx shift = 0, firstTime = true;
    idx current_qry = 0;
    idx current_sub = 0;
    if (matches){
        *matches = 0;
    }
    for(; strchr("0123456789MIDNSHPX=*+", *ptr) && ptr < lastpos; ++ptr) {
        idx number;
        ptr = scanUntilLetterOrSpace(ptr, &number, lastpos);

        if( *ptr == ' ' || *ptr == '\t' ) {
            break;
        }


        if( firstTime && strchr("MXDI=", *ptr) ) {
            if( number > PositionsToSkip ) {
                number = number - PositionsToSkip;
            }
        }

        if (matches && *ptr == 'M'){
            (*matches)+=number;

        }
        if( *ptr == 'N') {
            current_sub += number;
            continue;
        }

        if (*ptr == 'S'){
            current_qry += number;
            continue;
        }

        if( *ptr == 'H' ) {
            if (!firstTime){
                continue;
            }
            current_qry += number;
            continue;
        }

        if( !strchr("MXDI=", *ptr) )
            continue;

        if( firstTime && *ptr == 'D') {
            current_sub += number;
            firstTime = false;
            continue;
        }
        if( firstTime ) {
            shift = current_qry;
            firstTime = false;
        }

        idx * addPtr = alignOut->add(2 * number);
        for(idx k = 0; k < number; ++k) {
            addPtr[2 * k] = (*ptr == 'I') ? -1 : current_sub + k;
            addPtr[2 * k + 1] = (*ptr == 'D') ? -1 : current_qry + k - shift;
            ++(*lenalign);
        }
        if( *ptr != 'D' )
            current_qry += number;
        if( *ptr != 'I' ) {
            current_sub += number;
        }
    }

    if( qryStart )
        *qryStart = shift;
    return (char *) ptr;
}

idx sVioseq2::convertSAMintoAlignmentMap(const char * fileContent, idx filesize, sVec<idx> * alignOut, sDic<idx> * rgm, idx minMatchLength, bool isMinMatchPercentage, idx maxMissQueryPercent, sDic<idx> *sub, sDic<idx> *qry, bool useRowInformationtoExtractQry,  sFil * samHeader, sFil *samcontent, sFil * samFooter)
{
    idx res = 0;
    sVioDB db;
    sFil baseFile;
    sVioseq2 v;
    res = v.ParseSam(fileContent, filesize, db, baseFile, alignOut, rgm, true, 0, sub, qry, minMatchLength, isMinMatchPercentage, maxMissQueryPercent, useRowInformationtoExtractQry, samHeader, samcontent, samFooter);

    return res;
}

const char * getLine(sStr & buf, const char * fileContent)
{
    if( !fileContent[0] ) {
        return fileContent;
    }

    int i;
    buf.cut(0);
    for(i = 0; fileContent[i] && fileContent[i] != '\n' && fileContent[i] != '\r'; ++i) {
    }

    buf.add(fileContent, (fileContent[i] == '\r') ? (i+1) : i);
    buf.add0();

    if( fileContent[i] == '\r' && fileContent[i + 1] == '\n' )
        ++i;

    return fileContent + i + 1;
}

idx checkElement(sStr & string, const char * element)
{
    return (strncmp(string.ptr(), element, sLen(element)) == 0 ) ? 1 : 0;
}

const char * getInfoFromHeader(const char * srcFile, sStr & out, const char * whatToLookFor)
{
    sStr line;
    line.cut(0);
    idx found = 0;
    for(idx sr = 0; *srcFile; sr++) {
        srcFile = getLine(line, srcFile);
        if( found == 1 ) {
            if( strncmp(line, " ", 1) != 0 && strncmp(line.ptr(1), " ", 1) != 0 ){
                srcFile = srcFile - line.length();
                break;
            }
            sString::cleanEnds(line, line.length(), sString_symbolsBlank, true);
            out.printf("%s ", line.ptr());
        } else if( found == 0 ) {
            found = checkElement(line, whatToLookFor);
            if( found == 1 ) {
                sString::cleanEnds(line, line.length(), sString_symbolsBlank, true);
                out.printf("%s", line.ptr(sLen(whatToLookFor)));
            }
            else if (checkElement(line, "ORIGIN")) {
                srcFile = srcFile - line.length();
                break;
            }
            continue;
        }
    }
    if (strncmp(whatToLookFor,"VERSION",7)==0 && out.length() && strncmp(out.ptr(0),"ORIGIN",6)!=0){
        line.cut(0);
        sString::cleanEnds(out, out.length(), sString_symbolsBlank, true);
        sString::searchAndReplaceSymbols(&line, out, out.length(), sString_symbolsSpace, 0, 0, true, true, false, true);
        out.cut(0);
        out.addString(line.ptr());

    }
    return srcFile;
}
const char * sVioseq2::extractGBSequence(const char * srcFile, sStr & sequenceOut)
{
    static sStr line;
    line.cut(0);
    sequenceOut.cut(0);
    idx foundOri = 0;
    for(idx sq = 0; *srcFile; sq++) {
        srcFile = getLine(line, srcFile);
        if( strncmp(line, "//", 2) == 0 )
            break;

        if( foundOri == 1 ) {
            sStr seq;
            sString::cleanEnds(line, line.length(), sString_symbolsBlank, true);
            sString::searchAndReplaceSymbols(&seq, line, line.length(), sString_symbolsSpace, 0, 0, true, true, false, true);
            for(idx e = 1; e < sString::cnt00(seq); e++) {
                sequenceOut.printf("%s", sString::next00(seq, e));
            }
        } else if( foundOri == 0 ) {
            foundOri = checkElement(line, "ORIGIN");
            continue;
        }
    }
    return srcFile;
}


idx sVioseq2::ParseGb(const char * fileContent, idx filesize, sVioDB &db, sFil & baseFile, sVec<Infopart> * iPart)
{
    idx acceptedCount = 0;
    sVioDBQualities = false;
    for(idx iL = 0; *fileContent; iL++) {
        sStr myLine;
        sStr locus, def, version, sequence;
        fileContent = getLine(myLine, fileContent);
        idx checkL = checkElement(myLine, "LOCUS");
        if( checkL == 1 ) {
            sString::cleanEnds(myLine, myLine.length(), sString_symbolsBlank, true);
            sStr spaceLine;
            sString::searchAndReplaceSymbols(&spaceLine, myLine, myLine.length(), sString_symbolsSpace, 0, 0, true, true, false, true);
            locus.printf(0, "%s", sString::next00(spaceLine, 1));
            fileContent = getInfoFromHeader(fileContent, def, "DEFINITION ");
            fileContent = getInfoFromHeader(fileContent, version, "VERSION     ");
            fileContent = extractGBSequence(fileContent, sequence);
            sStr locusDef("%s%s", version.length() ? version.ptr() : locus.ptr(), def.length() ? def.ptr() : "");
            bool accept = addNewSequence(baseFile, db, locusDef.ptr(), locusDef.length(), sequence.ptr(), 0, sequence.length());
            acceptedCount += (idx) accept;
        }
    }
    writeSortVioDB(baseFile, db, iPart);
    return acceptedCount;
}

idx sVioseq2::ParseSam(const char * fileContent, idx filesize, sVioDB &db, sFil & baseFile, sVec<idx> * alignOut, sDic<idx> * rgm, bool alignOnly, sVec<Infopart> * iPart, sDic<idx> * sub, sDic<idx> * qry, idx minMatchLength, bool isMinMatchPercentage, idx maxMissQueryPercent, bool useRowInformationtoExtractQry, sFil * samHeader, sFil *samData, sFil * samFooter)
{

    bool extractSubFromDictionary = sub && (sub->dim() > 0) ? true : false;
    bool extractQryFromDictionary = (qry && (qry->dim() > 0)) ? true : false;
    sStr SEQ, QUA, qty, tmp, idQry;
    sStr seqrev;
    udx acceptedCount = 0;
    sVioDBQualities = false;
    const char * buf = fileContent;
    const char * const lastPos = fileContent + filesize;
    const idx p100pcnt = lastPos - fileContent + 1;

    idx flag, subStart, score, lenalign = 0, qryStart;

    real minMatchLengthPerc = (real)minMatchLength/100;
    udx cntFound = 0;
    udx cntReads = 0;
    sStr idSubStr, t;

    bool alignflag;
    idx errCode = 0;

    for(idx iAl = 0; buf < lastPos; ++iAl) {
        const char * prevPosB = buf;
        if( *buf == '@' ) {
            const char * ptr = skipUntilEOL(buf, lastPos);
            if( samHeader ) {
                samHeader->add(buf, ptr - buf);
            }
            buf = ptr;
            continue;
        }
        if( samData ) {
            const char * eol = skipUntilEOL(buf, lastPos);
            const idx lnLen = eol - buf;

            const char * flagP = sString::skipWords(buf, lnLen, 1, "\t");
            const char * rname = sString::skipWords(buf, lnLen, 2, "\t");
            const char * cigar = sString::skipWords(buf, lnLen, 5, "\t");
            const char * seq_ptr = sString::skipWords(buf, lnLen, 9, "\t");
            idx samFlag = 0;
            scanNumUntilEOL(flagP, &samFlag, eol);
            if( (samFlag & 0x4)  || (rname && *rname == '*') || (cigar && *cigar == '*') || (seq_ptr && *seq_ptr == '*') ) {
                if( samFooter ) {
                    samFooter->add(buf, eol - buf);
                }
            } else {
                if( seq_ptr ) {
                    samData->add(buf, seq_ptr - buf);
                    seq_ptr = sString::skipWords(seq_ptr, lnLen- (seq_ptr - buf), 1, "\t");
                    if( seq_ptr && seq_ptr[0] != '*' ) {
                        seq_ptr = sString::skipWords(seq_ptr, lnLen - (seq_ptr - buf), 1, "\t");
                    }
                    if( seq_ptr ) {
                        samData->add(seq_ptr, eol - seq_ptr);
                    } else {
                        samData->add("\n", 1);
                    }
                }
            }
        }
        buf = scanAllUntilSpace(buf, &idQry, lastPos, false);

        const char * id = idQry.ptr(0);
        idx idlen = idQry.length();
        idx idQryA = id ? atoidx(id) : sNotIdx;
        if( qry ) {
            if (extractQryFromDictionary){
                idQryA = sFilterseq::getrownum(*qry, id);
            }
            else {
                *qry->set(id, 0, &idQryA) = prevPosB - fileContent;
            }
        }
        if(useRowInformationtoExtractQry){
            idQryA = (idx)cntReads;
        }
        if( idQryA == sNotIdx ) {
            buf = skipUntilEOL(buf, lastPos);
            continue;
        }
        buf = scanNumUntilEOL(buf, &flag, lastPos);
        if( flag == sNotIdx )
            continue;
        idx dir_flag = 0;
        if( flag & 0x4 ) {
            alignflag = false;
        } else {
            alignflag = true;
        }
        if( flag & 0x10 ) {
            dir_flag = sBioseqAlignment::fAlignBackwardComplement | sBioseqAlignment::fAlignBackward;
        } else {
            dir_flag = sBioseqAlignment::fAlignForward;
        }
        idSubStr.cut(0);
        buf = scanAllUntilSpace(buf, &idSubStr, lastPos);
        const char * idSubS = idSubStr;
        if( (!idSubS || (idSubS[0] == '*' && idSubS[1] == 0)) && alignflag ) {
            alignflag = false;
        }
        if( alignflag && (*idSubS == '>' || *idSubS == '@') ) {
            ++idSubS;
        }
        idx idSub = sNotIdx;
        if( alignflag && rgm && rgm->dim() ) {
            idx * pid = rgm->get(idSubS);
            if( pid )
                idSub = *pid;
        }
        if( idSub == sNotIdx && sub ) {
            if (alignflag && extractSubFromDictionary){
                idSub = sFilterseq::getrownum(*sub, idSubS);
                if (idSub == sNotIdx){
                    errCode = -1;
                    break;
                }
            }
        }
        if( alignflag && idSub == sNotIdx ) {
            sscanf(idSubS, "%" DEC "", &idSub);
        }
        if( idSub == sNotIdx && alignOut ) {
            buf = skipUntilEOL(buf, lastPos);
            ++cntReads;
            continue;
        }

        idx LeftPositionsToSkip = 0;
        idx RightPositionsToSkip = 0;

        buf = scanNumUntilEOL(buf, &subStart, lastPos);
        if( subStart )
            subStart--;
        subStart = subStart + LeftPositionsToSkip;

        buf = scanNumUntilEOL(buf, &score, lastPos);
        if( score == sNotIdx )
            continue;

        buf = skipBlanks(buf, lastPos);
        if( (*buf == '\n' || *buf == '\r') || buf >= lastPos )
            continue;

        idx ofsThisAl = 0, headerSize, numMatches=0;
        if( (alignOut != 0) && (alignflag == true) ) {
            ofsThisAl = alignOut->dim();
            headerSize = sizeof(sBioseqAlignment::Al) / sizeof(idx);
            alignOut->add(headerSize);
            buf = cigar_parser(buf, lastPos, alignOut, &lenalign, &qryStart, LeftPositionsToSkip + RightPositionsToSkip, &numMatches);
        } else {
            buf = scanAllUntilSpace(buf, 0, lastPos);
        }
        idx len;
        buf = scanAllUntilSpace(buf, 0, lastPos);
        buf = scanAllUntilSpace(buf, 0, lastPos);
        buf = scanAllUntilSpace(buf, 0, lastPos);
        buf = scanAllUntilSpace(buf, &SEQ, lastPos);
        len = SEQ.length() - 1;


        if( strstr(SEQ, "*") || (len <= 0) ) {
            buf = skipUntilEOL(buf, lastPos);
            continue;
        }
        if( len > 0 ) {
            len = len - LeftPositionsToSkip;
        }
        buf = scanAllUntilSpace(buf, &QUA, lastPos);

        if( !alignOnly ) {
            if( (iAl % 10000) && PROGRESS(acceptedCount, buf - fileContent, p100pcnt, 80) ) {
                errCode = -3;
                break;
            }
            if( m_recLimit && m_recLimit < acceptedCount ) {
                errCode = -2;
                break;
            }
            if( QUA.length() == 2 && QUA.ptr()[0] == '*' ) {
                QUA.resize(SEQ.length()-1);
                memset(QUA.ptr(), 40 + '!', SEQ.length()-1);
                QUA.add0();
            }
            if( flag & 0x10 ) {
                seqrev.cut(0);
                seqrev.add(SEQ.ptr(), len);
                idx revll = len - 1;
                for(idx ll = 0; ll < len; ++ll, --revll) {
                    SEQ[ll] = sBioseq::mapRevATGC[sBioseq::mapComplementATGC[sBioseq::mapATGC[(idx) seqrev[revll]]]];
                }
                if( (SEQ.length() == QUA.length()) ) {
                    seqrev.cut(0);
                    seqrev.add(QUA.ptr(), len);
                    idx revll = len - 1;
                    for(idx ll = 0; ll < len; ++ll, --revll) {
                        QUA[ll] = seqrev[revll];
                    }
                }
            }
            const char * seq = SEQ.ptr();
            const char * qua = (SEQ.length() == QUA.length()) ? QUA.ptr() : 0;
            if (qua){
                sVioDBQualities = true;
            }

            bool accept = addNewSequence(baseFile, db, id, idlen, seq, qua, len);
            acceptedCount += (idx) accept;
        }
        ++cntReads;
        if(isMinMatchPercentage)
            minMatchLength = len*minMatchLengthPerc;
        if( minMatchLength && numMatches < minMatchLength ) {
            alignOut->cut(ofsThisAl);
            continue;
        }
        if ( maxMissQueryPercent && lenalign && ((100*(lenalign-numMatches))/lenalign > maxMissQueryPercent)){
            alignOut->cut(ofsThisAl);
            continue;
        }

        buf = skipUntilEOL(buf, lastPos);
        ++cntFound;
        if( (alignOut != 0) && (alignflag == true) ) {

            idx dimAlign = lenalign * 2;
            if( dimAlign == 0 ) {
                alignOut->cut(ofsThisAl);
                continue;
            }
            sBioseqAlignment::Al * hdr = (sBioseqAlignment::Al *) alignOut->ptr(ofsThisAl);
            hdr->setIdSub(idSub);
            hdr->setIdQry(idQryA);
            hdr->setScore(score);
            hdr->setFlags(dir_flag);
            hdr->setLenAlign(lenalign);
            hdr->setDimAlign(dimAlign);

            idx * m = hdr->match();

            qryStart = qryStart + LeftPositionsToSkip;
            if( (subStart == sNotIdx) || (subStart < 0) ) {
                alignOut->cut(ofsThisAl);
                continue;
            }
            if( (qryStart == sNotIdx) || (qryStart < 0) ) {
                alignOut->cut(ofsThisAl);
                continue;
            }
            hdr->setSubStart(subStart);
            hdr->setQryStart(qryStart);

            hdr->setDimAlign(sBioseqAlignment::compressAlignment(hdr, m, m));
            alignOut->cut(ofsThisAl + headerSize + hdr->dimAlign());
        }
    }
    if( !alignOnly && !errCode ) {
        return writeSortVioDB(baseFile, db, iPart);
    }
    PROGRESS(cntFound, 100, 100, 90);
    return errCode ? errCode : cntReads;
}

bool sVioseq2::addNewSequence(sFil & baseFile, sVioDB &db, const char *id, idx idlen, const char *seq, const char *qua, idx seqlen)
{
    if (seqlen <= 0){
        return false;
    }
    sStr prefix(sMex::fSetZero);
    idx unique = 0;
    prefix.cut(0);
    prefix.add(0,sizecomb);
    bool isValid = true;
    char *cpr = prefix.ptr(0);
    sBioseq::compressATGC((sStr *) cpr, seq, sizecomb, false, 0);
    if( (isParallelversion) && !parallelizationfilter(prefix.ptr(), sizecomb, comb) ) {
        isValid = false;
    }
    if( !complexityWindow ) {
        if( !comb ) {
            addRecID(db, id, idlen);
            idNum++;
        }
        if( !isValid ) {
            ++iNN;
            return false;
        }
    }
    Rec rec;
    idx rptcount = extractRptCount(id, &idlen);

    bool isQual = qua == 0 ? 0 : 1;
    rec.ofsSeq = baseFile.length();
    char * cpy = baseFile.add(0, (seqlen) / 4 + 1);
    sStr qty(sMex::fSetZero);
    qty.resize(seqlen / 8 + 1);
    rec.lenSeq = sBioseq::compressATGC((sStr *) cpy, seq, seqlen, false, &qty);

    baseFile.cut(rec.ofsSeq + (rec.lenSeq - 1) / 4 + 1);

    bool isok = (complexityWindow != 0) ? sFilterseq::complexityFilter(cpy, rec.lenSeq, complexityWindow, complexityEntropy) : true;


    if( isQual ) {
        char *pqu;
        if( isok && rec.lenSeq ) {
            if( (flags & eParseQuaBit) ) {
                idx lqua = (rec.lenSeq - 1) / 8 + 1;
                char * pqu = baseFile.add(0, lqua);
                memset(pqu, 0, lqua);
                for(idx iq = 0; iq < rec.lenSeq; ++iq)
                    if( qua[iq] > 53 )
                        pqu[iq / 8] |= ((idx) 1) << (iq % 8);
            } else {
                pqu = baseFile.add(0, rec.lenSeq);
                for(idx iq = 0; iq < rec.lenSeq; ++iq) {
                    pqu[iq] = ((qua[iq] - 33) > 0) ? (qua[iq] - 33) : 0;
                }
            }
        }
    }

    if (isok == false){
        baseFile.cut(rec.ofsSeq);
        return false;
    }

    if( complexityWindow && comb == 0) {
        addRecID(db, id, idlen);
        idNum++;
    }
    if (!isValid){
        ++iNN;
        return false;
    }

    char *currQua = 0;
    if( isQual ) {
        currQua = baseFile.ptr(rec.ofsSeq) + ((rec.lenSeq - 1) / 4 + 1);
        char * Nb = (char *) qty.ptr();
        for(idx is = 0; is < rec.lenSeq; is++) {
            if( Nb[is / 8] != 0 ) {
                if( Nb[is / 8] & (0x01 << (is % 8)) ) {
                    currQua[is] = 0;
                }
            } else
                is += 7;
        }
    }

    unique = tree->addSequence(rec.ofsSeq, rec.lenSeq);
    if( unique == -1 ) {
        rec.countSeq = rptcount;
        *vofs.add() = rec;
        uniqueCount++;
        ids.vadd(1, uniqueCount - 1);
        ods.vadd(1, iNN);
    } else {
        Rec *origRec = vofs.ptr(unique);
        if( isQual ) {
            char *origQua = baseFile.ptr(origRec->ofsSeq) + ((origRec->lenSeq - 1) / 4 + 1);
            updateQualities(origQua, currQua, origRec->countSeq, origRec->lenSeq, rptcount);
        }
        origRec->countSeq += rptcount;
        baseFile.cut(rec.ofsSeq);
        removeCount++;
        ids.vadd(1, unique);
    }
    idsN.vadd(1, iNN);

    ++iNN;
    return true;
}

idx sVioseq2::writeSortVioDB(sFil & baseFile, sVioDB &db, sVec<Infopart> * iPart, bool checkQuaBit)
{
    baseFile.remap(sMex::fReadonly);
    const char * separator = "\xFF";
    sVec<Rec> vofsSort(sMex::fBlockDoubling);
    sVec<idx> inSort;
    inSort.cut(0);
    sVec<idx> simSort;
    simSort.cut(0);
    sVec<idx> outSort;
    outSort.resize(uniqueCount);
    Rec rec;

    tree->inOrderTree3(0, &inSort, &simSort);

    idx longres = 0;
    sVec<idx> quaBitLen;
    quaBitLen.cut();
    PROGRESS(longres, 90, 100, 90);
    for(idx inx = 0; inx < inSort.dim(); inx++) {
        rec = vofs[inSort[inx]];
        {
            idx sim = simSort[inx];
            idx rpt = rec.countSeq;
            if( sim == -1 ) {
                sim = 0;
            }
            rec.countSeq = (sim << 32) | (rpt & 0xFFFFFFFF);
            longres += rpt;
        }
        *vofsSort.add() = rec;
        outSort[inSort[inx]] = inx;
        db.AddRecord(eRecREC_TYPE, (void *) &rec, sizeof(Rec));
        db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 1);
        db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 2);

        if (checkQuaBit){
            char * cpy = baseFile.ptr(rec.ofsSeq);
            db.AddRecord(eRecSEQ_TYPE, (void *) cpy, (rec.lenSeq - 1) / 4 + 1);
            char *pqu = cpy + (rec.lenSeq - 1) / 4 + 1;
            if( pqu[0] != separator[0] ) {
                pqu++;
                quaBitLen.vadd(1, inx);
                db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 3);
                db.AddRecord(eRecQUA_TYPE, (void *) pqu, (rec.lenSeq - 1) / 8 + 1);
            }
        }
        else {
            db.AddRecordRelationshipCounter(eRecREC_TYPE, 0, 3);
            char *cpy = baseFile.ptr(rec.ofsSeq);
            db.AddRecord(eRecSEQ_TYPE, (void *) cpy, (rec.lenSeq - 1) / 4 + 1);
            if (sVioDBQualities == true){
                char *pqu = cpy + (rec.lenSeq - 1) / 4 + 1;
                db.AddRecord(eRecQUA_TYPE, (void *) pqu, rec.lenSeq);
            }
        }
        PROGRESS(longres, 90, 100, 90);
    }

    db.AllocRelation();
    idx invert = 1;
    PROGRESS(longres, 95, 100, 90);
    if( isParallelversion ) {
        for(idx i = 0; i < ids.dim(); i++) {
            Infopart tmp;
            tmp.origID = idsN[i] + 1;
            tmp.partID = outSort[ids[i]] + 1;
            tmp.numID = comb;
            iPart->vadd(1, tmp);
            invert = -1;
            PROGRESS(longres, 94, 100, 90);
        }
        if( comb == 0 ) {
            for(idx i = 0; i < idNum; i++) {
                db.AddRelation(eRecID_TYPE, 1, i + 1, i + 1);
                PROGRESS(longres, 95, 100, 90);
            }
        }
    } else {
        for(idx i = 0; i < ids.dim(); i++) {
            db.AddRelation(eRecID_TYPE, 1, i + 1, outSort[ids[i]] + 1);
        }
    }

    PROGRESS(longres, 96, 100, 90);
    if (checkQuaBit){
        for(idx i = 0; i < uniqueCount; i++) {
            PROGRESS(longres, 96, 100, 90);
            db.AddRelation(eRecREC_TYPE, 1, i + 1, (ods[inSort[i]] + 1) * invert);
            db.AddRelation(eRecREC_TYPE, 2, i + 1, i + 1);
        }
        for(idx i = 0; i < quaBitLen.dim(); i++) {
            PROGRESS(longres, 97, 100, 90);
            db.AddRelation(eRecREC_TYPE, 3, quaBitLen[i] + 1, i + 1);
        }
    }
    else {
        for(idx i = 0; i < uniqueCount; i++) {
            PROGRESS(longres, 98, 100, 90);
            db.AddRelation(eRecREC_TYPE, 1, i + 1, (ods[inSort[i]] + 1) * invert);
            db.AddRelation(eRecREC_TYPE, 2, i + 1, i + 1);
            if (sVioDBQualities == true){
                db.AddRelation(eRecREC_TYPE, 3, i + 1, i + 1);
            }
        }
    }

    db.Finalize(m_isSingleFile);
    PROGRESS(longres, 99, 100, 90);

    return longres;
}

idx sVioseq2::extractRptCount(const char *id, idx *idlen)
{
    idx il, iddiclen = 0;
    const char * nxt = id + *idlen;
    const char *bb = "H#=";
    const char *b;
    idx a;
    idx idhiverpt = 0;
    idx rptcount = 1;
    for(il = 0; id + il < nxt && id[il] != '\n' && id[il] != '\r'; ++il) {
        if( !iddiclen && il > 2 && (id[il] == ' ' || id[il] == '\t') )
            iddiclen = il;
        if( id[il] != *bb ) {
            continue;
        }
        b = bb;
        a = il;
        while( 1 ) {
            if( *b == 0 ) {
                idhiverpt = il;
                rptcount = 0;
                while( id[a] != '\n' ) {
                    rptcount = rptcount * 10 + (id[a++] - '0');
                }
                break;
            }
            if( id[a++] != *b++ ) {
                break;
            }
        }
        b = bb;
    }
    if( !iddiclen ) {
        iddiclen = il;
    }
    if( rptcount < 0 ) {
        rptcount = 1;
    }
    if( idhiverpt != 0 ) {
        il = idhiverpt;
    }
    if( !il ) {
        return -1;
    }
    *idlen = il;
    return rptcount;
}
