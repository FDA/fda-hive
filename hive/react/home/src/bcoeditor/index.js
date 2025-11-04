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
import React, {Component} from 'react';
import ReactDOM from 'react-dom';
import { useLocation } from 'react-router-dom';
import "antd/dist/antd.css";
import {Tabs, Button, notification, message} from 'antd';

import WrappedProvenanceForm from "./components/provenance_pane";
import WrappedExtraInfoForm from "./components/extrainfo_pane";
import InputsPane from "./components/inputs_pane";
import SummaryModal from "./components/summary_modal";

import SHA1 from "./encoding.js"

//import { HeaderContainer } from './header-footer/header/header_container';
import { HeaderContainer } from '../hivelib/Header/index.js';
import { FooterContainer } from '../hivelib/Footer/footer_container';
import { CustomRequest as RequestConstructor } from "../hivelib/modal/request_modal.js";

import {getPrefix} from "./config";

//import "./css/styles.css";

const TabPane = Tabs.TabPane;
const errorMessage = 'Some information is missing or incorrect. Please correct the fields highlighted and try again.';
//const prefix = "comb.fda.gov/usr/ekaterina.minina/dna.cgi";

class BcoEditor extends Component{

    constructor(props){
        super(props);
        this.allVals = this.getDocLocs();

        if(this.allVals.id){//load existing object
            this.loadObject(this.allVals.id);
            this.objID = this.allVals.id;
        }

        this.state={
            pipelinetabs:{},
            validLocs: [false, true, false],
            provenanceVals: {},
            inputsVals: {},
            extraInfoVals: {},
            summaryVals: {},
            summaryVisible: false,
            createBCODisabled: false,
            submitted: false
        };
    }

    loadObject(id){
        let paramXhr = new XMLHttpRequest();
        paramXhr.addEventListener('load', () => {
            let params = paramXhr.responseText;
            let paramsJson = JSON.parse(params);

            console.log(paramsJson);

            let inputsVals={ inputObjs: paramsJson.inputObjs};
            let provenanceVals={
                bcoName: paramsJson.bcoName,
                contributors: paramsJson.contributors,
                license: paramsJson.license,
                obsoleteAfter: paramsJson.obsoleteAfter,
                review: paramsJson.review,
                startTime: paramsJson.startTime,
                derivedFrom: paramsJson.derivedFrom,
                endTime: paramsJson.endTime
            };
            let extraInfoVals={
                algorithmicError: paramsJson.algorithmicError,
                keywords: paramsJson.keywords,
                xref: paramsJson.xref,
                externalDataEndpoints: paramsJson.externalDataEndpoints,
                empiricalError: paramsJson.empiricalError,
                usabilityDomain: paramsJson.usabilityDomain
            }

            this.setState({inputsVals: inputsVals, provenanceVals: provenanceVals, extraInfoVals: extraInfoVals});
        });
        paramXhr.open("POST", getPrefix());
        paramXhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        paramXhr.send("cmdr=propget&mode=json&ids=" + id);
    }

    onCreateBCO = () => {
        // disable button for short period to prevent accidental double clicks
        this.setState({ createBCODisabled: true });
        this.setState({ submitted: true });

        setTimeout(() => this.setState({ createBCODisabled: false }), 1000);

        let provenanceRefPromise;
        let inputsRefPromise;
        let extraInfoRefPromise;
        if (this.provenanceRef) {
            provenanceRefPromise = this.provenanceRef.props.form.validateFields();
        }
        if (this.inputsRef) {
            inputsRefPromise = this.inputsRef.props.form.validateFields();
        }
        if (this.extraInfoRef) {
            extraInfoRefPromise = this.extraInfoRef.props.form.validateFields();
        }

        this.allFields = [];

        Promise.allSettled([provenanceRefPromise, inputsRefPromise,  extraInfoRefPromise])
            .then((results) => {
                let shouldCreate = true;
                for (let i = 0; i < results.length; ++i) {
                    if (results[i].value && results[i].status === "fulfilled") {
                        this.markValid(i, true);
                    } else {
                        this.markValid(i, false);
                        shouldCreate = false;
                    }
                }
                if (shouldCreate) {
                    this.allFields.push(this.provenanceRef.state.fieldsJson);
                    this.allFields.push(this.inputsRef.state.fieldsJson);
                    this.allFields.push(this.extraInfoRef.state.fieldsJson);
                    this.collectAndCreate();
                }
                else {
                    message.error(errorMessage);
                }
            });
    };

