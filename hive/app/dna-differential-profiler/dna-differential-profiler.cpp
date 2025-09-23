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

#include <qlib/QPrideProc.hpp>
#include <ssci/bio.hpp>
#include <violin/violin.hpp>

class DnaDifferentialProfiler : public sQPrideProc
{
    public:
        enum enumFileType
        {
            aaProfile = 0x01,
            freqProfile = 0x02,
            histProfile = 0x04,
            noiseProfile = 0x08,
            infoProfile = 0x10,
            snpProfile = 0x20
        };

        struct ProfilerTypes {
            enumFileType eType;
            const char *namefile;
            idx posinVector;
        };

    private:
        struct ProfHandler{
             idx poolprofCSV;
             sFil *profCSV;
             idx fileNamePos;
             ProfilerTypes profType;
             const char *currPos;
             idx profCSVlength;
        };

        struct ProfileInfo
        {
                idx profobjID;
                sVec<idx> profCursors;
        };

        struct GeneStats
        {
                idx num;
                idx sum;
                idx max;
        };

        struct AnnotInfo
        {
                idx gene_id;
                idx gene_start;
                idx gene_end;
        };


        struct RowInfo {
                idx iSub;
                idx position;

                char letter;
                real freq[6];


                idx colVal[11];
                idx coverage, maxCvg, avgCvg, numCvgItems;
                idx progressProfile;

                RowInfo()
                {
                    sSet(this);
                }

                RowInfo (const RowInfo &rinfo)
                {
                    init (rinfo);
                }

                RowInfo& operator=(const RowInfo &rinfo)
                {
                    init(rinfo);
                    return *this;
                }
                void init(const RowInfo &rinfo)
                {
                    if (this != &rinfo){
                        memcpy(this, &rinfo, sizeof(RowInfo));
                    }
                }


                idx readfromSNP(sBioseqSNP::SNPRecord * src, bool extractSNP, idx *coverage)
                {
                    iSub = src->iSub;
                    position = src->position;
                    letter = src->letter;
                    if (extractSNP){
                        for(idx i = 0; i < 6; ++i) {
                            freq[i] = src->freq(i);
                        }
                    }
                    if (coverage){
                        *coverage = src->coverage();
                    }
                    return 0;
                }

                void addCoverageStats(idx cov)
                {
                    if (cov > maxCvg){
                        maxCvg = cov;
                    }
                    avgCvg += cov;
                    numCvgItems += 1;
                }

                real getAvgCov ()
                {
                    return (numCvgItems == 0) ? 0 : (avgCvg / numCvgItems);
                }


        };

        struct InputParameters
        {
                idx sourceColumn;
                ProfilerTypes profType;
                idx colRow;
                idx minVal;
                idx maxVal;
                idx comparator;
                idx value;
        };

        struct OutputParameters
        {
                idx outputFormat;
                ProfilerTypes outProfType;
                idx colRow;
                idx colNamePosition;
        };

        struct Jobs {
                idx inputIndex;
                idx inputNumParams;
                idx outputIndex;
                idx outputNumParams;
                idx outfilenamePosition;
                idx numUsedFiles;
                idx SNPProfile;
                idx outputRangeFilter;
                idx minCvgAllowed;
                idx maxCvgAllowed;
                real thresholdSNPpercentage;
                bool outputSNPType;
                bool outputDifferentiator;
                bool outputGeneTable;
                bool useAnnotInformation;
        };

    private:
        idx _req;
        sFilPool _pool;
        sVec <ProfHandler> _profHandlers;
        sVec <ProfileInfo> _profs;
        sStr nameFiles00;

        sVec <InputParameters> inpParam;
        sVec <OutputParameters> outParam;

        sHiveseq *_bioseq;
        sStr annotObjIds00;
        sIonAnnotBase * _ionAnnot;
        idx _numAnnotTypes;

        sDic <idx> geneDictionary;
        sVec<GeneStats> geneStat;
        sVec <AnnotInfo> geneInfo;
        bool createDictionaryGeneTable;

        idx m_progressLast;
        idx m_progressUpdate;

    protected:
        static ProfilerTypes _prof_types[];
        idx numproftypes;


        sStr localBuf;

    public:
        static const char * csvParseStr(const char *src, const char *srcend, char *buf, size_t bn, unsigned char *row[], idx rn, idx sep, idx flags, idx *colnum, sStr *err = 0);

        DnaDifferentialProfiler(const char * defline00,const char * srv) : sQPrideProc(defline00,srv), _pool(1000), _profHandlers(sMex::fExactSize), _profs(sMex::fExactSize), _bioseq(0), _ionAnnot(0) , m_progressLast(0), m_progressUpdate(0)
        {
            init ();
        }

        ~DnaDifferentialProfiler()
        {
            delete _bioseq;
            delete _ionAnnot;
        }

        virtual idx OnExecute(idx);

        void init(idx req = 0)
        {
            _req = req;
            _profs.cut(0);
            _profHandlers.cut(0);
            nameFiles00.cut(0);
            numproftypes = 0;
            outParam.cut(0);
            inpParam.cut(0);
            createDictionaryGeneTable = false;
            m_progressLast = 0;
            m_progressUpdate = 0;
            _numAnnotTypes = 0;
            if (_ionAnnot != 0){
               delete _ionAnnot;
           }
           _ionAnnot = 0;
            if (_bioseq){
                delete _bioseq;
            }
            _bioseq = 0;
            while(_prof_types[++numproftypes].posinVector >= 0);

        }

        bool restartProfilers ()
        {
            for (idx i = 0; i < _profHandlers.dim(); ++i){
                ProfHandler *phand = _profHandlers.ptr(i);
                phand->currPos = phand->profCSV->ptr(0);
            }
            return true;
        }

        bool restartStats ()
        {
            for (idx i = 0; i < geneStat.dim(); ++i){
                geneStat[i].num = 0;
                geneStat[i].sum = 0;
                geneStat[i].max = 0;
            }
            return true;
        }

        idx numProfilers(){
            return _profs.dim();
        }

