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
import React, { Component } from "react";
import "antd/dist/antd.css";

import components from "./components/view_collector.js";


import {Tabs} from 'antd';


const TabPane = Tabs.TabPane;
class DataPreview extends Component {
    constructor(props){
        super(props);

        this.state = {
                        defaultActiveKey: '0'
                    };
    };
    componentDidMount(){
        if(typeof this.props.tabDataPreview === 'string'){
            this.props.tabs.forEach((tab,i)=>{
                if(tab.title === this.props.tabDataPreview){
                    this.setState({defaultActiveKey: `${i}`})
                }
            })
        }
    }

    componentDidUpdate(prevProps, prevState) {
        if(prevProps.tabDataPreview !== this.props.tabDataPreview){
           if(typeof this.props.tabDataPreview === 'string'){
                this.props.tabs.forEach((tab,i)=>{
                    if(tab.title === this.props.tabDataPreview){
                        this.setState({defaultActiveKey: `${i}`})
                    }
                })
           }
        }
    };

    createView = (viewDescr) => {
        var ActualComponent = components[viewDescr].component;
        return <ActualComponent ids={this.props.ids} />;
    };

    onTabClick = (key) => {
        this.setState({defaultActiveKey: `${key}`})
    }

    render(){
        return (
            <div className="dashboard-table-container">
                <Tabs
                    animated={false}
                    activeKey={this.props.tabs.length <  parseInt(this.state.defaultActiveKey) ? '0' : this.state.defaultActiveKey}
                    onTabClick={this.onTabClick}
                >
                    {
                        this.props.tabs.map((tab, i) => {
                            if(!components[tab]) return; // eslint-disable-line
                            return (
                                <TabPane tab={components[tab].title} key={i}>
                                    {this.createView(tab)}
                                </TabPane>
                            )
                        })
                    }
                </Tabs>
            </div>
        );
    };

}


 export default DataPreview;