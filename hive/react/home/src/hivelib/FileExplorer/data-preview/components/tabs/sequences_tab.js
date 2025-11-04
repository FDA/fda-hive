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

const seqUrl = "cmdr=seqList&out=num|id|len&ids=";

export default class SequencesTab extends Component{
    constructor(props){
        super(props);

        this.state={ reload: '' }
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
            <TableViewFetch
                ids={this.props.ids}
                url={seqUrl}
                idName={"ids"}
                raw={false}
                columnSizes={{"#": 100, "len": 150, "rpt": 100}}
                reload={this.state.reload}
            />
        </div>);
    }
}