        idx addHandler(const sUsrObj & profiler_obj, ProfilerTypes profType)
        {
            localBuf.cut(0);
            const char *filepathname = profiler_obj.getFilePathname(localBuf, "%s", profType.namefile);
            if( filepathname && sFile::exists(filepathname) ) {
                ProfHandler *phand = _profHandlers.add();
                phand->poolprofCSV = _pool.declare(filepathname, sMex::fReadonly);
                phand->fileNamePos = nameFiles00.length();
                nameFiles00.add(filepathname, sLen(filepathname));
                nameFiles00.add0();
                phand->profType = profType;
                phand->profCSV = _pool.request(phand->poolprofCSV);
                phand->currPos = phand->profCSV->ptr(0);
                phand->profCSVlength = phand->profCSV->length();
                return phand - _profHandlers.ptr(0);
            }
            return -1;
        }

        idx addNewProfObj (const sUsrObj & profiler_obj, idx objID, idx listOfFiles)
        {
            ProfileInfo *prof = _profs.add();
            prof->profobjID = objID;
            prof->profCursors.resize(numproftypes);
            idx num_of_files = 0;
            for (idx i = 0; i < numproftypes; ++i){

                if (listOfFiles & _prof_types[i].eType){
                    idx id = addHandler(profiler_obj, _prof_types[i]);
                    if (id >= 0){
                        prof->profCursors[_profHandlers.ptr(id)->profType.posinVector] = id;
                        ++num_of_files;
                    }
                }
                else {
                    prof->profCursors[i] = -1;
                }
            }

            return  num_of_files;
        }

        bool loadReferences(const sUsrObj & profiler_obj, sStr *error)
        {
            sVec<sHiveId> alignerIDs;
            sHiveId alignerID;
            sHiveId::parseRangeSet(alignerIDs, profiler_obj.propGet00("parent_proc_ids"));
            if (!alignerIDs.dim()){
                error->printf("No alignment object was found");
                return false;
            }
            alignerID = alignerIDs[0];
            sUsrFile aligner(alignerID, user);
            sStr subject;
            aligner.propGet00("subject", &subject, ";");
            sStr path;
            aligner.getFilePathname00(path, "alignment.hiveal" _ "alignment.vioal" __);
            sHiveal hiveal(user, path);
            if( !hiveal.isok() || hiveal.dimAl()==0){
                error->printf("No alignments are detected or the alignment file %s is missing or corrupted\n", alignerID.print());
                return false;
            }
            _bioseq = new sHiveseq (user, subject.ptr(), hiveal.getSubMode(), false, false, error);
            if(_bioseq->dim()==0) {
                error->printf("Reference '%s' sequences are missing or corrupted", subject.length() ? subject.ptr() : "unspecified");
                return false;
            }

            return true;

        }

        bool extractTypeandColumn (const char *src, idx inputLen, idx *profType, idx *sourceColumn)
        {
            if (!inputLen){
                inputLen = sLen (src);
            }
            idx j;
            for(j=0;j<inputLen && src[j];j++){
                if(src[j] == '?'){
                    break;
                }
            }



            return true;
        }

        bool loadParameters(sVec <Jobs> *jobList, sStr *err);
        bool createGeneDictionary(sStr *err);
        idx loadProfileIDs(idx listofTypes, sStr *error)
        {
            sVec<sHiveId> profileIDs;
            if (formHiveIdValues("dip_profileID", profileIDs) <= 0) {
                logOut(eQPLogType_Error, "Empty profileID list for %" DEC " request; terminating\n", _req);
                reqSetInfo(_req, eQPInfoLevel_Error, "Need a non-empty list of profiling results");
                return false;
            }
            idx num_profiles_opened = 0;
            for (idx i=0; i< profileIDs.dim(); i++) {

                sUsrFile profileObject(profileIDs[i], user);
                if (!profileObject.Id()) {
                    error->printf("No data for profile ID %s", profileIDs[i].print());
                    continue;
                }
                idx open_files = addNewProfObj (profileObject, profileIDs[i].objId(), listofTypes);
                if (open_files == 0){
                    error->printf("Can not access files from objID %s", profileIDs[i].print());
                    return false;
                }
                if (i==0){
                    if (!loadReferences(profileObject, error)){
                        error->printf("Can not access reference Information for profile ID %s", profileIDs[i].print());
                        return false;
                    }
                }
                num_profiles_opened += open_files;
            }
            return num_profiles_opened;
        }

        bool reqInitFile(sMex & fil, const char * name, const char *ext, sStr * path_buf = 0)
        {
            static sStr local_path_buf;
            if( !path_buf ) {
                path_buf = &local_path_buf;
            }
            const char * path = reqAddFile(*path_buf, "%s%s", name, ext);
            if( path && fil.init(path) && fil.ok() ) {
#ifdef _DEBUG
                logOut(eQPLogType_Trace, "Created %s", path);
#endif
                return true;
            } else {
                logOut(eQPLogType_Error, "Failed to create %s%s", name, ext);
                return false;
            }
        }

        bool prepareOutHeaders (sFil *out, Jobs *currentJob, bool mutOption = false, sStr *error = 0);
        bool calculateDifferences (sFil *out1, sFil *out2, Jobs * cJob);

        const char *getNextSNP(ProfHandler *prof, RowInfo *Line, bool extractSNP, idx *coverage)
        {
            const char *buf = prof->currPos;
            const char *bufend = prof->profCSV->ptr() + prof->profCSVlength;

            if (*buf < '0' || *buf > '9'){
                buf = skipToNextLine(buf, bufend);
            }
            if( buf && buf < bufend ) {
                sBioseqSNP::SNPRecord tempLine;
                const char *nextLine = sBioseqSNP::SNPConcatenatedRecordNext(buf, &tempLine, bufend);
                Line->readfromSNP(&tempLine, extractSNP, coverage);
                return nextLine;
            }
            return 0;
        }

        static const char* skipToNextLine(const char *buf, const char *bufend)
        {
            while (buf < bufend && *buf && *buf != '\r' && *buf != '\n')
                buf++;
            while (buf < bufend && (*buf == '\r' || *buf == '\n'))
                buf++;
            return buf;
        }

