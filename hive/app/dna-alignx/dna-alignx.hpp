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
#pragma once
#ifndef sDnaAlignX_hpp
#define sDnaAlignX_hpp

#include <slib/core/dic.hpp>
#include <slib/std/file.hpp>
#include <violin/violin.hpp>
#include <qlib/QPrideProc.hpp>
#include <dmlib/dmlib.hpp>

using namespace slib;

class DnaAlignX
{
    public:
        typedef enum
        {
            eIndexerMessage_Wait = -1,
            eIndexerMessage_Error = 0,
            EIndexerMessage_Started,
            eIndexerMessage_Done,
            eIndexerMessage_Aborted
        } EIndexerMessage;

        DnaAlignX(sQPrideProc & qprideproc)
            : qp(qprideproc), subjectIdxLockId(0), flagSet(sBioseqAlignment::fAlignForward), subbiomode(sBioseq::eBioModeShort), qrybiomode(sBioseq::eBioModeShort), separateHiveseqs(false), qryInFastQ(false), keepOriginalSubId(false),
              keepOriginalQryId(false), keepRefNs(true), scoreFilter(0), seedSize(28), evalueFilter(0), minMatchLength(75),
               maxMissQueryPercent(15), frmProduceRandomReadsForNT(false), Sub(qp.user), Qry(qp.user)
        {
        }
        virtual ~DnaAlignX();

        virtual void readParams(sVar * pForm, sUsr * user, sVec<sUsrProc> & objs);

        EIndexerMessage IndexSubject(sStr & subjectIndexDir);

        //! Sort and uniquify subject IDs
        /*! \param buf string from formValue with subject IDs like "foo.1.2" or "1.2" or "1" or "http://example.com/hive/1.2";
         *  \param subIdList the container to return the vector of sorted and unique sHiveIds
         *  \returns true if there was a change in the list of subject HiveIds and false if HiveIds
         *          were already sorted and unique */
        inline bool subjectSort(sStr * buf = 0, sVec<sHiveId> * subIdList = 0)
        {
            static sStr lbuf;

            if( !buf ) {
                buf = &lbuf;
                buf->cut0cut();
                qp.formValue("subject", buf);
            }

            sVec<sHiveId> lsubIdList, suSubIdList /*sort unique*/;
            if( !subIdList ) {
                subIdList = &suSubIdList;
            }
            sHiveId::parseRangeSet(lsubIdList, *buf, 0);
            sSort::sort(lsubIdList.dim(), lsubIdList.ptr());
            for(idx i = 0; i < lsubIdList.dim() ; ++i ) {
                sHiveId * hid = subIdList->add();
                *hid = lsubIdList[i];
            }
            idx ic = 0;
            for(idx i = 0; i < subIdList->dim(); i++) {
                if( i && *(subIdList->ptr(i)) == *(subIdList->ptr(i - 1)) ) {
                    continue;
                }
                *subIdList->ptr(ic++) = lsubIdList[i];
            }
            subIdList->cut(ic);
            if( subIdList->dim() != lsubIdList.dim() ) {
                return true;
            } else {
                for(idx i = 0; i < lsubIdList.dim(); ++i) {
                    if( lsubIdList[i] != *(subIdList->ptr(i)) )
                        return true;
                }
            }
            return false;
        }

        inline const char * subjectGet(sStr * buf = 0, sVec<sHiveId> * subIdList = 0, bool doSort = true)
        {
            static sStr lbuf;

            if( !buf ) {
                buf = &lbuf;
                buf->cut0cut();
            }
            qp.formValue("subject", buf);

            sVec<sHiveId> lsubIdList;
            if(!subIdList) {
                subIdList = &lsubIdList;
            }

            if( doSort && subjectSort(buf, subIdList) ) {
                buf->cut0cut();
                sHiveId::printVec(*buf, *subIdList, "\n", false);
            }

            Sub.empty();
            Sub.parse(buf->ptr(), subbiomode, false, qp.user);
            return buf->ptr();
        }

        inline udx updateSubjectProp(void)
        {
            if( !qp.objs.dim() )
                return 0;
            sUsrObj & o = qp.objs[0];
            sVec<sHiveId> subIdList;
            if( subjectSort(0, &subIdList) ) {
                return o.propSetHiveIds("subject", subIdList);
            }
            return 0;
        }
        virtual idx subjectGetChunkSize() const
        {
            return 0;
        }
        virtual bool subjectIndexPersistent() const
        {

            return qp.cfgInt(0, "dna-alignx.singleSubjectIndexPersistent", 1) > 0;
        }
        // confirms subject is applicable for algorithm
        virtual bool subjectVerify(const char * tax, sStr * err)
        {
            if( tax && tax[0] && sString::compareChoice("NT", tax, 0, true, 0, true) != -1 ) {
                if( err ) {
                    err->printf("NT reference cannot be used");
                }
                return false;
            }
            return true;
        }
        virtual DnaAlignX::EIndexerMessage subjectDump(const char * subjectPath, idx start = 0, idx cnt = 0);

