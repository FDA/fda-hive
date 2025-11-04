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
import 'antd/dist/antd.css';
import './uploader.css';

import ComputerUploader from './view_uploader/ComputerUploader';

import { HeaderContainer } from '../hivelib/Header/index';
import { LoginContainer } from '../hivelib/Login';
import { FooterContainer } from '../hivelib/Footer/footer_container';
import controllers from '../hivelib/controller/controller_collector';
import { ProjectToolbar } from "../hivelib/project/ProjectToolbar";

const { CookieConstructor } = controllers;

export class Uploader2App extends React.Component{
    constructor(props){
        super(props);
        this.props = props;
        this.state = {
            isGuest: true,
            reload: 1
        }
        this.Cookie = new CookieConstructor({})
        //this.Cookie.handleEachCookie({value: document.location.href} , 'follow')

    }

    componentDidMount(){
        //check if guest
        if (this.isGuest() !== 'Guest') {
            this.setState({ isGuest:false })
        }
    }

    isGuest = () =>{
        return this.Cookie.getCookie('user_name');
    }

    logIn = () => {
        if (this.isGuest() !== 'Guest') {
            this.setState( (state) => { return{ isGuest: false, reload: state.reload + 1 }})
        }
    }

    render(){
        return (
            <div
             style={{height: '100vh'}}
            >
                <div className="header" style={{backgroundColor: '#334061', position:'relative'}}>
                    <HeaderContainer reload={this.state.reload}/>
                </div>
                <div className="project">
                    <ProjectToolbar
                        onProjectSwitch={() => {}}
                    />
                </div>
                <div
                    style={{
                        height:'calc(100vh - 40px)',
                        display: 'flex',
                    }}
                >
                    {
                        this.state.isGuest &&
                        <div className='uploader-login'>
                            <div
                                style={{marginTop: '50px' }}
                            >
                                <LoginContainer login={ () => this.logIn() }/>
                            </div>
                        </div>
                    }
                    {
                        !this.state.isGuest &&
                        <div className="uploader">
                            <ComputerUploader folder={this.state.folder} user={this.state.user}/>
                        </div>
                    }
               </div>
                <FooterContainer footerStyle='dashboard__main__footer'/>
            </div>
        );
    }
}