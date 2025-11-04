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
import {Table, Tag, Progress , Icon } from "antd";

import { timeElapsed } from '../../../../modal/time_modal'
import { CustomRequest as RequestConstructor } from "../../../../modal/request_modal";
import{ TableButton } from "../../../view/table-button_view"

import { ProgressModal } from '../modal-window-component'

const infoColorMap = {
    'Trace':"blue",
    'Info':"blue",
    'Debug':"blue",
    'Warning':"orange",
    'Error':"red"
};

class ProgressView extends React.Component{
    mainColumns = [{
        title: 'Process Name',
        dataIndex: 'name',
        key: 'title',
        width: '37%',
        render: (text, column) => {
            return ( <span> {column.name + ": " + column.reqID} </span>);
        }
    },{
        title: 'Parts',
        dataIndex: 'pieces',
        key: 'pieces',
        render: (el,column) => {
            let parts = el && el === 1 ? `ID: ${column.objID}` : el > 1 ? `x${el}` : null;
            return (
                    <TableButton
                        onClick={()=> this.handleModal(column,'parts') }
                        style={{ display: !parts ? 'none' : ''}}
                        content={parts}
                    />
            );
        }
    },
    {
        title: 'Progress',
        dataIndex: 'progress100',
        key: 'progress100',
        render:(text, column) => {
            //"waiting", "processing", "running", "suspended", "done", "killed", "progError", "SysError"

            let status = column.status === "Running" || column.status === "Processing" ? 'active'
                       : column.status === "Killed" || column.status === "ProgError" || column.status === "SysError" ? 'exception'
                       : column.status === "Waiting" ||  column.status === "Suspended" ? 'normal'
                       : column.status === "Done" ? 'success'
                       : 'normal';

            let stroke_color = column.status === "Waiting" ||  column.status === "Suspended" ? '#faad14' : '';

            let show_precent = column.status === "Killed" || column.status === "ProgError" || column.status === "SysError" ?  `: ${text}%`
                             : '';

            return(<div style={{ width: 95, position: 'relative' }}>
                    <Progress
                        percent={text}
                        size="small"
                        status={status}
                        strokeColor={stroke_color}
                    />
                    <span style={{fontSize:'12px' , display: 'block' , marginTop: '-3px' , fontStyle: 'italic'}}>
                        {`${column.status}${show_precent}`}
                    </span>
                  </div>)
        }
    },{
        title: 'Severity',
        dataIndex: 'severity',
        key: 'error',
        render: (text,column) =>{
            if(  text && text !== '' && text != '0'){
                return (
                        <Tag
                            onClick={()=> this.handleModal(column, parseInt(column.pieces) <= 1 ? 'messages' : 'parts') }
                            style={{ cursor: 'pointer', boxShadow: '0px 3px 6px #c9c9c9'}}
                            color={infoColorMap[text]}
                            key={text}>{text}
                        </Tag>
                );

            }
            return(<Tag color={infoColorMap[text]} key={text}>{text}</Tag>)
        }
    },{
        title: 'Elapsed',
        dataIndex: 'execTime',
        key: 'elapsed',
        render: (text) => <span>{timeElapsed( parseInt(text) )}</span>
    },{
        title: 'Waited',
        dataIndex: 'waitTime',
        key: 'waited',
        render: (text) => <span>{timeElapsed( parseInt(text) )}</span>
    },{
        title: 'Object ID',
        dataIndex: 'objID',
        key: 'objID'
    },{
        title: 'Action',
        dataIndex: 'action',
        key: 'action',
        render: (text, column) => {
            let action;
            if(column.status === "Running") action = "Kill";
            else if (column.status === "Waiting") action = "Suspend";
            else if (column.status === "Suspended") action = "Resume";
            else if (column.status === "Killed" || column.status === "ProgError" || column.status === "SysError") action = "Run";
            else if (column.status === "Done") action = "Run";

            let params ={
                cmdr: "-qpReqSetAction",
                req: column.reqID,
                act: action,
                isGrp: column.reqID == column.grpID // eslint-disable-line
            }

            let icons = {
                Kill: 'stop' ,
                Suspend: 'pause' ,
                Resume: 'play-circle' ,
                Run: 'reload'
            }

            return (<TableButton
                        onClick={()=> this.handleAction(params)}
                        content={ (<><Icon style={{marginTop: '2px'}} type={icons[action]} /> {action} </> )    }
                        style={{display: !icons[action] ? 'none' : ''}}
                    />)
        }
    }];

