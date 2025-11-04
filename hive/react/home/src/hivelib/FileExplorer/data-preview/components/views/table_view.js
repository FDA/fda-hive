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
import { Table } from "antd";

import Papa from "papaparse";

export default class TableView extends React.Component{
    componentDidMount(){
        this.urlExchangeParameter = this.props.urlExchangeParameter;
        this.fetchData = this.props.fetchData;
        this.loadRawCheck = this.props.loadRawCheck;
        if(this.props.url){
           this.handleLoadData()
        }
    }

    componentDidUpdate(prevProps){
        if(this.props.ids !== prevProps.ids){
            this.handleLoadData()
        }
        if(prevProps.reload !== this.props.reload){
            this.handleLoadData()
        }
    }

    handleLoadData = () =>{
        let url = this.urlExchangeParameter(this.props.url, this.props.idName, this.props.ids);
        this.props.handleLoading(true)
        if(this.props.raw){
                this.loadRawCheck(url);
           }else{
                this.fetchData(url);
           }
    }

    parseTable(data){
        let toReturn = {columns:[], dataSource:[]};

        let dataArr = Papa.parse(data, {header:true}).data;

        if(this.props.callbackParsed)
            this.props.callbackParsed(this, dataArr);

        for(let i = 0; i < dataArr.length; i++){
            if(i === 0){//get headers only from the first row
                for(let key in dataArr[i]){
                    let colDescr = {title: key, dataIndex: key};
                    if(this.props.columnSizes && this.props.columnSizes[key])
                        colDescr.width = this.props.columnSizes[key];
                    if(this.props.callbackRendered){
                        if(this.props.callbackCol && this.props.callbackCol.regexp){
                            let reg = new RegExp (this.props.callbackCol.value);
                            if(reg.test(key))
                                colDescr.render = ((text) => this.props.callbackRendered(text, key));
                        }
                    }

                    toReturn.columns.push(colDescr);
                }
            }
            let curRow = dataArr[i];
            curRow.key = i;
            toReturn.dataSource.push(curRow);
        }

        return toReturn;
    }

    render(){
        if(this.visible === false) return (<div/>);
        if(!this.props.data) return (<h3>Empty Table :( </h3>);

        let parsedTable = this.parseTable(this.props.data);

        return (
                <Table
                    columns={parsedTable.columns}
                    dataSource={parsedTable.dataSource}
                    onRow={(record) => ({
                        onClick: (event) => {
                            if(this.props.selectCallback) this.props.selectCallback(record)
                    }})}
                    size={'small'}
                    pagination={{size:'small'}}
                />
                )
    }
}