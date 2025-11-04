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
import React, { PureComponent } from "react";
import axios from "axios";
import Papa from "papaparse";

import { urlExchangeParameter , getPrefix } from './../../../../modal/url_modal';
const getDataTblUrl = "cmdr=-qpData&req=&grp=1&dname=_.csv&default=error:%20134730%20_.csv%20not%20found";

export default class BasicView extends PureComponent{
    constructor (props){
        super(props);
        this.urlPrefix = getPrefix();
        let url = '';
        this.state={options: props.options, ids: props.ids, url: url };
        this.urlExchangeParameter = urlExchangeParameter;
    };

    componentWillUnmount(){
        clearTimeout(this.rawRequest)
    }

    trim = (s) => {
        s = s.replace(/(^\s*)|(\s*$)/gi,"");
        s = s.replace(/[ ]{2,}/gi," ");
        s = s.replace(/\n /,"\n");
        return s;
    }

    fetchData = async (dataUrl, stateKey) => {
        if(!stateKey) stateKey = "data";
        axios
            .get(getPrefix() + '?' + dataUrl)
            .then (response => {
                let tmp = {};
                tmp[stateKey] = response.data;
                this.setState((state,props)=>{
                    return{
                       ...tmp,
                       loading:false
                    }

                });
            })
            .catch(error => alert(error));
    }

    loadRawCheck = async(loadUrl, stateKey) => { //once receive response, will just set the req in the state
        if(!stateKey) stateKey = "data";
        axios
            .get(getPrefix() + '?' + loadUrl)
            .then (response => {
                let parsedResponse = Papa.parse(response.data, {header:true});

                if(parsedResponse.data){
                    let curStat = parseInt(parsedResponse.data[0].stat);
                    if(curStat === 5){
                        //this.setState({tblReq:parsedResponse.data[0].reqID});
                        this.fetchData(this.urlExchangeParameter(getDataTblUrl, "req", parsedResponse.data[0].reqID), stateKey);
                    }
                    else if (curStat > 5){
                        let tmp = {};
                        tmp[stateKey] = "error";
                        this.setState((state,props)=>{
                            return{
                               ...tmp,
                               loading:false
                            }

                        });
                        console.log("server error returned");
                        return;
                    }
                    else{
                        this.rawRequest = setTimeout(()=> {this.loadRawCheck("cmdr=-qpRawCheck&req=" + parsedResponse.data[0].reqID, stateKey)}, 3000);
                    }
                }
            })
            .catch(error => alert(error));
    }

    render(){
        return (<div> Basic View </div>);
    };
};