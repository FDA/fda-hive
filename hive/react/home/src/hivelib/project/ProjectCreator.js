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
import { Button, Form, Input, message, Select } from "antd";
import { RequestConstructor } from "../modal/request_modal";

const ProjectCreatorForm = Form.create({ name: 'create_project' })(
  class extends React.Component {

    constructor(props) {
      super(props);
      this.state = {
        loading: false,
      };
    }

    handleSubmit = async (e) => {
      e.preventDefault();
      this.props.form.validateFields(async (err, values) => {
        if (err) {
          message.error("Please complete all fields");
          return;
        }
        this.setState({ loading: true });
        let parameters = {
          cmdr: 'createProject',
          ['name']: values.title,
          ['description']: values.description,
          ['type']: values.type,
        };
        let request = new RequestConstructor({ parameters });
        try {
          const response = await request.handleFetch();
          const json = await response.json();
          if (!response.ok) {
            if (json.error && json.error.length) {
              throw new Error(json.error);
            } else {
              throw new Error('HTTP response error ' + response.status + ' ' + response.statusText);
            }
          }
          const projectID = json.projectID;
          message.success("New project " + projectID + " has been successfully created");
          if (this.props.onProjectCreation) {
            this.props.onProjectCreation(projectID);
          }
        } catch (error) {
          message.error('Error: ' + error.message);
          console.error(error);
        } finally {
            this.setState({ loading: false })
        }
      });
    }

    render() {
      const { getFieldDecorator } = this.props.form;
      return (
        <Form onSubmit={this.handleSubmit} layout="horizontal">
          <Form.Item label="Project Title">
            {getFieldDecorator('title', {
              rules: [
                {
                  required: true,
                  message: "Please input your project's title"
                },
              ],
            })(
              <Input disabled={this.state.loading} />
            )}
          </Form.Item>
          <Form.Item label="Project Description">
            {getFieldDecorator('description', {
              rules: [
                {
                  required: true,
                  message: "Please input the project's description"
                },
              ],
            })(
              <Input disabled={this.state.loading} />
            )}
          </Form.Item>
          <Form.Item label="Project Type">
            {getFieldDecorator('type', {
              rules: [
                {
                  required: true,
                  message: "Please input the project's type"
                },
              ],
            })(
              <Select disabled={this.state.loading} >
                <Select.Option value="project">Project</Select.Option>
                <Select.Option value="HIVE_Development_Project_List">Scientific Project</Select.Option>
                <Select.Option value="HIVE_Development_Regulatory_Project">Regulatory Project</Select.Option>
              </Select>
            )}
          </Form.Item>
          <Form.Item>
            <Button type="primary" htmlType="submit" disabled={this.state.loading}>Create Project</Button>
          </Form.Item>
        </Form>
      );
    }

  }
);

export { ProjectCreatorForm };