        bool getNext(Jobs *cJob, idx iprof, RowInfo *Line, idx *progressProf = 0)
        {
            idx lastProfiler = -1;
            sStr aux;
            idx cn = 20;
            unsigned char *rows[cn];
            aux.resize(1024);
            idx colnum;
            idx progressProfilers = 0;

            idx numClosedFiles = 0;
            for (idx icol = 0; icol < cJob->inputNumParams; ++icol){
                InputParameters *input = inpParam.ptr(cJob->inputIndex + icol);

                ProfileInfo *profInfo = _profs.ptr(iprof);
                ProfHandler *prof = _profHandlers.ptr(profInfo->profCursors[input->profType.posinVector]);

                if (prof->profCSVlength > 0){
                    progressProfilers += ( (prof->currPos-prof->profCSV->ptr()) * 100 / prof->profCSVlength);
                }

                if (prof->currPos == 0){
                    numClosedFiles++;
                    continue;
                }
                if (input->profType.eType != lastProfiler){
                    if (prof->profType.eType == snpProfile){
                        idx coverage = 0;
                        prof->currPos = getNextSNP(prof, Line, cJob->SNPProfile, &coverage);
                        Line->colVal[icol] = coverage;
                        Line->coverage = validateRanges(coverage, input->minVal, input->maxVal);
                        if (input->comparator == 1 && Line->coverage < input->minVal){
                            Line->coverage = 0;
                            Line->colVal[icol] = coverage;
                        }
                        lastProfiler = input->profType.eType;
                    }
                    else {
                        const char *buf = prof->currPos;
                        const char *bufend = prof->profCSV->ptr() + prof->profCSVlength;
                        if (*buf < '0' || *buf > '9'){
                            buf = skipToNextLine(buf, bufend);
                        }
                        if( buf && buf < bufend ) {

                            prof->currPos = csvParseStr(buf, bufend, aux.ptr(), 1024, rows, cn, ',', 0, &colnum);
                            if (prof->profType.eType & (infoProfile)){
                                sscanf((const char *) rows[0], "%" DEC, &Line->iSub);
                                sscanf((const char *) rows[1], "%" DEC, &Line->position);
                            }
                        }
                        else {
                            prof->currPos = 0;
                            numClosedFiles++;
                            continue;
                        }
                    }
                }

                if (input->sourceColumn >= 0){
                    sscanf((const char *) rows[input->sourceColumn], "%" DEC, &Line->colVal[icol]);
                }

                lastProfiler = input->profType.eType;
                if (prof->currPos == 0){
                    ++numClosedFiles;
                }
            }
            if (progressProf){
                *progressProf = (progressProfilers / cJob->inputNumParams);
            }

            return (numClosedFiles == cJob->inputNumParams) ? true : false;
        }

        bool updateMinRec(RowInfo *r1, RowInfo *r2)
        {
            if ((r2->iSub < r1->iSub) || ((r2->iSub == r1->iSub) && (r2->position < r1->position))){
                r1->iSub = r2->iSub;
                r1->position = r2->position;
                return true;
            }
            return false;
        }

        bool compareThresholdValue (idx val1, idx val2, idx threshold)
        {
            if( (val1 < threshold) && (val2 >= threshold) ) {
                return true;
            }
            if( (val2 < threshold) && (val1 >= threshold) ) {
                return true;
            }
            return false;
        }

        bool compareAbsoluteDifference (idx val1, idx val2, idx diff)
        {
            if ( ((val1 + diff) >= val2) && ((val1 - diff) <= val2)){
                return false;
            }
            return true;
        }

        idx validateRanges (idx val, idx min, idx max){
            if ((min >= 0) && (val < min)){
                return min;
            }
            if ((max >= 0) && (val > max)){
                return max;
            }
            return val;
        }

        bool compareRec(RowInfo *r1, RowInfo *r2, Jobs *cJob, idx inpNum)
        {
            bool different = false;
            for (idx irow = 0; (irow < cJob->inputNumParams) && !different; ++irow){
                InputParameters *input = inpParam.ptr(cJob->inputIndex + irow);
                if (input->comparator < 0){
                    continue;
                }
                idx val1 = validateRanges(r1->colVal[input->colRow], input->minVal, input->maxVal);
                idx val2 = validateRanges(r2->colVal[input->colRow], input->minVal, input->maxVal);

                switch (input->comparator) {
                    case 1:
                        different = compareThresholdValue (val1, val2, input->value);
                        break;
                    case 2:
                        different = compareAbsoluteDifference (val1, val2, input->value);
                        break;
                    case 3:
                        different = compareAbsoluteDifference (val1, val2, val1 * input->value);
                        break;
                    case 4:
                        different = compareAbsoluteDifference (val1, val2, (val1 * input->value) / 100);
                        break;
                    case 5:
                        break;
                    default:
                        break;
                }
            }
            return different;
        }

        bool emptyValues (RowInfo *r1, Jobs *cJob, idx inpNum)
        {
            for (idx irow = 0; (irow < cJob->inputNumParams); ++irow){
                InputParameters *input = inpParam.ptr(cJob->inputIndex + irow);
                if (input->comparator < 0){
                    continue;
                }
                if (input->comparator == 5){
                    continue;
                }
                if (r1->colVal[input->colRow] > input->minVal){
                    return false;
                }
            }
            return true;
        }

        bool findMutation(RowInfo *rec, char *consensus = 0, idx *let = 0, real _thresholdMutationCall = 0.2)
        {
            real max = -1.0;
            idx mut = 0;
            for (idx i = 0; i < 6; ++i){
                real freq = rec->freq[i];
                if (freq >= _thresholdMutationCall){
                    if (max < freq){
                        max = freq;
                        mut = i;
                    }
                }
            }
            if (consensus){
                if (mut < 4){
                    *consensus = sBioseq::mapRevATGC[mut];
                }
                else if (mut == 4){
                    *consensus = '+';
                }
                else {
                    *consensus = '-';
                }
            }
            if (let){
                *let = mut;
            }

            return (max == -1.0) ? false : true;

        }

        idx parseGeneInformation (const char *str00)
        {
            unsigned char *rows[5];
            idx num_genes = 0;
            sStr aux;
            aux.resize(1024);
            idx colnum;
            geneInfo.cut(0);

            for (const char *p = str00; p; p = sString::next00(p)){
                csvParseStr(p, p + sLen(p), aux.ptr(), 1024, rows, 5, ',', 0, &colnum);
                AnnotInfo *annot = geneInfo.add();
                sscanf((const char *) rows[0], "%" DEC, &annot->gene_start);
                sscanf((const char *) rows[1], "%" DEC, &annot->gene_end);
                idx * index = geneDictionary.get(rows[2], sLen(rows[2]));
                annot->gene_id = *index;
                num_genes++;
            }
            return num_genes;
        }

