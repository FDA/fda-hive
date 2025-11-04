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
import React, {PureComponent} from "react";

import TableView from "../views/table_view";
import PlotlyGraphView from "../views/plotlygraph_view";

import { Button , Icon} from "antd";
import withFetch from "../controllers/fetch_controller";

const PlotlyGraphViewFetch = withFetch(PlotlyGraphView);
const TableViewFetch = withFetch(TableView);

const hitlistUrl = "cmdr=alCount&objLink=parent_proc_ids&start=0&cnt=50";
const graphUrl = "cmdr=-qpRawSubmit&check=1&svc=tblqryx4&oper=list&cols=1-3,8-19,22-27&hdr=1&minmaxCols=3-15&minmaxMainCol=5&abscissaCol=0&resolution=200";
const tqs = [{
    op: "load-SNPprofile",
    arg:{
        obj: "",
        sub: "",
        autoAddMissingRows:true
    }
}];

const graphConfig=[{
    xcol: "Position",
    ycol: "Count Forward",
    type: "lines",
    name: "Count Forward"
},{
    xcol: "Position",
    ycol: "Count Reverse",
    type: "lines",
    name: "Count Reverse"
}];

export default class ProfileTab extends PureComponent{
    state = {
        graphUrl: null,
        tbl: "SNPprofile-0.csv",
        layout: {title: 'Select Item from Table Above to Preview'},
        reload: ''

    };

    componentDidUpdate(prevProps){
        if(this.props.ids !== prevProps.ids){
            this.setState((state,props)=>{
                return {
                        graphUrl: null,
                        layout: {title: 'Select Item from Table Above to Preview'},
                        ids: props.ids
                        }

            });
        }
    }

    selectTableCallback = (rowSelected) => {
        console.log(rowSelected);
        //Reference
        let title = rowSelected.hasOwnProperty('Reference') ? rowSelected['Reference'] : this.state.layout.title;
        let nTqs = tqs;
        nTqs[0].arg.obj = this.props.ids;
        nTqs[0].arg.sub = rowSelected.id;
        console.log(nTqs)
        let nUrl = graphUrl + "&tqs=" + encodeURI(JSON.stringify(nTqs));
        this.setState({graphUrl: nUrl, tbl: "SNPprofile-" + rowSelected.id + ".csv",  loading: true , layout: {title: this.titleAdjustment(title)}});
    };

    titleAdjustment = (str) => {

        if(str.length < 78) return str;

        let title = '';

        let words = str.split(' ');
        let count = 0;
        words.forEach((word) => {
            if(count + word.length > 78){
                title += ' <br> ';
                count = 0
            }
            title = `${title + word} `
            count = count + word.length + 1
        })

        return title;
    }
    handleReload = (e) =>{
        this.setState((state,props)=> {
            return {reload: !state.reload}
        })
    }

    render(){
        return (<div>
                    <div style={{display:'block'}}>
                        <Button key="refresht" onClick={this.handleReload}>
                              <Icon type="reload" />
                              Refresh
                        </Button>
                    </div>
                    <h2>Hitlist</h2>
                    <p>Select item in table to preview graph.</p>
                    <TableViewFetch
                            ids={this.props.ids}
                            url={hitlistUrl}
                            idName={"objs"}
                            raw={false}
                            selectCallback={this.selectTableCallback}
                            reload={this.state.reload}
                    />
                    <PlotlyGraphViewFetch
                            componentInline={true}
                            ids={this.state.ids}
                            url={this.state.graphUrl}
                            layout={this.state.layout}
                            raw={true}
                            config={graphConfig}
                            reload={this.state.reload}
                    />
        </div>);
    }
}