    collectAndCreate(){
        let collectedJson = {};
        let objectHolder = "Inbox";
        for(let kk in this.allFields){
            let curFields = this.allFields[kk];

            for( let key in curFields){
                if(curFields[key].children){ // have only 2 layers here
                    collectedJson[key] ={};

                    let tmpObj = {};
                    for(let cKey in curFields[key].children){
                        if(curFields[key].children[cKey].value)
                            tmpObj[cKey] = curFields[key].children[cKey].value;
                    }

                    if(curFields[key].multi && (Object.keys(tmpObj).length !== 0 || !curFields[key].hasDefault)){ //create list with locations
                        let i=1;
                        collectedJson[key] = {};
                        if (Object.keys(tmpObj).length !== 0){
                            collectedJson[key][i] = tmpObj;
                        }

                        for(i; curFields[key].actChildren && i < curFields[key].actChildren.length+1;){
                            let tmpObjNxt = {};

                            let searchLoc = curFields[key].actChildren[i-1].children ? curFields[key].actChildren[i-1].children : curFields[key].actChildren[i-1];
                            for(let cKey in searchLoc){
                                let actKey = cKey.substring(0, cKey.lastIndexOf("-"));
                                if(searchLoc[cKey].value)
                                    tmpObjNxt[actKey] = searchLoc[cKey].value;
                            }

                            if (Object.keys(tmpObj).length !== 0){
                                collectedJson[key][++i]=tmpObjNxt;
                            }
                            else {
                                collectedJson[key][i++]=tmpObjNxt;
                            }
                        }

                        if (collectedJson[key] && Object.keys(collectedJson[key]).length === 0 && collectedJson[key].constructor === Object){
                            delete collectedJson.key;
                        }
                    }
                    else{ //just copy
                        collectedJson[key] = tmpObj;
                    }
                }
                else if(curFields[key].actChildren){ //non multi child
                    collectedJson[key] ={};
                    let i=1;
                    let tmpVal = curFields[key].value;

                    collectedJson[key][i]=tmpVal;
                    for(i; curFields[key].actChildren && i < curFields[key].actChildren.length+1;){
                        let searchLoc = curFields[key].actChildren[i-1].children ? curFields[key].actChildren[i-1].children : curFields[key].actChildren[i-1];
                        tmpVal = searchLoc.value;

                        i++;
                        collectedJson[key][i]=tmpVal;
                    }
                }
                else if (curFields[key].value){
                    collectedJson[key] = curFields[key].value;
                }
            }
        }

        //save object
        let actualJsonSubmit = {};
        actualJsonSubmit["bco"] = {"_id": "$newid()", "_type":"bco_brief"};
        Object.assign(actualJsonSubmit.bco, collectedJson);

        let paramXhr = new XMLHttpRequest();
        paramXhr.addEventListener('load', () => {
            let params = paramXhr.responseText;
            let paramsJson = JSON.parse(params);

            console.log(paramsJson);
            this.setState({disabledDownload: false});
            this.objID = paramsJson.bco["_id"];

            this.setReferenceToObject(objectHolder, this.objID);
        });
        paramXhr.open("POST", getPrefix());
        paramXhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        paramXhr.send("cmdr=propset2&parse=" + JSON.stringify(actualJsonSubmit));

        notification.config({
            placement: 'bottomRight',
        });
        notification.info({
            message: 'Object Created',
            description:
                "Your object was created. Please check the home page",
        });
    }

    setReferenceToObject = (objName, childId) => {
        let parametersGet = {
            cmd: "objList" ,
            mode: "json" ,
            type: "^sysfolder$" ,
            prop: "name" ,
            search: objName
        }

        let requestGet = new RequestConstructor({ parameters: parametersGet });

        requestGet.handleFetch()
          .then((response) => response.json())
          .then((json) => json.objs.map(result => result))
          .then((newData) => {
            let parametersSet = {
                cmd: "objCut",
                ids:  childId,
                dest: newData[0]["_id"],
                src: "all"
            }

            let requestSet = new RequestConstructor({ parameters: parametersSet });

            requestSet.handleFetch()
            .then((response) => response.json())
            .then((newData) => {
                return newData;
            })
            .catch((error) => console.log("ObjectReferencing:" + error));
          })
          .catch((error) => console.log("ObjectIdError:" + error));
    }

