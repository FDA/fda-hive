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
import {Form, Input, Icon, Button, DatePicker, Select} from 'antd';
import moment from 'moment';

const {Option} = Select;

class ParentPane extends Component{
    marked = false;
    addElement = (fieldLoc, keyName) => {
        let currentElement = JSON.parse(JSON.stringify(fieldLoc[keyName])); // deep copy
        currentElement.multi = false;
        let curCount = 1;
        let curKey = keyName;
        if(fieldLoc[keyName].childCnt){
            curCount = fieldLoc[keyName].childCnt + 1;
        }
        if(currentElement.children){
            for(let key in currentElement.children){
                delete currentElement.children[key].value;
            }
        }
        curKey += "-" + curCount;
        currentElement.name = curKey;
        delete currentElement.actChildren;
        delete currentElement.value;
        currentElement = this.updateChildrenNames(currentElement, curCount);
        currentElement.itemKeyAdded = Date.now();

        if(!fieldLoc[keyName].actChildren)
            fieldLoc[keyName].actChildren = [];
        fieldLoc[keyName].actChildren.push(currentElement);
        fieldLoc[keyName].childCnt = curCount;

        return currentElement;
    };

    removeElement = (fieldName, keyName) => {
        let fieldsJson = this.state.fieldsJson;
        let fields = fieldsJson[fieldName].actChildren;

        fields.forEach( (x, index) => {
            if (x.children[keyName]) {
                fields.splice(index, 1);
            }
        });

        fieldsJson[fieldName].actChildren = fields;
        fieldsJson[fieldName].childCnt = fields.length;

        this.setState({fieldsJson: fieldsJson});
    }

    managePresets(){
        let currentFields = this.state.fieldsJson;
        let currentLoc;

        for(let key in this.props.vals){
            let curVal = this.props.vals[key];
            currentLoc = currentFields[key];

            if(curVal === undefined) continue;
            if(curVal instanceof Object){
                let totalKeys = 0;
                for(let valKey in curVal){
                    if(!isNaN(parseFloat(valKey))) {
                        let valval = curVal[valKey]; //second level
                        let currentElem = currentLoc;
                        let actKey;

                        if(totalKeys > 0) currentElem = this.addElement(currentFields, key);

                        if(valval instanceof Object){
                            for (let keyInVal in valval){
                                let valOfKeyInVal = valval[keyInVal];
                                actKey = keyInVal;
                                if(totalKeys > 0) actKey = keyInVal + "-" + totalKeys;
                                currentElem.children[actKey].value = valOfKeyInVal;
                            }
                        }
                        else currentElem.value = valval;

                        totalKeys++;
                    }
                    else currentLoc.children[valKey].value = curVal[valKey];
                }
            }
            else currentLoc.value = curVal;
        }
        this.marked = true;
    };

    updateChildrenNames(elem, num){
        let toReturn = {};

        for(let key in elem){
            if(key === "multi" || key === "title" || key === "type" || key === "required" || key === "name"){
                toReturn[key] = elem[key];
                continue;
            }
            else if (key === "children"){
                toReturn[key] = this.updateChildrenNames (elem[key], num);
                continue;
            }

            let nKey = key + "-" + num;
            toReturn[nKey] = elem[key];
            for(let subKey in toReturn[nKey]){
                if (subKey === "requiredIfAdd") {
                    toReturn[nKey]['required'] = elem[key][subKey];
                }
            }
        }
        return toReturn;
    }