        bool addGeneStats (idx profile_id, AnnotInfo *gene, idx start_range, idx end_range, idx value)
        {

            GeneStats *igene = geneStat.ptr((profile_id * numProfilers()) + gene->gene_id);

            idx start_r = gene->gene_start < start_range ? start_range : gene->gene_start;
            idx end_r = gene->gene_end > end_range ? end_range : gene->gene_end;

            idx range = end_r - start_r + 1;
            idx valToAdd = range * value;
            igene->sum += valToAdd;
            if( value > igene->max ) {
                igene->max = value;
            }
            igene->num += range;
            return true;
        }

        bool printGeneTable (sFil *out, sStr *err)
        {
            out->printf("gene_name,");

            idx numProf = numProfilers();
            for (idx i = 0; i < numProf; ++i){
                out->printf(",prof_%" DEC "_avgCov", _profs.ptr(i)->profobjID);
                out->printf(",prof_%" DEC "_maxCov", _profs.ptr(i)->profobjID);
            }
            out->add("\n",1);


            idx idlen;
            for (idx ig = 0; ig < geneDictionary.dim(); ++ig){
                const char * id = (const char *) (geneDictionary.id(ig, &idlen));
                out->add(id, idlen);
                for (idx iprof = 0; iprof < numProf; ++iprof){
                    GeneStats *igene = geneStat.ptr ((iprof * numProfilers()) + ig);
                    out->add(",", 1);
                    if (igene->num == 0){
                        out->add("0", 1);
                    }
                    else {
                        out->addNum(igene->sum / igene->num);
                    }
                    out->add(",", 1);
                    out->addNum(igene->max);
                }
                out->add("\n", 1);
            }
            out->add0cut();
            return true;
        }

        void printMutation(sFil &out, RowInfo *currPos, RowInfo *currStatus, idx minCvg, real thresholdSNP, bool useAnnotInfo = false)
        {
            const char *id = 0;
            if (_bioseq->dim() > currPos->iSub-1){
                id = _bioseq->id(currPos->iSub-1);
                sString::escapeForCSV(out, id);
            }
            else{
                out.addNum(currPos->iSub);
            }
            out.add(":", 1);
            out.addNum(currPos->position);
            for (idx i = 0; i < numProfilers(); ++i){
                out.add(",", 1);
                if ((currPos->position == currStatus[i].position) && (minCvg < currStatus[i].coverage )){
                    char mut;
                    idx let;
                    RowInfo *rec = &currStatus[i];
                    if (findMutation(rec, &mut, &let, thresholdSNP)){
                        out.printf("%c->%c:%.2lf:%" DEC,rec->letter,mut, rec->freq[let],rec->colVal[0]);
                    }
                    else {
                        out.add("-::", 3);
                        out.addNum(rec->colVal[0]);
                    }
                }
                else {
                    out.add("0", 1);
                }
            }
            if (useAnnotInfo){
                idx pos = currPos->position;
                if (id){
                    sStr aux;
                    idx auxlen =sString::copyUntil(&aux, id, 0, " ");
                    const char * annotInfo =_ionAnnot->getRecordInfo(aux.ptr(),auxlen,pos,pos,true);
                    if (!annotInfo){
                        localBuf.cut(0);
                        bool success = sString::extractNCBIInfoSeqID(0, &localBuf, aux.ptr(), auxlen);
                        if (success){
                            annotInfo =_ionAnnot->getRecordInfo(localBuf.ptr(),localBuf.length(),pos,pos,true);
                        }
                    }
                    out.addString(",",1);
                    out.addString(annotInfo);
                }

            }
            out.addString("\n", 1);
        }

        void printOutput (sFil &out, RowInfo * startPos, RowInfo *currPos, RowInfo *prevStatus, bool *moveFwd, Jobs *cJob, bool isEmpty = false)
        {
            idx differential = 0;
            idx num_genes = 0;
            const char *id = 0;
            if (_bioseq->dim() > startPos->iSub-1){
                id = _bioseq->id(startPos->iSub-1);
                sString::escapeForCSV(out, id);
            }
            else{
                out.addNum(startPos->iSub);
            }
            out.add(",", 1);
            out.addNum(startPos->position);
            out.add(",", 1);
            out.addNum(currPos->position - 1);
            idx numProf = numProfilers();
            idx avg_coverage[numProf];
            for (idx i = 0; i < numProf; ++i){
                for (idx iout = 0; iout < cJob->outputNumParams; ++iout){
                    OutputParameters *outrow = outParam.ptr(cJob->outputIndex+iout);
                    if (outrow->outputFormat == -1){
                        continue;
                    }
                    out.add(",", 1);
                    if (moveFwd[i] && !isEmpty){
                        idx num;
                        if (outrow->outputFormat == -2){
                            num = prevStatus[i].getAvgCov();
                            avg_coverage[i] = num;
                            if (num <= cJob->minCvgAllowed){
                                out.add("0,0", 3);
                            }
                            else {
                                out.addNum(num);
                                out.add(",", 1);
                                out.addNum(prevStatus[i].maxCvg);
                                differential += 1;
                            }
                        }
                        else{
                            num = prevStatus[i].colVal[outrow->colRow];
                            out.addNum(num);
                        }
                    }
                    else {
                        if (outrow->outputFormat == -2){
                            avg_coverage[i] = 0;
                            out.add("0,0", 3);
                        }
                        else {
                            out.add("0", 1);
                        }
                    }
                }
            }
            if (cJob->outputDifferentiator){
                out.add(",",1);
                out.addNum(differential%numProf);
            }
            if( cJob->outputGeneTable && (differential%numProf != 0) ) {
                idx startpos = startPos->position;
                idx endpos = (currPos->position < startpos) ? startpos : currPos->position;

                if( id ) {
                    sStr aux;
                    idx auxlen = sString::copyUntil(&aux, id, 0, " ");
                    const char * geneInfo = _ionAnnot->getGeneInfo(aux.ptr(), auxlen, startpos, endpos);
                    if( geneInfo ) {

                        num_genes = parseGeneInformation(geneInfo);
                    }
                }

                for (idx i = 0; i < numProf; ++i){
                    for(idx inumGene = 0; inumGene < num_genes; ++inumGene) {
                        addGeneStats(i, geneInfo.ptr(inumGene), startpos, endpos, avg_coverage[i]);
                    }
                }
            }
            if (cJob->useAnnotInformation){
                idx startpos = startPos->position;
                idx endpos = (currPos->position < startpos) ? startpos : currPos->position;
                if (id){
                    sStr aux;
                    idx auxlen =sString::copyUntil(&aux, id, 0, " ");
                    const char * annotInfo =_ionAnnot->getRecordInfo(aux.ptr(),auxlen,startpos,endpos,true);
                    if (!annotInfo){
                        localBuf.cut(0);
                        bool success = sString::extractNCBIInfoSeqID(0, &localBuf, aux.ptr(), auxlen);
                        if (success){
                            annotInfo =_ionAnnot->getRecordInfo(localBuf.ptr(),localBuf.length(),startpos,endpos,true);
                        }
                    }
                    out.addString(",",1);
                    out.addString(annotInfo);
                }

            }
            out.addString("\n", 1);
        }

