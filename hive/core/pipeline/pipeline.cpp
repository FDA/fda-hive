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

#include <slib/utils.hpp>
#include <slib/std/file.hpp>
#include <slib/std/string.hpp>

#include <violin/hiveproc.hpp>

#include <qpsvc/qpsvc.hpp>
#include <qpsvc/qpsvc-dna-alignx.hpp>

#include <ulib/uobj.hpp>
#include <ulib/uquery.hpp>

static const int   delaySec = 60;
static const char* workflow = "workflow";
static const char* getWorkDirName = "getWorkDir";

#if _DEBUG
void debugPrint(const char * title, sVar& form)
{
    ::fprintf(stderr, "Form '%s' ==>\n", title);
    for(idx i = 0; i < form.dim(); i++) {
        const char * key = static_cast<const char*>(form.id(i));
        const char * value = form.value(key);
        ::fprintf(stderr, "  '%s' = '%s'\n", key, value);
    }
    ::fprintf(stderr, "Form '%s' <==\n", title);
    fflush(stderr);
}
#else
#define debugPrint(a,b) void(0)
#endif

class sPipelineBuiltin_getWorkDir;

class Pipeline : public sHiveProc
{
    typedef sHiveProc TParent;

    protected:
        idx _currStep;
        idx _finalStep;
        int _priority;
        idx _algReq;
        sStr _objidPath;
        sUsrQueryEngine* _sQLan;

    public:
        Pipeline(const char * defline00, const char * srv)
            : sHiveProc(defline00, srv), _currStep(0), _finalStep(0), _priority(0), _algReq(0), _sQLan(0)
        {
            sBioseq::initModule(sBioseq::eACGT);
        }

        virtual ~Pipeline()
        {
            delete _sQLan;
        }

        virtual idx OnExecute(idx) {
            const sUsrObjPropsTree * tree = objs.dim() ? objs[0].propsTree() : 0;
            if( !tree ) {
                logOut(eQPLogType_Error, "Getting obj prop tree failed, pipeline step %" DEC ", Pipeline execution cancelled \n", _currStep);
                return 1;
            }
            getStepCount(tree);
            if( !getCurrStep() ) {
                _currStep = 1;
                executeStep();
                return 0;
            }
            const idx stat = waitForTheStep();
            if( stat == eQPReqStatus_Done ) {
                if( _currStep > _finalStep ) {
                    reqSetStatus(reqId, eQPReqStatus_Done);
                    reqProgress(-1, 100, 100);
                } else {
                    executeStep();
                }
            }
            return 0;
        }

        virtual sRC OnSplit(idx req, idx &cnt) {
            cnt = 1;
            return sRC::zero;
        }

        idx getCurrStepNo() const {
            return _currStep;
        }

        void test();

    protected:

        bool getReqProperties( sVar& form, int& currentForm, int& total );

        idx launchTool(sVar& form);

        bool executeStep() {
            sVar submissionForm;
            int currentForm = 0;
            int total = 1;

            if( !getReqProperties(submissionForm, currentForm, total) ) {
                return true;
            }
            idx algReq = launchTool(submissionForm);

            if( algReq <= 0 ) {
                logOut(eQPLogType_Error, "Launching failed, pipeline step %" DEC ", Pipeline execution cancelled \n", _currStep);
                return false;
            }
            reqProgress(_currStep, _currStep, _finalStep);
            _currStep++;
            reqSetData(reqId, "runningSvc", "%" DEC, algReq);
            reqSetData(reqId, "stepNo", "%" DEC, _currStep);
            reqReSubmit(reqId, cfgInt(0, "pipeline.ResubDelay", delaySec));
            logOut(eQPLogType_Debug, "waiting for %" DEC " to finish ", algReq);
            return true;
        }

        void collectReqIds(const idx group, sDic<idx> & reqs)
        {
            sVec<sQPrideBase::Request> grp;
            requestGetForGrp(group, &grp);
            for(idx i = 0; i < grp.dim(); ++i) {
                const idx r = grp[i].reqID;
                if( !reqs.get(&r, sizeof(r)) ) {
                    idx * p = reqs.set(&r, sizeof(r));
                    if( p ) {
                        *p = grp[i].stat;
                    }
                    collectReqIds(r, reqs);
                }
            }
        }

