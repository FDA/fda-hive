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
import { Modal, Button , Form, Input, Alert } from 'antd';
import modals from '../../hivelib/modal/modal_collector';
const { TextArea } = Input;

const { data_modal, request_modal } = modals;
const {isTrueObject} = data_modal;
const { CustomRequest } = request_modal;

export class RequestToAddNetwork extends React.Component{
    constructor(props){
        super(props)
        this.state = {  }
        this.formItemLayout = {
            labelCol: { span: 8 },
            wrapperCol: { span: 14 },
        };
        this.selectedFiles = null;
    }

    componentDidMount = () => {
        this.setState((state,props) => {
            let okVisible = typeof props.okVisible === 'boolean' ? props.okVisible : true
            return {
                        visible: false,
                        okVisible,
                        submittedInfo: null
                   }
        });
    };
    showModal = () => {
        this.setState({
          visible: true,
        });
    };

    hideModal = () => {
        this.setState({
          visible: false,
        });
    };


    handleCancel = e => {
        // Clear selected items
        this.hideModal()

    };

    handleOpen = () => {
        this.setState({visible:true})
    }
    handleFormSubmit =(info) => {
        if(isTrueObject(info)){
            this.setState({submittedInfo:info})
        }else{
            this.setState({submittedInfo:{info: JSON.stringify(info)}})
        }

    }
    alertDescription =   ( <>please email HIVE ( <a class="link underline" href="mailto:hive@fda.hhs.gov?subject=HIVE%20Storage%20Location%20Network">click here</a> ) with your request</>)

    render(){
        return (
            <div style={{marginBottom: "20px"}}
            >
                <Button
                    icon={this.props.buttonIcon ? this.props.buttonIcon : ''}
                    type={typeof this.props.buttonType === 'string' ? this.props.buttonType : null}
                    disabled={this.props.disabled}
                    block={this.props.buttonBlock ? this.props.buttonBlock : false}
                    size={this.props.buttonSize ? this.props.buttonSize : null}
                    onClick={this.handleOpen}
                    shape={typeof this.props.buttonShape === 'string' ? this.props.buttonShape : null }
                >
                  {this.props.buttonName ? this.props.buttonName : ''}
                </Button>

                {   this.state.visible &&
                    <Modal
                      destroyOnClose={true}
                      title='Request to Add Location'
                      bodyStyle={{ height:'fit-content', padding: '25px'}}
                      style={{Width:'fit-content'}}
                      visible={this.state.visible}
                      footer={null}
                      onCancel={this.handleCancel}
                      okButtonProps={!this.state.okVisible ? {style:{display:'none'}} : ''}
                    >
                        <WrappedRequestForm submitted={(text)=>this.handleFormSubmit(text)}/>
                        {this.state.submittedInfo && Object.keys(this.state.submittedInfo).map((key,i) => <Alert
                                                                                                            key={i}
                                                                                                            message={this.state.submittedInfo[key]}
                                                                                                            description={key === 'error' ? this.alertDescription : ''}
                                                                                                            type={key}
                                                                                                            showIcon/>
                                                                                                )}
                    </Modal>
                }
            </div>
        )
    }
}

class RequestForm extends  React.Component{
    constructor(props){
        super(props)
        this.state = {  }
        this.formItemLayout = {
            labelCol: { span: 8 },
            wrapperCol: { span: 14 },
        };
        this.selectedFiles = null;
        const { getFieldDecorator } = this.props.form;
        this.getFieldDecorator = getFieldDecorator;
    }
    componentDidMount(){
        //! For future when we have proper RECORD VIEWER , for now leave fields as hard coded
        // let parameters = {
        //                   cmdr: 'propspec',
        //                   type: 'HIVE_storage_location_request',
        //                   mode: 'json'
        //                 }
        // let request = new RequestConstructor({parameters})
        // request.handleFetch()
    }
    handleSubmit =(e)=>{
        e.preventDefault();
        this.props.form.validateFieldsAndScroll((err, values) => {
            if (!err) {
                let parameters = {
                                  cmdr: 'propset',
                                  'prop.HIVE_storage_location_request._type': 'HIVE_storage_location_request',
                                  'prop.HIVE_storage_location_request.email': values.email,
                                  'prop.HIVE_storage_location_request.location_list': values.location_list
                                }
                let request = new CustomRequest({parameters})
                request.handleFetch()
                    .then(response => response.text())
                    .then(text => {
                        this.sendEmail(values , text.split('=')[1])
                    })
                    .catch(err =>{
                        this.props.submitted({error: 'Error accurred during submission process.'})
                    })
            }
        })
    }
    sendEmail = (values , requestnum) => {
        let mssg = `
            Hello,

            A request ${requestnum} to add location to HIVE Network has been submitted at ${window.location.host} by ${values.email}.
            Location requested to add to HIVE Network:
            ${values.location_list}
        `;
        let autoreply = `
            Dear HIVE User,

            Thank you for submitting a request ${requestnum}.
            Location requested to add to HIVE Network:
            ${values.location_list}
            You will be contacted shortly regarding next steps in the process by our team.

            Thank you,
            HIVE at the U.S. Food and Drug Administration (FDA)
        `
        const parameters = {
            cmdr: 'sendmail',
            message: mssg,
            email: values.email,
            subject: 'Request to Add Storage Location',
            autoreply: autoreply
        }
        let request = new CustomRequest({parameters})
        request.handleFetch()
            .then(response => response.text())
            .then(text => {
                if(text){
                    try{
                        let parsed = JSON.parse(text)
                        this.props.submitted(parsed)
                    }catch(e) {
                        this.props.submitted({success: text})
                    }
                }
            })
            .catch(err =>{
                this.props.submitted({error: 'Error accurred during submission process.'})
            })

    }
    render(){
        return(
            <Form {...this.formItemLayout} onSubmit={this.handleSubmit}>
                            <Form.Item label="HIVE E-mail">
                                {this.getFieldDecorator('email', {
                                    rules: [
                                    {
                                        type: 'email',
                                        message: 'The input is not valid E-mail!',
                                    },
                                    {
                                        required: true,
                                        message: 'Please input your E-mail!',
                                    },
                                    ],
                                })(<Input />)}
                            </Form.Item>
                            <Form.Item  key={"location_list"} label="Location List">
                                    {this.getFieldDecorator('location_list', {
                                        rules: [
                                            {
                                                required: true,
                                                message: 'Input location list'
                                            }
                                        ],
                                    })(<TextArea />)}
                                </Form.Item>
                                <Form.Item key={"request"} wrapperCol={{ span: 12, offset: 6 }}>
                                    <Button type="primary" htmlType="submit">
                                        Request to Add Location
                                    </Button>
                                </Form.Item>
                        </Form>
        )
    }

}

const WrappedRequestForm = Form.create({ name: 'request' })(RequestForm);
