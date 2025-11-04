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
import {Form, Button, Input, Table, Badge, Collapse} from 'antd';
import React from "react";

import {getPrefix} from "../config";

const { Panel } = Collapse;

//const prefix = "comb.fda.gov/usr/ekaterina.minina/dna.cgi";

const paramColumns = [
    {
      title: 'Parameter',
      dataIndex: 'parameter',
      key: 'param'
    },
    {
      title: 'Value',
      dataIndex: 'value',
      key: 'val',
    }
];

const inputsColumns = [
    {
      title: 'Object ID',
      dataIndex: 'objID',
      key: 'objID'
    },
    {
      title: 'Title',
      dataIndex: 'title',
      key: 'title',
    }
];

class InputsPane extends React.Component{
    constructor(props) {
        super(props);

        this.state = {fieldsJson: {
            inputObjs:{
                title: "Objects used for pipeline",
                type:"string",
                required: "true",
                value: this.props.objs
            }
        }, objs: {}, typeInfo:{}, objToFiles:{}, inputObjsInfo:{}, outputObjs:{}};
    };

    requestOutputFiles = (listInputObjs) => {
        //url for files generated
        //propget&mode=json&files=*&ids=...
        let filesXhr = new XMLHttpRequest();
        filesXhr.addEventListener('load', () => {
            let params = filesXhr.responseText;
            let paramsJson;
            try{
                paramsJson = JSON.parse(params)
            } catch(e){
                console.log(e);
                return;
            }

            let paramsArr = paramsJson;
            if(!(paramsJson instanceof Array)) paramsArr = [paramsJson];

            let objToFiles={};
            for(let i = 0; i < paramsArr.length; i++){
                objToFiles[paramsArr[i]._id] = paramsArr[i]._file;
            }

            this.setState({objToFiles: objToFiles});
        });
        filesXhr.open("POST", getPrefix());
        filesXhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        filesXhr.send("cmdr=propget&mode=json&files=*&prop=none&ids=" + listInputObjs);
    }

    handleSubmit = (event) =>{
        event.preventDefault();
        const {form} = this.props; //my current form;
        let inputsVal = form.getFieldValue("inputObjs");

        //url for computation parameters
        //propget&mode=json&ids=...
        let paramXhr = new XMLHttpRequest();
        paramXhr.addEventListener('load', () => {
            let params = paramXhr.responseText;
            let paramsJson;
            try{
                paramsJson = JSON.parse(params);
            } catch (e){
                console.log(e);
                return;
            }
            let paramsArr = paramsJson;
            let compTypes = [];
            let objToParams = {};

            if(!(paramsJson instanceof Array)) paramsArr = [paramsJson];

            let outfileObjIds = inputsVal;

            for(let i=0; i < paramsArr.length; i++){
                let objId = paramsArr[i]._id;
                let compType = paramsArr[i]._type;

                if(compTypes.indexOf(compType) < 0) compTypes.push(compType);

                objToParams[objId] = paramsArr[i];

                if (paramsArr[i].batch_children_proc_ids !== undefined && typeof paramsArr[i].batch_children_proc_ids === "object") {
                    const batchChildrenList = Object.values(paramsArr[i].batch_children_proc_ids);
                    if (batchChildrenList.length > 0) {
                        outfileObjIds = outfileObjIds + "," + batchChildrenList.join(",");
                    }
                }
            }

            this.requestTypeDescr (compTypes);
            this.setState({objs: objToParams});

            this.requestOutputFiles(outfileObjIds);
        });
        paramXhr.open("POST", getPrefix());
        paramXhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        paramXhr.send("cmdr=propget&mode=json&ids=" + inputsVal);
    };