        idx waitForTheStep()
        {
            sDic<idx> reqs;
            collectReqIds(reqId, reqs);
            for(idx i = 0; i < reqs.dim(); ++i) {
                const idx * const r = (idx *)reqs.id(i);
                const idx * const stat = reqs.ptr(i);
                if( *r > reqId ) {
                    if( *stat <= eQPReqStatus_Suspended ) {
                        reqReSubmit(reqId, cfgInt(0, "pipeline.pipelineResubDelay", delaySec));
                        logOut(eQPLogType_Debug, "waiting for %" DEC " to finish, on step %" DEC, *r, _currStep);
                        return *stat;
                    }
                    if( *stat >= eQPReqStatus_Killed ) {
                        reqSetStatus(reqId, eQPReqStatus_ProgError);
                        reqSetInfo(reqId, eQPInfoLevel_Error, "Process %" DEC " is failed, aborting at step %" DEC, *r, _currStep);
                        return *stat;
                    }
                }
            }
            return eQPReqStatus_Done;
        }

        bool recordSubmittedProcessIDs(sHiveId& pid)
        {
            if( pid ) {
                sStr buf;
                pid.print(buf);
                buf.add0(3);

                const char * gs[] = {_objidPath.ptr()};
                const char * vs[] = {buf.ptr()};
                sStr stepObjIDName("st%" DEC "_objid", _currStep);
                if( objs[0].propSet(stepObjIDName.ptr(), gs, vs, 1, false) == 1 ) {
                    return true;
                }
            }
            return false;
        }

        virtual sUsrQueryEngine * queryEngineInit(sUsrQueryEngine * qe);

    private:

        bool getCurrStep()
        {
            if( _sQLan ) {
                delete _sQLan;
            }
            _sQLan = queryEngineFactory();
            sStr final;
            reqGetData(reqId, "stepNo", final.mex());
            if( !final ) {
                return false;
            }
            sscanf(final, "%" DEC, &_currStep);
            final.cut0cut();
            reqGetData(reqId, "runningSvc", final.mex());
            sscanf(final, "%" DEC, &_algReq);
            return true;
        }

        void getStepCount(const sUsrObjPropsTree * tree) {
            sStr stepNodeName;
            const sUsrObjPropsNode * node = NULL;
            _finalStep = 0;
            do {
                ++_finalStep;
                stepNodeName.printf(0, "st%" DEC "_type", _finalStep);
                node = tree->find(stepNodeName);
            } while( node );
            --_finalStep;
        }

        bool processDefaultName(const char* name, const char* path, const char* value, sVar& form)
        {
            if( sIsExactly(name, "svc") && value ) {
                form.inp(name, value);
                return false;
            }
            if( sIsExactly(name, "objid") && path) {
                _objidPath.printf(0, "%s", path);
                return true;
            }
            if( sIsExactly(name, "type") ) {
                return true;
            }
            return false;
        }

        bool processQLang(const char* query, sVariant& result) {
            sStr errorMsg;
            if( query && sIs("$(", query) ) {
                _sQLan->evalTemplate(query, strlen(query), result, &errorMsg);
            } else if( query && sIs("query://", query) ) {
                _sQLan->eval(query + 8, strlen(query) - 8, result, &errorMsg);
            } else {
                return false;
            }
            if( errorMsg ) {
                reqSetInfo(reqId, sQPrideBase::eQPLogType_Error, "%s", errorMsg.ptr());
                return false;
            }
            return true;
        }

        const char * printFormKey(const char * objTypeName, const char * name, const char * path = 0) {
            static sStr keybuf;
            keybuf.printf(0, "prop.%s.%s", objTypeName, name);
            if( sLen(path) ) {
                keybuf.printf(".%s", path);
            }
            return keybuf.ptr();
        }

        const char* cutOutPrefix(const char* name) {
            if( sIs("st", name) ) {
                const char* rname = strchr(name, '_');
                if( rname != NULL && *rname != '\0' ) {
                    rname++;
                }
                name = rname;
            }
            return name;
        }

        void createObjidPath(const sUsrObjPropsNode * node) {
            sStr nodeName("st%" DEC "_objid", _currStep);
            const sUsrObjPropsNode * newNode = const_cast<sUsrObjPropsNode *>(node)->ipush(nodeName.ptr(), 0);
            if( newNode && newNode->path() ) {
                _objidPath.printf(0, "%s", newNode->path());
            } else {
                logOut(eQPLogType_Error, "Cannot find or create objID path, step  %" DEC ", pipeline execution cancelled", _currStep);
            }
        }

