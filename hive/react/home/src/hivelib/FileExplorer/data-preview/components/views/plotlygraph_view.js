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
import React from "react";
import Papa from "papaparse";

import createPlotlyComponent from 'react-plotly.js/factory';
import Plotly from 'plotly.js/dist/plotly-cartesian';

const PlotlyComponent = createPlotlyComponent(Plotly);

/*
    config is an array with:
    xcol = name of x-axies
    ycols = array of y-column names
    mode = type of graph (line, bar, scatter etc)
    type = title of graph
*/

export default class PlotlyGraphView extends React.Component{
    constructor(props){
        super(props);
        this.state = { url:null};
    };
    componentDidMount(){
        this.urlExchangeParameter = this.props.urlExchangeParameter;
        this.fetchData = this.props.fetchData;
        this.loadRawCheck = this.props.loadRawCheck;

        let url = this.props.url ? this.urlExchangeParameter(this.props.url, this.props.idName, this.props.ids) : null;
        this.setState((state,props) =>{
            return{
                url,
                data: props.dataToUse ? props.dataToUse : state.data,

                plotlyClass: ''
            }
        })
        if (!this.props.dataToUse && this.props.url ){
            this.props.handleLoading(true)
            if(this.props.raw){
                this.loadRawCheck(url);
            }
            else{
                this.fetchData(url);
            }
        }
    }

    componentDidUpdate(prevProps, prevState){
        let hasChanges = false;
        let loadRawCheck = false
        let fetchData = false;
        let plotlyClass = '';
        let data ;
        let url = this.props.url ? this.urlExchangeParameter(this.props.url, this.props.idName, this.props.ids) : null;
        if(prevProps.ids !== this.props.ids){
            hasChanges = true;

            data = "";
            if(this.props.raw && url){ loadRawCheck= true }
            else if(url){ fetchData = true }
        }

        if(this.props.dataToUse !== prevProps.dataToUse && this.props.dataToUse !== undefined){
            hasChanges = true;
            data = this.props.dataToUse;
        }

        if(this.props.url !== prevProps.url){
            hasChanges = true;
            url = this.props.url;
            data = this.props.url ? this.state.data : "";

            if(this.props.raw && this.props.url ){ loadRawCheck= true }
            else if(this.props.url){ fetchData = true }
        }

        if(this.props.data !== prevProps.data){
            hasChanges = true;

            if((this.props.data === "" || this.props.data === []) && !this.props.loading) plotlyClass = 'hive-plotly-nodata';
            if(this.props.data === "error" ) plotlyClass = 'hive-plotly-error';
        }

        if(this.props.loading !== prevProps.loading){
            hasChanges = true;
            if(this.props.loading){
                plotlyClass = ''
            }
            if(!this.props.loading){
                if(this.props.data === "" || this.props.data === []) plotlyClass = 'hive-plotly-nodata';
                if(this.props.data === "error" ) plotlyClass = 'hive-plotly-error';
            }
        }

        if(this.props.reload !== prevProps.reload){
            if(this.props.raw && url){ loadRawCheck= true }
            else if(url){ fetchData = true }
        }
        if(hasChanges){
            this.setState((state,props) => {
                let new_state = { plotlyClass: plotlyClass };

                if(url !== undefined) new_state.url = url;
                if(data !== undefined) new_state.data = data;

                return new_state;
            })
        }

        if(loadRawCheck || fetchData)this.props.handleLoading(true)
        if(loadRawCheck){
            this.loadRawCheck(url);
        }
        else if(fetchData){
            this.fetchData(url);
        }

    };

    prepareData = (dataToPrep) => {
        let toReturn = [];
        if(!dataToPrep){
            return toReturn;
        }
        if(dataToPrep === "error"){
            return toReturn;
        }
        let dataArr = Papa.parse (dataToPrep, {header:true});

        if(!dataArr.data) return;
        dataArr = dataArr.data;

        if(this.props.callbackParsed) {
            this.props.callbackParsed(this, dataArr);
        }

        if(this.props.config && this.props.config[0].type === "pie"){
            let curTrace = {type: "pie", values: [], labels: []};

            for(let i = 0; i < dataArr.length; i++){
                if(isNaN(parseFloat(dataArr[i][this.props.config[0].val]))) continue;

                curTrace.values.push(parseFloat(dataArr[i][this.props.config[0].val]));
                curTrace.labels.push(dataArr[i][this.props.config[0].label]);
            }
            toReturn.push(curTrace);
            return toReturn;
        }
        for(let i = 0; i < this.props.config.length; i++){
            let curTrace = {x:[], y:[]};
            for(let j = 0; j < dataArr.length; j++){
                curTrace.x.push(dataArr[j][this.props.config[i].xcol]);
            }
            for(let j = 0; j < dataArr.length; j++){
                curTrace.y.push(dataArr[j][this.props.config[i].ycol]);
            }

            curTrace.type = this.props.config[i].type;
            curTrace.name = this.props.config[i].name;

            toReturn.push(curTrace);
        }
        return toReturn;
    };

    render(){
        return (<div
                      className={this.props.spinnerWrapClass ? this.props.spinnerWrapClass : 'hive-inline-block'}
                >
                            <PlotlyComponent
                                className={this.state.plotlyClass}
                                data={this.prepareData(this.props.data)}
                                layout={this.props.layout}
                                />
                </div>);
    };
};