    constructor(props){
        super(props);
        this.urlMainProgress = "cmdr=-qpGRList&showreqs=0&reqObjID=";
        this.activeModal = 'parts'
        this.state={
            data:[],
            req: 0,
            tablesVisible:{main: true, parts: false, messages: false },
            viewModal: false,
            processName: ''
        };

    };

    componentDidMount(){
        this.urlExchangeParameter = this.props.urlExchangeParameter;
        this.fetchData = this.props.fetchData;
        this.ids = this.props.ids
        this.handleLoadData()
    }

    handleModal = (column, name) =>{
        this.activeModal = name
        // there can be either one or multiple parts
        this.setState({ viewModal: true, modalData: column, processName: `${column.name} (${column.reqID})` })
    }

    handleAction = (parameters) => {
        let request = new RequestConstructor({parameters});
        request.handleFetch().then(( response ) => {
            if(response.status < 400 && response.data !== 'unknown'){
                this.handleLoadData()
            } else {
                alert(`Error. Could not ${parameters.act || 'handle action for'} ${parameters.req ? 'req' + parameters.req : 'item'}`)
            }
        })
    }

    componentDidUpdate(prevProps, prevState){
        if(prevProps.ids !== this.props.ids){
            this.ids = this.props.ids
            this.handleLoadData()
        }
        if(prevProps.reload !== this.props.reload){
            this.handleLoadData()
        }
        if(this.props.data !== prevProps.data){
            this.setState((state , props) => {
                let data = this.parseData(props.data);
                if (Array.isArray(data)){
                    data = data.map((el) => {
                        if(Object.keys(el).length > 0) return this.setKey(el);
                        return; // eslint-disable-line
                    })
                }
                return{ data: data}

            })
        }
    };

    setKey = (data) => {
        let key = `${data.reqID}-${data.objID}-${data.svcID}`
        data['key'] = key
        if(data.children && Array.isArray(data.children) && data.children.length > 0){
            data.children.forEach((child, i) => {
                if(Object.keys(child).length > 0){
                    if(`${child.reqID}-${child.objID}-${child.svcID}` === key ){
                        data.children.splice(i,1)
                    } else {
                        this.setKey(child)
                    }
                }
            });
            if(data.children.length === 0) data.children = null;
        }
        return data;
    }

    handleLoadData = () =>{
        let url = this.urlExchangeParameter(this.urlMainProgress, "reqObjID", this.props.ids);
        this.props.handleLoading(true)
        this.fetchData(url).then((response) => {
            if( !response || (response && response.status >= 400 )  || response.data === 'unknown'){
                this.setState((state,props) => {
                    return{ errorMsg: 'Error. Please refresh Progress' }
                })
            }
            this.props.handleLoading(false)
        });


    }

    parseData = (data) => {
        if(typeof data === "object"){
            return [data.Head];
        }
        return [{}];
    };

    closeViewModal = () =>{
        this.setState({ viewModal: false })
    }

    renderModal = () => (<ProgressModal
                            activeTable={this.activeModal}
                            modalData={this.state.modalData}
                            view={true}
                            processName={this.state.processName}
                            closeViewModal={() => this.closeViewModal()}
                            curObjID={this.props.ids}
                        />)
    filterCols = () => {
        if(!this.state.data || this.state.data.length === 0 ){
            return this.mainColumns
        }
        return this.mainColumns.filter((col) => {
            if(col.dataIndex === 'severity' && (this.state.data[0].severity === "" || !this.state.data[0].severity)){
                return false
            }
            return true
        })
    }

    render(){
        return (<div>
                    <Table
                        columns={this.filterCols()}
                        rowKey={record => record.key }
                        dataSource={this.state.data}
                        pagination={true}
                        scroll={{ y: 500 }}
                        locale={{emptyText:  !this.state.data.length ? this.state.errorMsg : 'No Data' }}
                    />
                    {this.state.viewModal ? this.renderModal() : ''}
               </div>
               );
    };
};

export default ProgressView;