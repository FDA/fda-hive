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

import { Button, Icon, Modal, Table, message } from "antd";
import { RequestConstructor } from "../modal/request_modal";

/* Converts a unix timestamp to javascript Date */
const unixToJSDate = (unixTime) => new Date(1000 * unixTime);
/* Convert a javascript Date into the string we want the user to see in our table */
const jsDateToString = (date) => date.toLocaleDateString();
/* String comparison function for table sorting */
const lexoCmp = (a, b) => a.localeCompare(b, undefined, { usage: "sort", sensitivity: "base" });
/* Object ID comparison function for table sorting. Assumes objID are strings */
const objIDCmp = (a, b) => parseInt(a) - parseInt(b);

/*
  Presents a table showing the user's visible projects.
  Calls the parent's handler function (onProjectSelection) with the
  new project ID whenever a new selection is made.
  */
class ProjectChooser extends React.Component {

    constructor(props) {
        super(props);
        this.state = {
            loading: false,
            listAvailableProjects: [],
        };
    }

    grabProjects = async () => {
        this.setState({ loading: true });
        const parameters = { cmdr: 'availableProjects' };
        const request = new RequestConstructor({ parameters });
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
            const projectData = [];
            projectData.push({
                projectID: "",
                title: "Personal",
                created: "",
            });
            for (const [objID, info] of Object.entries(json.data)) {
                projectData.push({
                    projectID: objID,
                    title: info["Title"],
                    created: info["created"] ? jsDateToString(unixToJSDate(info["created"])) : "",
                });
            }
            this.setState({ listAvailableProjects: projectData });
        } catch (error) {
            message.error('Error: ' + error.message);
            console.error(error);
        } finally {
            this.setState({ loading: false });
        }
    }

    componentDidMount = () => {
        this.grabProjects();
    }

    getTableColumns = () => {
        return [
            {
                title: 'Project ID',
                dataIndex: 'projectID',
                key: 'projectID',
                sortDirections: ['ascend', 'descend'],
                sorter: (a, b) => {
                    if (a.projectID === "") {
                        return 1;
                    } else if (b.projectID === "") {
                        return -1;
                    } else {
                        return objIDCmp(a.projectID, b.projectID);
                    }
                },
                defaultSortOrder: 'descend',
            },
            {
                title: 'Title',
                dataIndex: 'title',
                key: 'title',
                sortDirections: ['ascend', 'descend'],
                sorter: (a, b) => {
                    if (a.projectID === "") {
                        return 1;
                    } else if (b.projectID === "") {
                        return -1;
                    } else {
                        return lexoCmp(a.title, b.title);
                    }
                },
            },
            {
                title: 'Created',
                dataIndex: 'created',
                key: 'created',
            }
        ];
    }

    handleProjectSelection = (_, selectedRows) => {
        if (this.props.onProjectSelection) {
            if (selectedRows[0]["projectID"] === "") {
                this.props.onProjectSelection(null);
            } else {
                this.props.onProjectSelection(selectedRows[0]["projectID"]);
            }
        }
    }

    render() {
        return (
            <>
                <Button
                    onClick={this.grabProjects}
                    title="Reload">
                    <Icon type="reload" />
                    Reload
                </Button>
                <Table
                    columns={this.getTableColumns()}
                    dataSource={this.state.listAvailableProjects}
                    loading={this.state.loading}
                    rowKey={(record) => record.projectID}
                    rowSelection={{
                        type: 'radio',
                        onChange: this.handleProjectSelection,
                    }}
                    pagination={{
                        pageSizeOptions: ['10', '20', '100'],
                        showSizeChanger: true
                    }}
                    size="small">
                </Table>
            </>
        );
    }

}

/*
  Presents the ProjectChooser as a Modal dialog launched by a button.
  ONLY calls the parent handler function (onProjectChoosen) with
  the new project ID when the OK button (or Return to Personal button) is pressed.

  The user's personal project is represented as projectID=null
*/
class ProjectChooserModal extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            visible: false,
            selectedProjectID: null,
        };
    }

    handleProjectSelection = (projectID) => {
        this.setState({
            selectedProjectID: projectID,
        });
    }

    handleOk = () => {
        if (this.props.onProjectChoosen) {
            this.props.onProjectChoosen(this.state.selectedProjectID);
        }
        this.setState({ visible: false });
    }

    handleCancel = () => {
        this.setState({ visible: false });
    }

    handleReturn = () => {
        this.setState({
            selectedProjectID: null,
            visible: false,
        });
        if (this.props.onProjectChoosen) {
            this.props.onProjectChoosen(null);
        }
    }

    openModal = () => {
        this.setState({ visible: true });
    }

    render() {
        return (
            <>
                <Modal
                    title="Available Projects"
                    visible={this.state.visible}
                    onOk={this.handleOk}
                    onCancel={this.handleCancel}
                    width={this.props.width}
                    footer={
                        <div style={{ display: 'flex', justifyContent: 'space-between', width: '100%' }}>
                            {/* Left-aligned */}
                            <Button
                                key="return"
                                onClick={this.handleReturn}
                                style={{ width: '140px', textAlign: 'center' }}
                            >
                                Return to Personal
                            </Button>

                            {/* Right-aligned */}
                            <div style={{ display: 'flex', gap: '8px' }}>
                                <Button
                                    key="ok"
                                    type="primary"
                                    onClick={this.handleOk}
                                    style={{ width: '100px' }}
                                >
                                    Ok
                                </Button>
                                <Button
                                    key="cancel"
                                    onClick={this.handleCancel}
                                    style={{ width: '100px' }}
                                >
                                    Cancel
                                </Button>
                            </div>
                        </div>
                    }
                >
                    <ProjectChooser onProjectSelection={this.handleProjectSelection} />
                </Modal>
                <Button
                    icon="project"
                    title="Change project"
                    onClick={this.openModal}>
                    Change Project
                </Button>
            </>
        );
    }

}


export { ProjectChooser, ProjectChooserModal };