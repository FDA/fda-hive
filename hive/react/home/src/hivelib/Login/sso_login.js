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
import { Button, message } from 'antd';
import './index.css';
import "antd/dist/antd.css";
import { CookieConstructor } from '../controller/cookie_controller';
import { getPrefix } from '../modal/url_modal';


export class SSOLogin extends PureComponent {
  constructor(props) {
    super(props);
    this.props = props;
    this.state = {
      fields: { }
    };
  }

  handleSubmit = () => {
    let cookie_controller = new CookieConstructor();
    let sso_url = cookie_controller.handleGetCookie("sso_url");

    if (sso_url === "") {
      message.error("There was an issue processing your request. Please try logging in with your username and password instead.", 20)
    }
    else {
      let follow = cookie_controller.handleGetCookie("follow");
      if (follow === "") {
        follow = "home";
      }
      let url = `${getPrefix()}?cmd=sso%26follow=${follow}`
      document.location.href = `${document.location.origin}${sso_url}${url}`;
    }
  }

  render() {
    return (
      <div className="piv">
      Or insert PIV card and click <span> </span>
      <Button type="primary" onClick={this.handleSubmit} className="piv_button">PIV Login</Button>
      </div>
    );
  }
}