        void process_eList(const sUsrObjPropsNode* child, const sUsrType2* stepType, sVar& paths, sVar& form) {
            if(child) {
                for(const sUsrObjPropsNode* child2 = child->firstChild(); child2; child2 = child2->nextSibling()) {
                    const char* value = child2->value();
                    const char* name = cutOutPrefix(child2->name());
                    if( value != NULL ) {
                        addToFormWithNameCheck(printFormKey(stepType->name(), name, paths[name]), value, form, stepType->getField(*user, name));
                    } else {
                        process_recursive(child2, stepType, paths, form);
                    }
                }
            }
        }

        const sUsrObjPropsNode* isStructure(const sUsrObjPropsNode* node)
        {
            switch( node->type() ) {
                case sUsrTypeField::eArray:
                case sUsrTypeField::eArrayTab:
                    return node->firstChild();
                case sUsrTypeField::eList:
                case sUsrTypeField::eListTab:
                    return node;
                default:
                    return NULL;
            }
        }

        void process_recursive(const sUsrObjPropsNode* child, const sUsrType2* stepType, sVar& paths, sVar& form) {
            child = isStructure(child);
            if( child ) {
                    process_eList(child, stepType, paths, form);
            } else {
                logOut(eQPLogType_Info, "Unknown recursive value type %d, pipeline step %" DEC ", value is not processed", child->type(), _currStep);
            }
        }

        void addToFormWithNameCheck(const char* fullValName, const char* value, sVar& form, const sUsrTypeField * field) {
            const char * defValue = field ? field->defaultValue() : 0;
            if ( field && strcmp(field->name(), "notify") == 0 ) {
                defValue = "Never";
            }
            const bool needQLangEval = value ? !sIsExactly(defValue, value) : false;
            sVariant result;
            if( fullValName[strlen(fullValName) - 1] == '.' ) {
                if( needQLangEval && processQLang(value, result) ) {
                    if(result.isList()) {
                        for( idx i = 0, ifield = 0; i < result.dim(); ++i ) {
                            sVariant* val = result.getListElt(i);
                            if( val ) {
                                const char* existenceCheck = 0;
                                sStr keybuf;
                                do {
                                    keybuf.printf(0, "%s%" DEC, fullValName, ifield++);
                                    existenceCheck = form.value(keybuf.ptr());
                                } while( existenceCheck );
                                form.inp(keybuf.ptr(), val->asString());
                            }
                        }
                    } else {
                        const char* existenceCheck = 0;
                        idx ifield = 0;
                        sStr keybuf;
                        do {
                            keybuf.printf(0, "%s%" DEC, fullValName, ifield++);
                            existenceCheck = form.value(keybuf.ptr());
                        } while( existenceCheck );
                        form.inp(keybuf.ptr(), result.asString());
                    }
                } else {
                    const char* existenceCheck = 0;
                    idx ifield = 0;
                    sStr keybuf;
                    do {
                        keybuf.printf(0, "%s%" DEC, fullValName, ifield++);
                        existenceCheck = form.value(keybuf.ptr());
                    } while( existenceCheck );
                    form.inp(keybuf.ptr(), value);
                }
            } else if( needQLangEval && processQLang(value, result) ) {
                form.inp(fullValName, result.asString());
            } else {
                form.inp(fullValName, value);
            }
        }

        void rebuildChild(sVec<const sUsrTypeField *>& fields, sStr& group, sVar& paths) {
            sStr dummy;
            for(idx ifield = 0; ifield < fields.dim(); ++ifield) {
                dummy.printf(0, "%s.%" DEC, group.ptr(), ifield);
                paths.inp(fields[ifield]->name(), dummy.ptr());
                if( fields[ifield]->dimChildren() > 0 ) {
                    sStr dummy2("%s", dummy.ptr());
                    sVec<const sUsrTypeField*> children;
                    fields[ifield]->getChildren(children);
                    rebuildChild(children, dummy2, paths);
                }
            }
        }

        void rebuildFlatTree(sVec<const sUsrTypeField *>& fields, int bsIndex, sVar& paths) {
            sStr group;
            for(idx ifield = 0; ifield < fields.dim(); ++ifield) {
                const sUsrTypeField * field = fields[ifield];
                if( !field->parent() ) {
                    group.printf(0, "%d%s", bsIndex++, (field->isMulti() ? "." : ""));
                    paths.inp(field->name(), group.ptr());
                    if( field->dimChildren() > 0 ) {
                        sVec<const sUsrTypeField*> children;
                        field->getChildren(children);
                        rebuildChild(children, group, paths);
                    }
                }
            }
        }
};

