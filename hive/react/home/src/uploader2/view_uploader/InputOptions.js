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
import { Select } from 'antd';

import { allUsers } from '../request'

const { Option } = Select;

export default class InputOptions extends React.Component {
    constructor(props){
        super(props)
        this.props = props;
        this.state={
            options: [],
            defaultValue: null
        }
    }
    componentDidMount(){
        if(this.props.show === 'users'){
            allUsers().then((userList)=>{
               let list = []
               userList.forEach((el,i) => {
                   if(i > 0 && i !== userList.length - 1 ){
                       el = el.split(',');
                       list.push({id:el[0], name: el[2]})
                   }
               })
               let defaultValue ;
               if(this.props.value){
                   list.forEach(item => {
                       if (item.name === this.props.value ){
                           defaultValue = item.id;
                       }
                   })
               }
               this.props.recordValue(defaultValue)
               this.setState({
                                 options: list,
                                 defaultValue
                             })
            })


        }
    }

    handleUserChange = option => {
        this.props.recordValue(option)
    }

    renderOptions = (options) => {
        let optionNodes = options.map((option , i) =>{
            return <Option key={option.id} value={option.id}>{option.name}</Option>;
        })
        return optionNodes;
    }

    render(){
        if(this.state.options.length > 0){
                return(
                        <Select
                           disabled={this.props.disabled ? this.props.disabled : false}
                           size="small"
                           style={{width:'100%'}}
                           defaultValue={this.state.defaultValue}
                           onChange={this.handleUserChange}
                        >
                            {this.renderOptions(this.state.options)}
                      </Select>
                )
           }else{
               return null;
           }
    }
}