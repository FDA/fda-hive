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
import { Button, Modal, Typography, message } from 'antd';
import { RequestConstructor } from '../modal/request_modal';

import { ProjectChooserModal } from './ProjectChooser';
import { ProjectAdminModal } from './ProjectAdmin';
import { ProjectCreatorForm } from './ProjectCreator';

const { Text } = Typography;

/* A short description of the current project. */
function ProjectDescriptor(props) {
    return (
        <Text>
            {props.projectID
                ? " " + props.projectID + ": " + props.projectTitle + " "
                : " Personal "
            }
        </Text>
    );
}

class ProjectToolbar extends React.Component {

    constructor(props) {
        super(props);
        this.state = {
            currentProjectID: null,
            currentProjectTitle: null,
            currentProjectDescription: null,
            createModalVisible: false,
        };
    }

    componentDidMount = () => {
        const sessionProjectID = sessionStorage.getItem("projectID");
        if (sessionProjectID) {
            this.handleProjectSwitch(sessionProjectID);
        }
    }

    handleProjectSwitch = async (projectID) => {
        if (!projectID) {
            sessionStorage.removeItem("projectID");
            sessionStorage.removeItem("projectName");
            this.setState({
                currentProjectID: null,
                currentProjectTitle: null,
                currentProjectDescription: null,
            });
        } else {
            const parameters = {
                cmdr: 'propget',
                mode: 'json',
                projectID: projectID,
                ids: projectID,
            };
            const req = new RequestConstructor({ parameters });
            try {
                const response = await req.handleFetch();
                if (!response.ok) {
                    throw new Error('HTTP response error ' + response.status + ' ' + response.statusText);
                }
                const data = await response.json();
                this.setState({
                    currentProjectID: data["_id"],
                    currentProjectTitle: data["Title"],
                    currentProjectDescription: data["Description_full"],
                });
                sessionStorage.setItem("projectID", data["_id"]);
                sessionStorage.setItem("projectName", data["Title"]);
            } catch (error) {
                message.error('Error: Unable to access project ID, switching to Personal');
                console.error(error);
                this.setState({
                    currentProjectID: null,
                    currentProjectTitle: null,
                    currentProjectDescription: null,
                });
                sessionStorage.removeItem("projectID");
                sessionStorage.removeItem("projectName");
            }
        }
        if (this.props.onProjectSwitch) {
            this.props.onProjectSwitch(projectID);
        }
    }

    hideCreateModal = () => {
        this.setState({ createModalVisible: false });
    }
    showCreateModal = () => {
        this.setState({ createModalVisible: true });
    }

    handleProjectCreation = async (projectID) => {
        await this.handleProjectSwitch(projectID);
        this.hideCreateModal();
    }

    render() {
        return (
            <div className="project" style={{ display: 'flex', alignItems: 'center' }}>
                <span style={{ fontWeight: 'bold' }}>Project:</span>
                <span style={{ margin: '5px'}}>
                    <ProjectDescriptor
                        projectID={this.state.currentProjectID}
                        projectTitle={this.state.currentProjectTitle}
                    />
                </span>
                <ProjectAdminModal
                    projectID={this.state.currentProjectID}
                    projectTitle={this.state.currentProjectTitle}
                    projectDescription={this.state.currentProjectDescription}
                    width={1200}
                />
                <Button
                    icon="file-add"
                    title="New Project"
                    onClick={this.showCreateModal}
                />
                <Modal
                    title="New Project"
                    visible={this.state.createModalVisible}
                    onCancel={this.hideCreateModal}
                    footer={null}
                >
                    <ProjectCreatorForm onProjectCreation={this.handleProjectCreation} />
                </Modal>
                <ProjectChooserModal
                    onProjectChoosen={this.handleProjectSwitch}
                    width={980}
                />
            </div>
        );
    }
}

export { ProjectToolbar };