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
import React, { PureComponent } from 'react';
import { Modal, Form } from 'antd';
import './index.css';
import "antd/dist/antd.css";
import { NormalLogin } from './normal_login';
import { SSOLogin } from './sso_login';
import { ForgotForm } from './forgot';
import { getPrefixPlain } from '../modal/url_modal';

const WrappedNormalLoginForm = Form.create({ name: 'normal_login' })(NormalLogin);
const WrappedForgotForm = Form.create({ name: 'forgot_password' })(ForgotForm);

export class LoginContainer extends PureComponent {
  constructor(props) {
    super(props);
    this.props = props
    this.state = {
      fields: { },
      modal_visible: false
    };
  }

  showModal = () => {
    this.setState({
      modal_visible: true,
    });
  };

  handleModalCancel = e => {
    this.setState({
      modal_visible: false,
    });
    var elementExists = document.getElementById("forgot_button");
    if (elementExists) {
      elementExists.disabled = false;
    }
  };

  render() {
    return(
        <>
          <h1>Please Login to Continue</h1>
          <WrappedNormalLoginForm login={this.props.login}/>
          <SSOLogin />

          <br></br>

          <div className="login-form">
            Or <a href={`${getPrefixPlain}/r/request-account`}>request an account!</a>
            <span className='sub' onClick={this.showModal}>Forgot Password</span>
          </div>

          <Modal
            title="Forgot Password?"
            visible={this.state.modal_visible}
            footer={[ ]}
            onCancel={this.handleModalCancel}
          >
            <WrappedForgotForm/>
          </Modal>
        </>
    )
  }
}