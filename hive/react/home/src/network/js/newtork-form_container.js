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
import React,{Component} from 'react';
import { Form ,Input , Switch , Select , Divider , Button, Tooltip} from 'antd';
import { InputFromHive } from '../../hivelib/CommonButtons/input-from-hive_button';
import { FileExplorerModal } from '../../hivelib/ModalWindows/file-explorer-modal_container';
import modals from '../../hivelib/modal/modal_collector';
const { sort_filter_modal ,csv_modal, request_modal , data_modal} = modals;
const { sortAlphAndCount } = sort_filter_modal;
const { ParseCSVtoArray } = csv_modal;
const { CustomRequest } = request_modal;
const { Option } = Select;
const { TextArea } = Input;
const {isTrueObject} = data_modal;

class RegistrationForm extends Component{
    constructor(props){
        super(props)
        this.props = props
        this.formItemLayout = {
            labelCol: { span: 8 },
            wrapperCol: { span: 14 },
        };
        const { getFieldDecorator } = this.props.form;
        this.getFieldDecorator = getFieldDecorator;
        this.allfields = {}
        this.state = {
            allfields: {},
            fields: {},
            submitOff: true,
            previewHidden: false,
        }
    }
    componentDidUpdate = (prevProps, prevState) => {
        if(prevProps.allfields !== this.props.allfields){
                if(isTrueObject(this.props.allfields)){
                     this.allfields = Object.assign({},this.props.allfields)
                }
        }
    }

    handleRequestProjects = () => {
        const parametersProjects = {
            cmdr: 'objQry',
            qry: "items=alloftype('^project$+', {project_status:['Manuscript Prep/Review','In-progress']});return items as csv(['_brief','project_status']);"
        }
        let requestProjects = new CustomRequest({parameters:parametersProjects})
        requestProjects.handleFetch()
        .then(response => response.text())
        .then( text => {
            let output = ParseCSVtoArray(text, ',');
            let output_header = output[0]
            output = sortAlphAndCount(output.slice(1), output[0].indexOf('_brief'));
            output = [output_header, ...output]
            let projectOptions = output && output.length && output.map((user,i) => {
                if( i > 0){
                    return (<Option value={ output[i][output[0].indexOf('id')] }>
                                <Tooltip placement="topLeft" title={`${output[i][output[0].indexOf('id')]} - ${output[i][output[0].indexOf('_brief')]} - ${output[i][output[0].indexOf('project_status')]}`}>
                                    { output[i][output[0].indexOf('_brief')] }
                                </Tooltip>
                            </Option> )
                } else {
                    return null;
                }
            })
            this.setState((state)=>{
                let fields = Object.assign( {} , state.fields);
                fields['submission_project'] = projectOptions;
                return {fields};
            })
        })
        .catch( error => {
            this.setState((state)=>{
                let fields = Object.assign( {} , state.fields);
                fields['submission_project'] = false;
                return {fields};
            })
        })
    }

    componentDidMount = () => {
        this.props.pubsub.subscribe('urladd', (eventData) => {
            this.props.form.setFieldsValue({
                st1_uri:  eventData.join('\n'),
            })
            this.setState({submitOff: eventData !== '' && eventData.length ? false : true})
        });

        this.handleRequestProjects()
    }

    handleSubmit = e => {
        e.preventDefault();
        this.props.form.validateFieldsAndScroll((err, values) => {
            if (!err) {
                const parameters = {
                    cmd: '-qpProcSubmit',
                    raw: 1,
                    svc: 'pipeline',
                }
                debugger;
                let fields = Object.assign( {} , this.allfields)
                fields['_type'] = 'svc-pipeline-download-dropbox';
                fields['submitter'] = 'pipeline&type=svc-pipeline-download-dropbox';
                let joined_values = Object.assign(fields, values )
                Object.keys(joined_values).forEach((key, i) => {
                    if(joined_values[key] !== undefined){
                        parameters[`prop.svc-pipeline-download-dropbox.${key}`] = joined_values[key]
                    }
                })

                let request = new CustomRequest({parameters})
                request.handleFetch()
                    .then(response => response.text())
                    .then(text => {
                        if(text){
                            let array_ids = text.split(',')
                            this.setState({archiverId:array_ids[1]})
                        }
                    })
                    .catch(err =>{
                        console.log(err)
                    })
            }
            else{
                console.log(err)
            }
        });
    };

    handleFileProcess = (val) => {
        if(parseInt(val) === 0){
            this.props.form.setFieldsValue({
                st2_run_screen:  false,
                st2_run_qc: false
            })
        }
    }
    recordValue = (key,value) =>{
        this.props.form.setFieldsValue({
            [key]:  value,
        })
    }