    generateField = (keyName, i) => {
        let currentField;
        if(keyName || keyName === 0) currentField = this.currentFieldLoc[keyName];
        else currentField = this.currentFieldLoc;
        const { getFieldDecorator } = this.props.form;
        let currentValue = currentField.value;

        let multiBtn = <div/>;
        let allChildren = <div/>;
        if(currentField.multi){
            let tmpFieldLoc={};
            Object.assign(tmpFieldLoc, this.currentFieldLoc);
            multiBtn =  (<div><Button type="dashed"
                    onClick={() => {this.addElement(tmpFieldLoc, keyName); this.setState({fieldsJson: this.state.fieldsJson});}}
                    style={{ width: '100%' , "marginBottom":"35px"}}>
                    <Icon type="plus" /> Add {currentField.title}
            </Button></div>);
        }
        if(currentField.actChildren){
            let tmp = this.currentFieldLoc;
            this.currentFieldLoc = currentField.actChildren;
            allChildren = (<div>{currentField.actChildren.map((child, j) => this.generateField(j))}</div>);
            this.currentFieldLoc = tmp;
        }

        if(!isNaN(parseInt(keyName)) && currentField.name)
            keyName = currentField.name;

        let myChildren;
        if(currentField.children){
            let tmp = this.currentFieldLoc;
            let hideDiv = currentField.multi && !currentField.hasDefault;
            this.currentFieldLoc = currentField.children;
            myChildren = <div key={keyName} style={{display: hideDiv ? 'none':'block'}}>{Object.keys(this.currentFieldLoc).map(this.generateField)}</div>;
            this.currentFieldLoc = tmp;
        }
        else if (currentField.type === "date"){
            if(currentField.value === undefined) currentValue = null;
            else currentValue = moment(currentField.value, 'MM/DD/YYYY HH:mm:ss');

            myChildren = <DatePicker showTime='true' format={"MM/DD/YYYY HH:mm:ss"} onChange={(e) => {this.handleDateChange (e,keyName); this.setState({fieldsJson: this.state.fieldsJson});}}/>;
        }
        else if(currentField.options){
            myChildren = <Select mode={currentField.mode} placeholder="Please select value" onChange={(e) => {this.handleSelectChange (e,keyName)}}>
                {currentField.options.map((option, j) => <Option value={option} key={j}>{option}</Option>)}
            </Select>;
        }
        else myChildren = <Input/>;

        if ((currentField?.itemAddedRemove) && i === 0){
            let parentField = this.state.fieldsJson[currentField.parentField];

            if (keyName.indexOf(currentField.removeAddedKey)>-1){
                myChildren = <div>
                            <div style={{ width: '50%', display:'inline-block'}}>{myChildren}</div>
                            <div style={{ width: '50%', display:'inline-block', textAlign:'right'}}>
                                <Button type="dashed" shape="circle" onClick={() => {this.removeElement(currentField.parentField, keyName)}}><b>X</b></Button>
                            </div>
                        </div>;
            }
        }

        let component;
        if (currentField){
            if (currentField.type !== "array" && currentField.type !== "select" && currentField.type !== "date"){
                component = (<Form.Item label={currentField.title} style={{marginBottom:"0px"}} key={keyName}>
                    {getFieldDecorator(keyName, {
                        rules:  [
                            { type: currentField.type },
                            { required: currentField.required },
                            { pattern: currentField.pattern, message: currentField.message }
                        ],
                        initialValue:currentValue
                    })(myChildren)}
                </Form.Item>);
            }
            else if(currentField.type === "date" || currentField.type === "select"){
                component = (<Form.Item label={currentField.title} style={{marginBottom:"0px"}} key={keyName}>
                    {getFieldDecorator(keyName, {initialValue:currentValue, rules:[{required:currentField.required}]})(myChildren)}
                </Form.Item>);
            }
            else{ //when field has children inside
                let hideDiv = currentField.multi && !currentField.hasDefault;
                component = <div style={{display: hideDiv ? 'none':'block'}}><Form.Item label={currentField.title} required={currentField.required} style={{marginBottom:"0px"}} key={keyName}>
                    {myChildren}
                </Form.Item></div>;
            }
        }

        return (<div key={keyName}>{component}{allChildren}{multiBtn}</div>);
    };

    handleDateChange = (event, fieldName) => {
        let d;
        if (event){
            d = event._d;
        }

        let node = this.findInJson(this.state.fieldsJson, fieldName);
        if(node === null){
            console.log(fieldName + " field is not found");
            return;
        }

        if (d){
            node.value = d.toLocaleString();
        }
        else{
            node.value = d;
        }
    };

    handleSelectChange = (value, fieldName) => {
        let node = this.findInJson(this.state.fieldsJson, fieldName);
        if(node === null){
            console.log(fieldName + " field is not found");
            return;
        }
        node.value = value;
    };

    findInJson = (node, name) => {
        if(node.name === name) return node;

        for(let key in node){
            if(key === name || node[key].name === name) return node[key];

            if(node[key].children){
                let ret = this.findInJson(node[key].children, name);
                if(ret !== null) return ret;
            }
            if(node[key].actChildren){
                let ret = this.findInJson(node[key].actChildren, name);
                if(ret !== null) return ret;
            }
        }

        return null;
    };

    handleBlur = (event) =>{
        let field = event._targetInst;
        if(field.type === "button" || field.key === "selection") return;

        let fieldName = "", fieldVal;
        if(field.pendingProps["data-__field"]){
            fieldName = field.pendingProps["data-__field"].name;
            fieldVal = event.target.value;
        }
        else return;

        if(fieldVal === undefined) return;

        fieldName = fieldName.substring(fieldName.lastIndexOf("_")+1);

        let jsonLoc = this.findInJson (this.state.fieldsJson, fieldName);
        if(jsonLoc === null){
            console.log(fieldName + " field is not found");
            return;
        }
        jsonLoc.value = fieldVal;
    };

    render(){
        if (Object.keys(this.props.vals).length > 0 && this.marked !== true){
            this.managePresets(this.props.vals)
        }

        this.currentFieldLoc = this.state.fieldsJson;
        const formItemLayout = {
            labelCol: { xs: { span: 24 }, sm: { span: 8 }},
            wrapperCol: { xs: { span: 24 }, sm: { span: 16 }},
            layout: "vertical"
        };

        return(
            <Form {...formItemLayout} onBlur={this.handleBlur}>
                {Object.keys(this.currentFieldLoc).map(this.generateField)}
            </Form>
        );
    }
};

export default ParentPane;