    markValid(loc, val){
        let validsArr = this.state.validLocs;
        validsArr[loc] = val;

        this.setState({validLocs: validsArr});
    };

    onDownloadBCO = () => {
        if(!this.provenanceRef || !this.extraInfoRef || !this.inputsRef){
            console.log("Not all fields filled out");
            return;
        }
        let provenance = this.provenanceRef.state.fieldsJson;
        let extraInfo = this.extraInfoRef.state.fieldsJson;
        let inputs = this.inputsRef.state;

        let inputObjs = Object.keys(inputs.objs);

        let validBCO = {
            "object_id": getPrefix() + "?cmdr=propget&ids=" + this.objID + "&mode=json",
            "spec_version": "1.3",
            "provenance_domain": {},
            "usability_domain": [extraInfo.usabilityDomain.value],
            "description_domain": {},
            "execution_domain": {},
            "parametric_domain": [],
            "io_domain": {
                input_subdomain: [],
                output_subdomain: []
            },
            "error_domain": {}
        };

        //-------error domain
        validBCO.error_domain = {
            "algorithmic_error": {text: extraInfo.algorithmicError.value},
            "empirical_error": {text: extraInfo.empiricalError.value}
        };

        //-------execution domain
        validBCO.execution_domain = {
            "script": [
                { uri: {uri: ""} }
            ],
            "script_driver": "",
            "software_prerequisites": [{
                name: "",
                version: "",
                uri: { uri: ""}
            }],
            "external_data_endpoints": [{
                name: "HIVE",
                url: getPrefix() + "?cmd=login"
            }],
            "environment_variables": {}
        };

        //-------provenance domain
        validBCO.provenance_domain = {
            name: provenance.bcoName.value,
            version: provenance.version.value,
            review:[], // from provenance
            derived_from: getPrefix() + "?cmdr=propget&mode=json&ids=" + provenance.derivedFrom.value,
            obsolete_after: provenance.obsoleteAfter.value,
            embargo: {
                start_time: provenance.startTime.value,
                end_time: provenance.endTime.value
            },
            created: "", //from id
            modified: "", //from id
            contributors: [], //from provenance
            license: provenance.license.value
        };
        //map reviewers
        let curChild = provenance.review.children;
        let reviewArr = validBCO.provenance_domain.review;
        let i = -1;
        do{
            let date = "date" + (i === -1 ? "" : "-" + (i+1));
            let name = "reviewer" + (i === -1 ? "" : "-" + (i+1));
            let affiliation = "reviewerAffiliation" + (i === -1 ? "" : "-" + (i+1));
            let email = "reviewerEmail" + (i === -1 ? "" : "-" + (i+1));
            let contribution = "reviewerContribution" + (i === -1 ? "" : "-" + (i+1));
            let orcid = "reviewerOrcid" + (i === -1 ? "" : "-" + (i+1));
            let reviewerComment = "reviewerComment" + (i === -1 ? "" : "-" + (i+1));
            let status = "status" + (i === -1 ? "" : "-" + (i+1));

            let purObj = {
                date: curChild[date].value,
                reviewer:{
                    name: curChild[name].value,
                    affiliation: curChild[affiliation].value,
                    email: curChild[email].value,
                    contribution: [curChild[contribution].value],
                    orcid: curChild[orcid].value
                },
                reviewer_comment: curChild[reviewerComment].value,
                status: curChild[status].value
            };

            for(let cKey in curChild){
                if(curChild[cKey].value){
                    reviewArr.push(purObj);
                    break;
                }
            }

            i++;
            curChild = (provenance.review.actChildren && provenance.review.actChildren[i]) ? provenance.review.actChildren[i].children : false;
        }while (curChild);
        //map contributors
        let curChild1 = provenance.contributors.children;
        let contrArr = validBCO.provenance_domain.contributors;
        i = -1;
        do{
            let name = "contributorName" + (i === -1 ? "" : "-" + (i+1));
            let affiliation = "affiliation" + (i === -1 ? "" : "-" + (i+1));
            let email = "email" + (i === -1 ? "" : "-" + (i+1));
            let contribution = "contribution" + (i === -1 ? "" : "-" + (i+1));
            let orcid = "orcid" + (i === -1 ? "" : "-" + (i+1));

            let purObj = {
                name: curChild1[name].value,
                affiliation: curChild1[affiliation].value,
                email: curChild1[email].value,
                contribution: [curChild1[contribution].value],
                orcid: curChild1[orcid].value
            };
            contrArr.push(purObj);
            i++;
            curChild1 = (provenance.contributors.actChildren && provenance.contributors.actChildren[i])  ? provenance.contributors.actChildren[i].children : false;
        }while (curChild1);


        //-------description domain
        validBCO.description_domain = {
            keywords:[],
            xref: [],
            platform: ["HIVE"],
            pipeline_steps: []
        };

        //keywords
        let curKeyword = extraInfo.keywords;
        i=-1;
        do{
            validBCO.description_domain.keywords.push(curKeyword.value);
            i++;
            curKeyword = (extraInfo.keywords.actChildren && extraInfo.keywords.actChildren[i]) ? extraInfo.keywords.actChildren[i] : false;
        }while (curKeyword);

        //xref
        let curXref = extraInfo.xref.children;
        let xrefArr = validBCO.description_domain.xref;
        i=-1;
        do{
            let namespace = "namespace" + (i === -1 ? "" : "-" + (i+1));
            let name = "xrefName" + (i === -1 ? "" : "-" + (i+1));
            let ids = "ids" + (i === -1 ? "" : "-" + (i+1));
            let access_time = "accessTime" + (i === -1 ? "" : "-" + (i+1));

            let purObj = {
                namespace: curXref[namespace].value,
                name: curXref[name].value,
                ids: [curXref[ids].value],
                access_time: curXref[access_time].value
            };
            xrefArr.push(purObj);
            i++;
            curXref = (extraInfo.xref.actChildren && extraInfo.xref.actChildren[i])  ? extraInfo.xref.actChildren[i].children : false;
        }while (curKeyword);

        //pipeline steps
        let prereqArr = validBCO.description_domain.pipeline_steps;
        let numInputObjs = Object.keys(inputs.objs).length;
        i=1;
        for (let objNum in inputs.objs){ //all the objects used = all pipeline steps
            let objType = inputs.objs[objNum]._type;
            let objName = inputs.typeInfo[objType].title;
            let objDescr = inputs.typeInfo[objType].description;

            let curStepObj = {
                step_number: i,
                name: objName,
                description: objDescr,
                input_list: [],
                output_list: []
            };

            //generate input list
            let curInputsInput = inputs.inputObjs[objType];
            for (let j=0; j < curInputsInput.length; j++)
                curStepObj.input_list.push({uri: getPrefix() + "?cmd=objFile&ids=" + curInputsInput[j]});


            //generate output list
            let curInputsOutputs = inputs.objToFiles[objNum];
            for (let j=0; curInputsOutputs && j < curInputsOutputs.length; j++)
                curStepObj.output_list.push({uri: getPrefix() + "?cmd=objFile&ids=" + objNum + "&filename=" + curInputsOutputs[j]});


            if (inputs.objs[objNum].batch_children_proc_ids !== undefined && typeof inputs.objs[objNum].batch_children_proc_ids === "object") {
                const batchChildrenList = Object.values(inputs.objs[objNum].batch_children_proc_ids);
                if (batchChildrenList !== null) {
                    for (let j = 0; j < batchChildrenList.length; ++j) {
                        const batchObj = batchChildrenList[j];
                        for(let k = 0; inputs.objToFiles[batchObj] && k < inputs.objToFiles[batchObj].length; k++) {
                            curStepObj.output_list.push({uri: getPrefix() + "?cmd=objFile&ids=" + batchObj + "&filename=" + inputs.objToFiles[batchObj][k]});
                        }
                    }
                }
            }

            //output objects
            let curOutputObjs = inputs.outputObjs[objType];
            for (let j=0; curOutputObjs && j < curOutputObjs.length; j++)
                curStepObj.output_list.push({uri: getPrefix() + "?cmd=propget&mode=json&ids=" + curOutputObjs[j]});


            prereqArr.push(curStepObj);
            if (i === 1)
                validBCO.io_domain.input_subdomain = curStepObj.input_list.map((x) => { return {"uri": x} });
            if (i === numInputObjs)
                validBCO.io_domain.output_subdomain = curStepObj.output_list.map((x) => { return {"mediatype":"application/octet-stream", "uri": x} });

            i++;
        }


        //-------parametric domain
        let paramArr = validBCO.parametric_domain;
        i=1;
        for (let objNum in inputs.objs){ //all the objects used = all pipeline steps
            let objType = inputs.objs[objNum]._type;
            let currentParams = inputs.objs[objNum];

            for(let key in currentParams){
                if(key.indexOf("_") === 0 && key !== "_type") continue;
                if(inputs.typeInfo[objType] && inputs.typeInfo[objType]._attributes[key].role && key !== "_type"){
                    if (inputs.typeInfo[objType]._attributes[key].role === "input") continue;
                    else if (inputs.typeInfo[objType] && inputs.typeInfo[objType]._attributes[key].role === "state") continue;
                }

                let printVal = currentParams[key];
                if(typeof printVal === "object")
                    printVal = JSON.stringify(printVal);
                if(printVal === false) printVal = "false";
                else if(printVal === true) printVal = "true";

                paramArr.push({
                    param: key,
                    value: printVal + "",
                    step: i + ""
                });
            }
            i++;
        }

        let checksum = SHA1(JSON.stringify(validBCO));
        validBCO.checksum = checksum;

        const element = document.createElement("a");
        const file = new Blob([JSON.stringify(validBCO).replace('checksum', 'etag')], {type: 'application/json'});
        element.href = URL.createObjectURL(file);
        element.download = provenance.bcoName.value + "-BCO.json";
        document.body.appendChild(element); // Required for this to work in FireFox
        element.click();
        element.remove();

        notification.config({
            placement: 'bottomRight',
        });
        notification.info({
            message: 'Download Started',
            description:
                "Please check your downloads folder for the BCO json",
        });
    };

