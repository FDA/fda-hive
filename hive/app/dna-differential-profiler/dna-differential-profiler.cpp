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
//            lastType = snpProfile
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
//             sSNPRecordIter *iterator;
             const char *currPos;
             idx profCSVlength;
        };

        struct ProfileInfo
        {
                idx profobjID;
                sVec<idx> profCursors;
        };

        struct RowInfo {
//                sStr rowbuf;
//                idx  tokens;
//                unsigned char *rows[];
                // pivots
                idx iSub;
                idx position;

                // Auxiliary elements
                char letter;
                real freq[6];

                // filtering columns
                idx filterCol[10];

                // output variables
                idx colVal[11];
                idx numCols;

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

                idx readCovfromSNP (sBioseqSNP::SNPRecord * src)
                {
                    colVal[0] = src->coverage();
                    return 0;
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

                idx readfromNoise(unsigned char *rows[], idx rn)
                {
                    sscanf((const char *)rows[0], "%" DEC, &iSub);
                    sscanf((const char *)rows[1], "%" DEC, &position);
                    sscanf((const char *)rows[6], "%" DEC, &colVal[0]);
                    sscanf((const char *)rows[7], "%" DEC, &colVal[1]);
                    numCols = 2;
                    return 0;
                }

        };

        struct InputParameters
        {
//                idx sourceFile;
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
        };

    private:
//        DnaDifferentialProfiler &_proc;
        idx _req;
        sFilPool _pool;
//        idx _heptagon_handle;
        sVec <ProfHandler> _profHandlers;
        sVec <ProfileInfo> _profs;
        sStr nameFiles00;

        sVec <InputParameters> inpParam;
        sVec <OutputParameters> outParam;

//        idx _thresholdCoverage;
//        real _thresholdMutationCall;
//        idx _optionMutationOutput;
//        idx _thresholdInfo;
        sHiveseq *_bioseq;

    protected:
        static ProfilerTypes _prof_types[];
        idx numproftypes;
//        idx numOutCols;


//        sStr localBuf;

    public:
        static const char * csvParseStr(const char *src, const char *srcend, char *buf, size_t bn, unsigned char *row[], idx rn, idx sep, idx flags, idx *colnum, sStr *err = 0);

        DnaDifferentialProfiler(const char * defline00,const char * srv) : sQPrideProc(defline00,srv), _pool(1000), _profHandlers(sMex::fExactSize), _profs(sMex::fExactSize)
        {
            init ();
        }

        ~DnaDifferentialProfiler()
        {
            delete _bioseq;
//            delete _pool;
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

        idx numProfilers(){
            return _profs.dim();
        }

        idx addHandler(const sUsrObj & profiler_obj, ProfilerTypes profType)
        {
            sStr localBuf;
            const char *filepathname = profiler_obj.getFilePathname(localBuf, "%s", profType.namefile);
            if( sFile::exists(filepathname) ) {
                ProfHandler *phand = _profHandlers.add();
                phand->poolprofCSV = _pool.declare(filepathname, sMex::fReadonly);
                phand->fileNamePos = nameFiles00.length();
                nameFiles00.add(filepathname, sLen(filepathname));
                nameFiles00.add0();
                // Find the type of file
//                if (!profType){
//                    sFilePath flnm (filepathname, "%%flnm");
//                    for(idx typeID = 0; typeID < numproftypes; ++typeID) {
//                        if (strcmp(_prof_types[typeID].namefile, flnm.ptr(0)) == 0){
//                            profType = _prof_types[typeID].eType;
//                            break;
//                        }
//                    }
//                }
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
            // Create a new profileInfo entry
            ProfileInfo *prof = _profs.add();
            prof->profobjID = objID;
            prof->profCursors.resize(numproftypes);
            // Add as many files associated to this profileEntry
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

//            for(const char *tc = list00; tc; tc = sString::next00(tc)) {
//                idx id = addHandler(profiler_obj, tc);
//                if (id >= 0){
//                    prof->profCursors[_profHandlers.ptr(id)->profType.posinVector] = id;
////                    prof->profCursors.vadd(1, id);
//                    ++num_of_files;
//                }
//            }
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
            _bioseq = new sHiveseq (user, subject.ptr(), hiveal.getSubMode(), false, false, error);//Sub.reindex();
            if(_bioseq->dim()==0) {
                error->printf("Reference '%s' sequences are missing or corrupted", subject.length() ? subject.ptr() : "unspecified");
                return false; // error
            }

            return true;

        }

        bool extractTypeandColumn (const char *src, idx inputLen, idx *profType, idx *sourceColumn)
        {
            if (!inputLen){
                inputLen = sLen (src);
            }
//            char *opt = input;
//            for (*)
            idx j;
            for(j=0;j<inputLen && src[j];j++){ // untill the end of the '?'
                if(src[j] == '?'){
                    break;
                }
            }

            // Compare the sourceFile and identify the


//                sString::searchAndReplaceSymbols()
            //                switch (inOption){
            //                    case 1:  // Coverage
            //                        sourceColumn = -1;
            //                        profType = snpProfile;
            //                        listOfTypesToExtract |= snpProfile;
            //                        break;
            //                    case 2:  // SNP's
            ////                        cJob->SNPProfile = true;
            //                        sourceColumn = -1;
            //                        profType = snpProfile;
            //                        listOfTypesToExtract |= snpProfile;
            //                        break;
            //                    case 3: // Left Tail
            //                        sourceColumn = 6;
            //                        profType  = infoProfile;
            //                        listOfTypesToExtract |= infoProfile;
            //                        break;
            //                    case 4: // Right Tail
            //                        sourceColumn = 7;
            //                        profType  = infoProfile;
            //                        listOfTypesToExtract |= infoProfile;
            //                        break;
            //                    case 5: // Length of Sequences
            //                        sourceColumn = 4;
            //                        profType  = infoProfile;
            //                        listOfTypesToExtract |= infoProfile;
            //                        break;
            //                    case 6: // Length of Alignments
            //                        sourceColumn = 5;
            //                        profType  = infoProfile;
            //                        listOfTypesToExtract |= infoProfile;
            //                        break;
            //                    default:
            //                        err->printf("input column not valid");
            //                        return false;
            //                }
            return true;
        }

        idx loadParameters(sVec <Jobs> *jobList, sStr *err);

        idx loadProfileIDs(idx listofTypes, sStr *error)
        {
            sVec<sHiveId> profileIDs;
            if (formHiveIdValues("dip_profileID", &profileIDs) < 0) {
                logOut(eQPLogType_Error, "Empty profileID list for %" DEC " request; terminating\n", _req);
                reqSetInfo(_req, eQPInfoLevel_Error, "Need a non-empty list of profiling results");
                return false;
            }
//            sVec<bool> incrementRefCounts(sMex::fExactSize);
//            sStr profileRefsBuf;
//            sStr profile_path;
            idx num_profiles_opened = 0;
            for (idx i=0; i< profileIDs.dim(); i++) {

                sUsrFile profileObject(profileIDs[i], user);
                if (!profileObject.Id()) {
                    error->printf("No data for profile ID %s", profileIDs[i].print());
                    continue;
                }
//                profile_path.cut(0);
//                if( !profileObject.getFilePathname(profile_path, "%s", _prof_types[snpProfile].namefile) ) {
//                    // this is a heptagon profile with one snp file for all references
//                    error.printf("No file for profile ID %s", profileIDs[i].print());
//                    continue;
//                }
//
//                if (true){
//                    idx initlength = profile_path.length();
//                    profile_path.add0();
//                    if( !profileObject.getFilePathname(profile_path, "%s", _prof_types[infoProfile].namefile) ) {
//                        profile_path.cut(initlength);
//                    }
//                }
                num_profiles_opened += addNewProfObj (profileObject, profileIDs[i].objId(), listofTypes);
                if (i==0){
                    if (!loadReferences(profileObject, error)){
                        error->printf("Can not access reference Information for profile ID %s", profileIDs[i].print());
                        return false;
                    }

                }
//                ++num_profiles_opened;
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

        idx prepareOutHeaders (sFil *out, Jobs *currentJob, bool mutOption = false);
        idx calculateDifferences (sFil *out1, sFil *out2, Jobs * cJob);

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

//        bool getNextInfo(ProfHandler *prof, RowInfo *Line)
//        {
//            if( prof ) {
//                const char *buf = prof->currPos;
//                const char *bufend = prof->profCSV->ptr() + prof->profCSVlength;
//
//                if (*buf < '0' || *buf > '9')
//                    buf = skipToNextLine(buf, bufend);
//
//                if( buf && buf < bufend ) {
//                    sStr aux;
//                    idx cn = 20;
//                    unsigned char *rows[cn];
//                    aux.resize(1024);
//                    idx colnum;
//                    prof->currPos = csv_parse_str (buf, bufend, aux.ptr(), 1024, rows, cn, ',', 0, &colnum);
//                    Line->readfromNoise(rows, colnum);
//                    return 0;
//                }
//            }
//            return 1;
//        }

        bool getNext(Jobs *cJob, idx iprof, RowInfo *Line)
        {
            idx lastProfiler = -1;
            sStr aux;
            idx cn = 20;
            unsigned char *rows[cn];
            aux.resize(1024);
            idx colnum;
//            bool snpExtract = cJob->SNPProfile ? false : true;

            idx numClosedFiles = 0;
            for (idx icol = 0; icol < cJob->inputNumParams; ++icol){
                InputParameters *input = inpParam.ptr(cJob->inputIndex + icol);

                ProfileInfo *profInfo = _profs.ptr(iprof);
                ProfHandler *prof = _profHandlers.ptr(profInfo->profCursors[input->profType.posinVector]);

                if (prof->currPos == 0){
                    numClosedFiles++;
                    continue;
                }
                if (input->profType.eType != lastProfiler){
                    // Read from the correct file
                    if (prof->profType.eType == snpProfile){
                        idx coverage = 0;
                        prof->currPos = getNextSNP(prof, Line, cJob->SNPProfile, &coverage);
                        Line->colVal[icol] = coverage;
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
                                // Read subject and position
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
//                // Validate colVal[icol] value
//                if ((input->minVal >= 0) && (Line->colVal[icol] < input->minVal)){
//                    Line->colVal[icol] = input->minVal;
//                }
//                if ((input->maxVal >= 0) && (Line->colVal[icol] > input->maxVal)){
//                    Line->colVal[icol] = input->maxVal;
//                }

                lastProfiler = input->profType.eType;
                if (prof->currPos == 0){
                    ++numClosedFiles;
                }
            }

            return (numClosedFiles == cJob->inputNumParams) ? true : false;
        }

//
//                for(idx icol = 0; icol < input->colRow; ++icol) {
//                    sscanf((const char *) rows[0], "%" DEC, &iSub);
//                }
//
//                Line->readCol(rows, colnum);
//                Line->readfromNoise(rows, colnum);
//                return 0;
//
//                const char *buf = prof->currPos;
//                const char *bufend = prof->profCSV->ptr() + prof->profCSVlength;
//
//                if (*buf < '0' || *buf > '9')
//                    buf = skipToNextLine(buf, bufend);
//
//                if( buf && buf < bufend ) {
//                    sStr aux;
//                    idx cn = 20;
//                    unsigned char *rows[cn];
//                    aux.resize(1024);
//                    idx colnum;
//                    prof->currPos = csv_parse_str (buf, bufend, aux.ptr(), 1024, rows, cn, ',', 0, &colnum);
//                    Line->readfromNoise(rows, colnum);
//                    return 0;
//                }
//
//                switch (prof->profType) {
//                        case aaProfile:
//                        case freqProfile:
//                        case histProfile:
//                        case noiseProfile:
//                            break;
//                        case infoProfile:
//                            return getNextInfo(prof, Line);
//                        case snpProfile:
//                            return getNextSNP(prof, Line);
//                }
//            }
//            return false;
//        }

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

//        template <class Tobj> static void sort(idx n, Tobj * arr)
        idx validateRanges (idx val, idx min, idx max){
            // Validate colVal[icol] value
            if ((min >= 0) && (val < min)){
                return min;
            }
            if ((max >= 0) && (val > max)){
                return max;
            }
            return val;
        }

        // Return:
        // true: if r1 and r2 are considered to be different
        // false: if they are similar due to the restrictions
        bool compareRec(RowInfo *r1, RowInfo *r2, Jobs *cJob, idx inpNum)
        {
//            InputParameters *input = inpParam.ptr(cJob->inputIndex + inpNum);
//            idx iprof = input->iProfiler;
            bool different = false;
            for (idx irow = 0; (irow < cJob->inputNumParams) && !different; ++irow){
                InputParameters *input = inpParam.ptr(cJob->inputIndex + irow);
                if (input->comparator < 0){
                    continue;
                }
                idx val1 = validateRanges(r1->colVal[input->colRow], input->minVal, input->maxVal);
                idx val2 = validateRanges(r2->colVal[input->colRow], input->minVal, input->maxVal);

                // must be synchronous to dip_diffComparator in UPTypeField
                switch (input->comparator) {
                    case 1:
                        // 1 - above or below Threshold
                        different = compareThresholdValue (val1, val2, input->value);
                        break;
                    case 2:
                        // 2 - Absolute Difference
                        different = compareAbsoluteDifference (val1, val2, input->value);
                        break;
                    case 3:
                        // 3 - 'X' Times Difference
                        different = compareAbsoluteDifference (val1, val2, val1 * input->value);
                        break;
                    case 4:
                        // 4 - Percentage Difference
                        different = compareAbsoluteDifference (val1, val2, (val1 * input->value) / 100);
                        break;
                    case 5:
                        // 5 - Mutation Difference
                        // Todo: implement mutation difference
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
            idx max = -1;
            idx mut = 0;
            for (idx i = 0; i < 6; ++i){
                real freq = rec->freq[i];
                if (freq > _thresholdMutationCall){
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

            return (max == -1) ? false : true;

        }

        void printMutation(sFil &out, RowInfo *currPos, RowInfo *currStatus)
        {
            if (_bioseq->dim() > currPos->iSub-1){
                const char *id = _bioseq->id(currPos->iSub-1);
                sString::escapeForCSV(out, id);
            }
            else{
                out.addNum(currPos->iSub);
            }
            out.add(":", 1);
            out.addNum(currPos->position);
            for (idx i = 0; i < numProfilers(); ++i){
                out.add(",", 1);
                if (currPos->position == currStatus[i].position){
                    char mut;
                    idx let;
                    RowInfo *rec = &currStatus[i];
                    if (findMutation(rec, &mut, &let)){
                        out.printf("%c->%c:%.2lf:%" DEC,rec->letter,mut, rec->freq[let],rec->colVal[0]);
                    }
                    else {
                        out.add("-::", 3);
                        out.addNum(rec->colVal[0]);
                    }
                }
                else {
                    // No Coverage
                    out.add("0", 1);
                }
            }
            out.addString("\n", 1);
        }

        void printOutput (sFil &out, RowInfo * startPos, RowInfo *currPos, RowInfo *prevStatus, bool *moveFwd, Jobs *cJob)
        {
            out.addNum(startPos->iSub);
            out.add(",", 1);
            out.addNum(startPos->position);
            out.add(",", 1);
            out.addNum(currPos->position - 1);
            for (idx i = 0; i < numProfilers(); ++i){
                for (idx iout = 0; iout < cJob->outputNumParams; ++iout){
                    OutputParameters *outrow = outParam.ptr(cJob->outputIndex+iout);
                    if (outrow->outputFormat == -1){
                        continue;
                    }
                    out.add(",", 1);
                    if (moveFwd[i]){
                        idx num = prevStatus[i].colVal[outrow->colRow];
                        out.addNum(num);
                    }
                    else {
                        out.add("0", 1);
                    }
                }
            }
            out.addString("\n", 1);
        }

        idx countSetBits(idx n)
        {
            idx c; // the total bits set in n
            for(c = 0; n; n >>= 1) {
                c += n & 1;
            }
          return c;
        }
};

idx DnaDifferentialProfiler::loadParameters(sVec <Jobs> *jobList, sStr *err)
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
                    listOfTypesToExtract |= infoProfile;
                    break;
            }
        }
    }
//    idx numFiles = loadProfileIDs(listOfTypesToExtract, err);
    loadProfileIDs(listOfTypesToExtract, err);
    jobList->resize(numJobs);
    numJobs = 0;
    for(const sUsrObjPropsNode * nodeEntry = tree->find("dip_entryArray"); nodeEntry; nodeEntry = nodeEntry->nextSibling("dip_entryArray")) {
        Jobs *cJob = jobList->ptr(numJobs);
        cJob->SNPProfile = false;
        cJob->inputNumParams = 0;
        cJob->outputNumParams = 0;
        idx numInputs = 0;
        idx numOutputs = 0;
        listOfTypesToExtract = 0;
        const sUsrObjPropsNode * nodeFirstInputRow = nodeEntry->find("dip_inputsArray");
        const sUsrObjPropsNode * nodeFirstOutputRow = nodeEntry->find("dip_outputsArray");

        cJob->inputIndex = inpParam.dim();
        cJob->outputIndex = outParam.dim();
        for (idx itype = 0; itype < numproftypes; ++itype){
            // It is important to store the inputs in order
            for(const sUsrObjPropsNode * nodeInputRow = nodeFirstInputRow; nodeInputRow; nodeInputRow = nodeInputRow->nextSibling("dip_inputsArray")) {
                idx inOption = nodeInputRow->findIValue("dip_diffOption");
                idx sourceColumn;
                idx profType;
//              const char * inOption = nodeInputRow->findValue("dip_diffOption");
//                extractTypeandColumn (inOption, 0, &profType, &sourceColumn);
                switch (inOption){
                    case 1:  // Coverage
                        sourceColumn = -1;
                        profType = snpProfile;
                        listOfTypesToExtract |= snpProfile;
                        break;
                    case 2:  // SNP's
//                        cJob->SNPProfile = true;
                        sourceColumn = -1;
                        profType = snpProfile;
                        listOfTypesToExtract |= snpProfile;
                        break;
                    case 3: // Left Tail
                        sourceColumn = 6;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        break;
                    case 4: // Right Tail
                        sourceColumn = 7;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        break;
                    case 5: // Length of Sequences
                        sourceColumn = 4;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        break;
                    case 6: // Length of Alignments
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
                            newInput->maxVal = nodeMinMax->findIValue("dip_diffmax");
                        }
                        else {
                            newInput->minVal = -1;
                            newInput->maxVal = -1;
                        }
                    }
                    newInput->sourceColumn = sourceColumn;
                    newInput->profType = _prof_types[itype];
                    newInput->comparator = nodeInputRow->findIValue("dip_diffComparator");
                    newInput->value = nodeInputRow->findIValue("dip_diffValue");
                    newInput->colRow = numInputs;
                    ++numInputs;
                }
            }
            for(const sUsrObjPropsNode * nodeOutputRow = nodeFirstOutputRow; nodeOutputRow; nodeOutputRow = nodeOutputRow->nextSibling("dip_outputsArray")) {
                idx outoption = nodeOutputRow->findIValue("dip_outputColumn");
                idx sourceColumn = -1;
                idx outformat = 0;
                idx profType;
                idx nameColPosition = nameFiles00.length();
                switch (outoption){
                    case 1:  // Coverage
                        sourceColumn = -1;
                        profType = snpProfile;
                        listOfTypesToExtract |= snpProfile;
                        nameFiles00.addString("coverage");
                        break;
                    case 2:  // SNP's
                        cJob->SNPProfile = true;
                        sourceColumn = -1;
                        profType = snpProfile;
                        outformat = -1;
                        listOfTypesToExtract |= snpProfile;
                        nameFiles00.addString("mut");
                        break;
                    case 3: // Left Tail
                        sourceColumn = 6;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        nameFiles00.addString("leftTail");
                        break;
                    case 4: // Right Tail
                        sourceColumn = 7;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        nameFiles00.addString("rightTail");
                        break;
                    case 5: // Length of Sequences
                        sourceColumn = 4;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        nameFiles00.addString("seqLength");
                        break;
                    case 6: // Length of Alignments
                        sourceColumn = 5;
                        profType  = infoProfile;
                        listOfTypesToExtract |= infoProfile;
                        nameFiles00.addString("alLength");
                        break;
                    default:
                        err->printf("output column not valid");
                        return false;
                }
                // Find the file and column in the input
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
                        // We need to add it
                        InputParameters *newInput = inpParam.add(1);
                        newInput->minVal = -1;
                        newInput->maxVal = -1;
                        newInput->sourceColumn = sourceColumn;
                        newInput->profType = _prof_types[itype];
                        newInput->comparator = -1; // important to be negative, as there is nothing to compare, it is just for output
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
        ++numJobs;
    }


    return true;
}

#define ST_START     1
#define ST_COLLECT   2
#define ST_TAILSPACE 3
#define ST_END_QUOTE 4

const char * DnaDifferentialProfiler::csvParseStr(const char *src, const char *srcend, char *buf, size_t bn, unsigned char *row[], idx rn, idx sep, idx flags, idx *colnum, sStr *err)
{
    idx trim, quotes, ch, state, r, j, t, inquotes;

    trim = 0; // flags;// & CSV_TRIM;
    quotes = flags;// & CSV_QUOTES;
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
                        bn++; /* crlf -> lf */
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
//                    PMNF(errno = EILSEQ, ": unexpected quote in element %d", (r + 1));
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
                    bn--; /* nope, just an escaped quote */
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

idx DnaDifferentialProfiler::prepareOutHeaders (sFil *out, Jobs *currentJob, bool mutOption)
{
    idx numProf = numProfilers();

    //    out1.printf("id,start,end");
    //    out2.printf("id:pos");
    //    for (idx i = 0; i < numProf; ++i){
    //        out1.printf(",profCov_%" DEC, _profs.ptr(i)->profobjID);
    //        out2.printf(",profMut_%" DEC, _profs.ptr(i)->profobjID);
    //    }
    //    out1.addString("\n", 1);
    //    out2.addString("\n", 1);

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
                out->printf("prof_%" DEC "_%s", _profs.ptr(i)->profobjID, colName);
            }
        }
    }
    out->addString("\n",1);
    return 0;
}

idx DnaDifferentialProfiler::calculateDifferences (sFil *out1, sFil *out2, Jobs *cJob)
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

    // Get first row
    RowInfo startPos, currPos;

    currPos.iSub = sIdxMax;
    currPos.position = sIdxMax;
    for (idx i = 0; i < numProf; ++i){
        cntFinished += getNext(cJob, i, &currStatus[i]);
        prevStatus[i] = currStatus[i];
        updateMinRec (&currPos, &currStatus[i]);
    }

    startPos = currPos;

    while (cntFinished < numProf){
        // check which ones should we move fwd
        foundMutation = 0;
        for (idx i = 0; i < numProf; ++i){
            if ((currPos.iSub == currStatus[i].iSub) && (currPos.position == currStatus[i].position)){
                moveFwd[i] = true;
            }
            else {
                //no coverage
                moveFwd[i] = false;
            }
            if (moveFwd[i] && (cJob->SNPProfile >= 0) && !emptyValues (&currStatus[i], cJob, i) && findMutation(&currStatus[i])){
                ++foundMutation;
            }
        }

        if (foundMutation){
            idx isValidCount = 1;
            if (foundMutation == numProf){
                // should validate that mutations are not the same
                char ref, mut;
                ref = currStatus.ptr(0)->letter;
                findMutation(&currStatus[0], &mut);

                for (idx i = 1; i < numProf; ++i){
                    char mutalt;
                    findMutation(&currStatus[i], &mutalt);
                    if ((ref == currStatus.ptr(i)->letter) && (mut == mutalt)){
                        ++isValidCount;
                    }
                }
            }
            if (isValidCount != numProf){
                printMutation(*out2, &currPos, currStatus);
            }
        }
        // Move forward
        for (idx i = 0; i< numProf; ++i){
            if (moveFwd[i]){
                cntFinished += getNext(cJob, i, &currStatus[i] );
            }
        }
        // move the current position
        ++currPos.position;

        // Check if we need to write a new entry
        bool updateStatus = false;
        for(idx i = 0; i < numProf && !updateStatus; ++i) {
            // Who has a valid position ?
            if( (currPos.iSub == currStatus[i].iSub) && (currPos.position == currStatus[i].position) ) {
                // Same position, has something else changed ?
                // Or it is still under minimum threshold
                if( !moveFwd[i] || emptyValues (&currStatus[i], cJob, i)) {
                    // Last position was no covered or empty
                    updateStatus = true;
                } else if( compareRec(&prevStatus[i], &currStatus[i], cJob, i) ) {
                    updateStatus = true;
                }
            } else {
                // No coverage, check if before there was a coverage, OR it was under minimum threshold
                if( moveFwd[i] && !emptyValues (&prevStatus[i], cJob, i)) {
                    // There was coverage before, something has changed
                    updateStatus = true;
                }
            }
        }
        // Has something changed in coverage?
        if( updateStatus ) {
//            if (cJob->SNPProfile == 0){
                printOutput(*out1, &startPos, &currPos, prevStatus, moveFwd, cJob);
//            }
            currPos.iSub = sIdxMax;
            currPos.position = sIdxMax;
            for(idx i = 0; i < numProf; ++i) {
                prevStatus[i] = currStatus[i];
                updateMinRec(&currPos, &currStatus[i]);
            }
            startPos = currPos;
        }
    }
    return 0;
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
    std::auto_ptr < sUsrObj > obj(user->objFactory(objID));

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

    // Read params

    sStr err;
    sVec <Jobs> jobList;
    if (!loadParameters(&jobList, &err)){
        reqSetInfo(req, eQPInfoLevel_Error, "%s", err.ptr());
        reqSetStatus(req, eQPReqStatus_ProgError);
        return false;
    }


    sStr pathfilename;
    sStr pathfilenameMut;

    for (idx ijob = 0; ijob < jobList.dim(); ++ijob){
        Jobs *currentJob = jobList.ptr(ijob);
        restartProfilers();
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
            prepareOutHeaders (&outMutFile, currentJob, true);
        }
        // Prepare headers
        prepareOutHeaders (&outFile, currentJob);
        calculateDifferences (&outFile, currentJob->SNPProfile ? &outMutFile : 0, currentJob);
        reqProgress(-1, 100, 100);
    }

    progress100Start = progress100End;
    progress100End = 100;
    reqProgress(-1, 100, 100);

    reqSetStatus(req, eQPReqStatus_Done); // change the status
    return 0;
}

int main(int argc, const char * argv[])
{
    sBioseq::initModule(sBioseq::eATGC);

    sStr tmp;
    sApp::args(argc,argv); // remember arguments in global for future

    DnaDifferentialProfiler backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"dna-differential-profiler",argv[0]));
    return (int)backend.run(argc,argv);
}
