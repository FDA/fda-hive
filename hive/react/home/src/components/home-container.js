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
import React, { Component } from "react";
import "antd/dist/antd.css";
import "@ag-grid-community/all-modules/dist/styles/ag-grid.css";
import "@ag-grid-community/all-modules/dist/styles/ag-theme-balham.css";
import { LogIn } from './login_container';
import { ProjectToolbar } from "../hivelib/project/ProjectToolbar";
import { FileExplorer } from '../hivelib/FileExplorer/index';
import { HeaderContainer } from '../hivelib/Header/index';
import controllers from '../hivelib/controller/controller_collector';

export class HomeTab extends Component {
  constructor(props){
    super(props)
    let { CookieConstructor } = controllers
    let cookie = new CookieConstructor()
    this.username = cookie.getCookie("user_name");
    let footer_config = {data: {"data-theme": 'light'}}
    sessionStorage.setItem('footer' , JSON.stringify(footer_config))
    this.state = {
      reloadExplorerCounter: 0, // increment this when we need to reload the FileExplorer, i.e. on project change
    };
  }
  render(){
    if(this.username === "Guest"){
        return <LogIn /> ;
    }else{
      return (
        <div className="dashboard">
          <HeaderContainer selected={this.props.selected} />
            <ProjectToolbar
              onProjectSwitch={() => {
                this.setState({reloadExplorerCounter: this.state.reloadExplorerCounter + 1});
              }}
            />
          <FileExplorer
            key={this.state.reloadExplorerCounter}
            recordTreeSelect={false}
            singleSelect={false}
            hasDataPreview={true}
            tabDataPreview={null}
            findObject={null}
            customPreview={null}
            tabs={null}
            hasUploader={true}
            height='100%'
            hasFooter={true}
          />
        </div>
      )
    }
  }
}