    onSummaryBCO = () => {
        if(!this.inputsRef) return;

        const allTypesInfo = this.inputsRef.state.typeInfo;
        const allTypesFields = this.inputsRef.state.objs;
        this.setState({summaryVals: allTypesInfo, objFields: allTypesFields, summaryVisible: true});
    };

    handleDownloadSummary = (checkedValues, mapping) => {
        const inputs = this.inputsRef.inputs;
        const outputs = this.inputsRef.outputs;

        let toReturn = "BCO ID: " + this.objID + "\nBCO Title: " + this.provenanceRef.state.fieldsJson.bcoName.value + "\n" + "Inputs:\n";
        for(let i=0; i < inputs.length; i++){
            toReturn += "\t" + inputs[i].objID + ":\t" + inputs[i].title + "\n";
        }

        toReturn += "Parameters:\n";
        for(let key in checkedValues){
            toReturn += "\t" + key + ":\n"; //each service
            let vals = this.inputsRef.state.objs[mapping[key]];
            for(let i = 0; i < checkedValues[key].length; i++){
                if(typeof vals[checkedValues[key][i]] === "object"){
                    let stringified = JSON.stringify(vals[checkedValues[key][i]], null, 4);
                    while(stringified.indexOf("    ") > 0){
                        stringified = stringified.replace("    ", "\t\t\t");
                    }
                    stringified = stringified.substring(0,stringified.lastIndexOf("}"));
                    stringified += "\t\t}";
                    toReturn += "\t\t" + checkedValues[key][i] + ":\t" + stringified + "\n";

                }
                else
                    toReturn += "\t\t" + checkedValues[key][i] + ":\t" + vals[checkedValues[key][i]] + "\n";
            }
        }

        toReturn += "Output Files:\r\n";
        for(let i = 0; i < outputs.length; i++){
            toReturn += "\t" + outputs[i].fileName + "\n";
        }

        while(toReturn.indexOf("\n") > 0){
            toReturn = toReturn.replace("\n", "___");
        }
        while(toReturn.indexOf("___") > 0){
            toReturn = toReturn.replace("___", "\r\n");
        }

        const element = document.createElement("a");
        const file = new Blob([toReturn], {type: 'text/plain'});
        element.href = URL.createObjectURL(file);
        element.download = this.provenanceRef.state.fieldsJson.bcoName.value + "-summary.txt";
        document.body.appendChild(element); // Required for this to work in FireFox
        element.click();
        element.remove();

        notification.config({
            placement: 'bottomRight',
        });
        notification.info({
            message: 'Download Started',
            description:
                "Please check your downloads folder for the BCO summary file",
        });
    };

