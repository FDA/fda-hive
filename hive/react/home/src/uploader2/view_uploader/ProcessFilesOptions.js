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
import { Checkbox } from 'antd';
import { connect } from 'react-redux';

import {
    addToFormArchiver
} from '../actions';


class ProcessFilesOptions extends React.Component {
  constructor(props){
      super(props);
      this.state = {
            checkedList: [],
            autoProcess: false
      };
      this.processFilesOptions=['Quality Control','Screening']
  }
  componentDidMount(){
      let form = {};
      this.handleFormAchiverAutoProcess(form)
      this.props.addToFormArchiver(form)
  }
  componentDidUpdate(prevProps, prevState){
      let form = {};
      this.handleFormAchiverAutoProcess(form)
      if(prevState.checkedList !== this.state.checkedList){
          this.handleFormArchiverChechedList(form);
      }
      this.props.addToFormArchiver(form)

  }
  handleFormArchiverChechedList = (form) => {
      if( this.state.checkedList.length === 0 ) {
        form['prop.svc-pipeline-upload.st2_run_qc'] = 0;
        form['prop.svc-pipeline-upload.st2_run_screen'] = 0;
      } else {
        form['prop.svc-pipeline-upload.st2_run_qc'] = this.state.checkedList.includes('Quality Control') ? 1 : 0;
        form['prop.svc-pipeline-upload.st2_run_screen'] = this.state.checkedList.includes('Screening') ? 1 : 0;
      }
      return form;
  }

  handleFormAchiverAutoProcess = (form) => {
      if( this.state.autoProcess === true ) {
        form['prop.svc-pipeline-upload.st2_dissect'] = -1;
        this.handleFormArchiverChechedList(form);
      } else if( this.state.autoProcess === false ) {
        form['prop.svc-pipeline-upload.st2_dissect'] = 0;
        form['prop.svc-pipeline-upload.st2_run_qc'] = 0;
        form['prop.svc-pipeline-upload.st2_run_screen'] = 0;
      }
  }

  onChange = checkedList => {
    this.setState({
        checkedList
    });
  };

  onCheckAutoProcess = e => {
    this.setState((state , props) => {
        return { autoProcess: !state.autoProcess }
    });
  };

  render() {
    // console.log(this.props.uploadState)
    return (
      <div>
        <div style={{ marginTop: '20px', marginBottom: '10px', borderBottom: '1px solid #E9E9E9' }}>
          <Checkbox
            disabled={this.props.uploadState === 'disabled' ||
                      this.props.uploadState === 'idle' ||
                      this.props.uploadState === 'error' ||
                      this.props.uploadState === 'done'  ? false : true
                     }
            onChange={this.onCheckAutoProcess}
            checked={this.state.autoProcess}
            autoFocus={true}
          >
            Automatically process file(s)
          </Checkbox>
        </div>
        <Checkbox.Group
          options={this.processFilesOptions}
          value={this.state.checkedList}
          autoFocus={true}
          onChange={this.onChange}
          disabled={
                    this.state.autoProcess &&
                    (this.props.uploadState === 'disabled' ||
                    this.props.uploadState === 'idle' ||
                    this.props.uploadState === 'error' ||
                    this.props.uploadState === 'done')  ? false : true
                   }
        />
      </div>
    );
  }
}
const mapStateToProps = (state) => {

        return {
            uploadState: state.uploadState,
        };
}

const mapDispatchToProps = dispatch => ({
  addToFormArchiver: i => dispatch(addToFormArchiver(i))
})
export default connect(mapStateToProps,mapDispatchToProps)(ProcessFilesOptions)