        virtual const char * queryGet(sStr * buf = 0)
        {
            static sStr lbuf;
            return qp.formValue("query", buf ? buf : &lbuf);
        }

        EIndexerMessage queryIndexChunk(sStr & queryFiles, idx qStart, idx qEnd);

        virtual bool queryVerify(sDic<sBioseq> * qry, sStr * err)
        {
            return true;
        }

        virtual EIndexerMessage Align(const char * query);

        virtual idx ParseAlignment(const idx keepAllMatches, sDic<idx> * unalignedList);

        //! Returns the extension of the output file for the aligner selected.
        /*! \returns Pointer to extension.
         */
        virtual const char * resultExtension(void) const = 0;

        virtual idx fillAlignmentMap(sFil & fl, sVec<idx> & alignmentMap, sDic<idx> * unalignedList) = 0;

        virtual bool getPathsForFinalProcessing(sStr & paths00)
        {
            return false;
        }
        idx FinalProcessing();

    protected:

        friend class DnaAlignXProc;

        sQPrideProc & qp;
        sDic<idx> idMap;
        idx subjectIdxLockId, flagSet;
        sBioseq::EBioMode subbiomode, qrybiomode;
        bool separateHiveseqs;
        sStr algorithm;
        bool qryInFastQ, keepOriginalSubId, keepOriginalQryId, keepRefNs;
        idx scoreFilter, seedSize;
        sStr subjectFile00;
        sStr additionalArguments, additionalCommandLineParameters;
        sStr resourceRoot, referenceAnnotationFile;
        real evalueFilter;
        idx minMatchLength, maxMissQueryPercent;
        bool frmProduceRandomReadsForNT;
        sHiveseq Sub, Qry;
        sDic<sHiveseq> QryList;

        const char * const getWorkDir(bool algo = false)
        {
            static sStr workDir;
            workDir.cut0cut(0);
            qp.cfgStr(&workDir, 0, "qm.tempDirectory");
            if( workDir ) {
                workDir.printf("%s-qry-%"DEC"/%s", qp.svc.name, qp.reqId, algo ? algorithm.ptr() : "");
            }
            return workDir;
        }
};

class DnaAlignXBlastOutput: public DnaAlignX
{
    public:
        DnaAlignXBlastOutput(sQPrideProc & qprideproc)
            : DnaAlignX(qprideproc), blastMode(sBioAlBlast::eBlastStandardOut)
        {
            qp.formValue("output_fmt", &output_format, "blast_out");
        }
        virtual ~DnaAlignXBlastOutput()
        {
        }
        const char * resultExtension(void) const
        {
            return output_format;
        }

        idx fillAlignmentMap(sFil & fl, sVec<idx> & alignmentMap, sDic<idx> * unalignedList)
        {
            idx cntFound = 0;
            const char * ext = resultExtension();
            if( strcmp(ext, "tsv") == 0 ) {
                cntFound = fl.recCnt(true);
            } else if( strcmp(ext, "blast_out") == 0 ) {
                // Default blast_out output files
                sIO log;
                cntFound = sBioAlBlast::SSSParseAlignment(&log, fl.ptr(), fl.length(), &alignmentMap, scoreFilter, minMatchLength, maxMissQueryPercent, idMap.dim() ? &idMap : 0, 0, 0, blastMode, unalignedList);
                if( log ) {
                    qp.logOut(qp.eQPLogType_Debug, "%s", log.ptr());
                }
            } else {
                qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Warning, "Unknown output format specified '%s'", ext);
                cntFound = -1;
            }
            return cntFound;
        }
        virtual bool subjectVerify(const char * tax, sStr * err)
        {
            if( tax && tax[0] && sString::compareChoice("NT", tax, 0, true, 0, true) != -1 ) {
                keepOriginalSubId = false;
                if( Sub.dimRefs() > 1 ) {
                    if( err ) {
                        err->printf("NT reference cannot be used in conjunction with others");
                    }
                    return false;
                }
            }
            return true;
        }

    protected:
        sBioAlBlast::eBlastOutmode blastMode;
        sStr output_format;
};