        idx countSetBits(idx n)
        {
            idx c;
            for(c = 0; n; n >>= 1) {
                c += n & 1;
            }
          return c;
        }

        idx progressReport(idx progress, idx percent)
        {
            idx rc = 1;
            time_t t0 = time(0);
            if( (percent > m_progressUpdate) || (t0 - m_progressLast) > 60 ) {
                m_progressUpdate = percent;
                m_progressLast = t0;
                rc = reqProgress(progress, percent, 100);
            }
            return rc;
        }

};

bool DnaDifferentialProfiler::loadParameters(sVec <Jobs> *jobList, sStr *err)
{
    const sUsrObjPropsTree * tree = objs[0].propsTree();
    if (!tree){
        err->printf("Variable data (Props Tree) is not accessible");
        return false;
    }

    idx numJobs = 0;
    idx listOfTypesToExtract = 0;
    for(const sUsrObjPropsNode * nodeEntry = tree->find("dip_entryArray"); nodeEntry; nodeEntry = nodeEntry->nextSibling("dip_entryArray"), numJobs++) {
        for(const sUsrObjPropsNode * nodeInputRow = nodeEntry->find("dip_inputsArray"); nodeInputRow; nodeInputRow = nodeInputRow->nextSibling("dip_inputsArray")) {
            idx option = nodeInputRow->findIValue("dip_diffOption");
            switch (option){
                case 1:
                case 2:
                    listOfTypesToExtract |= snpProfile;
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                    listOfTypesToExtract |= infoProfile;
                    break;
            }
        }
        for(const sUsrObjPropsNode * nodeOutputRow = nodeEntry->find("dip_outListColumns"); nodeOutputRow; nodeOutputRow = nodeOutputRow->nextSibling("dip_outListColumns")) {
            idx option = nodeOutputRow->findIValue("dip_outputColumn");
            switch (option){
                case 1:
                case 2:
                    listOfTypesToExtract |= snpProfile;
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                    listOfTypesToExtract |= infoProfile;
                    break;
            }
        }
    }
    bool validAnnotFile = false;
    if (formValues00("dip_annotationObjID", &annotObjIds00, ",")){
        validAnnotFile = true;
        sHiveIon hionAnnot(user, annotObjIds00.ptr(), 0, "ion" __);
        if (hionAnnot.ionCnt == 0){
            err->printf("can't open annotation files\n");
            validAnnotFile = false;
        }

        _ionAnnot = new sIonAnnotBase (hionAnnot.pathList00.ptr());
    }
    else {
        _ionAnnot = 0;
        annotObjIds00.cut(0);
    }
    loadProfileIDs(listOfTypesToExtract, err);
    if (err->length()){
        return false;
    }
    jobList->resize(numJobs);
    numJobs = 0;
    for(const sUsrObjPropsNode * nodeEntry = tree->find("dip_entryArray"); nodeEntry; nodeEntry = nodeEntry->nextSibling("dip_entryArray")) {
        Jobs *cJob = jobList->ptr(numJobs);
        cJob->SNPProfile = false;
        cJob->inputNumParams = 0;
        cJob->outputNumParams = 0;
        cJob->thresholdSNPpercentage = 0.2;
        cJob->minCvgAllowed = 0;

        idx numInputs = 0;
        idx numOutputs = 0;
        listOfTypesToExtract = 0;
        const sUsrObjPropsNode * nodeFirstInputRow = nodeEntry->find("dip_inputsArray");
        const sUsrObjPropsNode * nodeFirstOutputRow = nodeEntry->find("dip_outListColumns");

        cJob->inputIndex = inpParam.dim();
        cJob->outputIndex = outParam.dim();
        for (idx itype = 0; itype < numproftypes; ++itype){
            for(const sUsrObjPropsNode * nodeInputRow = nodeFirstInputRow; nodeInputRow; nodeInputRow = nodeInputRow->nextSibling("dip_inputsArray")) {
                idx inOption = nodeInputRow->findIValue("dip_diffOption");
                idx sourceColumn;
                idx profType;
                bool getSNPthreshold = false;
                bool storeMinMaxValsforCoverage = false;
                switch (inOption){
                    case 1:
                        sourceColumn = -1;
                        profType = snpProfile;
                        listOfTypesToExtract |= snpProfile;
                        storeMinMaxValsforCoverage = true;
                        break;
                    case 2:
                        sourceColumn = -1;
                        profType = snpProfile;
                        listOfTypesToExtract |= snpProfile;
                        getSNPthreshold = true;
                        break;
                    case 3:
                        sourceColumn = 6;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        break;
                    case 4:
                        sourceColumn = 7;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        break;
                    case 5:
                        sourceColumn = 4;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        break;
                    case 6:
                        sourceColumn = 5;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        break;
                    default:
                        err->printf("input column not valid");
                        return false;
                }
                if (profType == _prof_types[itype].eType){
                    InputParameters *newInput = inpParam.add(1);
                    {
                        const sUsrObjPropsNode * nodeMinMax = nodeInputRow->find("dip_minmax");
                        if (nodeMinMax){
                            newInput->minVal = nodeMinMax->findIValue("dip_diffmin");
                            if (!newInput->minVal){
                                newInput->minVal = -1;
                            }
                            newInput->maxVal = nodeMinMax->findIValue("dip_diffmax");
                            if (!newInput->maxVal){
                                newInput->maxVal = -1;
                            }
                        }
                        else {
                            newInput->minVal = -1;
                            newInput->maxVal = -1;
                        }
                    }
                    if (storeMinMaxValsforCoverage){
                        cJob->minCvgAllowed = newInput->minVal;
                        cJob->maxCvgAllowed = newInput->maxVal;
                    }
                    newInput->sourceColumn = sourceColumn;
                    newInput->profType = _prof_types[itype];
                    newInput->comparator = nodeInputRow->findIValue("dip_diffComparator");
                    newInput->value = nodeInputRow->findIValue("dip_diffValue");
                    if (getSNPthreshold){
                        cJob->thresholdSNPpercentage = (real)validateRanges (nodeInputRow->findRValue("dip_diffValue")*100, 0, 100) / 100;

                    }
                    newInput->colRow = numInputs;
                    ++numInputs;
                }
            }
            for(const sUsrObjPropsNode * nodeOutputRow = nodeFirstOutputRow; nodeOutputRow; nodeOutputRow = nodeOutputRow->nextSibling("dip_outListColumns")) {
                idx outoption = nodeOutputRow->findIValue("dip_outputColumn");
                idx sourceColumn = -1;
                idx outformat = 0;
                idx profType;
                idx nameColPosition = nameFiles00.length();
                switch (outoption){
                    case 1:
                        sourceColumn = -1;
                        outformat = -2;
                        profType = snpProfile;
                        listOfTypesToExtract |= snpProfile;
                        nameFiles00.addString("coverage");
                        break;
                    case 2:
                        cJob->SNPProfile = true;
                        sourceColumn = -1;
                        profType = snpProfile;
                        outformat = -1;
                        listOfTypesToExtract |= snpProfile;
                        nameFiles00.addString("mut");
                        break;
                    case 3:
                        sourceColumn = 6;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        nameFiles00.addString("leftTail");
                        break;
                    case 4:
                        sourceColumn = 7;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        nameFiles00.addString("rightTail");
                        break;
                    case 5:
                        sourceColumn = 4;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        nameFiles00.addString("seqLength");
                        break;
                    case 6:
                        sourceColumn = 5;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        nameFiles00.addString("alLength");
                        break;
                    default:
                        err->printf("output column not valid");
                        return false;
                }
                if (profType == _prof_types[itype].eType){
                    OutputParameters *newOutput = outParam.add(1);
                    newOutput->outProfType = _prof_types[itype];
                    newOutput->outputFormat = outformat;
                    newOutput->colNamePosition = nameColPosition;
                    nameFiles00.add0();

                    bool columnExists = false;
                    for (idx i = 0; i < numInputs && !columnExists; ++i){
                        InputParameters *input = inpParam.ptr(cJob->inputIndex+i);
                        if ( (input->profType.eType == newOutput->outProfType.eType) && (input->sourceColumn == sourceColumn)){
                            newOutput->colRow = input->colRow;
                            columnExists = true;
                        }
                    }
                    if (!columnExists){
                        InputParameters *newInput = inpParam.add(1);
                        newInput->minVal = -1;
                        newInput->maxVal = -1;
                        newInput->sourceColumn = sourceColumn;
                        newInput->profType = _prof_types[itype];
                        newInput->comparator = -1;
                        newInput->value = 0;
                        newInput->colRow = numInputs;
                        newOutput->colRow = numInputs;
                        ++numInputs;
                    }
                    ++numOutputs;
                }
                else {
                    nameFiles00.cut(nameColPosition);
                }
            }
        }
        cJob->numUsedFiles = countSetBits (listOfTypesToExtract);
        cJob->inputNumParams = numInputs;
        cJob->outputNumParams = numOutputs;
        cJob->outfilenamePosition = nameFiles00.length();
        nameFiles00.printf("job%" DEC, numJobs+1);
        nameFiles00.add0();
        cJob->outputSNPType = nodeEntry->findBoolValue("dip_outputSNP");
        cJob->outputRangeFilter = nodeEntry->findIValue("dip_outputRangeFilter");
        cJob->outputDifferentiator = nodeEntry->findBoolValueOrDefault("dip_outputDifferential");
        cJob->outputGeneTable = nodeEntry->findBoolValueOrDefault("dip_outputGeneTable");
        if (cJob->outputGeneTable){
            const char *geneType = nodeEntry->findValue("dip_annotationGeneType", 0);
            if (geneType) {
                _ionAnnot->setGeneType(geneType, 0);
                createDictionaryGeneTable = true;
            }
        }
        cJob->useAnnotInformation = nodeEntry->find("dip_outputAnnot");
        if (validAnnotFile && cJob->useAnnotInformation){

            sStr typeList00;
            if (!formValues00("dip_annotationType", &typeList00, 0)){
                _numAnnotTypes = _ionAnnot->getNumType(&typeList00, 0);
                if (!_numAnnotTypes){
                    err->printf("We can't extract any annotation types from the files");
                    return false;
                }
            }
            const char *error = _ionAnnot->prepareQuerywithTypes(typeList00.ptr(), 1);
            if (error){
                err->printf("%s",error);
                return false;
            }

        }
        else {
            cJob->useAnnotInformation = false;
        }
        ++numJobs;
    }


    return true;
}

