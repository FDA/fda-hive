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
import React from 'react';
import {Form} from 'antd';

import ParentPane from "./parent_pane.js";

const formName = "provenance";

class ProvenancePane extends ParentPane{
    state = {fieldsJson: {
        bcoName:{
            title: "BCO Name",
            type:"string",
            required: "true"
        },
        version:{
            title: "Version",
            type: "string",
            pattern: /^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$/,
            message: (<p>Must adhere to semantic versioning e.g. 2.3.4. See <a href='https://semver.org' target='_blank' rel='noopener noreferrer'>semver.org</a> for more information</p>)
        },
        review:{
            title: "Review",
            type: "array",
            multi: "true",
            children:{
                date:{
                    title: "Date",
                    type: "date",
                    itemAddedRemove: "true",
                    removeAddedKey: "date-",
                    parentField: "review"
                },
                reviewer:{
                    title: "Reviewer",
                    component: "Contributor",
                    requiredIfAdd: "true"
                },
                reviewerAffiliation:{
                    title: "Affiliation",
                    type: "string"
                },
                reviewerEmail:{
                    title: "Email",
                    type: "email"
                },
                reviewerContribution:{
                    title: "Contribution",
                    type: "select",
                    mode:"multiple",
                    requiredIfAdd: "true",
                    options:["authoredBy", "contributedBy", "createdAt", "createdBy", "createdWith", "curatedBy",
                        "derivedFrom", "importedBy", "importedFrom", "providedBy", "retrievedBy", "retrievedFrom", "sourceAccessedBy"]
                },
                reviewerOrcid:{
                    title: "ORC ID",
                    type: "string"
                },
                reviewerComment:{
                    title: "Reviewer Comment",
                    type: "string"
                },
                status: {
                    title: "Status",
                    type: "string",
                    mode:"single",
                    requiredIfAdd: "true",
                    options:["unreviewed","in-review","approved","rejected","suspended"]
                }
            }
        },
        derivedFrom: {
            title: "Derived From",
            type: "string"
        },
        obsoleteAfter: {
            title: "Obsolete After",
            type: "date"
        },
        startTime: {
            title: "Embargo Start Time",
            type: "date"
        },
        endTime:{
            title: "Embargo End Time",
            type: "date"
        },
        contributors:{
            title: "Contributors",
            multi: "true",
            hasDefault: "true",
            required: false,
            children:{
                contributorName:{
                    title: "Contributor Name",
                    type:"string",
                    itemAddedRemove: "true",
                    removeAddedKey: "contributorName-",
                    parentField: "contributors",
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
                    title: "Contribution",
                    type: "select",
                    mode:"multiple",
                    required: "true",
                    options:["authoredBy", "contributedBy", "createdAt", "createdBy", "createdWith", "curatedBy",
                        "derivedFrom", "importedBy", "importedFrom", "providedBy", "retrievedBy", "retrievedFrom", "sourceAccessedBy"]
                },
                orcid:{
                    title: "ORC ID",
                    type: "string"
                }
            }
        },
        license:{
            title: "License",
            type: "string",
            required: "true"
        }
    }};

    handleSubmit = (event) =>{

    };
};



const WrappedProvenanceForm = Form.create({ name: formName })(ProvenancePane);
export default WrappedProvenanceForm;