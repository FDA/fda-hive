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
import { Button, Divider, Icon, Form, message, Modal, Radio, Select, Table, Tooltip, Typography } from "antd";
import { RequestConstructor } from "../modal/request_modal";
import { getPrefix } from "../modal/url_modal";

const { Paragraph, Title } = Typography;
const { Option } = Select;

/* String comparison function for table sorting */
const lexoCmp = (a, b) => a.localeCompare(b, undefined, { usage: "sort", sensitivity: "base" });

// A button that removes the specified user from a project.
// Calls onRemove with the userID and projectID upon completion
function RemoveUser(props) {

  const removeUser = async () => {
    const parameters = {
      cmdr: 'removeFromProject',
      projectID: props.projectID,
      userId: props.userID,
    };
    const req = new RequestConstructor({ parameters });
    try {
      const response = await req.handleFetch();
      const json = await response.json();
      if (!response.ok) {
        if (json.error && json.error.length) {
          throw new Error(json.error);
        } else {
          throw new Error('HTTP response error ' + response.status + ' ' + response.statusText);
        }
      }
      if (props.onRemove) {
        props.onRemove();
      }
      message.success('User removed');
    } catch (error) {
      message.error('Error: ' + error.message);
      console.error(error);
    }
  }

  let tooltipMessage = '';
  if (!props.isAdmin) {
    tooltipMessage = 'Not authorized to remove users with current role. Ask a project administrator to give you role Admin';
  } else if (props.isSelf) {
    tooltipMessage = 'Cannot remove self. Ask another project administrator to remove you';
  } else {
    tooltipMessage = 'User will no longer have access to project';
  }

  return (
    <Tooltip title={tooltipMessage}>
      <Button
        disabled={!props.isAdmin || props.isSelf}
        onClick={removeUser}
        type="danger">
        Remove
      </Button>
    </Tooltip>);
}

/* A table listing the members of a project */
function MemberTable(props) {

  const handleRemove = (userID, projectID) => {
    if (props.onRemove) {
      props.onRemove(userID, projectID);
    }
  }

  const columns = [
    {
      title: 'Name',
      dataIndex: 'name',
      key: 'name',
    },
    {
      title: 'Role',
      dataIndex: 'role',
      key: 'role',
    },
    {
      title: 'Actions',
      dataIndex: 'actions',
      key: 'actions',
      render: (_, record) => (
          <RemoveUser
            isAdmin={props.isAdmin}
            isSelf={record['id'] === props.currentUserID}
            onRemove={handleRemove}
            projectID={props.projectID}
            userID={record['id']} />
      ),
    }
  ];
  return (
    <Table
      columns={columns}
      dataSource={props.data}
      loading={props.loading} />
  );
}

