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
import { CookieConstructor } from '../controller/cookie_controller';

export class ChangePassword extends PureComponent {
    constructor(props) {
      super(props);
      this.state = {
        confirmDirty: false,
        is_reset: false,
      };
      this.info_from_url= {
        login: null,
        pswd: null,
        x: null
      };
      this.url = null;  // comment for testing
      // this.url = "https://comb.fda.gov/usr/hannah.howell/dna.cgi?cmdr=pswdSet&login=hannah.howell@fda.hhs.gov&pswd=186826127&x=1635722190";  // for testing expired
      // this.url = "https://comb.fda.gov/usr/hannah.howell/dna.cgi?cmdr=pswdSet&login=hannah.howell@fda.hhs.gov&pswd=186826127&x=1672532190";  // for testing unexpired
      // this.url = "https://comb.fda.gov/usr/hannah.howell/dna.cgi?cmdr=pswdSet&login=hannah.howell@fda.hhs.gov";  // for testing
    }

    componentDidMount = () => {
      this.url = document.URL;  // comment for testing
      const url_obj = new URL(this.url);
      for (const obj in this.info_from_url) {
        this.info_from_url[obj] = url_obj.searchParams.get(obj);
      }

      let now = new Date();
      if (this.info_from_url.x !== null && (Math.floor(this.info_from_url.x) < Math.floor(now)/1000)) {
        // x is expired
        console.log("Expired request. Redirecting...");
        // redirect to forgot page
        document.location.href = "/cmd=forgot";
        return;
      }

      if (this.info_from_url.login === null) {
        let cookie_controller = new CookieConstructor();
        let login = cookie_controller.handleGetCookie("last_login");
        if (login !== "") {
          this.info_from_url.login = login;
        }
      }

      if (this.info_from_url.pswd !== null && this.info_from_url.x !== null) {
        this.setState({ is_reset: true });
      }
    }

    validateToNextPassword = (rule, value, callback) => {
      const { form } = this.props;
      if (value && this.state.confirmDirty) {
        form.validateFields(['confirm'], { force: true });
      }
      else if (value && !(/^(?=.{8,}$)(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9]).*$/).test(value)) {
        callback('Your password does not meet the requirements');
      }
      callback();
    };

    compareToFirstPassword = (rule, value, callback) => {
      const { form } = this.props;
      if (value && value !== form.getFieldValue('new_pass1')) {
        callback('Your password does not match');
      } else {
        callback();
      }
    };

    handleSubmit = e => {
      e.preventDefault();
      this.props.form.validateFields((err, values) => {
        if (!err) {
          let parameters = {
            cmdr: "pswdChange",
            login: this.info_from_url.login,
            newpass1: values.new_pass1,
            newpass2: values.new_pass2,
          };

          if (this.state.is_reset) {
            parameters["pswd"] = this.info_from_url.pswd;
            parameters["x"] = this.info_from_url.x;
          }
          else {
            parameters["pswd"] =  values.current_pass;
          }

          let request = new RequestConstructor({parameters: parameters});
          request.handleFetch()
            .then((response) => response.text())
            .then((responseText) => {
              let error = responseText.indexOf("error") > -1 || responseText.indexOf("err") > -1;
              if (error) {
                message.error(responseText, 10);
                throw new Error("There was an error");  // TODO better error message
              }
              // if no errors
              console.log("Your password has been changed");
              message.success("Your password has been successfully changed", 10);
              // redirect to login page
              document.location.href = "/cmd=login";
            })
            .catch((err) => {
              console.log("Unable to process request", err);  // TODO better error handling
            });
        }
      });
    }

    render() {
      return (
        <Form onSubmit={this.handleSubmit} className="password-form">
          { !this.state.is_reset ?
            <Form.Item>
              {this.props.form.getFieldDecorator('current_pass', {
                rules: [{ required: true, message: 'Please input your current password' }],
              })(
                <Input
                  prefix={<Icon type="lock" style={{ color: 'rgba(0,0,0,.25)' }} />}
                  type="password"
                  placeholder="Current Password"
                />,
              )}
            </Form.Item>
          : null }

          <p>New Password must be 8 characters long and contain at least one lowercase letter, one uppercase letter, and one number</p>

          <Form.Item hasFeedback>
            {this.props.form.getFieldDecorator('new_pass1', {
              rules: [{ required: true, message: 'Please input your new Password' },
              { validator: this.validateToNextPassword }],
            })(
              <Input.Password
                prefix={<Icon type="lock" style={{ color: 'rgba(0,0,0,.25)' }} />}
                type="password"
                placeholder="New Password"
              />,
            )}
          </Form.Item>

          <Form.Item hasFeedback>
            {this.props.form.getFieldDecorator('new_pass2', {
              rules: [{ required: true, message: 'Please confirm your new Password' },
              { validator: this.compareToFirstPassword }],
            })(
              <Input.Password
                prefix={<Icon type="lock" style={{ color: 'rgba(0,0,0,.25)' }} />}
                type="password"
                placeholder="New Password"
              />,
            )}
          </Form.Item>

          <Form.Item>
            <Button type="primary" htmlType="submit" className="password-form-button">
              Reset Password
            </Button>
          </Form.Item>
        </Form>
      );
    }
  }