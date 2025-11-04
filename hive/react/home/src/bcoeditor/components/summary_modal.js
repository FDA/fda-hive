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
import {Form, Checkbox, Modal} from 'antd';

class SummaryModal extends Component{
    state = {visible:false, typeInfo: {}};

    componentDidMount(){
        this.setState({typeInfo: this.props.vals, typeToChecked:{}});
        this.defaultChecked={};
        this.objToValues={};
    };

    componentDidUpdate(props, state){
        if(this.props.vals !== props.vals)
            this.setState({typeInfo: this.props.vals});
    }

    handleClose = (isOk) => {
        this.props.onClose();

        if(isOk){
            let checkedValues = Object.keys(this.state.typeToChecked).length !== 0 ? this.state.typeToChecked : this.defaultChecked; //all checked values

            this.props.handleOk(checkedValues, this.objToValues);
        }
    };

    onChange = (checkedValues, type) => {
        let ttc = Object.assign(this.state.typeToChecked);
        ttc[type] = checkedValues;
        this.setState({typeToChecked: ttc});
    };

    renderInfo = (type) => {
        let allAttr = this.state.typeInfo[type]._attributes;
        let typeOptions = [];

        let objValues;
        //check if any values need to be pre-checked
        //let's find object for this type
        for(const objNum in this.props.objFields){
            if(this.props.objFields[objNum]._type === type){ //found our object!
                objValues = this.props.objFields[objNum];
                this.objToValues[type] = objNum;
            }
        }

        let defaultChecked=[];
        //create an array with all checkboxes. avoid fields that start with underscore
        for (let key in allAttr){
            if(key.indexOf("_") === 0) continue;
            if(allAttr[key].role !== "state" && allAttr[key].role !== "input" && allAttr[key].role !== "output"){
                if(allAttr[key].default_value != objValues[key])
                    defaultChecked.push(key);
                typeOptions.push({ label: allAttr[key].title, value: key });
            }
        }

        if(defaultChecked.length > 0){
            this.defaultChecked[type] = defaultChecked;
        }

        return (<div>
            {"Parameters for " + this.state.typeInfo[type].title} <br/>
            <Checkbox.Group options={typeOptions} defaultValue={defaultChecked} onChange={(checkedValues) => this.onChange(checkedValues, type)} key={type} />
            <br/> <br/>
        </div>);
    };

    render(){
        return(
            <Modal visible={this.props.visible} onOk={() => this.handleClose(true)} onCancel={() => this.handleClose(false)}>
                {Object.keys(this.state.typeInfo).map((type) => {return this.renderInfo(type);})}
            </Modal>
        );
    }
};

export default SummaryModal;