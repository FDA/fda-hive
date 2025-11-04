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
import { connect } from 'react-redux';
import { App as UploaderBlock } from './UploaderBlock/UploaderBlock';
import { UploadNotifications } from './UploadNotifications';
import FilesTable from './FilesTable';
import ProcessFilesOptions from './ProcessFilesOptions';
import { FileModal } from '../modal/FileModal';

class ComputerUploader extends React.PureComponent {
    constructor(props){
        super(props);
        this.props = props;
        this.state = {
            checkedList:[],
            disabled:false
        }
        this.fileModal = new FileModal({});
        this.radioStyle  = {
          display: 'block',
          height: '30px',
          lineHeight: '30px',
        };
        this.processFilesOptions = ['Automatically process file(s)','Index','Quality Control','Screening']
    }
    componentDidUpdate(prevProps) {
        if(this.props.uploadState !== prevProps.uploadState){
        this.setState((state,props)=>{
            let disabled = props.uploadState === 'disabled' ||
                           props.uploadState === 'idle' ||
                           props.uploadState === 'error' ||
                           props.uploadState === 'done'  ? false : true
            if(disabled !== state.disabled) return{disabled}

        })
        }
    }

    render(){
        return(
            <>
                <div className="uploader--body">
                    <UploadNotifications style={{textAlign:'center'}} show={false} />
                    <div className="uploader--form--content">
                        <div className="uploader--form--uploader">
                            <UploaderBlock
                                disabled={this.state.disabled}
                                fileModal={this.fileModal}
                            />
                            <ProcessFilesOptions />
                        </div>
                        <div className="uploader--form--files">
                            <h1>HIVE Uploader</h1>
                            {this.props.uploadState === 'done'
                                ? <h4>Upload finished</h4>
                                : <h4>Drop or select items from your computer to upload to your HIVE home page</h4>
                            }
                            <br/>
                            <FilesTable
                              disabled={this.state.disabled}
                              fileModal={this.fileModal}
                            />
                        </div>
                    </div>
                </div>
            </>
        )
    }
}

const mapStateToProps = (state) => {
    return {  uploadState: state.uploadState };
}

export default connect(mapStateToProps)(ComputerUploader);