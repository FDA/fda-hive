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
import { Form } from 'antd';
import { connect } from 'react-redux';
import controllers from '../../../../hivelib/controller/controller_collector';
import { addToFormArchiver } from '../../../actions';
import InputFromHive from '../../InputFromHive';

class ChooseFolder extends React.Component{
    constructor(props){
        super(props)
        this.state ={ }
        this.CookieConstructor = new controllers.CookieConstructor()
    }
    recordValue = (key,value) =>{
        let form = {}
        form[key] = value
        this.props.addToFormArchiver(form)
    }
    componentDidMount(){
        let initialFolderId = this.CookieConstructor.getCookie('folder_open')
        if (!initialFolderId.length) {
            initialFolderId = 0
        }
        this.recordValue('prop.svc-pipeline-upload.folder', parseInt(initialFolderId))
        this.setState({
            initialFolderId
        })

    }

    render(){
        return(
               <>
                   <Form.Item layout='vertical' className="hv--input--file-explorer" label="Upload to folder">
                        <InputFromHive
                                       disabled={(this.props.uploadState === 'disabled' ||
                                                 this.props.uploadState === 'idle' ||
                                                 this.props.uploadState === 'error' ||
                                                 this.props.uploadState === 'done')  ? false : true
                                                }
                                       recordValue={(value) => this.recordValue('prop.svc-pipeline-upload.folder', value)}
                                       show='path'
                                       buttonName="Choose"
                                       modalOkText="SELECT"
                                       modalSingleSelect={true}
                                       recordTreeSelect={true}
                                       value={this.state.initialFolderId}
                        />
                   </Form.Item>
               </>
        )
    }
}
const mapStateToProps = (state) => {
    return {
             uploadState: state.uploadState,
             filesList: state.filesList,
           };
}

const mapDispatchToProps = dispatch => ({
  addToFormArchiver: i => dispatch(addToFormArchiver(i))
})
export default connect(mapStateToProps,mapDispatchToProps)(ChooseFolder)