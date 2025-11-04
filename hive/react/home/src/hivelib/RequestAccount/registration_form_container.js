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
import React from 'react';
import 'antd/dist/antd.css';
import { Form, Input, Button } from 'antd';
import { RequestConstructor } from '../modal/request_modal';

const { TextArea } = Input;

export class RegistrationForm extends React.Component {
  constructor(props) {
    super(props);
    this.props = props;
    this.state = {
      info_specs: [],
      email_fail: false,
      all_request_success: false,
      mailto: null,
    };
    this.formItemLayout = {  // visual properties of the form
      labelCol: {
        xs: { span: 24 },
        sm: { span: 8 },
      },
      wrapperCol: {
        xs: { span: 24 },
        sm: { span: 16 },
      },
    };
    this.tailFormItemLayout = {  // visual properties of the form
      wrapperCol: {
        xs: {
          span: 24,
          offset: 0,
        },
        sm: {
          span: 16,
          offset: 8,
        },
      },
    };

    this.values = {};
  }

  componentDidMount() {
    let parameters = {cmdr: "propspec", type: "HIVE_public_account_request", mode: "json"};
    let request = new RequestConstructor({parameters: parameters});
    request.handleFetch()
      .then((response) => response.json())
      .then((data) => {
        let info_specs = [];
        for (const property in data._attributes) {
          // if the property is not read only
          if (data._attributes[property].hasOwnProperty("is_readonly") && !data._attributes[property]["is_readonly"]) {
            // if the property is not hidden
            if (!data._attributes[property].hasOwnProperty("is_hidden") || !data._attributes[property]["is_hidden"]) {
              let propspecs = data._attributes[property];
              propspecs["prop_name"] = property;
              info_specs.push(propspecs);  // add the property to info_specs
            }
          }
        }
        this.setState({ info_specs: info_specs });  // set the state of info_specs
      });
  }

  // makes and returns the <Form.Item> array based off of the information retrieved from the propspecs request
  make_form_items = () => {
    let info_specs = this.state.info_specs;
    let form_items = [];
    for (const element of info_specs) {
      let label = element.title;
      let id = element.prop_name;
      let isreqired = !(element.hasOwnProperty("is_optional") && element.is_optional);

      // if required, construct message
      let message = null;
      if (isreqired) {
        message = `${label} is required`;
      }

      // if not a phone number or email, allow whitespace
      let whitespace = false;
      if (id !== "phone_number" && id !== "email") {
        whitespace = true;
      }

      // determine the input type
      let input_style = null;
      if (element.type === "string") {
        input_style = <Input />;
      }
      else if (element.type === "text") {
        input_style = <TextArea rows={4}/>;
      }

      // create the rules array.  If email, add extra rule
      let rules = [{ required: isreqired, message: message, whitespace: whitespace}];
      if (id === "email") {
        rules.splice(0, 0, {type: "email", message: "The input is not a valid Email"});
      }

      // create the <Form.Item> and push to array
      let form_item = <Form.Item label={label} key={id} hasFeedback> {this.props.form.getFieldDecorator(id, {rules: rules})(input_style)} </Form.Item>;
      form_items.push(form_item);
    }
    return form_items;
  }

  // handles when the form is submitted
  handleSubmit = e => {
    e.preventDefault();
    this.props.form.validateFieldsAndScroll((err, values) => {
      if (!err) {  // if the form has all required items filled
        this.generate_mailto();
        document.getElementById("register_button").disabled = true;
        let parameters = {cmdr: "propset"};
        parameters["prop.HIVE_public_account_request._type"] = "HIVE_public_account_request";
        for (const property in values) {  // for each property in the passed values (not this.values)
          if (values[property] !== undefined && values[property] !== "") {  // if it's not undefined or empty
            // add the property and its value to the parameters
            let prop_name = `prop.HIVE_public_account_request.${property}`;
            parameters[prop_name] = values[property];

            // populate this.values with the info entered
            this.values[property] = {};
            this.values[property]["info_entered"] = values[property];

            for (const obj of this.state.info_specs) {
              if (obj.prop_name === property) {
                // populate this.values with the title (ie. "First Name", "Phone Number", etc.)
                this.values[property]["title"] = obj.title;
                break;
              }
            }
          }
        }

        let request = new RequestConstructor({parameters: parameters});
        request.handleFetch()
          .then((response) => response.text())
          .then((responseText) => {
            let error = responseText.indexOf("error") > -1 || responseText.indexOf("err") > -1;
            if (error) { // add ! to test without sending request
              throw new Error("There was an error");  // TODO better error message
            }
            // if no errors, send an email
            this.send_email(responseText);
          })
          .catch((err) => {
            console.log("Unable to process request");  // TODO better error handling
            // if error, make user manually send email
            this.setState({ email_fail: true });
          });
      }
    });
  }

