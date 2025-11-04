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
import { Table, Icon } from "antd";
import Papa from "papaparse";

import {getPrefix} from './../../../../modal/url_modal';

export default class DownloadFilesView extends React.Component{
    constructor(props){
        super(props);
        this.downloadUrl = "cmdr=propget&files=*.{csv,json,png,tsv,txt}&mode=csv&prop=none&ids=";

        this.state={ previewData: ''};
    };

    componentDidMount(){
        this.urlExchangeParameter = this.props.urlExchangeParameter;
        this.fetchData = this.props.fetchData;
        this.loadRawCheck = this.props.loadRawCheck;

        this.tableColumns =[
            {
                title: "Download",
                dataIndex:"download",
                render: (node, row, i) => (<a href={getPrefix() + "?cmd=objFile&ids=" + this.props.ids +"&filename=" + row.file}>
                        <Icon type="download"/>
                    </a>
                )
            },
            {
                title: "File Name",
                dataIndex: "file"
            }
        ];

        this.handleLoadData()
    }

    componentDidUpdate(prevProps, prevState){
        if(prevProps.ids !== this.props.ids){
             this.handleLoadData()
        }
        if (prevProps.data !== this.props.data) {
            this.setState((state, props)=>{
               let preview_data = this.prepData(props.data);
               return {
                        previewData: preview_data
                      }
            });
        }
        if(this.props.reload !== prevProps.reload){
           this.handleLoadData();
        }
    };

    handleLoadData = () => {
        let url = this.urlExchangeParameter(this.downloadUrl, "ids", this.props.ids);
        this.props.handleLoading(true);
        this.fetchData(url);
    }

    prepData = (data) => {
        let toReturn = [];
        let parsedData = Papa.parse(data, {header:true}).data;

        for(let i=0; i < parsedData.length; i++){
            if(parsedData[i].id === "") continue;
            let tmp = {};
            tmp.file = parsedData[i].value;
            tmp.download = parsedData[i].name;
            tmp.key = i;

            toReturn.push(tmp);
        }

        return toReturn;
    }

    render(){
        if (!this.state.previewData || this.state.previewData.length === 0){
            return(<h3>Empty Table :( </h3>)
        }
        else {
            return (
                    <Table
                        columns={this.tableColumns}
                        dataSource={this.state.previewData}
                        />
                   );
        }
    };
};