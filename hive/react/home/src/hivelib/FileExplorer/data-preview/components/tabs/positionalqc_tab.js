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
import { Button , Icon} from "antd";

import PlotlyGraphView from "../views/plotlygraph_view";
import BasicView from "../views/basic_view";
import withFetch from "../controllers/fetch_controller";


const PlotlyGraphViewFetch = withFetch(PlotlyGraphView);


const lengthwiseUrl = "cmdr=-qpRawSubmit&check=1&svc=tblqryx4&oper=list&objs=&tbl=.qc2.sumPositionTable.csv&resolution=200";
const progressUrl = "cmdr=-qpRawCheck&showreqs=0&reqObjID=";

/*
launch qc url:cmdr=launchSvc&key=dna-qc&query=
progress urls: regular qp raw check
qc url:
    lengthwise count:qpbg_tblqryx4://.qc2.sumPositionTable.csv//resolution=200
        google graph line view (x-axis: positions, y's: count a,c,g,t)
    quality url: same as for count.
        visible only when genome
        algo google graph line view (x-axis: positions, y's: quality a,c,g,t)

*/
const lengthwiseConfig=[{
    xcol: "pos",
    ycol: "countA",
    type: "lines",
    name: "A"
},{
    xcol: "pos",
    ycol: "countC",
    type: "lines",
    name: "C"
},{
    xcol: "pos",
    ycol: "countG",
    type: "lines",
    name: "G"
},{
    xcol: "pos",
    ycol: "countT",
    type: "lines",
    name: "T"
}];

const qualityConfig=[{
    xcol: "pos",
    ycol: "qualityA",
    type: "lines",
    name: "A"
},{
    xcol: "pos",
    ycol: "qualityC",
    type: "lines",
    name: "C"
},{
    xcol: "pos",
    ycol: "qualityG",
    type: "lines",
    name: "G"
},{
    xcol: "pos",
    ycol: "qualityT",
    type: "lines",
    name: "T"
}];

export default class PositionalQCTab extends BasicView{
    constructor(props){
        super(props);

        this.state={
            progressData: "",
            graphData: "",
            reload: ''
        }

    }
    componentDidMount(){
       let progressLoad = this.urlExchangeParameter (progressUrl, "reqObjID", this.props.ids);
        this.fetchData(progressLoad, "progressData");

        let graphLoad = this.urlExchangeParameter(lengthwiseUrl, "objs", this.props.ids);
        this.loadRawCheck(graphLoad, "graphData");

        //load progress and lengthwise urls, will just pass data on to views if there is data
    }

    componentDidUpdate(prevProps, prevState){
        if(this.props.ids !== prevProps.ids){
            let progressLoad = this.urlExchangeParameter (progressUrl, "reqObjID", this.props.ids);
            this.fetchData(progressLoad, "progressData");

            let graphLoad = this.urlExchangeParameter(lengthwiseUrl, "objs", this.props.ids);
            this.loadRawCheck(graphLoad, "graphData");

            this.setState({progressData: "", graphData: "", progressVisible: false});
        }
    }

    onClickLaunch = (item, key, keyPath, domEvent) =>{
        let url = this.urlExchangeParameter("cmdr=launchSvc&key=dna-qc&query=", "query", this.props.ids);
        this.fetchData(url);
        this.setState({progressVisible:true});
    }
    handleReload = (e) =>{
        this.setState((state,props)=> {
            return {reload: !state.reload}
        })
    }

    render(){

        return (
                <>
                    <div style={{display:'block'}}>
                        <Button key="refresht" onClick={this.handleReload}>
                              <Icon type="reload" />
                              Refresh
                        </Button>
                    </div>
                    <PlotlyGraphViewFetch
                            componentInline={true}
                            ids={this.props.ids}
                            url={lengthwiseUrl}
                            idName={"objs"}
                            raw={true}
                            config={lengthwiseConfig}
                            dataToUse={this.state.graphData}
                            layout={{title: "Lengthwise position count", xaxis: {title: "Position"}, yaxis: {title: "Count"}}}
                            key={0}
                            reload={this.state.reload}
                            view="lengthwise"
                         />
                    <PlotlyGraphViewFetch
                              componentInline={true}
                              ids={this.props.ids}
                              url={lengthwiseUrl}
                              idName={"objs"}
                              raw={true}
                              config={qualityConfig}
                              dataToUse={this.state.graphData}
                              layout={{title: "Quality position count", xaxis: {title: "Position"}, yaxis: {title: "Quality"}}}
                              key={1}
                              reload={this.state.reload}
                              //callbackParsed={this.onGraphLoaded}
                              view="quality"
                           />
                </>
            );
    }
}