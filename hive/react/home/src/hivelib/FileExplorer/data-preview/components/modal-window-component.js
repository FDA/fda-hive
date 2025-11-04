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
import React from "react"
import { Modal , Table , Progress , Tag , Breadcrumb , Button, Icon} from "antd"
import { timeElapsed , formatDatetime} from '../../../modal/time_modal'
import { CustomRequest as RequestConstructor } from "../../../modal/request_modal"
import{ TableButton } from "../../view/table-button_view"
import shortid from 'shortid'


export class ProgressModal extends React.Component {
    constructor( props ){
        super(props);
        this.infoColorMap = {
            'Trace':"blue",
            'Info':"blue",
            'Debug':"blue",
            'Warning':"orange",
            'Error':"red"
        };
        this.titleToData = {
            messages: 'mssgsData',
            parts: 'partsData'
        }
        this.modalData = {}
        this.state = {
            breadCrumb: null,
            activeTable: 'parts',
            loading: false,
            partsData: [],
            mssgsData: []
        }
    }

    partsColumns = [
        {
            title: 'Severity',
            dataIndex: 'severity',
            key: 'error',
            width: '100px',
            render: (text) => <Tag color={this.infoColorMap[text]} key={text}>{text}</Tag>,
            filters: [
                {
                  text: 'Trace',
                  value: 'Trace',
                },
                {
                  text: 'Info',
                  value: 'Info',
                },
                {
                  text: 'Debug',
                  value: 'Debug',
                },
                {
                  text: 'Warning',
                  value: 'Warning',
                },
                {
                  text: 'Error',
                  value: 'Error',
                }
            ],
            onFilter: (value, record) => {
                return record.severity.indexOf(value) === 0},
         },
        {
            title: 'Part',
            dataIndex: 'name',
            key: 'title',
            width: '100px',
            render: (text, column) => {
                return ( <span> {column.reqID} </span>);
            }
        },{
            title: 'Messages',
            dataIndex: 'mssgs',
            key: 'mssgs',
            width: '150px',
            render: (text, column) => {
                return  column.severity === "" || !column.severity ? null : ( <TableButton
                            onClick={()=> this.handleViewMessages(column) }
                            content="view"
                        />
                    );
            }
        },{
            title: 'Progress',
            dataIndex: 'progress100',
            key: 'progress100',
            render:(text, column) => {
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
            },
            filters: [
                {
                  text: 'Running',
                  value: 'Running',
                },
                {
                  text: 'Processing',
                  value: 'Processing',
                },
                {
                  text: 'Killed',
                  value: 'Killed',
                },
                {
                  text: 'ProgError',
                  value: 'ProgError',
                },
                {
                  text: 'SysError',
                  value: 'SysError',
                },
                {
                  text: 'Waiting',
                  value: 'Waiting',
                },
                {
                  text: 'Suspended',
                  value: 'Suspended',
                },
                {
                  text: 'Done',
                  value: 'Done',
                }
            ],
            onFilter: (value, record) => {
                console.log(value , record)
                return record.status.indexOf(value) === 0},
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
                }
    ];

    mssgsColumns = [
        {
            title: 'Severity',
            dataIndex: 'level',
            key: 'error',
            width: '100px',
            render: (text) => <Tag color={this.infoColorMap[text]} key={text}>{text}</Tag>,
            filters: [
                {
                  text: 'Trace',
                  value: 'Trace',
                },
                {
                  text: 'Info',
                  value: 'Info',
                },
                {
                    text: 'Debug',
                    value: 'Debug',
                },
                {
                    text: 'Warning',
                    value: 'Warning',
                },
                {
                    text: 'Error',
                    value: 'Error',
                },
              ],
              onFilter: (value, record) => record.level.indexOf(value) === 0
        },{
            title: 'Date',
            dataIndex: 'date',
            key: 'date',
            width: '100px',
            render: (text) => <span>{formatDatetime(text,true)}</span>,
        },{
            title: 'Message',
            dataIndex: 'msg',
            key: 'msg'
        }
    ];

    handleViewMessages = (column) => {
        this.setState((state) => {
            let breadCrumb = Object.assign({} , state.breadCrumb)
            breadCrumb['messages'] = `${column.name} (${column.reqID}) messages`
            return{
                activeTable: 'messages',
                loading: true,
                breadCrumb: breadCrumb
            }

        })

        const parameters = {
            cmdr: "-qpRQList",
            req: column.reqID,
            showmsg: 1,
            objs: this.props.curObjID
        }

        let request = new RequestConstructor({parameters});
        request.handleFetch()
        .then(response =>{
            if(response.status < 400 ){
                return response.json();
            }else{
                throw new Error('no responses')
            }
        }).then(data =>{
            data = data.Head
            let messages;
            if(data && !data.messages && data.parts){
                let part = data.parts.filter((part) => part.reqID == column.reqID) // eslint-disable-line eqeqeq
                part = part ? part[0] : null ;
                if(!part) {
                    throw new Error(`Part with reqID ${column.reqID} was not found.`)
                }
                if(part) messages = part.messages.reverse()
            }else if(data && data.messages && Array.isArray(data.messages)){
                messages = data.messages.reverse()
                messages.forEach((msg)=>{
                    msg['key'] = shortid.generate()
                })
            }else{
                throw new Error('No MESSAGES were found')
            }

            this.setState((state) => {
                return{
                    mssgsData: messages,
                    loading: false
                }
            })
        })
        .catch((error)=>{
            this.setState({
                    loading: false,
                    errorMsg: error
                })
        })
    }

