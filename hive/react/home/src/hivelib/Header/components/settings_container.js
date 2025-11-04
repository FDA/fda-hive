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
import { EditableTable } from "./token_management";

import { Modal, Button } from 'antd';

export class SettingsContainer extends React.Component {
    constructor(props) {
        super(props);
        this.props = props;
        this.state = {
            visible: false
        }
    }
    componentDidMount(){
        this.props.header_pubsub.subscribe('modal_visiblity_toggled',this.toggleVisibility);
    }

    toggleVisibility = (value) => {
        this.setState({
            visible: value
        })
    }

    handleOk = e => {
        this.props.header_pubsub.publish('modal_visiblity_toggled', false);
    };

    handleCancel = e => {
        this.props.header_pubsub.publish('modal_visiblity_toggled', false);
    };

    render() {
        if(!this.state.visible){
            return '';
        }
        return (
            <>
                <Modal
                title="Settings"
                visible={this.state.visible}
                onOk={this.handleOk}
                onCancel={this.handleCancel}
                width={800}
                footer={[
                    <Button key="back" onClick={this.handleCancel}>
                        Close
                    </Button>,
                ]}
                >
                    <EditableTable/>
                </Modal>
            </>
        )
    }
}