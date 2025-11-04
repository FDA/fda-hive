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
import {Form} from 'antd';

import ParentPane from "./parent_pane.js";

const formName = "extrainfo";

class ExtraInfoPane extends ParentPane{
    state = {fieldsJson: {
        usabilityDomain:{
            title: "Usability Domain",
            type:"string",
            required: "true"
        },
        keywords:{
            multi: "true",
            title:"Description Domain: Keywords",
            type: "string"
        },
        xref:{
            title: "Description Domain: XREF",
            children:{
                namespace:{
                    title: "Namespace",
                    type: "string"
                },
                xrefName:{
                    title: "Name",
                    type: "string"
                },
                ids:{
                    title: "ID",
                    type: "string"
                },
                accessTime:{
                    title: "Acess Time",
                    type: "date"
                }
            }
        },
        externalDataEndpoints:{
            title: "External Data Endpoints",
            multi: "true",
            hasDefault: "true",
            children:{
                externalDataEndpointsName:{
                    title: "Service Accessed",
                    required: "true",
                    type: "string"
                },
                url:{
                    title: "URL",
                    required: "true",
                    type: "string"
                }
            }
        },
        empiricalError:{
            title: "Empirical Error",
            required: "true",
            type: "string"
        },
        algorithmicError:{
            title: "Algorithmic Error",
            required: "true",
            type: "string"
        }
    }};

    handleSubmit = (event) =>{

    };
};



const WrappedExtraInfoForm = Form.create({ name: formName })(ExtraInfoPane);
export default WrappedExtraInfoForm;