    onCloseModal = () => {
        this.setState({summaryVisible: false});
    };

    saveProvenanceRef = (provenanceRef) =>{
        this.provenanceRef = provenanceRef;
    };

    saveInputsRef = (inputsRef) =>{
        this.inputsRef = inputsRef;
    };

    saveExtraInfoRef = (extraInfoRef) =>{
        this.extraInfoRef = extraInfoRef;
    };

    getDocLocs (){
        let parseString = document.location.href;
        parseString = parseString.substring(parseString.indexOf("?")+1);

        let allChunks = parseString.split("?");
        let toReturn = {};

        for(let i=0; i < allChunks.length; i++){
            let chunk = allChunks[i];
            toReturn[chunk.split("=")[0]] = chunk.split("=")[1];
        }

        const queryParams = new URLSearchParams(window.location.search);
        toReturn['objs'] = queryParams.get('objs');

        return toReturn;
    };

    onTabChange = (key) => {
        if (this.state.submitted){
            switch (key) {
                case '0':
                    if (!this.provenanceRef) setTimeout(() => this.provenanceRef.props.form.validateFields(), 100);
                  break;
                case '1':
                    if (!this.inputsRef) setTimeout(() => this.inputsRef.props.form.validateFields(), 100);
                  break;
                case '2':
                  if (!this.extraInfoRef) setTimeout(() => this.extraInfoRef.props.form.validateFields(), 100);
                  break;
                default:
                  break;
              }
        }
    };

