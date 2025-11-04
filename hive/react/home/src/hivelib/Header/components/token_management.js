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
import { Table, Input, Button, Popconfirm, Form, notification, Icon, Tooltip } from 'antd';
import { RequestConstructor } from '../../modal/request_modal';


const EditableContext = React.createContext();

const EditableRow = ({ form, index, ...props }) => (
  <EditableContext.Provider value={form}>
    <tr {...props} />
  </EditableContext.Provider>
);

const EditableFormRow = Form.create()(EditableRow);

class EditableCell extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      editing: false,
    };
  }

  toggleEdit = () => {
    this.setState((state,props)=> {
        return {editing: !state.editing}
      });
  }

  save = e => {
    const { record, handleSave } = this.props;
    this.form.validateFields((error, values) => {
      if (error && error[e.currentTarget.id]) {
        return;
      }
      this.toggleEdit();
      handleSave({ ...record, ...values });
    });
  };

  getInputType = (title) => {
    if (title === "Expiration Date"){
      return "datetime-local";
    }
    return "";
  }

  renderCell = form => {
    this.form = form;
    const { children, dataIndex, record, title } = this.props;
    const { editing } = this.state;
    return editing ? (
      <Form.Item style={{ margin: 0 }}>
        {form.getFieldDecorator(dataIndex, {
          rules: [
            {
              required: true,
              message: `${title} is required.`,
            },
          ],
          initialValue: record[dataIndex],
        })(
        <Input type={this.getInputType(title)} ref={node =>{this.input = node}} onPressEnter={this.save} onBlur={this.save} />
        )}
      </Form.Item>
    ) : (
      <div
        className="editable-cell-value-wrap"
        style={{ paddingRight: 24 }}
        onClick={this.toggleEdit} // TODO onClick for not the div? just the text in the div?
      >
        {children}
      </div>
    );
  };

  render() {
    const {
      editable,
      children,
      ...restProps
    } = this.props;
    return (  // raising warning? handleSave and dataIndex
      <td {...restProps}>
        {editable ? (
          <EditableContext.Consumer>{this.renderCell}</EditableContext.Consumer>
        ) : (
          children
        )}
      </td>
    );
  }
}

export class EditableTable extends React.Component {
  constructor(props) {
    super(props);
    this.columns = [
      {
        title: 'Token Name',
        dataIndex: 'account_token_name',
        width: 175,
        editable: true,
        render: (text,record) => {
          return (
            <p>
              {text} &nbsp;
              <Icon id={"name_edit_" + record.account_token_id} type="edit" style={{color: "black", visibility: 'hidden'}}/>
            </p>
          )
        }
      },
      {
        title: 'Expiration Date',
        dataIndex: 'account_token_expiration',
        width: 275,
        editable: true,
        render: (text,record) => {
          if (record.account_token_expiration !== "None") {
            let css_id = "expir_trash_" + record.account_token_id;
            let style = record.is_expired ? {color: "#cc2b12"} :{}
            return (
              <p style={style}> {text}
                              &nbsp; &nbsp;
              <Icon id={"expir_edit_" + record.account_token_id} type="edit" style={{color: "black", visibility: 'hidden'}}/>
              &nbsp; &nbsp;
                <Tooltip title="Remove Expiration Date">
                  <a id={css_id} className="expir_trash" onClick={(e) => this.handleNoExpir(record,e)} style={{visibility: 'hidden'}}>
                    <Icon type="delete" theme="twoTone" twoToneColor="#cc2b12" />
                  </a>
                </Tooltip>
              </p>);
          }
          else {
            return (<p> {text} &nbsp;
              <Icon id={"expir_edit_" + record.account_token_id} type="edit" style={{color: "black", visibility: 'hidden'}}/> </p>);
          }
        }
      },
      {
        title: '',
        dataIndex: 'operations',
        width: 50,
        render: (text, record) => {
            return (this.state.dataSource.length >= 1 ? (
              <Popconfirm title="Delete this token?" onConfirm={() => this.handleDelete(record)}>
                <Tooltip title="Delete Token">
                  <a><Icon type="delete" theme="twoTone" twoToneColor="#cc2b12" /></a>
                </Tooltip>
              </Popconfirm>
            ) : null)
          }
        }
    ];
    this.columns = this.columns.map(col => {
      if (!col.editable) {
        return col;
      }
      return {
        ...col,
        onCell: record => ({
          record,
          editable: col.editable,
          dataIndex: col.dataIndex,
          title: col.title,
          handleSave: this.handleSave,
        }),
      };
    });

    this.components = {
      body: {
        row: EditableFormRow,
        cell: EditableCell,
      },
    };

    this.user_id = null;

    this.state = {
      dataSource: [],
      count: 0,
      logged_in: false,
    };

  }