class DnaAlignXSAMOutput: public DnaAlignX
{
    public:
        DnaAlignXSAMOutput(sQPrideProc & qprideproc)
            : DnaAlignX(qprideproc)
        {
        }
        virtual ~DnaAlignXSAMOutput()
        {
        }
        const char * resultExtension(void) const
        {
            return "sam";
        }
        idx fillAlignmentMap(sFil & fl, sVec<idx> & alignmentMap, sDic<idx> * unalignedList)
        {
            return sVioseq2::convertSAMintoAlignmentMap(fl.ptr(), fl.length(), &alignmentMap, idMap.dim() ? &idMap : 0, minMatchLength, maxMissQueryPercent);
        }

};

class DnaAlignXBlastProteinPackager: public DnaAlignXBlastOutput
{
    public:
        DnaAlignXBlastProteinPackager(sQPrideProc & qprideproc)
            : DnaAlignXBlastOutput(qprideproc)
        {
        }
        virtual ~DnaAlignXBlastProteinPackager()
        {
        }
        virtual idx ParseAlignment(const idx keepAllMatches, sDic<idx> * unalignedList)
        {
            const idx ret = DnaAlignXBlastOutput::ParseAlignment(keepAllMatches, unalignedList);

            idx cnt = 0;
            for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
                // Add check to see if there is any regex if not then skip; to avoid database call if no regex
                // look for files with specific extension in outPath which has result file prefix in it like /tmp/blastx
                sStr dst, src("%s%"DEC".%s", getWorkDir(true), cnt, resultExtension()); // ex: blastx123.blast_out
                sFilePath flnm(src, "req-%%flnm");
                if( !qp.reqAddFile(dst, flnm) || !sFile::rename(src, dst) ) {
                    qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Warning, "Failed to save slice output");
                    qp.logOut(qp.eQPLogType_Warning, "Failed to save output for %s", flnm.ptr());
                    return -1;
                }
                qp.logOut(qp.eQPLogType_Debug, "moved %s to %s\n", src.ptr(), dst.ptr());
            }
            return ret;
        }
        virtual bool getPathsForFinalProcessing(sStr & paths)
        {
            idx cnt = 0;
            for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
                sFilePath buf(getWorkDir(true), "req-%%flnm%"DEC".%s", cnt, resultExtension());
                qp.grpDataPaths(qp.grpId, buf, &paths, qp.vars.value("serviceName"), " ");
            }
            return paths;
        }
};


class DnaAlignXBlast: public DnaAlignXBlastOutput
{
    public:
        DnaAlignXBlast(sQPrideProc & qprideproc)
            : DnaAlignXBlastOutput(qprideproc)
        {
        }
        virtual ~DnaAlignXBlast()
        {
        }

        virtual idx subjectGetChunkSize() const
        {
            // makeblastdb fails on machines will small memory (24GB) on NT (>100GB)
            return (idx) 32 * 1024 * 1024 * 1024;
        }
};

class DnaAlignXBlastX: public DnaAlignXBlastProteinPackager
{
    public:
        DnaAlignXBlastX(sQPrideProc & qprideproc)
            : DnaAlignXBlastProteinPackager(qprideproc)
        {
            blastMode = sBioAlBlast::eBlastProteinOut;
            keepOriginalQryId=true;
            keepOriginalSubId=true;
        }
        virtual ~DnaAlignXBlastX()
        {
        }
        bool subjectVerify(const char * tax, sStr * err)
        {
            return true;
        }
        virtual bool subjectIndexPersistent() const
        {
            return false;
        }
        virtual DnaAlignX::EIndexerMessage subjectDump(const char * subjectPath, idx start = 0, idx cnt = 0)
        {
            // for blastx we need to grab all object files (protein fastAs?) and unpack them if needed
            // this is all here because we are not supporting proteins in as sBioseqSet
            sStr ssStr;
            const char * sublist = subjectGet(&ssStr);
            sStr tokenizedSubList;
            sString::searchAndReplaceSymbols(&tokenizedSubList, sublist, 0, "\n;,", 0, 0, true, true, false, true);
            for(const char * id = tokenizedSubList; id; id = sString::next00(id)) {
                sUsrObj * obj = qp.user->objFactory(sHiveId(id));
                if( !obj ) {
                    qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Object %s not accessible", id);
                    return eIndexerMessage_Error;
                }
                sUsrFile * fobj = dynamic_cast<sUsrFile *>(obj);
                if( !fobj ) {
                    qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Object %s not a file", id);
                    return eIndexerMessage_Error;
                }
                sStr filePath;
                fobj->getFile(filePath);
                if( filePath ) {
                    sFilePath symlinksPath(filePath, "%s%s.%%ext", subjectPath, fobj->Id().print());
                    if( sFile::symlink(filePath, symlinksPath) ) {
                        dmLib dm;
                        sStr log;
                        if( dm.unpack(symlinksPath, algorithm, &log, &log, sQPrideProc::reqProgressFSStatic, &qp, qp.svc.lazyReportSec) ) {
                            for(const dmLib::File * curFile = dm.first(); !dm.end(curFile); curFile = dm.next(curFile)) {
                                subjectFile00.printf("%s", curFile->location());
                                subjectFile00.add0();
                            }
                            subjectFile00.add0();
                        } else {
                            qp.logOut(qp.eQPLogType_Error, "unpacking file %s failed: %s", filePath.ptr(), log.ptr());
                            return eIndexerMessage_Error;
                        }
                    } else {
                        qp.logOut(qp.eQPLogType_Error, "symlinking file %s failed", filePath.ptr());
                        return eIndexerMessage_Error;
                    }
                } else {
                    qp.logOut(qp.eQPLogType_Error, "Object '%s' has no data", id);
                    return eIndexerMessage_Error;
                }
                delete obj;
            }
            return eIndexerMessage_Done;
        }
};