    render() {
        return (
            <>
            <Form {...this.formItemLayout} onSubmit={this.handleSubmit}>
                <Form.Item label="Name" key={"name"} hasFeedback>
                    {this.getFieldDecorator('name', {
                        rules: [
                        {
                            message: 'Provide an identifiable name for the process.',
                        },
                        {
                            required: false,
                        },
                        ],
                    })(<Input />)}
                </Form.Item>
                <Form.Item  key={"st1_uri"} hidden label="URLs and/or Identifiers">
                    {this.getFieldDecorator('st1_uri', {
                        rules: [
                        {
                            message: 'List URLs, numeric identifiers or accessions, one per line. Use http://, ftp:// prefix in URLs. If the value contains ", double it and surround by quotes.',
                        },
                        {
                            required: true,
                            message: 'Please select items from the tree'
                        },
                        ],
                    })(<TextArea />)}
                </Form.Item>
                <Divider>Processing</Divider>
                <Form.Item key={"st2_dissect"} label="Automatically process file(s)" hasFeedback>
                    {this.getFieldDecorator('st2_dissect', {
                        rules: [{ required: false}],
                        initialValue: '0'
                    })(
                        <Select onChange={(e)=> this.handleFileProcess(e)}>
                            <Option value="-1">All</Option>
                            <Option value="0">No processing. indexing, screenning, etc</Option>
                            <Option value="1">Top level only</Option>
                            <Option value="2">2 levels only</Option>
                        </Select>,
                    )}
                </Form.Item>
                <Form.Item key={"st2_run_screen"} label="Screen">
                    { this.getFieldDecorator('st2_run_screen', { valuePropName: 'checked', initialValue: false  })(<Switch />)}
                </Form.Item>
                <Form.Item key={"st2_run_qc"} label="Quality Control">
                    { this.getFieldDecorator('st2_run_qc', { valuePropName: 'checked', initialValue: false  })(<Switch />)}
                </Form.Item>
                <Divider>System</Divider>
                <Form.Item key={"submission_project"} hidden={!this.state.fields.submission_project} label="Project" hasFeedback>
                    {this.getFieldDecorator('submission_project', {
                        rules: [{ required: false}],
                    })(
                        <Select >
                            {this.state.fields.submission_project}
                        </Select>
                    )}
                </Form.Item>
                <Form.Item key={"folder"}  label="Destination Folder">
                    {this.getFieldDecorator('folder', {
                            rules: [{ required: false}],
                        })( <InputFromHive
                                       recordValue={(value) => this.recordValue('folder', value)}
                                       show='path'
                                       size={'default'}
                                       buttonName="Choose"
                                       modalOkText="SELECT"
                                       modalSingleSelect={true}
                                       recordTreeSelect={true}
                                       value={'this.state.initialFolderId'}
                            />)
                    }
                </Form.Item>
                <Form.Item key={"isPostponed"} label="Postpone execution">
                    { this.getFieldDecorator('isPostponed', { valuePropName: 'checked', initialValue: false })(<Switch />)}
                </Form.Item>
                <Form.Item key={"reqPriority"} label="Priority of Submission" hasFeedback>
                    {this.getFieldDecorator('reqPriority', {
                        rules: [{ required: false}],
                        initialValue: "0"
                    })(
                        <Select >
                            <Option value="0">normal - execute ASAP in collaborative environment</Option>
                            <Option value="100">polite submission - let other users small computations to squeeze through</Option>
                            <Option value="1000">polite submission - let other users small computations to squeeze through</Option>
                            <Option value="1000000">extremely nice - execute only when most computers are not busy</Option>
                        </Select>
                    )}
                </Form.Item>
                <Form.Item key={"submit"} wrapperCol={{ span: 12, offset: 6 }}>
                    <Button type="primary" htmlType="submit" disabled={this.state.submitOff}>
                        Import Selected Resources
                    </Button>
                </Form.Item>
            </Form>
            <FileExplorerModal
                                    buttonType={'primary'}
                                    buttonBlock={true}
                                    disabled={!this.state.archiverId}
                                    hasDataPreview={true}
                                    findObject={{_id:this.state.archiverId, _type:"svc-pipeline-download-dropbox", _js_component: ['progress'] }}
                                    customPreview={null}
                                    buttonName='Preview Progress'
                                    okVisible={false}
                                    cancelText={'Close'}
                                    hidden={this.state.previewHidden}
                         />
            </>
        );
    }
}

const WrappedRegistrationForm = Form.create({ name: 'register' })(RegistrationForm);

export class NetworkForm extends Component{
    constructor(props){
        super(props)
        this.props = props;
        this.state = {
            fields: true
        }
        this.needed_fields = ['submitter','scissors','_type','st1_uri','st1_submitter','st1_onUserBehalf' ,
        'st1_splitOnFrontEnd' ,'st1_svc' , 'st1_type' , 'st2_dissect' ,
        'st2_run_screen', 'st2_run_qc' , 'st2_inputFile' , 'st2_onUserBehalf', 'st2_datasource',
        'st2_svc' , 'st2_type' , 'random_seed' , 'slice', 'nrepeat' , 'submission_project' , 'folder' ,
        'isPostponed' , 'reqPriority' , 'split' , 'splitOnFrontEnd' ]
    }
    componentDidMount(){
        const parameters = {
            cmdr: 'propspec',
            type: 'svc-pipeline-download-dropbox',
        }
        let request = new CustomRequest({parameters})
        request.handleFetch()
            .then( response => response.text())
            .then( text => {
                let output = ParseCSVtoArray(text, ',');
                let needed_fields = [...this.needed_fields]
                let fields = {}
                for(let i = 1; i < output.length; i++) {
                    let name_index = output[0].indexOf('name');
                    let default_value_index = output[0].indexOf('default_value');
                    let nf_index = needed_fields.indexOf(output[i][name_index])
                    if( nf_index > -1){
                        needed_fields.splice(nf_index , 1 )
                        fields[output[i][name_index]] = output[i][default_value_index]
                    }
                }
                needed_fields.forEach(item =>{
                    fields[item] = ''
                } )
                this.setState({
                    fields:fields
                })
            })
            .catch( error =>{
                console.log(error)
            })
    }

    render(){
        return(
            <>
              <WrappedRegistrationForm key={'registration-form'} pubsub={this.props.pubsub} allfields={this.state.fields}/>
            </>
        )
    }
}

// { this.props.fields.map((field) => {

//     return(
//         <Form.Item label={field.title}>
//             {getFieldDecorator( field.name , {
//             rules: [
//                 {
//                     type: field.name,
//                     message: field.description,
//                 },
//                 {
//                     required: !parseInt(field.is_optional_fg),
//                 },
//             ],
//             })(<Input />)}
//         </Form.Item>
//     )

// }) }