bool DnaDifferentialProfiler::createGeneDictionary(sStr *err)
{
    if (!_ionAnnot){
        err->printf("There is not annotation object to extract gene information");
        return false;
    }

    sStr genes00;
    _ionAnnot->getAllGenes(&genes00, 0);

    if (!genes00.length()){
        err->printf("Extracting gene information failed");
        return false;
    }

    if (!geneDictionary.parse00(genes00.ptr(), genes00.length())){
        err->printf("Gene dictionary is empty");
        return false;
    }

    geneStat.resize(geneDictionary.dim() * numProfilers());
    return true;
}

#define ST_START     1
#define ST_COLLECT   2
#define ST_TAILSPACE 3
#define ST_END_QUOTE 4

const char * DnaDifferentialProfiler::csvParseStr(const char *src, const char *srcend, char *buf, size_t bn, unsigned char *row[], idx rn, idx sep, idx flags, idx *colnum, sStr *err)
{
    idx trim, quotes, ch, state, r, j, t, inquotes;

    trim = 0;
    quotes = flags;
    state = ST_START;
    inquotes = 0;
    ch = r = j = t = 0;

    while ( src<srcend && *src && *src != '\r' && *src != '\n'){
        ch = *src;

        ++src;
        switch(state) {
            case ST_START:
                if( ch != '\n' && ch != sep && isspace(ch) ) {
                    if( !trim ) {
                        buf[j++] = ch;
                        bn--;
                        t = j;
                    }
                    break;
                } else if( quotes && ch == '"' ) {
                    j = t = 0;
                    state = ST_COLLECT;
                    inquotes = 1;
                    break;
                }
                state = ST_COLLECT;
            case ST_COLLECT:
                if( inquotes ) {
                    if( ch == '"' ) {
                        state = ST_END_QUOTE;
                        break;
                    }
                } else if( ch == sep || ch == '\n' ) {
                    row[r++] = (unsigned char *) buf;
                    rn--;
                    if( ch == '\n' && t && buf[t - 1] == '\r' ) {
                        t--;
                        bn++;
                    }
                    buf[t] = '\0';
                    bn--;
                    buf += t + 1;
                    j = t = 0;
                    state = ST_START;
                    inquotes = 0;
                    if( ch == '\n' ) {
                        rn = 0;
                    }
                    break;
                } else if( quotes && ch == '"' ) {
                    return 0;
                }
                buf[j++] = ch;
                bn--;
                if( !trim || isspace(ch) == 0 ) {
                    t = j;
                }
                break;
            case ST_TAILSPACE:
            case ST_END_QUOTE:
                if( ch == sep || ch == '\n' ) {
                    row[r++] = (unsigned char *) buf;
                    rn--;
                    buf[j] = '\0';
                    bn--;
                    buf += j + 1;
                    j = t = 0;
                    state = ST_START;
                    inquotes = 0;
                    if( ch == '\n' ) {
                        rn = 0;
                    }
                    break;
                } else if( quotes && ch == '"' && state != ST_TAILSPACE ) {
                    buf[j++] = '"';
                    bn--;
                    t = j;
                    state = ST_COLLECT;
                    break;
                } else if( isspace(ch) ) {
                    state = ST_TAILSPACE;
                    break;
                }
                if (err){
                    err->printf("bad end quote in element %" DEC, (r + 1));
                }
                return 0;
        }
    }
    if( ch == -1 ) {
        return 0;
    }
    if( bn == 0 ) {
        if (err){
            err->printf("not enough space in buffer to store data");
        }
        return 0;
    }
    if( rn ) {
        if( inquotes && state != ST_END_QUOTE ) {
            if (err){
                err->printf("bad end quote in element");
            }
            return 0;
        }
        row[r] = (unsigned char *) buf;
        buf[t] = '\0';
    }

    if (colnum){
        *colnum = r+1;
    }
    return src;
}

