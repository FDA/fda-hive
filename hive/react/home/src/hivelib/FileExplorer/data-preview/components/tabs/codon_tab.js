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
import React, {Component} from "react";
import PlotlyGraphView from "../views/plotlygraph_view";
import {Button , Icon} from "antd";
import withFetch from "../controllers/fetch_controller";


const PlotlyGraphViewFetch = withFetch(PlotlyGraphView);

const codonUrl = "cmdr=objFile&filename=.qc2.codonQCTable.csv&ids=";
const complexityUrl = "cmdr=objFile&filename=.qc2.ComplexityTable.csv";

export default class CodonTab extends Component{
    constructor(props){
        super(props);
        this.state = {
            reload: ''
        }
        this.codonConfig = [{
            xcol: "Table Name",
            ycol: "Not Coding",
            type: "bar",
            name: "Not Coding"
        },{
            xcol: "Table Name",
            ycol: "Protein Coding",
            type: "bar",
            name: "Pretein Coding"
        }];

        this.complexityConfig=[{
            label: "Reads",
            val: "Count",
            type: "pie"
        }];

        this.layoutConfig = {
                              yaxis: {title: "Position",
                                      automargin: true
                                     },
                              xaxis: {automargin: true,
                                      title: { text: "Table Name",
                                               standoff: 90
                                             }
                                      },
                              title: "Codon Quality Control",
                              barmode: "stack" ,
                             }
    }

    handleReload = (e) =>{
        this.setState((state,props)=> {
            return {reload: !state.reload}
        })
    }

    render(){
        return (<>
                    <div style={{display:'block'}}>
                                <Button key="refresht" onClick={this.handleReload}>
                                      <Icon type="reload" />
                                      Refresh
                                </Button>
                    </div>
                    <PlotlyGraphViewFetch ids={this.props.ids}
                                     url={codonUrl}
                                     idName={"ids"}
                                     config={this.codonConfig}
                                     layout={this.layoutConfig}
                                     reload={this.state.reload}
                                     componentInline={true}
                                     />
                    <PlotlyGraphViewFetch ids={this.props.ids}
                                     url={complexityUrl}
                                     idName={"ids"}
                                     config={this.complexityConfig}
                                     layout={{title: "Complexity Status" }}
                                     reload={this.state.reload}
                                     componentInline={true}
                                     />
              </>);
    }
}