  // sends the user and hive their respective emails
  send_email = (responseText) => {
    // get the request id from the response text
    let id = responseText.substring(responseText.indexOf("=") + 1);

    // hive_message is the string that will be emailed to hive
    let hive_message = `Hello,\n\nA public account request #${id} has been submitted at ${window.location.origin} by ${this.values.first_name.info_entered} ${this.values.last_name.info_entered}.\n\n`;

    for (const obj in this.values) {  // iteratively populates the string with all the portions of the form that are filled
      hive_message = hive_message + `${this.values[obj].title}: ${this.values[obj].info_entered}\n`;
    }

    // user_message is the string that will be emailed to the user
    let user_message = `Dear ${this.values.first_name.info_entered} ${this.values.last_name.info_entered},\n\nThank you for submitting an account request #${id}. You will be contacted shortly regarding next steps in the process by our team. We look forward to working with you.\n\nThank you,\nHIVE at the U.S. Food and Drug Administration (FDA)\n${window.location.origin}`;

    let parameters = {cmd: "sendmail", message: hive_message, email: this.values.email.info_entered, subject: "Account Request", autoreply: user_message};
    let request = new RequestConstructor({parameters: parameters});
    request.handleFetch()
      .then((response) => response.text())
      .then((responseText) => {
        let error = responseText.indexOf("error") > -1 || responseText.indexOf("err") > -1;
        if (error) {
          throw new Error("There was an error");  // TODO better error message
        }
        console.log("Email sent");  // for testing
        this.setState({ all_request_success: true });
      })
      .catch((err) => {
        console.log("Unable to send email");  // TODO better error handling
        this.setState({ email_fail: true });
      });
  }

  // creates and returns a mailto string with all the appropriate information
  generate_mailto = () => {
    let parameters = {cmdr: "file", filename: "json/config.json"}
    let request = new RequestConstructor({parameters: parameters});
    request.handleFetch()
      .then(response => response.json())
      .then(json => {
        let mailto_email = json.system.email;

        let mailto_subject = "Account Request";

        // populates the mailto_body string with all the portions of the form that are filled
        let mailto_body = `Hello,\n\nI received an error while attempting to create a public account request at ${window.location.origin}.  My information is as follows:\n\n`;
        for (const element in this.values) {
          mailto_body = mailto_body + `${this.values[element].title}: ${this.values[element].info_entered}\n`;
        }
        mailto_body = mailto_body + `\nThank You,\n${this.values.first_name.info_entered} ${this.values.last_name.info_entered}`;

        // combines the mailto components
        let mailto_string = `mailto:${mailto_email}?subject=${encodeURIComponent(mailto_subject)}&body=${encodeURIComponent(mailto_body)}`;
        this.setState({ mailto: mailto_string });
      });
  }

  render() {
    return (
      <>
      <Form {...this.formItemLayout} onSubmit={this.handleSubmit}>
        {this.make_form_items()}
        <Form.Item {...this.tailFormItemLayout}>
          <Button type="primary" htmlType="submit" id="register_button">
            Register
          </Button>
        </Form.Item>
      </Form>

      { this.state.all_request_success && !this.state.email_fail ?
        <>
        <h3>Request sent successfully to {this.values.email.info_entered}</h3>
        <p>You will be contacted shortly regarding next steps in the process by our team. We look forward to working with you.</p>
        </>
      : null }

      { this.state.email_fail ?
        <>
        <h3>There seems to have been an error sending your request</h3>
        <p>Please send an email to our team using <a href={this.state.mailto}>this link</a>.</p>
        </>
      : null }
      </>
    );
  }
}