  getProps = () => {  // gets the most updated version of the token list
    const parameters = {cmdr: 'objList', type: 'user-settings', props: 'account_token', mode: 'json'};
    let request = new RequestConstructor({ parameters: parameters })

    let dataSource = [];
    request.handleFetch()
      .then(response => response.json())
      .then(data => {
        if (this.user_id === null) {
          this.user_id = data.objs[0]._id;  // sets the user id for later requests
        }

        for (const property in data.objs[0].account_token) {
          dataSource.push(data.objs[0].account_token[property]);  // adds all tokens to the dataSource
        }
        let tempArray = [];
        let created_exists = false;
        for (const element of dataSource) {  // reformats the data in dataSource to be easier to access
          for (const property in element) {
            let token = element[property];
            token.account_token_id = property;

            if (!token.hasOwnProperty('account_token_name')) {  // if the token doesn't have a name, display 'None'
              token.account_token_name = "None";
            }

            if (!token.hasOwnProperty('account_token_created')) {  // if the token doesn't have a creation date, display 'None'
              token.account_token_created = "None";
            }
            else {
              created_exists = true;
            }

            if (!token.hasOwnProperty('account_token_expiration')) {  // if the token doesn't have an expiration data, display 'None'
              token.account_token_expiration = "None";
              token.is_expired = false;
            }
            else {
              let expir_date = new Date(token.account_token_expiration);
              let now = new Date();

              token.is_expired = Math.floor(expir_date) < Math.floor(now);  // determine if the token is expired
            }
            tempArray.push(token);
          }
        }
        if (created_exists && this.columns[1].title !== 'Creation Date') {  // only adds the 'created' column if at least one token has it
          this.columns.splice(1, 0, {
              title: 'Creation Date',
              dataIndex: 'account_token_created',
              width: 225,
            },);
        }
        this.setState({ dataSource: tempArray, count: tempArray.length });
      });
  }

  componentDidMount = () => {
    this.getProps();
  }

  handleDelete = record => {  // handles delete request of a token
    let promise_array = [];
    for (const property in record) {
      if(property !== "account_token_id" && property !== "is_expired" && record[property] !== "None") {  // only deletes existing props
        const parameters = {cmdr: 'propDel', ids: this.user_id, prop: property, path: record.account_token_id};
        let request = new RequestConstructor({ parameters: parameters });
        promise_array.push(request.handleFetch());
      }
    }
    Promise.all(promise_array).then(this.getProps);
  };

  copyTextToClipboard = async (text) => {  // copies the token key to the clipboard
    if ('clipboard' in navigator) {
      return await navigator.clipboard.writeText(text);
    } else {
      return document.execCommand('copy', true, text);
    }
  }

  handleCopy = (tokenInfo) => {  // copies the token key when the copy-button is pressed and displays success or failure
    this.copyTextToClipboard(tokenInfo)
      .then(() => {
        const args = {
          message: 'Token Copied',
          duration: 2,
        };
        notification.open(args);
      })
      .catch((err) => {
        const args = {
          message: 'Copy Button Failed',
          description: 'Please copy the token manually.',
          duration: 5,
        };
        notification.open(args);
        console.log(err);
      });
  }

  openNotification = (tokenInfo) => {  // opens a notification with the token key after generating
    const btn = (
      <Button type="primary" size="small" onClick={() => this.handleCopy(tokenInfo)}>
        <Icon type="copy" />
      </Button>
    );
    const args = {
      message: 'Token:',
      description: `${tokenInfo}\nCopy this Token and store it in a safe place. You will NOT be able to access this token again.`,
      btn,
      duration: 0,
    };
    notification.open(args);
  };

  handleAdd = () => {  // handles generating a new token request
    const parameters = {cmdr: 'genToken', name: 'New Token'};
    let request = new RequestConstructor({ parameters: parameters })
    request.handleFetch()
      .then(response => response.text())
      .then(data => {
        if (data.includes("error")) {
          const args = {
            message: 'Unable to generate a new token at this time',
            description: data,
            duration: 0,
          };
          notification.open(args);
        }
        else {
          this.openNotification(data);
        }
        this.getProps();
      });
  };

