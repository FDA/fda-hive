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
import {Form, Input, Select} from 'antd';

const { Option } = Select;


class Contributor extends Component{
    fieldsJson = {
        contributorName:{
            title: "Contributor Name",
            type:"string",
            required: "true"
        },
        affiliation:{
            title: "Affiliation",
            type: "string"
        },
        email:{
            title: "Email",
            type: "email"
        },
        contribution:{
            title: "Affiliation",
            type: "select",
            required: "true",
            options:["authoredBy", "contributedBy", "createdAt", "createdBy", "createdWith", "curatedBy",
                "derivedFrom", "importedBy", "importedFrom", "providedBy", "retrievedBy", "retrievedFrom", "sourceAccessedBy"]
        },
        orcid:{
            title: "ORC ID",
            type: "string"
        }
    };

    handleBlur = (event) => {
        let field = event._targetInst;
        if(field.type === "button") return;

        let fieldName = field.pendingProps.id;
        let fieldVal = event.target.value;

        fieldName = fieldName.substring(fieldName.lastIndexOf("_")+1);

        console.log(fieldName + " has value " + fieldVal);
    };

    generateField = (keyName, i) => {
        let currentField = this.fieldsJson[keyName];
        let selectOptions = (<Input/>);
        let getFieldDecorator = this.props.fieldDecorator;

        if(currentField.options){
            selectOptions = <Select placeholder="Please select contribution">
                {currentField.options.map((option, i) => <Option value={option}>{option}</Option>)}
            </Select>;
        }

        return (<Form.Item label={currentField.title}>
                {getFieldDecorator(keyName, {
                    rules: [
                        { type: currentField.type !== "array" ?  currentField.type : "" },
                        { required: currentField.required}
                    ]
                })(selectOptions)}
            </Form.Item>);
    };

    render(){
        return(<div>
            <Form.Item label={"Contributor"} onBlur={this.handleBlur}>
                {Object.keys(this.fieldsJson).map(this.generateField)}
            </Form.Item>
        </div>);
    }
};

export default Contributor;