DnaDifferentialProfiler::ProfilerTypes DnaDifferentialProfiler::_prof_types[] =
{
    {
        aaProfile,
        "AAprofile.csv",
        0
    },
    {
        freqProfile,
        "FreqProfile.csv",
        1
    },
    {
        histProfile,
        "HistProfile.csv",
        2
    },
    {
        noiseProfile,
        "Noise.csv",
        3
    },
    {
        infoProfile,
        "ProfileInfo.csv",
        4
    },
    {
        snpProfile,
        "SNPprofile.csv",
        5
    }
};

bool DnaDifferentialProfiler::prepareOutHeaders (sFil *out, Jobs *currentJob, bool mutOption, sStr *error)
{
    idx numProf = numProfilers();
    if (mutOption){
        out->addString("id:pos");
        for (idx i = 0; i < numProf; ++i){
            out->printf(",prof_%" DEC "_mut", _profs.ptr(i)->profobjID);
        }
    }
    else {
        out->addString("id,start,end");
        for (idx i = 0; i < numProf; ++i){
            for (idx icol = 0; icol < currentJob->outputNumParams; ++icol){
                OutputParameters *outrow = outParam.ptr(currentJob->outputIndex + icol);
                if (outrow->outputFormat == -1){
                    continue;
                }
                out->add(",", 1);
                const char *colName = nameFiles00.ptr(outrow->colNamePosition);
                if (outrow->outputFormat == -2){
                    out->printf("prof_%" DEC "_avg_%s", _profs.ptr(i)->profobjID, colName);
                    out->printf(",prof_%" DEC "_max_%s", _profs.ptr(i)->profobjID, colName);
                }
                else {
                    out->printf("prof_%" DEC "_%s", _profs.ptr(i)->profobjID, colName);
                }
            }
        }
    }
    if (currentJob->outputDifferentiator){
        out->printf(",differential Coverage");
    }
    if (currentJob->useAnnotInformation){
        out->printf(",Annotation");
    }
    out->addString("\n",1);
    return true;
}

