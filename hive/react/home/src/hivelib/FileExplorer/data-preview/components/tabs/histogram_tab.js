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
import PlotlyGraphView from "../views/plotlygraph_view";
import TableView from "../views/table_view";
import { Button, Icon } from "antd";
import withFetch from "../controllers/fetch_controller";


const PlotlyGraphViewFetch = withFetch(PlotlyGraphView);
const TableViewFetch = withFetch(TableView);

const histogramCountsUrl =
  "cmdr=-qpRawSubmit&check=1&svc=tblqryx4&oper=list&resolution=200&objs=&tbl=.qc2.countsAtPositionTable.csv";

export default class HistogramTab extends Component {
  constructor(props){
    super(props)
    this.state = {
                    graphsStyle:{ display:  'block'},
                    tableStyle: { display:  'block'},
                    reload: ''
                  };
    this.lengthConfig = [
                          {
                            xcol: "position",
                            ycol: "count",
                            type: "lines",
                            responsive: true
                          }
                        ];
    this.qualityConfig = [
                          {
                            xcol: "position",
                            ycol: "quality",
                            type: "bar",
                            responsive: true
                          }
                            ];
  };

  visibleOrNot = (view, dataArr) => {
//      if (dataArr.length < 11){
//         this.setState({ graphsStyle:{ display:  'none'}})
//      }
//      else {
//         this.setState({ graphsStyle:{ display:  'block'}})
//      }
  }
  handleReload = (e) =>{
        this.setState((state,props)=> {
            return {reload: !state.reload}
        })
    }

  render() {

    return (
      <>
        <div style={{display:'block'}}>
            <Button key="refresht" onClick={this.handleReload}>
                  <Icon type="reload" />
                  Refresh
            </Button>
        </div>
        <TableViewFetch
          ids={this.props.ids}
          url={histogramCountsUrl}
          idName={"objs"}
          raw={true}
          callbackParsed={this.visibleOrNot}
          view={"table"}
          reload={this.state.reload}
        />
        <div style={this.state.graphsStyle}>
            <PlotlyGraphViewFetch
              componentInline={true}
              ids={this.props.ids}
              url={histogramCountsUrl}
              idName={"objs"}
              raw={true}
              config={this.lengthConfig}
              layout={{ title: "Length Count" , xaxis: {title: "Position"}, yaxis: {title: "Count"} }}
              reload={this.state.reload}
              //callbackParsed={this.visibleOrNot}
              view={"graph"}

            />
            <PlotlyGraphViewFetch
              componentInline={true}
              ids={this.props.ids}
              url={histogramCountsUrl}
              idName={"objs"}
              raw={true}
              config={this.qualityConfig}
              layout={{ title: "Quality" , xaxis: {title: "Position"}, yaxis: {title: "Quality"} }}
              reload={this.state.reload}
              //callbackParsed={this.visibleOrNot}
              view={"graph"}
            />
        </div>
      </>
    );
  }
}