    componentDidMount(){
        this.setState((state,props)=>{
            this.modalData = JSON.parse(JSON.stringify(props.modalData))
            return {
                breadCrumb:{
                    parts: props.processName,
                },
                activeTable: props.activeTable ?  props.activeTable : state.activeTable,
                modalData: this.modalData,
                loading: true
           }
        },this.handleLoadData)


    }
    componentDidUpdate(prevProps, prevState){
        //modalData
    }
    handleAction = (parameters) => {
        this.setState({loading:true})
        let request = new RequestConstructor({parameters});
        request.handleFetch().then(( response ) => {
            if(response.status < 400 && response.data !== 'unknown'){
                this.handleLoadData() // refresh
            } else {
                throw new Error(`Error. Could not ${parameters.act || 'handle action for'} ${parameters.req ? 'req' + parameters.req : 'item'}`)
            }
        })
        .catch((error)=>{
            alert(error)
        })

    }
    handleLoadData = () =>{
        if(this.state.activeTable === 'messages'){
            let params = {
                cmdr:'-qpRQList',
                showmsg:1,
                objs: this.props.curObjID,
                req: this.modalData.reqID
            }
            this.requestTableData(params)
        }else if( this.state.activeTable === 'parts'){
            let params = {
                cmdr: '-qpRQList',
                showreqs:1,
                reqObjID: this.modalData.objID,
                req: this.modalData.reqID,
                cnt: this.modalData.pieces
            }
            this.requestTableData(params)
        }
    }

    requestTableData = (parameters) =>{
        let request = new RequestConstructor({parameters});
        request.handleFetch()
        .then(( response ) => {
            if(response.status < 400 && response.data !== 'unknown'){
                return response.json()
            } else {
                throw new Error(`Error. Could not ${parameters.act || 'handle action for'} ${parameters.req ? 'req' + parameters.req : 'item'}`)
            }
        })
        .then((json) => {
            this.setState((state,props)=>{
                let obj = {loading:false}
                let data = json.Head[state.activeTable]
                if(!data && state.activeTable === 'messages'  ){
                    data = json.Head.parts
                                ? json.Head.parts.filter((part) => part.reqID == parameters.req)
                                : null
                    data = data ? data[0].messages : null
                    data = data ? data.reverse() : data
                }
                obj[this.titleToData[state.activeTable]] = data
                return obj
            })
        })
        .catch((error)=>{
            alert(error)
            this.setState({loading:false})
        })

    }

    onBreadCrumb = (table) => {
        this.setState((state,props)=>{
           return{ activeTable: table}
        })
    }

    handleBreadCrumb = (breadCrumb) => {
        //process
        //part
        if(!breadCrumb) return;
        let bc_keys = Object.keys(breadCrumb)
        return(<Breadcrumb>
                   { bc_keys.map( (key , i ) => {
                        return ( <Breadcrumb.Item style={{cursor:'pointer'}} onClick={ () =>  this.onBreadCrumb(key) }> {breadCrumb[key]}</Breadcrumb.Item>)
                    })}
                </Breadcrumb>
        )
    }
    getHeight =() => {
        return ( window.innerHeight - 350 )
    }
    filterCols = () =>{
        return this.partsColumns.filter((col) => {
            if((col.dataIndex === 'mssgs' || col.dataIndex === 'severity')&& (this.modalData.severity === "" || !this.modalData.severity)){
                return false
            }
            return true
        })
    }
    render(){
        return(
            <Modal
                title={this.handleBreadCrumb(this.state.breadCrumb)}
                visible={this.props.view}
                footer={null}
                onCancel={() => this.props.closeViewModal() }
                width={1000}
            >

                <div style={{display:'block'}} className="home">
                    <Button onClick={()=>{ // class home so btn has correct style
                        this.setState({loading:true})
                        this.handleLoadData()
                        }}>
                        <Icon type="reload" />
                        Refresh
                    </Button>
                </div>
                <Table
                    loading={this.state.loading}
                    columns={this.filterCols()}
                    rowKey={record => {
                        let key = `${record.grpID}${record.reqID}${record.objID}${record.svcID}` // generating random id each time Table renders, makes Table collapese on rerender
                        record['key'] = key
                        return key;
                    }}
                    size={'small'}
                    scroll={{ y: this.getHeight() }}
                    dataSource={this.state.partsData}
                    pagination={{defaultPageSize: 50 , pageSizeOptions: ['50' , '100', '150' , '200'] , size: "small" ,showSizeChanger: true}}
                    onExpand={this.onExpand}
                    style={{display:this.state.activeTable !== 'parts' ? 'none' : ''}}
                    key="table-1"
                />
                <Table
                    loading={this.state.loading}
                    columns={this.mssgsColumns}
                    rowKey={record => record.key}
                    size={'small'}
                    scroll={{ y: this.getHeight() }}
                    dataSource={this.state.mssgsData }
                    pagination={{defaultPageSize: 50 , pageSizeOptions: ['50' , '100', '150' , '200'] , size: "small" ,showSizeChanger: true}}
                    onExpand={this.onExpand}
                    style={{display:this.state.activeTable !== 'messages' ? 'none' : ''}}
                    locale={{emptyText:  this.state.errorMsg ? this.state.errorMsg : 'No Data' }}
                    key="table-2"
                />
            </Modal>
        )
    }
}