class AddMember extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      userToAdd: null,
      role: null,
    };
  }

  componentDidUpdate(prevProps) {
    const projectChanged = this.props.projectID !== prevProps.projectID;
    const userInfoChanged = this.props.userInfo !== prevProps.userInfo;
    const membersChanged = this.props.projectMembers !== prevProps.projectMembers;
    if (projectChanged || userInfoChanged || membersChanged) {
      this.clearUserSelections();
    }
  }

  clearUserSelections = () => {
    this.setState({
      userToAdd: null,
      role: null
    });
  }

  setUserToAdd = (id) => {
    this.setState({ userToAdd: id });
  }

  addUser = async () => {
    const parameters = {
      cmdr: 'addToProject',
      projectID: this.props.projectID,
      userId: this.state.userToAdd,
      role: this.state.role,
    };
    const req = new RequestConstructor({ parameters });
    try {
      const response = await req.handleFetch();
      const json = await response.json();
      if (!response.ok) {
        if (json.error && json.error.length) {
          throw new Error(json.error);
        } else {
          throw new Error('HTTP response error ' + response.status + ' ' + response.statusText);
        }
      }
      if (this.props.onMemberAdd) {
        this.props.onMemberAdd(this.state.userToAdd, this.state.role, this.props.projectID);
      }
      message.success('Added user successfully');
    } catch (error) {
      message.error('Error: ' + error.message);
      console.error(error);
      this.clearUserSelections();
    }
  }

  render() {
    const availableUsers = this.props.userInfo.map((user) => {
      const disabled = this.props.projectMembers.map((member) => member.id).includes(user.id);
      return (<Option key={user.id} value={user.id} disabled={disabled}>{user.name}</Option>)
    });
    // Boolean() is important to coerce null values to false
    const isFormFilled = Boolean(this.state.userToAdd) && Boolean(this.state.role);
    const canAdd = this.props.isAdmin && isFormFilled;
    let addButtonTooltip = '';
    if (!this.props.isAdmin) {
      addButtonTooltip = 'Not authorized to add users with current role. Ask a project administrator to give you role Admin';
    } else if (!isFormFilled) {
      addButtonTooltip = 'Must select user and role';
    } else {
      addButtonTooltip = 'Add user to project';
    }
    return (
      <>
        <Form>
          <Form.Item
            label="User"
            name="user">
            <Select
              disabled={!this.props.isAdmin}
              onSelect={this.setUserToAdd}
              showSearch={true}
              placeholder="User to add"
              optionFilterProp="children"
              value={this.state.userToAdd}>
              {availableUsers}
            </Select>
          </Form.Item>
          <Form.Item label="Role" name="role">
            <Radio.Group
              buttonStyle='solid'
              disabled={!this.props.isAdmin}
              onChange={(e) => { this.setState({ role: e.target.value }); }}
              value={this.state.role}>
              <Tooltip title={this.props.isAdmin ? 'Browse and read permissions' : ''}>
                <Radio.Button value="V">Viewer</Radio.Button>
              </Tooltip>
              <Tooltip title={this.props.isAdmin ? 'Browse, read, write, and download permissions' : ''}>
                <Radio.Button value="D">Data Handler</Radio.Button>
              </Tooltip>
              <Tooltip title={this.props.isAdmin ? 'Browse, read, write, download, and execute permissions' : ''}>
                <Radio.Button value="C">Contributor</Radio.Button>
              </Tooltip>
              <Tooltip title={this.props.isAdmin ? 'Browse, read, write, download, execute, delete, and administration permissions' : ''}>
                <Radio.Button value="A">Admin</Radio.Button>
              </Tooltip>
            </Radio.Group>
          </Form.Item>
          <Form.Item>
            <Tooltip title={addButtonTooltip}>
              <Button
                disabled={!canAdd}
                onClick={this.addUser}
                type="primary">
                Add
              </Button>
            </Tooltip>
          </Form.Item>
        </Form>
      </>
    );
  }

}

const WrappedAddMemberForm = Form.create({ name: 'addMember' })(AddMember);

class ProjectAdmin extends React.Component {

  constructor(props) {
    super(props);
    this.state = {
      projectMembers: [],
      userInfo: [],
      tableLoading: false,
      currentUserID: null,
    };
  }

  async fetchProjMembers() {
    const parameters = {
      cmdr: 'projectMembers',
      projectID: this.props.projectID,
    };
    const req = new RequestConstructor({ parameters });
    this.setState({ tableLoading: true });
    try {
      const response = await req.handleFetch();
      const json = await response.json();
      if (!response.ok) {
        if (json.error && json.error.length) {
          throw new Error(json.error);
        } else {
          throw new Error('HTTP response error ' + response.status + ' ' + response.statusText);
        }
      }
      const data = json.data;
      const memberTable = [];
      const findNameById = (id) => {
        let name = 'userID ' + id;
        for (const user of this.state.userInfo) {
          if (user['id'] === id) {
            name = user['name'];
            break;
          }
        }
        return name;
      }
      for (const [userId, role] of Object.entries(data)) {
        memberTable.push({
          id: userId,
          role: role,
          name: findNameById(userId),
        });
      }
      memberTable.sort((a, b) => {
        if (a.id === this.state.currentUserID) {
          return -1;
        } else if (b.id === this.state.currentUserID) {
          return 1;
        } else {
          return lexoCmp(a.name, b.name);
        }
      });
      this.setState({ projectMembers: memberTable });
    } catch (error) {
      message.error('Error: ' + error.message);
      console.error(error);
    } finally {
      this.setState({ tableLoading: false });
    }
  }

  handleMemberChange = () => {
    this.fetchProjMembers();
  }