class DnaAlignXTBlastX: public DnaAlignXBlastProteinPackager
{
    public:
        DnaAlignXTBlastX(sQPrideProc & qprideproc)
            : DnaAlignXBlastProteinPackager(qprideproc)
        {
            blastMode = sBioAlBlast::eBlastProteinOut;
            subbiomode = sBioseq::eBioModeLong;
            keepOriginalSubId=true;
            keepOriginalQryId=true;
        }
        virtual ~DnaAlignXTBlastX()
        {
        }

};

// Uses blast output
class DnaAlignXBlat: public DnaAlignXBlastOutput
{
    public:
        DnaAlignXBlat(sQPrideProc & qprideproc)
            : DnaAlignXBlastOutput(qprideproc)
        {
        }
        virtual ~DnaAlignXBlat()
        {
        }
        bool subjectVerify(const char * tax, sStr * err)
        {
            if( sString::compareChoice("NT", tax, 0, true, 0, true) != -1 ) {
                if( Sub.dimRefs() > 1 ) {
                    if( err ) {
                        err->printf("NT reference cannot be used in conjunction with others");
                    }
                    return false;
                }
            }
            return true;
        }
        virtual idx subjectGetChunkSize() const
        {
            // blat cannot handle references files more than ~3-4Gb
            return (idx) 1 * 1024 * 1024 * 1024;
        }
};

class DnaAlignXBowtie: public DnaAlignXSAMOutput
{
    public:
        DnaAlignXBowtie(sQPrideProc & qprideproc)
            : DnaAlignXSAMOutput(qprideproc)
        {
        }
        virtual ~DnaAlignXBowtie()
        {
        }
};

class DnaAlignXBowtie2: public DnaAlignXSAMOutput
{
    public:
        DnaAlignXBowtie2(sQPrideProc & qprideproc)
            : DnaAlignXSAMOutput(qprideproc)
        {
        }
        virtual ~DnaAlignXBowtie2()
        {
        }
};

class DnaAlignXBWA: public DnaAlignXSAMOutput
{
    public:
        DnaAlignXBWA(sQPrideProc & qprideproc)
            : DnaAlignXSAMOutput(qprideproc)
        {
        }
        virtual ~DnaAlignXBWA()
        {
        }
};