    requestTypeDescr (typeArr){ //this is information about computations. what general algorithm name and description is
        for(let i = 0; i < typeArr.length; i++){
            let paramXhr = new XMLHttpRequest();
            paramXhr.addEventListener('load', () => {
                let response = paramXhr.responseText;
                let responseJson;
                try{
                    responseJson = JSON.parse(response);
                }catch(e){
                    console.log(e);
                    return;
                }

                let inputsMapping={};
                let outputFields={};
                let stateTypeInfo = {};
                for(let i = 0; i < responseJson.length; i++){
                    if(!stateTypeInfo[responseJson[i].name]){
                        stateTypeInfo[responseJson[i].name] = responseJson[i];
                        outputFields[responseJson[i].name] = [];

                        for (let key in responseJson[i]._attributes){
                            if(responseJson[i]._attributes[key].role && responseJson[i]._attributes[key].role === "input"){
                                if(!inputsMapping[responseJson[i].name]) inputsMapping[responseJson[i].name] = [];

                                inputsMapping[responseJson[i].name].push(key);
                            }
                            else if(responseJson[i]._attributes[key].role && responseJson[i]._attributes[key].role === "output" &&
                                responseJson[i]._attributes[key].type && responseJson[i]._attributes[key].type === "obj"){
                                if(outputFields[responseJson[i].name].indexOf(key) < 0)
                                    outputFields[responseJson[i].name].push(key);
                            }
                        }
                    }
                }


                let curInputsMap = {};
                let outputObjs = {};
                if(Object.keys(this.state.objs).length > 0){
                    let allInfoArr = [];
                    for (let obj in this.state.objs){
                        let moreInfoArr = [];
                        let curType = this.state.objs[obj]._type;
                        let curObjInputs = inputsMapping[curType];

                        for(let j = 0; curObjInputs && j < curObjInputs.length; j++){
                            let curInput = curObjInputs[j];
                            for(let kk in this.state.objs[obj][curInput]){
                                if(moreInfoArr.indexOf(this.state.objs[obj][curInput][kk]) < 0){
                                    moreInfoArr.push(this.state.objs[obj][curInput][kk]);
                                }
                                allInfoArr.push(this.state.objs[obj][curInput][kk]);
                            }
                        }
                        curInputsMap[curType] = moreInfoArr;

                        let curOutputObjs = [];
                        for(let j = 0; outputFields[responseJson[i].name] && j < outputFields[responseJson[i].name].length; j++){
                            let curOutputField = outputFields[responseJson[i].name][i];
                            let curOutputFieldVal = this.state.objs[obj][curOutputField];

                            for(let kk in curOutputFieldVal){
                                if(curOutputObjs.indexOf(curOutputFieldVal[kk]) === -1)
                                    curOutputObjs.push(curOutputFieldVal[kk]);
                            }
                        }
                        outputObjs[curType] = curOutputObjs;
                    }

                    let moreObjsXhr = new XMLHttpRequest();
                    moreObjsXhr.addEventListener('load', () => {
                        let params = moreObjsXhr.responseText;
                        let paramsJson;
                        try{
                            paramsJson = JSON.parse(params);
                        }catch(e){
                            console.log(e);
                            console.log("Unable to parse parameter JSON, assuming empty");
                            paramsJson = [];
                        }

                        if( !(paramsJson instanceof Array)){
                            paramsJson = [paramsJson];
                        }
                        let inputsMapping = {};
                        for(let i = 0; i < paramsJson.length; i++){
                            inputsMapping[paramsJson[i]._id] = paramsJson[i].name;
                        }
                        this.setState({inputObjsInfo:inputsMapping});
                    });
                    moreObjsXhr.open("POST", getPrefix());
                    moreObjsXhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
                    moreObjsXhr.send("cmdr=propget&mode=json&ids=" + allInfoArr.join(","));

                    this.setState({inputObjs: curInputsMap});
                }

                this.setState({typeInfo: stateTypeInfo, outputObjs: outputObjs});
            });
            paramXhr.open("POST", getPrefix()); // small requests
            paramXhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            paramXhr.send("cmdr=propspec3&types=" + typeArr.join(","));
        }
    };