bool DnaDifferentialProfiler::calculateDifferences (sFil *out1, sFil *out2, Jobs *cJob)
{
    sVec <RowInfo > prevStatus;
    sVec <RowInfo > currStatus;
    sVec <bool> moveFwd;
    idx numProf = numProfilers();
    prevStatus.resize(numProf);
    currStatus.resize(numProf);
    moveFwd.resize(numProf);
    idx foundMutation = 0;
    idx cntFinished = 0;

    RowInfo startPos, currPos;

    currPos.iSub = sIdxMax;
    currPos.position = sIdxMax;
    for (idx i = 0; i < numProf; ++i){
        cntFinished += getNext(cJob, i, &currStatus[i]);
        prevStatus[i] = currStatus[i];
        updateMinRec (&currPos, &currStatus[i]);
    }

    startPos = currPos;

    idx counter_report = 0;
    idx progressPercentage = 0;

    while (cntFinished < numProf){

        if( !progressReport(counter_report, progressPercentage) ) {
            return false;
        }
        counter_report += 1;
        foundMutation = 0;
        for (idx i = 0; i < numProf; ++i){
            if ((currPos.iSub == currStatus[i].iSub) && (currPos.position == currStatus[i].position)){
                moveFwd[i] = true;
                prevStatus[i].addCoverageStats(currStatus[i].coverage);
            }
            else {
                moveFwd[i] = false;
            }
            if (moveFwd[i] && (cJob->SNPProfile >= 0) && !emptyValues (&currStatus[i], cJob, i) && findMutation(&currStatus[i], 0, 0, cJob->thresholdSNPpercentage)){
                ++foundMutation;
            }
        }

        if (foundMutation){
            bool isValid = true;
            if (foundMutation == (numProf - cntFinished)){
                char ref, mut;
                ref = currStatus.ptr(0)->letter;
                findMutation(&currStatus[0], &mut, 0, cJob->thresholdSNPpercentage);

                idx isValidCount = 1;
                for (idx i = 1; i < numProf; ++i){
                    char mutalt;
                    findMutation(&currStatus[i], &mutalt, 0, cJob->thresholdSNPpercentage);
                    if ((ref == currStatus.ptr(i)->letter) && (mut == mutalt)){
                        ++isValidCount;
                    }
                }
                if (isValidCount == numProf && cJob->outputSNPType){
                    isValid = false;
                }
            }

            if (isValid){
                printMutation(*out2, &currPos, currStatus, cJob->minCvgAllowed, cJob->thresholdSNPpercentage,cJob->useAnnotInformation);
            }
        }
        progressPercentage = 0;
        for (idx i = 0; i< numProf; ++i){
            idx progressPerc = prevStatus[i].progressProfile;
            if (moveFwd[i]){
                progressPerc = 0;
                bool finished = getNext(cJob, i, &currStatus[i], &progressPerc);
                prevStatus[i].progressProfile = progressPerc;
                if (finished){
                    currStatus[i].progressProfile = 100;
                    cntFinished += 1;
                }
            }
            progressPercentage += progressPerc;
        }
        progressPercentage /= numProf;
        ++currPos.position;

        bool updateStatus = false;
        idx nextSubId = 0;
        for(idx i = 0; i < numProf && !updateStatus; ++i) {
            if( (currPos.iSub == currStatus[i].iSub) && (currPos.position == currStatus[i].position) && (!emptyValues (&currStatus[i], cJob, i)) ) {
                if( !moveFwd[i]) {
                    updateStatus = true;
                } else if( compareRec(&prevStatus[i], &currStatus[i], cJob, i) ) {
                    updateStatus = true;
                }
            } else {
                if( moveFwd[i] && !emptyValues (&prevStatus[i], cJob, i)) {
                    updateStatus = true;
                }
                else if ((currPos.iSub != currStatus[i].iSub)){
                    nextSubId += 1;
                }
            }
        }
        if (nextSubId == numProf){
            updateStatus = true;
        }
        if( updateStatus ) {
            if ( (currPos.iSub != startPos.iSub) || (currPos.position - startPos.position) >= cJob->outputRangeFilter){
                printOutput(*out1, &startPos, &currPos, prevStatus, moveFwd, cJob);
            }
            startPos = currPos;
            currPos.iSub = sIdxMax;
            currPos.position = sIdxMax;
            for(idx i = 0; i < numProf; ++i) {
                prevStatus[i] = currStatus[i];
                if (currStatus[i].progressProfile < 100){
                    updateMinRec(&currPos, &currStatus[i]);
                }
            }
            if( (startPos.iSub == currPos.iSub) && (startPos.position < currPos.position)) {
                if ( (currPos.position - startPos.position) >= cJob->outputRangeFilter){
                    printOutput(*out1, &startPos, &currPos, prevStatus, moveFwd, cJob, true);
                }
            }
            startPos = currPos;
        }
    }
    return true;
}

idx DnaDifferentialProfiler::OnExecute(idx req)
{
#ifdef _DEBUG
    for (idx i=0; i<pForm->dim(); i++) {
        const char * key = static_cast<const char*>(pForm->id(i));
        const char * value = pForm->value(key);
        printf("%s = %s\n", key, value);
    }
#endif

    sHiveId objID;
    objID = objs[0].Id();
    std::unique_ptr < sUsrObj > obj(user->objFactory(objID));

    if( !obj.get() || !obj->Id() ) {
        logOut(eQPLogType_Info, "Object %s not found or access denied", objID.print());
        reqSetInfo(req, eQPInfoLevel_Error, "Object %s not found or access denied", objID.print());
        return 1;
    } else {
        logOut(eQPLogType_Info, "processing object %s\n", objID.print());
    }

    init(req);
    reqProgress(-1,1,100);
    progress100Start = 1;
    progress100End = 10;


    sStr err;
    sVec <Jobs> jobList;
    if (!loadParameters(&jobList, &err)){
        reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return false;
    }

    if (createDictionaryGeneTable && !createGeneDictionary(&err)){
        reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return false;
    }

    sStr pathfilename;
    sStr pathfilenameMut;

    for (idx ijob = 0; ijob < jobList.dim(); ++ijob){
        Jobs *currentJob = jobList.ptr(ijob);
        restartProfilers();
        restartStats ();
        progress100Start = progress100End;
        progress100End = progress100Start + (80 / jobList.dim());
        reqProgress(-1, 10, 100);

        sFil outFile;
        sFil outMutFile;
        pathfilename.cut(0);
        if( !reqInitFile(outFile, nameFiles00.ptr(currentJob->outfilenamePosition), ".csv", &pathfilename) ) {
            reqSetStatus(req, eQPReqStatus_ProgError);
            return false;
        }
        if (currentJob->SNPProfile){
            if( !reqInitFile(outMutFile, nameFiles00.ptr(currentJob->outfilenamePosition), "_mut.csv", &pathfilename) ) {
                reqSetStatus(req, eQPReqStatus_ProgError);
                return false;
            }
            if (!prepareOutHeaders (&outMutFile, currentJob, true, &err)){
                reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
                reqSetStatus(req, eQPReqStatus_ProgError);
                return false;
            }
        }
        if (!prepareOutHeaders (&outFile, currentJob, false, &err)){
            reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
            reqSetStatus(req, eQPReqStatus_ProgError);
            return false;
        }
        if (!calculateDifferences (&outFile, currentJob->SNPProfile ? &outMutFile : 0, currentJob)){
            reqSetInfo(req, eQPInfoLevel_Error, "Killed by the user");
            reqSetStatus(req, eQPReqStatus_ProgError);
            return false;
        }
        if (currentJob->outputGeneTable){
            sFil outGeneFile;
            if( !reqInitFile(outGeneFile, nameFiles00.ptr(currentJob->outfilenamePosition), "_gene.csv", &pathfilename) ) {
                reqSetStatus(req, eQPReqStatus_ProgError);
                return false;
            }
            if (!printGeneTable (&outGeneFile, &err)){
                reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
                reqSetStatus(req, eQPReqStatus_ProgError);
                return false;
            }
        }
        reqProgress(-1, 100, 100);
    }

    progress100Start = progress100End;
    progress100End = 100;
    reqProgress(-1, 100, 100);

    reqSetStatus(req, eQPReqStatus_Done);
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eACGT);

    sStr tmp;
    sApp::args(argc,argv);

    DnaDifferentialProfiler backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-differential-profiler",argv[0]));
    return (int)backend.run(argc,argv);
}
