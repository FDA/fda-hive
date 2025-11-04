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
import "antd/dist/antd.css";
import { Button } from "antd";
import { connect } from 'react-redux';
import modals from '../../hivelib/modal/modal_collector';

import { handleProgress , clearUpload } from '../actions'
import FileExplorer from '../FileExplorer/FileExplorer';

class ProgressButtons extends React.Component {
    // handleProgress ... uploadState
    // 'Drop file'                 == "disabled" .......................... (e) => "idle"
    // 'start Upload'              == "idle" .............................. (e) => "preparing"
    // 'pause Upload' -- 'Cancel'  == "uploading" || "resumed" -- "clear" . (e) => "paused"
    // 'resume Upload' -- 'Cancel' == "paused" -- "clear" ................. (e) => "resumed"
    // 'retry'  -- 'Cancel'        == "error" -- "clear"  ................. (e) => "uploading"
    // 'new Upload'                == "done" -- "clear"

    handleCancel = async () => {
        if(this.props.uploadState !== 'preparing'){
            await this.props.handleProgress('disabled');
        }

        await this.props.clearUpload(true);
    }

    render(){
        return <>
                  { (this.props.uploadState === 'disabled' || this.props.uploadState === 'idle') &&
                        <>
                             <Button
                                 onClick={() =>this.props.handleProgress('preparing')}
                                 type="primary"
                                 shape="round"
                                 style={{paddingLeft: '30px',paddingRight: '30px',marginRight:'15px'}}
                                 disabled={this.props.uploadState === 'disabled' || !this.props.isOnline  ? true : false}
                              > Start Upload
                              </Button>
                        </>
                   }
                   { (this.props.uploadState === 'preparing' || this.props.uploadState === 'uploading'  || this.props.uploadState === 'resuming') &&
                        <>
                            <Button
                                onClick={this.handleCancel}
                                shape="round"
                                style={{paddingLeft: '30px',paddingRight: '30px',marginRight:'15px'}}
                                disabled={!this.props.isOnline ? true : false}
                            > Cancel
                            </Button>
                            <Button
                                onClick={() =>this.props.handleProgress('pausing')}
                                type="primary"
                                icon='pause'
                                shape="round"
                                style={{paddingLeft: '30px',paddingRight: '30px'}}
                                disabled={this.props.uploadState === 'resuming' || this.props.uploadState === 'preparing' || !this.props.isOnline ? true : false}
                            >Pause Upload
                            </Button>
                        </>
                    }
                   { (this.props.uploadState === 'paused'  || this.props.uploadState === 'pausing') &&
                        <>
                            <Button
                                onClick={this.handleCancel}
                                shape="round"
                                style={{paddingLeft: '30px',paddingRight: '30px',marginRight:'15px'}}
                                disabled={!this.props.isOnline ? true : false}
                            > Cancel
                            </Button>
                            <Button
                                onClick={() =>this.props.handleProgress('resuming')}
                                type="primary"
                                icon='play-circle'
                                shape="round"
                                style={{paddingLeft: '30px',paddingRight: '30px'}}
                                disabled={!this.props.isOnline || this.props.uploadState === 'pausing' ? true : false}
                            > Resume Upload
                            </Button>
                        </>
                    }
                   { this.props.uploadState === 'error' &&
                          <>
                            <Button
                                onClick={() =>this.props.handleProgress('preparing')}
                                type="primary"
                                icon='play-circle'
                                shape="round"
                                style={{paddingLeft: '30px',paddingRight: '30px',marginRight:'15px'}}
                                disabled={!this.props.isOnline ? true : false}
                            > Retry
                            </Button>
                            <Button
                                onClick={() => this.props.clearUpload(true)}
                                shape="round"
                                style={{paddingLeft: '30px',paddingRight: '30px'}}
                                disabled={!this.props.isOnline ? true : false}
                            >  New Upload
                            </Button>
                        </>
                    }
                   { this.props.uploadState === 'done' &&
                       <>
                        <Button
                             onClick={() => this.props.clearUpload(true)}
                             type="primary"
                             shape="round"
                             style={{paddingLeft: '25px',paddingRight: '25px',marginRight:'15px'}}
                             disabled={!this.props.isOnline ? true : false}
                        >
                             New Upload
                        </Button>
                        <Button
                             href={`${modals.url_modal.getPrefix()}?${this.props.svcPipelineLink}`}
                             type="primary"
                             shape="round"
                             style={{paddingLeft: '25px',paddingRight: '25px',marginRight:'15px'}}
                             disabled={!this.props.svcPipelineLink ? true : false}
                        >
                            Go To Pipeline
                        </Button>
                        <FileExplorer
                                    disabled={!this.props.svcArchiverId || !this.props.isOnline}
                                    hasDataPreview={true}
                                    //tabDataPreview='Progress'
                                    findObject={{_id:this.props.svcArchiverId, _type:"svc-pipeline-upload", _js_component: ['progress'] }}
                                    customPreview={null}
                                    buttonName='Preview Archiver'
                                    buttonShape='round'
                                    okVisible={false}
                                    cancelText={'Close'}
                         />
                        </>
                    }
              </>
        }
}

const mapStateToProps = (state) => {
        return {
            uploadState: state.uploadState,
            svcArchiverId: state.svcArchiverId,
            svcPipelineLink: state.svcPipelineLink,
            isOnline: state.isOnline
        };
}
const mapDispatchToProps = dispatch => ({
  handleProgress: i => dispatch(handleProgress(i)),
  clearUpload: i => dispatch(clearUpload(i)),
})

export default connect(mapStateToProps,mapDispatchToProps)(ProgressButtons);