    generateObjInfo = (objId, i) => {
        if(Object.keys(this.state.typeInfo).length === 0)
            return <div key={i}>No information was loaded</div>;

        let currentParams = this.state.objs[objId];

        let toPrintStep = "Step " + (i+1);
        let toPrintName = currentParams.name;

        let type = currentParams._type;
        let inputFields = [];

        let tableData=[];
        for(let key in currentParams){
            if(key.indexOf("_") === 0) continue;
            if(this.state.typeInfo[type] && this.state.typeInfo[type]._attributes[key].role){
                if (this.state.typeInfo[type]._attributes[key].role === "input"){
                    inputFields.push(key);
                    continue;
                }
                else if (this.state.typeInfo[type] && this.state.typeInfo[type]._attributes[key].role === "state") continue;
            }

            let printVal = currentParams[key];
            if(typeof printVal === "object")
                printVal = JSON.stringify(printVal);
            if(printVal === false) printVal = "false";
            else if(printVal === true) printVal = "true";

            tableData.push({key: key, value: printVal, parameter: key});
        }
        let toPrintParamTable = (<Table columns={paramColumns} dataSource={tableData} pagination={tableData.length < 10 ? false: true} scroll={{x:"max-content"}}></Table>);

        let tableData1=[];
        for(let i = 0; i < inputFields.length; i++){
            let ommitObjs = currentParams[inputFields[i]];
            for(let key in ommitObjs){
                tableData1.push({objID: ommitObjs[key], title: this.state.inputObjsInfo[ommitObjs[key]], key: i});
            }
        }
        let toPrintInputsTable = (<Table columns={inputsColumns} dataSource={tableData1} pagination={tableData1.length < 10 ? false: true} scroll={{x:"max-content"}}></Table>);
        if(i===0){ //original inputs
            this.inputs = tableData1;
        }


        let tableData2=[];

        let batchChildrenList = null;
        if (currentParams.batch_children_proc_ids !== undefined && typeof currentParams.batch_children_proc_ids === "object") {
            batchChildrenList = Object.values(currentParams.batch_children_proc_ids);
        }
        if (batchChildrenList !== null) {
            for (let j = 0; j < batchChildrenList.length; ++j) {
                const batchObj = batchChildrenList[j];
                for(let i = 0; this.state.objToFiles[batchObj] && i < this.state.objToFiles[batchObj].length; i++){
                    tableData2.push({fileName: j + "-" + this.state.objToFiles[batchObj][i], key: objId + "-" + j + "-" + i});
                }
            }
        }

        for(let i = 0; this.state.objToFiles[objId] && i < this.state.objToFiles[objId].length; i++){
            tableData2.push({fileName: this.state.objToFiles[objId][i], key:i});
        }
        let toPrintFilesTable = (<Table columns={[{
            title: 'File Name',
            dataIndex: 'fileName',
            key: 'fileName',
        }]} dataSource={tableData2} pagination={tableData2.length < 10 ? false: true} scroll={{x:"max-content"}}></Table>);
        if(i === Object.keys(this.state.objs).length-1){ //final outputs
            this.outputs = tableData2;
        }

        return (<div key={i}>
            <h2>{toPrintStep + ": " + objId + " - " + toPrintName}</h2>
            <Collapse expandIconPosition={"left"}>
                <Panel header={<h4>{"Parameters   "} <Badge count={tableData.length} style={{ backgroundColor: '#fff', color: '#999', boxShadow: '0 0 0 1px #d9d9d9 inset' }}/></h4>} key="1">
                    <div>{toPrintParamTable}</div>
                </Panel>
                <Panel header={<h4>{"Inputs    "} <Badge count={tableData1.length} style={{ backgroundColor: '#fff', color: '#999', boxShadow: '0 0 0 1px #d9d9d9 inset' }}/></h4>} key="2">
                    <div>{toPrintInputsTable}</div>
                </Panel>
                <Panel header={<h4>{"Outputs    "} <Badge count={tableData2.length} style={{ backgroundColor: '#fff', color: '#999', boxShadow: '0 0 0 1px #d9d9d9 inset' }}/></h4>} key="3">
                    <div>{toPrintFilesTable}</div>
                </Panel>
            </Collapse>
            <br/>
            <br/>
        </div>);
    };

    handleChange = (event) => {
        if(event._targetInst.type === "button") return;

        let curInput = this.state.fieldsJson.inputObjs;
        curInput.value = (this.props.form.getFieldsValue()).inputObjs;
    };

    render(){
        const { getFieldDecorator } = this.props.form;

        return (<div>
            <Form onSubmit={this.handleSubmit} onBlur={this.handleChange}>
                <Form.Item label={"Objects used for pipeline"}>
                    {getFieldDecorator('inputObjs', {
                        rules: [{
                            required: true
                        }],
                        initialValue: this.props.objs
                    })(<Input/>)}
                </Form.Item>
                <Button htmlType="submit">Generate Information</Button>
                <div>{Object.keys(this.state.objs).sort().map(this.generateObjInfo)}</div>
            </Form>
        </div>);
    }
};

const WrappedInputsPane = Form.create({ name: 'normal_login' })(InputsPane);
export default WrappedInputsPane;
