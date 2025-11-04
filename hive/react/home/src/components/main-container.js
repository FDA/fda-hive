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
import * as UrlModal from '../hivelib/modal/url_modal';
import { HeaderContainer } from '../hivelib/Header/index';
import controllers from '../hivelib/controller/controller_collector';
import { FooterContainer } from '../hivelib/Footer/footer_container';
import { Link } from 'react-router-dom';

export class MainTab extends Component {
  UrlPrefix = UrlModal.getPrefix();
  UrlPrefixPlain = UrlModal.getPrefixPlain();
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
        <>
          <HeaderContainer selected={this.props.selected}/>
          <div className="main-container">
            <div className="centered-text">
              <div style={{display:'inline-block'}}><img style={{marginBottom:'25px'}} src={`${UrlModal.getPrefixPlain()}/img/hiveicon.jpg`} width="64" alt='' /></div>
              <div style={{marginLeft:'10px', fontSize: '30px', fontWeight: 650, display:'inline-block'}}>Welcome to HIVE
                <p style={{color: '#0275d8', fontSize: '13px'}}>High-performance Integrated Virtual Environment</p>
              </div>
              <div>
                  <p>HIVE is a cloud-based environment optimized for the storage and analysis of extra-large data, like Next Generation Sequencing data, Mass Spectroscopy files, Confocal Microscopy Images and others.</p>
                  <p>HIVE uses a variety of advanced scientific and computational visualization graphics, so to get the <b>most</b> from your HIVE experience you must use a supported browser. These include Google Chrome, Mozilla Firefox and Safari.</p>

                  <p>A few exemplary analytical outputs are displayed below for your enjoyment. But before you can take advantage of all that HIVE has to offer and create these objects for yourself, you'll need to
                    <a href={`${this.UrlPrefix}?cmd=userReg`} style={{color: 'blue'}}> register</a>.
                  </p>
                  <div className="img" style={{display: 'inline-block', margin:'10px 5px'}}><img src={`${UrlModal.getPrefixPlain()}/img/alignImg.png`} width="510px" style={{objectFit: 'cover', height: '240px'}} alt=''/></div>
                  <div className="img" style={{display: 'inline-block', margin:'10px 5px'}}><img src={`${UrlModal.getPrefixPlain()}/img/profileImg.png`} width="450px" style={{objectFit: 'cover', height: '240px'}} alt=''/></div><br/>
                  <p style={{textAlign: 'center', marginTop: '20px'}}> <b>Need more info?</b> Try browsing the tabs above or
                    <Link to="?tab=about" style={{color: 'blue'}}> click here</Link>.</p>

                  <div className="note-box">
                      <h3 style={{marginTop: '0px'}}>Changelog:</h3>
                      <h4 style={{marginTop: '0px'}}>HIVE Release 23.05</h4>
                      <div className="category-sections columns ">
                          <ul className="category-section"><p>New Tools:</p>
                              <li>samtools-fastq v1.17</li>
                          </ul>
                          <ul className="category-section"><p>Features:</p>
                              <li>FastP: output file name format</li>
                              <li>Attachment handling for dockets</li>
                              <li>Change Email Notification Setting's Process Default</li>
                              <li>HISAT2 added safe mode to process big files</li>
                              <li>Improve progress update</li>
                              <li>Increased password length requirement to 15 characters</li>
                          </ul>
                          <ul className="category-section"><p>Bug Fixes:</p>
                              <li>RNASEq pipeline fails due to very long names</li>
                              <li>SRA Fastq Downloader</li>
                              <li>HIVE Network did not work with Project correctly</li>
                              <li>Generic launcher fails under heavy load</li>
                          </ul>
                      </div>
                  </div>
              </div>

              <div style={{display:'inline-block'}}><img style={{marginBottom:'25px'}} src={`${UrlModal.getPrefixPlain()}/img/processSvc.gif`} width="64"  alt='' /></div>
              <div style={{marginLeft:'10px', fontSize: '30px', fontWeight: 650, display:'inline-block'}}>What can you do here?
                <p style={{color: '#0275d8', fontSize: '13px'}}>Major services provided by HIVE</p>
              </div>
              <div>
                What you can do here:<br/>
                <ul>
                    <li><img border="0" src={`${UrlModal.getPrefixPlain()}/img/plus.gif`} width="18" alt='' />Download/upload from local directories or external databases using accession IDS or URLs, securely store and analyze big data</li>
                    <li><img border="0" src={`${UrlModal.getPrefixPlain()}/img/user-share.gif`} width="18" alt='' />Customize and propagate read/write permissions for your data and subsequent analytical results</li>
                    <li><img border="0" src={`${this.UrlPrefixPlain}/img/processSvc.gif`} width="18" alt='' />Arrange tools in pipelines to perform comprehensive analysis on data including sequencing, SNP profiling, recombinant discovery, clustering and more</li>
                    <li><img border="0" src={`${this.UrlPrefixPlain}/img/download.gif`} width="18" alt='' />Download/export results in a variety of available formats</li>
                <br/>
                </ul>
                What you can't do here:<br/>
                <ul>
                    <li>Access or analyze ANY data without the appropriate authorization</li>
                    <li>Buy a lottery ticket</li>
                    <li>Solve world hunger (yet)</li>
                </ul>
                <br/>
                Ready for more? <a href={`${this.UrlPrefix}?cmd=userReg`} style={{color: 'blue'}} >Register now</a>.
              </div>
            </div>
          </div>

          <FooterContainer footerStyle={"footer"}/>
        </>
      )
    }
  }
}