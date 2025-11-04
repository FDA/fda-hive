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
import TableView from "../views/table_view";
import {Button, Icon} from "antd";
import withFetch from "../controllers/fetch_controller";

const TableViewFetch = withFetch(TableView);


const eigenvectorsUrl = "cmdr=-qpRawSubmit&check=1&svc=tblqryx4&oper=list&objs=&tbl=Vectors.csv&cols=0-15";
const contributorsUrl = "cmdr=-qpRawSubmit&check=1&svc=tblqryx4&oper=list&objs=&tbl=ContributorGraph.csv&cols=0-15";

export default class EigenvectorsTab extends Component{
    constructor(props){
        super(props);
        this.state={
            reload: ''
        }
    }
    mapping={};

    contributorCallback = (text, column) => {
        let num = parseFloat(text);
        let offset;
        let offsetText;
        if(!text || isNaN(num))
            return text;

        // return (<div style={{alignContent: "center", height: "28px"}}>
        //     <div  style={{backgroundColor: "black", width:"2px", margin: "0 1px 0 50%", height: "80%"}}/>
        //     <div> {text}</div>
        // </div>);

        offsetText = text.toString().length;
        let minMax = this.mapping[column];
        if(num < 0){
            //100 - offset
            //`calc(-${offset}% - ${offsetText*3}px - 20px)`
            offset = 30 * (num) / minMax.min;
            return (<div className="pipeWrapper">
                <div className="pipeContainer" style={{width:offset+"%", marginRight: offset + "%", backgroundColor:"#0000ff96"}}>
                    <div className="pipeText" style={{right: `${100 }%`,width: offsetText*9 + "px"}} >
                        {text}
                        <Icon type="caret-left" />
                    </div>
                </div>
            </div>);
        }
        else{
            offset = 30 * (num) / minMax.max;
            return (<div className="pipeWrapper">
                <div className="pipeContainer" style={{width:offset+"%", marginLeft: offset + "%", backgroundColor:"#ff000080"}}>
                    <div className="pipeText"  style={{right: offsetText*-9 + "px"}}><Icon type="caret-right" />{text}</div>
                </div>
            </div>);
        }
    };

    contributorsParsed = (tableComponent, parsedTbl) => {
        //will create mapping from each contribution coefficient col to max and min
        let reg = new RegExp (/Contribution Coefficient [0-9]/);
        console.log(parsedTbl);

        for(let i=0; i < parsedTbl.length; i++){
            for (let col in parsedTbl[i]){
                if(reg.test(col)){ //found a contributor col
                    let val = parseFloat(parsedTbl[i][col]);
                    let mappingObj;
                    if(!tableComponent.props.mapping[col]){
                        mappingObj={min:1, max:-1};
                    }
                    else{
                        mappingObj = tableComponent.props.mapping[col];
                    }
                    if(val < mappingObj.min)
                        mappingObj.min = val;
                    if(val > mappingObj.max)
                        mappingObj.max = val;

                    tableComponent.props.mapping[col] = mappingObj;
                }
            }
        }

    };

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
            <h2>Contributors</h2>
            <TableViewFetch
                ids={this.props.ids}
                url={contributorsUrl}
                idName={"objs"}
                raw={true}
                callbackRendered={this.contributorCallback} callbackCol={{regexp:true, value:/Contribution Coefficient [0-9]/}}
                callbackParsed={this.contributorsParsed}
                reload={this.state.reload}
                mapping={this.mapping}
           />
            <h2>Eigenvectors</h2>
            <TableViewFetch
                ids={this.props.ids}
                url={eigenvectorsUrl}
                idName={"objs"}
                raw={true}
                reload={this.state.reload}
            />
        </div>);
    }
}