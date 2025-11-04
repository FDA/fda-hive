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
import { Form , Input ,Checkbox } from "antd";
import Papa from "papaparse";

//import URLS from "./../../../../data/urls.js";
import {UNIXtimeConverter} from "./../../../../modal/time_modal";

export default class DetailView extends React.Component {
    constructor(props){
        super(props);

        this.propget = 'cmdr=propget&ids='
        this.propspec = 'cmdr=propspec&mode=csv&type='
        this.type = 'type';
        this.detail_data = 'detail_data';
        this.type_detail = 'type_detail';

        this.state = { rvData:[] };

        this.formItemLayout = {
           labelCol: {
            xs: { span: 20 },
            sm: { span: 7 },
          },
          wrapperCol: {
            xs: { span: 20},
            sm: { span: 13},
          },
        };
        this.tailFormItemLayout = {
          wrapperCol: {
            xs: {
              span: 20,
              offset: 0,
            },
            sm: {
              span:  13,
              offset:  7,
            },
          },
        };

    };

    componentDidMount(){
        this.urlExchangeParameter = this.props.urlExchangeParameter;
        this.fetchData = this.props.fetchData;
        this.loadRawCheck = this.props.loadRawCheck;

        this.handleGetData();

    }

    componentDidUpdate(prevProps, prevState){
        if(prevProps.ids !== this.props.ids){
            this.handleGetData()
        }

        if(prevProps[ this.detail_data ] !== this.props[ this.detail_data ] ){
            this.handleGetType();
        }

        if(prevProps[ this.type ] !== this.props[ this.type ]){
            this.getPropSpec(this.props[this.type]._type)
        }

        if(prevProps[this.type_detail] !== this.props[this.type_detail]){

            let all_fields = Papa.parse( this.props[this.type_detail] , {header:true}).data;

            let parsedData = Papa.parse( this.props[this.detail_data] , {header:true}).data;

            let rvData = []

            all_fields.forEach(( fld ) => {
                let fld_name = fld.name;
                if(fld.is_hidden_fg === '0'){
                    parsedData.forEach(( item ) => {
                        if(item.name === fld_name){
                            let new_fld = Object.assign(fld)
                            new_fld['value'] = item.value;
                            rvData.push(new_fld);
                        }
                    })
                }
            })

            this.setState({ rvData })

        }
    }

    handleGetType = () => {
        let url = this.urlExchangeParameter (this.propget, "ids", this.props.ids);
        url += '&mode=json&prop=_type';
        this.fetchData(url,this.type);
    }

    handleGetData = () => {
        let url = this.urlExchangeParameter (this.propget, "ids", this.props.ids);
        url += '&mode=csv';
        this.fetchData(url,this.detail_data);
    }

    getPropSpec = (type) => {
        let url = this.urlExchangeParameter (this.propspec, "type", type);
        this.fetchData(url,this.type_detail);
    }

    renderRV = () => {
        let rvData = this.state.rvData
        if(rvData.length === 0) return(<p> No Data :( </p>);

        let records = rvData.map((record , i )=>{
            return (this.handleField(record , i))
        })
        return (<Form {...this.tailFormItemLayout}>{records}</Form>);
    }

    handleField = (record , i) => {
        if(record.type === 'bool'){
            return(
                <Form.Item
                   key={i}
                   style={{marginBottom:'10px'}}
                >
                    <Checkbox
                        checked={Boolean(parseInt(record.value))}
                        readonly='true'
                    >
                    {record.title}
                    </Checkbox>
                </Form.Item>
            );
        }
        let value = record.value;
        if(record.type === 'datetime'){
            value = UNIXtimeConverter(value);
        }
        return (
            <Form.Item style={{marginBottom:'10px'}} {...this.formItemLayout} key={i} label={record.title}>
              <Input size="small" readonly='true' value={value}/>
            </Form.Item>
        )
    }

    render(){
         return(<>
                    {this.renderRV()}
                </>
                );
    }
}