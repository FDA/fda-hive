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
import { Modal, Button , Tag } from 'antd';
import { FileExplorer } from '../../hivelib/FileExplorer/index';
//import "./file_explorer.css";

import { connect } from 'react-redux';

//button name transfered
//single select
class FileExplorerModal extends React.Component{
    constructor(props){
        super(props)
        this.state = {  }

        this.selectedFiles = null;
    }

    componentDidMount = () => {
        this.setState((state,props) => {
            let okVisible = typeof props.okVisible === 'boolean' ? props.okVisible : true
            return {
                        visible: false,
                        okVisible
                   }
        });
    };
    showModal = () => {
        this.setState({
          visible: true,
        });
    };

    hideModal = () => {
        this.setState({
          visible: false,
        });
    };

    handleOk = async(e) => {
        // Send this.state.selectedFiles to ExtensionModal
        if(this.selectedFiles){
            this.props.modalLocalFiles(this.selectedFiles)
        }

        // Clear selected items
        this.selectedFiles = null;
        this.hideModal()
    };

    handleCancel = e => {
        // Clear selected items
        this.selectedFiles = null;
        this.hideModal()

    };

    handleFileSelection = (files) => {
        this.selectedFiles = files;
    };

    openFileExplorer = () => {
        this.setState({visible:true})
    }

    renderCustomPreview = (items) => {
        this.handleFileSelection(items);

        if(items.length){
           let tags = items.map(item => {
                let name = item.name ? item.name
                         : item._brief ? item._brief
                         : item.title ? item.title
                         : item._id ? item._id
                         : item.key;
                let id = item._id ? item._id : item.key;

                return <Tag
                           className="hv-modal-tag"
                           id={id}
                           key={id}
                           closable
                        >
                           <b>{name}</b>
                        </Tag>
            })

            return(<div id="test"> {tags} </div>);
        }
    }

    render(){
        return (
            <>
                <Button
                    disabled={this.props.disabled}
                    block={this.props.buttonBlock ? this.props.buttonBlock : false}
                    size={this.props.buttonSize ? this.props.buttonSize : null}
                    onClick={this.openFileExplorer}
                    shape={typeof this.props.buttonShape === 'string' ? this.props.buttonShape : null }
                >
                  {this.props.buttonName ? this.props.buttonName : 'Browse HIVE Objects'}
                </Button>

                {   this.state.visible &&
                    <Modal
                      wrapClassName="hv-modal"
                      closable={false}
                      bodyStyle={{minHeight:'680px' , maxHeight:'1080px' , height:'fit-content', padding: '0px'}}
                      style={{minWidth:'400px', maxWidth:'1300px', width:'fit-content'}}
                      width={'1080'}
                      visible={this.state.visible}
                      onOk={this.handleOk}
                      okText={this.props.modalOkText ? this.props.modalOkText :'ADD'}
                      onCancel={this.handleCancel}
                      cancelText={this.props.cancelText ? this.props.cancelText : 'Cancel' }
                      okButtonProps={!this.state.okVisible ? {style:{display:'none'}} : ''}
                    >
                          <FileExplorer
                            recordTreeSelect = { typeof this.props.recordTreeSelect === 'boolean' ? this.props.recordTreeSelect : false}
                            singleSelect={typeof this.props.modalSingleSelect === 'boolean' ? this.props.modalSingleSelect : false}
                            hasDataPreview={typeof this.props.hasDataPreview === 'boolean' ? this.props.hasDataPreview : true}
                            tabDataPreview={this.props.tabDataPreview ? this.props.tabDataPreview : null}
                            findObject={this.props.findObject ? this.props.findObject : null}
                            customPreview={this.props.customPreview === null || this.props.customPreview ? this.props.customPreview : this.renderCustomPreview}
                            tabs={this.props.tabs ? this.props.tabs : null}
                            height={'100% - 53px'}
                            selection={files => this.handleFileSelection(files)}
                            hasUploader={false}
                            isDraggable={false}
                          />
                    </Modal>

                }
            </>
        )
    }
}

export default connect( null  )(FileExplorerModal);