    CustomTabTitle = ({ title, showAsterisk }) => (
        <span>
            {title} {showAsterisk && this.state.submitted && <span style={{ color: 'red' }}>*</span>}
        </span>
    );

    render(){
        let downloadBtn, summaryBtn;
        if(this.state.validLocs.indexOf(false) >= 0 && !this.allVals.id){
            downloadBtn = ( <Button onClick={this.onDownloadBCO} disabled>
                Download BCO JSON
            </Button>);
            summaryBtn = ( <Button onClick={this.onSummaryBCO} disabled>
                Download BCO Summary
            </Button>);
        }
        else{
            downloadBtn = ( <Button onClick={this.onDownloadBCO}>
                Download BCO JSON
            </Button>);
            summaryBtn = ( <Button onClick={this.onSummaryBCO}>
                Download BCO Summary
            </Button>);
        }

        return (<div>
            <div className="header" style={{backgroundColor: '#334061'}}>
                        <HeaderContainer />
            </div>
            <Tabs animated={false} onChange={this.onTabChange}>
                <TabPane tab={<this.CustomTabTitle title="Provenance Domain" showAsterisk={!this.state.validLocs[0]} />} key={0}>
                    <div className="pane">
                        <WrappedProvenanceForm wrappedComponentRef={this.saveProvenanceRef} vals={this.state.provenanceVals}/>
                    </div>
                </TabPane>
                <TabPane  tab={<this.CustomTabTitle title="Computation from Pipeline" showAsterisk={!this.state.validLocs[1]} />} key={1} forceRender={true}>
                    <div className="pane">
                        <InputsPane wrappedComponentRef={this.saveInputsRef}
                            objs={this.state.inputsVals.inputObjs ? this.state.inputsVals.inputObjs :this.allVals.objs}/>
                    </div>
                </TabPane>
                <TabPane tab={<this.CustomTabTitle title="Extra Information" showAsterisk={!this.state.validLocs[2]} />} key={2}>
                    <div className="pane">
                        <WrappedExtraInfoForm wrappedComponentRef={this.saveExtraInfoRef} vals={this.state.extraInfoVals}/>
                    </div>
                </TabPane>
            </Tabs>
            <br/> <br/><br/><br/><br/>
            <div className="buttonDiv">
                <Button onClick={this.onCreateBCO} disabled={this.state.createBCODisabled}>
                    Create BCO
                </Button>
                {downloadBtn}
                {summaryBtn}
            </div>
            <div style={{position: 'fixed',  bottom: '0', width: '100%', 'background-color': '#ffffff'}}>
            <FooterContainer footerStyle={"dashboard__main__footer"}/>
            <SummaryModal visible={this.state.summaryVisible} vals={this.state.summaryVals} objFields={this.state.objFields} onClose={this.onCloseModal} handleOk={this.handleDownloadSummary}/>
            </div>
        </div>);
    }
}

export default BcoEditor