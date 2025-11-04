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
import { Form, Icon, Input, Button, message } from 'antd';
import './index.css';
import "antd/dist/antd.css";
import { RequestConstructor } from '../modal/request_modal';


export class NormalLogin extends PureComponent {
  constructor(props) {
    super(props);
    this.props = props;
    this.state = {
      fields: { }
    };
    this.url = "";
  }

  handleSubmit = e => {
    e.preventDefault();
    this.props.form.validateFields((err, values) => {
      if (!err) {
        let parameters = {cmdr: "login", login: values.email, pswd: values.password};
        let request = new RequestConstructor({parameters: parameters});
        request.handleFetch()
          .then((response) => response.text())
          .then((responseText) => {
            let error = responseText.indexOf("error") > -1 || responseText.indexOf("err") > -1;
            if (error) {
              message.error(responseText, 10);
              throw new Error("There was an error");  // TODO better error message
            }
            if(!this.props.login){
              // if no errors
              console.log("No errors detected");
              this.url = document.URL;  // comment for testing
              const url_obj = new URL(this.url);
              let follow = url_obj.searchParams.get("follow");
              document.location.href = follow !== null ? "/cmd=" + follow : "/cmd=home";
            }
            this.props.login()
          })
          .catch((err) => {
            console.log("Unable to process request");  // TODO better error handling
            // if error, make user try again
            // only allow so many attempts?
          });
      }
    });
  }

  render() {
    return (
      <Form onSubmit={this.handleSubmit} className="login-form">
        <Form.Item>
          {this.props.form.getFieldDecorator('email', {
            rules: [{ required: true, message: 'Please input your Email or Username' }],
          })(
            <Input
              prefix={<Icon type="user" style={{ color: 'rgba(0,0,0,.25)' }} />}
              placeholder="Email or Username"
            />,
          )}
        </Form.Item>
        <Form.Item>
          {this.props.form.getFieldDecorator('password', {
            rules: [{ required: true, message: 'Please input your Password' }],
          })(
            <Input
              prefix={<Icon type="lock" style={{ color: 'rgba(0,0,0,.25)' }} />}
              type="password"
              placeholder="Password"
            />,
          )}
        </Form.Item>
        <Form.Item>
          <Button type="primary" htmlType="submit" className="login-form-button">
            Log in
          </Button>
        </Form.Item>
      </Form>
    );
  }
}