class DnaAlignXTophat: public DnaAlignXSAMOutput
{
    public:
        DnaAlignXTophat(sQPrideProc & qprideproc)
            : DnaAlignXSAMOutput(qprideproc)
        {
            subbiomode = sBioseq::eBioModeLong;
            qrybiomode = sBioseq::eBioModeLong;
            qryInFastQ = true;
            keepOriginalSubId = true;
            keepOriginalQryId = false;
        }
        virtual ~DnaAlignXTophat()
        {
        }
        virtual bool subjectVerify(const char * tax, sStr * err)
        {
            if( Sub.dimRefs() > 1 ) {
                if( err ) {
                    err->printf("Only a single reference can be used");
                }
                return false;
            }
            return DnaAlignX::subjectVerify(tax, err);
        }
        virtual bool queryVerify(sDic<sBioseq> * qry, sStr * err)
        {
            if( qry->dim() > 2 || qry->dim() < 1 ) {
                if( err ) {
                    err->printf("Only one or two input query files can be used");
                }
                return false;
            }
            return true;
        }
        idx ParseAlignment(const idx keepAllMatches, sDic<idx> * unalignedList)
        {
            const idx ret = DnaAlignXSAMOutput::ParseAlignment(keepAllMatches, unalignedList);
            // pickup accepted_hits.bam into reqData
            idx cnt = 0;
            for(const char * p = subjectFile00; p; p = sString::next00(p), ++cnt) {
                sStr pathT("%s%"DEC"/accepted_hits_sorted.bam", getWorkDir(true), cnt);
                pathT.add0(2);
                const char * dst = qp.reqAddFile(pathT, _bam);
                if( !sFile::rename(pathT, dst) ) {
                    qp.reqSetInfo(qp.reqId, qp.eQPInfoLevel_Error, "Failed to save slice result");
                    qp.reqSetStatus(qp.reqId, qp.eQPReqStatus_ProgError);
                    return -2;
                }
            }
            return ret;
        }
        virtual bool getPathsForFinalProcessing(sStr & paths)
        {
            return qp.grpDataPaths(qp.grpId, _bam, &paths, qp.vars.value("serviceName"), " ");
        }
//        virtual idx fillAlignmentMap(sFil & fl, sVec<idx> & alignmentMap, sDic<idx> * unalignedList)
//        {
//            sDic<idx> subIds;
////            // > 1 million reference sequences, mmap to disk to avoid heap memory abuse
////            if( Sub.dim() > 1000000 ) {
////                sStr tempSubDir;
////                cfgStr(&tempSubDir, 0, "qm.tempDirectory");
////                tempSubDir.printf("%"DEC"-%s", reqId, "subids.dic");
////                subIds.init(tempSubDir);
////            }
//            // Create dictionary for Sub Id's
//            sFilterseq::parseDicBioseq(subIds, Sub, 0);
//            return sVioseq2::convertSAMintoAlignmentMap(fl.ptr(), fl.length(), &alignmentMap, idMap.dim() ? &idMap : 0, minMatchLength, maxMissQueryPercent, &subIds, 0, false);
//        }

    private:
        static const char * const _bam;
};
const char * const DnaAlignXTophat::_bam = "req-accepted_hits_sorted.bam";

class DnaAlignXMultiple: public DnaAlignX
{
        typedef DnaAlignX TParent;
    public:
        DnaAlignXMultiple(sQPrideProc & qprideproc)
            : DnaAlignX(qprideproc)
        {
            subbiomode = sBioseq::eBioModeLong;
            qrybiomode = sBioseq::eBioModeLong;
        }
        virtual ~DnaAlignXMultiple()
        {
        }
        virtual void readParams(sVar * pForm, sUsr * user, sVec<sUsrProc> & objs)
        {
            TParent::readParams(pForm, user, objs);
            if( objs.dim() ) {
                sUsrObj & o = objs[0];
                o.propSet("query", queryGet());
            }
        }
        bool subjectVerify(const char * tax, sStr * err)
        {
            if( Sub.dim() < 2 ) {
                if( err ) {
                    err->printf("Only multiple references can be used");
                }
                return false;
            }
            return DnaAlignX::subjectVerify(tax, err);
        }
        virtual const char * queryGet(sStr * buf = 0)
        {
            return subjectGet(buf);
        }
        const char * resultExtension(void) const
        {
            return "ma";
        }
        virtual idx fillAlignmentMap(sFil & fl, sVec<idx> & alignmentMap, sDic<idx> * unalignedList)
        {
            return sBioseqAlignment::readMultipleAlignment(&alignmentMap, fl.ptr(), fl.length(), sBioseqAlignment::eAlRelativeToMultiple, 0, true);
        }
};

class DnaAlignXMafft: public DnaAlignXMultiple
{
    public:
        DnaAlignXMafft(sQPrideProc & qprideproc)
            : DnaAlignXMultiple(qprideproc)
        {
        }
        virtual ~DnaAlignXMafft()
        {
        }
};

class DnaAlignXClustal: public DnaAlignXMultiple
{
    public:
        DnaAlignXClustal(sQPrideProc & qprideproc)
            : DnaAlignXMultiple(qprideproc)
        {
        }
        virtual ~DnaAlignXClustal()
        {
        }
};

#endif // sDnaAlignX_hpp

