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
import { Input , Tooltip } from 'antd';

import FileExplorer from '../FileExplorer/FileExplorer';
import { findFolderPath } from '../request'
import { shortenPath } from '../modal/TextModal'


export default class InputFromHive extends React.Component {
    constructor(props){
        super(props)
        this.props = props;
        this.state={
            inputValue: null,
            toolTip: null
        }
    }

    componentDidMount(){
        let file = {
            key: this.props.value
        }
        this.handleModalLocalFiles([file])
    }

    componentDidUpdate(prevProps, prevState){
        if(prevProps.value !== this.props.value){
           let file = {
                key: `${this.props.value}`
           }
           this.handleModalLocalFiles([file])
        }
    }
    handleModalLocalFiles = (files) => {
        if(this.props.show === 'path'){
            //!! Handle if folder to choose only one item
            if(files[0].key === 0 || files[0].key === '0' ){
                this.setState({
                    inputValue:'',
                    toolTip:'Please select folder'
                  })
            }else{
                findFolderPath(files[0].key).then((path) => {
                    path = path.join(' / ')
                    this.setState({
                                    inputValue:shortenPath(path),
                                    toolTip:path
                                  })
                })
            }

            this.props.recordValue(files[0].key)
        }
    }

    renderInput = () =>
            <Input
                    disabled={this.props.disabled ? this.props.disabled : false}
                    size="small"
                    addonAfter={
                                <FileExplorer
                                    disabled={this.props.disabled ? this.props.disabled : false}
                                    hasDataPreview={false}
                                    tabs={ { title: 'Folders', types: ['^folder$']} }
                                    buttonSize="small"
                                    buttonName={this.props.buttonName}
                                    modalOkText={this.props.modalOkText}
                                    modalSingleSelect={this.props.modalSingleSelect}
                                    recordTreeSelect={this.props.recordTreeSelect}
                                    modalLocalFiles={(files) => this.handleModalLocalFiles(files)}

                                />
                              }
                    value={this.state.inputValue}
           />

    render(){
        if(this.state.toolTip !== null){
            return(
                <Tooltip  placement="bottomLeft" title={this.state.toolTip}>
                    {this.renderInput()}
                </Tooltip>
            )
        }else {
           return(
               <>
                    {this.renderInput()}
               </>
           )
        }
    }
}