  equal_dates = (old_string, new_string) => {  // compares dates
    if (old_string === "None") {
      if (new_string === "None") {
        return true;
      }
      return false;
    }

    let old_date = new Date(old_string);
    let new_date = new Date(new_string);

    return Math.floor(old_date / 1000) === Math.floor(new_date / 1000);
  }

  handleSave = row => {  // saves name or expiration date when the user enters an updated one
    const newData = [...this.state.dataSource];
    const index = newData.findIndex(item => row.account_token_id === item.account_token_id);
    const item = newData[index];

    let old_name = item.account_token_name;
    let old_expir_date = item.account_token_expiration;

    let new_name = row.account_token_name;
    let new_expir_date = row.account_token_expiration;

    const parameters = {cmd: 'propset', raw: '1' };
    let prop_phrase = 'prop.' + this.user_id + '.';

    let new_info = null;

    if (old_name !== new_name) {  // if the token name was the updated information
      prop_phrase = prop_phrase + 'account_token_name.';
      new_info = new_name;
      for (const element of this.state.dataSource) {
        if (element.account_token_name !== "None") {
          let temp_prop_phrase = prop_phrase + element.account_token_id;
          parameters[temp_prop_phrase] = element.account_token_name;
        }
      }
    }
    else if (!this.equal_dates(old_expir_date, new_expir_date)) {  // if the expiration date was the updated information
      prop_phrase = prop_phrase + 'account_token_expiration.';
      let new_date = new Date(new_expir_date);
      new_info = Math.floor(new_date / 1000)
      for (const element of this.state.dataSource) {
        if (element.account_token_expiration !== "None") {
          let temp_prop_phrase = prop_phrase + element.account_token_id;
          parameters[temp_prop_phrase] = element.account_token_expiration;
        }
      }
    }
    else {  // if nothing was changed, return
      console.log('No change in information detected');
      this.getProps(); // incase 2 tabs are open ???
      return;
    }

    prop_phrase = prop_phrase + row.account_token_id;

    parameters[prop_phrase] = new_info;

    let request = new RequestConstructor({ parameters: parameters })
    request.handleFetch()
      .then(response => response.text())
      .then(data => {
        this.getProps();
        const args = {
          message: 'Information Updated',
          duration: 2,
        };
        notification.open(args);
      });
  };

  handleNoExpir = (record,e) => {  // handles deleting token expiration date
    e.stopPropagation()
    if (record.account_token_expiration !== "None") {
      const parameters = {cmdr: 'propDel', ids: this.user_id, prop: 'account_token_expiration', path: record.account_token_id};
      let request = new RequestConstructor({ parameters: parameters })
      request.handleFetch()
        .then(response => response.text())
        .then(data => this.getProps());
    }
  };

  render() {
    return (
      <div>
        <Button onClick={this.handleAdd} type="primary" style={{ marginBottom: 16 }}>
          Generate New Token
        </Button>
        <Table
          rowKey="account_token_id"
          components={this.components}
          rowClassName={() => 'editable-row'}
          bordered
          dataSource={this.state.dataSource}
          columns={this.columns}

          onRow={(record, rowIndex) => {
            let edit_name_id = "name_edit_" + record.account_token_id;
            let edit_expir_id = "expir_edit_" + record.account_token_id;
            let trash_expir_id = "expir_trash_" + record.account_token_id;
            return {
              onMouseEnter: event => {  // mouse enter row
                let edit_name = document.getElementById(edit_name_id);
                if (edit_name) { edit_name.style.visibility = "visible"; }

                let edit_expir = document.getElementById(edit_expir_id);
                if (edit_expir) { edit_expir.style.visibility = "visible"; }

                let trash_expir = document.getElementById(trash_expir_id);
                if (trash_expir && record.account_token_expiration !== "None") {
                  trash_expir.style.visibility = "visible";
                }
              },
              onMouseLeave: event => { // mouse leave row
                let edit_name = document.getElementById(edit_name_id);
                if (edit_name) { edit_name.style.visibility = "hidden"; }

                let edit_expir = document.getElementById(edit_expir_id);
                if (edit_expir) { edit_expir.style.visibility = "hidden"; }

                let trash_expir = document.getElementById(trash_expir_id);
                if (trash_expir && record.account_token_expiration !== "None") {
                  trash_expir.style.visibility = "hidden";
                }
              },
            };
          }}
        />
      </div>
    );
  }
}