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
import 'antd/dist/antd.css';
import './index.css';
import { Form, Icon, Input, Button, message } from 'antd';
import { RequestConstructor } from '../modal/request_modal';



export class ForgotForm extends PureComponent {
  constructor(props) {
    super(props);
    this.props = props;
    this.state = {
      fields: { }
    };
  }

  handleSubmit = e => {
    e.preventDefault();
    this.props.form.validateFields((err, values) => {
      if (!err) {
        document.getElementById("forgot_button").disabled = true;
        document.getElementById("loading_icon").style.visibility = "visible";

        let parameters = {cmdr: "forgot", login: values.email};
        let request = new RequestConstructor({parameters: parameters});
        request.handleFetch()
          .then((response) => response.text())
          .then((responseText) => {
            document.getElementById("loading_icon").style.visibility = "hidden";
            let error = responseText.includes("error");
            if (error) {
              throw new Error(responseText);
            }
            // if no errors
            message.success("You should be receiving an email shortly with how to reset your password", 10);
          })
          .catch((err) => {
            let text = err.message;
            if(text === "" || !text){
              text = "Unable to process request, please try again"
            }
            message.error(text, 10);

            document.getElementById("forgot_button").disabled = false;
          });
      }
    });
  }

  render() {
    return (
      <Form onSubmit={this.handleSubmit} className="login-form">
        <Form.Item>
          {this.props.form.getFieldDecorator('email', {
            rules: [{type: "email", message: "The input is not a valid Email"}, { required: true, message: 'Please input your Email' }],
          })(
            <Input
              prefix={<Icon type="user" style={{ color: 'rgba(0,0,0,.25)' }} />}
              placeholder="Email"
            />,
          )}
        </Form.Item>

        <Form.Item>
          <Button id="forgot_button" type="primary" htmlType="submit" className="login-form-button">
            Send Recovery Email
          </Button>
          <Icon id="loading_icon" type="loading" style={{visibility: "hidden"}}/>
        </Form.Item>
      </Form>
    );
  }
}