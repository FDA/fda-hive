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
import { addURLParam , getPrefix } from './url_modal';

    // 1. takes object of parameters
    // 2. analyzes parameters and turns them into encoded string
    // 3. construct full url
    // 3. creates request
    // 4. returns response

export class RequestConstructor {
    constructor(props) {
        this.parameters = props.parameters;
        this.cgi = props.cgi ? props.cgi : null;
        this.url_parameters = props.url_parameters ? props.url_parameters : null;
    }

    startRequest = () => {
        if( (typeof this.parameters  !== "object" || this.parameters === null) && this.url_parameters === null ) {
            console.error(`this.parameters is not an object`);
            return;
        }
        if(typeof this.parameters  === "object" && this.parameters !== null){
            this.combineParameters();
        }
        this.constructFullURL();
    }

    handleFetch = async (fetchparametes = {}) => {
        this.startRequest()
        if(!this.full_url){
            return;
        }
        let response = await fetch(this.full_url,fetchparametes);
        return response;
    }

    // returns a string of all parameters
    // TODO: ??? if there is a param that is undefined should I just fail the request all together
    // or send a request without parameter ???
    // solution skip the parameter
    combineParameters = () =>{
        let param_keys = Object.keys(this.parameters);

        let url_parameters = '';

        param_keys.forEach((param,i) => {
            let param_value = this.handleParamValue(this.parameters[param])
            if(param_value !== undefined) url_parameters = addURLParam( url_parameters , param , param_value );
        })

        this.url_parameters = url_parameters;
        return url_parameters;
    }

    handleParamValue = (param_value) => {
        let type = typeof param_value;
        if( Array.isArray(param_value) === true ) type = 'array';

        switch(type){
            case 'bigint':
            case 'boolean':
            case 'number':
                return param_value.toString();
            case 'string':
                return param_value;
            case 'object':
                return JSON.stringify(param_value);
            case 'array':
                if(typeof param_value[0] !== 'object' || !Array.isArray( param_value[0] )  ) {
                    return param_value.join(',')
                }else{
                    return JSON.stringify(param_value);
                }
            default:
               console.error('Type of param_value can only be array, object, number, boolean, or bigint');
               return undefined;
        }
    }
    getFullURL = () => {
        this.combineParameters()
        this.constructFullURL()
        return this.full_url;
    }
    constructFullURL = () => {
        this.full_url = getPrefix(this.cgi) + this.url_parameters;
    }

}

export class CustomRequest extends RequestConstructor {
    constructor(props){
        super(props)
        if (sessionStorage.getItem("projectID")) {
            this.parameters.projectID = sessionStorage.getItem("projectID");
        }
    }

}