class sPipelineBuiltin_getWorkDir : public qlang::BuiltinFunction
{
    public:
        const Pipeline& m_pipeline;
        const qlang::BuiltinFunction* wdParent;

        sPipelineBuiltin_getWorkDir(const Pipeline& pipl, const qlang::BuiltinFunction* wdFunc)
            : m_pipeline(pipl), wdParent(wdFunc)
        {
            _name.printf(0, "builtin %s() pipeline function", getWorkDirName);
        }

        virtual bool call(sVariant& result, qlang::Context& ctx, sVariant* topic, sVariant* args, idx nargs) const {
            qlang::sUsrInternalContext * ictx = dynamic_cast<qlang::sUsrInternalContext*>(&ctx);
            sStr path;
            result.setNull();
            if( ictx && ictx->getQPride() ) {
                ictx->getQPride()->cfgStr(&path, 0, "pipeline.TempDirectory");
                if( path ) {
                    path.printf("%" DEC "/", m_pipeline.reqId);
                }
            }
            if( !path && wdParent ) {
                sVariant res;
                wdParent->call(res, ctx, topic, args, nargs);
                if( !res.isNull() ) {
                    path.printf(0, "%s", res.asString());
                }
            }
            if( path && path[0] && m_pipeline.getCurrStepNo() > 0 ) {
                result.setSprintf("%sst_%" DEC "/", path.ptr(), m_pipeline.getCurrStepNo());
            }
            return !result.isNull();
        }
};

sUsrQueryEngine * Pipeline::queryEngineInit(sUsrQueryEngine * qe)
{
    TParent::queryEngineInit(qe);
    if( qe ) {
        static sPipelineBuiltin_getWorkDir myGetWorkDir(*this, qe->getContext().getBuiltin(getWorkDirName));
        qe->registerBuiltinFunction(getWorkDirName, myGetWorkDir);
    }
    return qe;
}

bool Pipeline::getReqProperties(sVar& form, int& currentForm, int& total)
{
    const sUsrObjPropsTree * tree = objs.dim() ? objs[0].propsTree() : 0;
    if( !tree ) {
        reqSetInfo(reqId, eQPInfoLevel_Error, "Pipeline requires object to run");
        reqProgress(_currStep, _currStep, _finalStep);
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return false;
    }
    sStr stepTypeName, buf("st%" DEC "_type", _currStep);
    formValue(buf, &stepTypeName);
    const sUsrType2* stepType = stepTypeName.ptr() ? sUsrType2::get(sHiveId(stepTypeName)) : 0;
    if( !stepType ) {
        reqSetInfo(reqId, eQPInfoLevel_Error, "Step %" DEC " unknown type '%s', pipeline terminated",
                   _currStep, stepTypeName.ptr() ? stepTypeName.ptr() : "undefined");
        reqProgress(_currStep, _currStep, _finalStep);
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return false;
    }
    sVec<const sUsrTypeField *> stepFields;
    stepType->getFields(*user, stepFields);
    sVar paths;
    rebuildFlatTree(stepFields, 17, paths);
    debugPrint("Paths", paths);
    form.inp(printFormKey(stepType->name(), "_type"), stepType->name());
    const char * proj = dynamic_cast<sUsrObj&>(objs[0]).propGet("submission_project");
    if( proj && proj[0] ) {
        form.inp(printFormKey(stepType->name(), "submission_project"), proj);
    }
    sVec<sHiveId> parents;
    sUsrFolder::attachedTo(&parents, *user, objs[0].Id());
    if( parents.dim() ) {
        sStr folder;
        parents[0].print(folder);
        form.inp(printFormKey(stepType->name(), "folder"), folder.ptr());
    }
    if( const udx projID = user->getProject() ) {
        sStr sp("%" UDEC, projID);
        form.inp("projectID", sp.ptr());
    }
    stepTypeName.cut0cut();
    formValue("name", &stepTypeName);
    buf.printf(0, "%s: Step %" DEC, stepTypeName.ptr(), _currStep);
    form.inp(printFormKey(stepType->name(), "name"), buf);
    buf.printf(0, "%s_st%" DEC, workflow, _currStep);
    const sUsrObjPropsNode * node = tree->find(buf);
    if(!node) {
        buf.printf(0, "st%" DEC, _currStep);
        node = tree->find(buf);
        if(!node) {
            logOut(eQPLogType_Error, "Cannot find step %" DEC " head, pipeline execution cancelled", _currStep);
            return false;
        }
    }

    for(; node; node = node->nextSibling(buf)) {
        for(const sUsrObjPropsNode* child = node->firstChild(); child; child = child->nextSibling()) {
            const char* value = child->value();
            const char* name = cutOutPrefix(child->name());
            const char* path = child->path();
            if( processDefaultName(name, path, value, form) ) {
                continue;
            }
            if( value != NULL ) {
                addToFormWithNameCheck(printFormKey(stepType->name(), name, paths[name]), value, form, stepType->getField(*user, name));
                const sUsrObjPropsNode* child2 = isStructure(child);
                if( child2 ) {
                    process_recursive(child2, stepType, paths, form);
                }
            } else {
                process_recursive(child, stepType, paths, form);
            }
        }
        if( !_objidPath ) {
            createObjidPath(node);
            if( !_objidPath ) {
                return false;
            }
        }
    }

    for(idx ifield = 0; ifield < stepFields.dim(); ++ifield) {
        const sUsrTypeField * field = stepFields[ifield];
        const char * dname = field->name();
        const char * nm = printFormKey(stepType->name(), dname, paths[dname]);
        if( form.is(nm) <= 0 ) {
            const char * dvalue = field->defaultValue();
            if( dvalue && *dvalue && !sIs("eval:", dvalue) ) {
                form.inp(nm, dvalue);
            }
        }
    }
    debugPrint("Step form", form);
    return true;
}