  fetchUserList = async () => {
    const parameters = {
      cmdr: 'usrList',
      primaryGrpOnly: 1,
      json: 1,
      active: 1,
    };
    const usrListReq = new RequestConstructor({ parameters });
    try {
      const response = await usrListReq.handleFetch();
      if (!response.ok) {
        throw new Error('HTTP response error ' + response.status + ' ' + response.statusText);
      }
      const data = await response.json();
      data.sort((a, b) => lexoCmp(a.name, b.name));
      this.setState({ userInfo: data });
    } catch (error) {
      message.error('Error: ' + error.message);
      console.error(error);
    }
  }

  fetchCurrentUserID = async () => {
    const parameters = {
      cmdr: 'userInfo',
    };
    const usrInfoReq = new RequestConstructor({ parameters });
    try {
      const response = await usrInfoReq.handleFetch();
      if (!response.ok) {
        throw new Error('HTTP response error ' + response.status + ' ' + response.statusText);
      }
      const data = await response.json();
      this.setState({ currentUserID: data["current_user"]["_id"].toString() });
    } catch (error) {
      message.error('Error: ' + error.message);
      console.error(error);
    }
  }

  isAdmin = () => {
    for (const projectMember of this.state.projectMembers) {
      if (projectMember.id === this.state.currentUserID) {
        return projectMember.role === 'Admin';
      }
    }
    return false;
  }

  fetchInitialData = async () => {
    await Promise.all([this.fetchCurrentUserID(), this.fetchUserList()]);
    this.fetchProjMembers();
  }

  componentDidMount() {
    this.fetchInitialData();
  }

  componentWillUnmount() {
  }

  componentDidUpdate(prevProps) {
    if (this.props.projectID !== prevProps.projectID) {
      this.fetchProjMembers();
    }
  }

  render() {
    const isAdmin = this.isAdmin();
    let recordViewerUrl = getPrefix() + '?cmd=record&ids=' + this.props.projectID;
    let recordViewerLinkLabel = 'Edit project metadata';
    if (!isAdmin) {
      recordViewerUrl += '&readonly=1';
      recordViewerLinkLabel = 'View project metadata';
    }
    return (
      <>
        <>
          <Title level={3}>Project {this.props.projectID} - {this.props.projectTitle}</Title>
          <Paragraph>{this.props.projectDescription}</Paragraph>
          <Button
            href={recordViewerUrl}
            size="small"
            target="blank"
            title={recordViewerLinkLabel}>
            {recordViewerLinkLabel}
          </Button>
          <Button
            onClick={this.fetchInitialData}
            size="small"
            title="Reload">
            <Icon type="reload" />
            Reload
          </Button>
        </>
        <Divider plain />
        <WrappedAddMemberForm
          userInfo={this.state.userInfo}
          onMemberAdd={this.handleMemberChange}
          projectID={this.props.projectID}
          projectMembers={this.state.projectMembers}
          isAdmin={isAdmin}
        />
        <Divider plain />
        <MemberTable
          data={this.state.projectMembers}
          isAdmin={isAdmin}
          currentUserID={this.state.currentUserID}
          userInfo={this.state.userInfo}
          loading={this.state.tableLoading}
          onRemove={this.handleMemberChange}
          projectID={this.props.projectID}
        />
      </>
    );
  }
}

/*
  Presents the Adminstration component as a Modal dialog launched by a button.
*/
class ProjectAdminModal extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      visible: false,
    };
  }

  hideModal = () => {
    this.setState({ visible: false });
  }

  showModal = () => {
    this.setState({ visible: true });
  }

  render() {
    return (
      <>
        <Modal
          title="Project settings"
          visible={this.state.visible}
          onOk={this.hideModal}
          onCancel={this.hideModal}
          width={this.props.width}
        >
          <ProjectAdmin
            projectID={this.props.projectID}
            projectTitle={this.props.projectTitle}
            projectDescription={this.props.projectDescription}
          />
        </Modal>
        <Button
          icon="setting"
          title="Project settings"
          disabled={!this.props.projectID}
          onClick={this.showModal}
        />
      </>
    );
  }

}

export { ProjectAdmin, ProjectAdminModal };