idx Pipeline::launchTool(sVar& submissionForm)
{
    sStr log, strObjList;
    sVec<sUsrProc> procObjs;
    sVariant wdir;
    processQLang("query://return getWorkDir();", wdir);
    logOut(eQPLogType_Info, "Starting step %" DEC ", workDir: %s \n", _currStep, wdir.asString());
    sQPride::Service Svc;
    serviceGet(&Svc, submissionForm["svc"], 0);

    logOut(eQPLogType_Trace, "Trying to create process for value set %" DEC, _currStep);
    idx err = sUsrProc::createProcesForsubmission(this, &submissionForm, user, procObjs, &Svc, &strObjList, &log);
    if( err ) {
        logOut(eQPLogType_Error, "Failed to create process for value set %" DEC ": %s", _currStep, log.ptr());
        reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to create process.");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    sHiveId processId = procObjs[0].Id();
    logOut(eQPLogType_Info, "Created process for value set %" DEC ": %s", _currStep, strObjList.ptr());
    logOut(eQPLogType_Trace, "Trying to customize submission for value set %" DEC, _currStep);
    idx cntParallel = TParent::customizeSubmission(&submissionForm, user, procObjs.ptr(0), &Svc, &log);
    if( !cntParallel ) {
        logOut(eQPLogType_Error, "Failed to customize submission for value set %" DEC ": %s", _currStep, log.ptr());
        reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to customize submission. %s", log.ptr());
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    idx reqSub;
    logOut(eQPLogType_Trace, "Trying standardized submission for value set %" DEC, _currStep);
    err = sUsrProc::standardizedSubmission(this, &submissionForm, user, procObjs, cntParallel, &reqSub, &Svc, 0, &strObjList, &log, 0, true);
    if( err ) {
        logOut(eQPLogType_Error, "Failed to submit process for value set %" DEC ": %s", _currStep, log.ptr());
        reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to submit process.");
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    sVec<idx> reqIds;
    grp2Req(reqSub, &reqIds);
    for(idx ir = 0; ir < reqIds.dim(); ++ir) {
        grpAssignReqID(reqIds[ir], reqId, ir);
    }
    if( !recordSubmittedProcessIDs(processId) ) {
        reqSetInfo(reqId, eQPInfoLevel_Error, "Failed to save step Id %" DEC, _currStep);
        reqSetStatus(reqId, eQPReqStatus_ProgError);
        return 0;
    }
    return reqSub;
}

void Pipeline::test()
{
    sStr input;
    sStr input2;
    input.printf("$(files(67832 as obj, null, null, null, \"*.{csv,vcf,json,png,tsv,txt,fasta,fastq,fa}\"))");

    input2.printf("query://.st1_accessions");

    sVariant res;
    processQLang(input2.ptr(), res);
    printf("%s", res.asString());
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc,argv);

    Pipeline backend("config=qapp.cfg" __,sQPrideProc::QPrideSrvName(&tmp,"pipeline",argv[0]));
    return (int)backend.run(argc,argv);
}
