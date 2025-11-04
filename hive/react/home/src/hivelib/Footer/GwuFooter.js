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
import './css/gwufooter.css';
import hiveLogoLarge from "../img/logos/gwu/gwu_smhs-full_logo_reduced.png";
import hiveLogoLargeDark from "../img/logos/gwu/gw_horizontal_2c_reduced.png";
 const GwuFooter = (props) => {
     console.log(props)
    const today = new Date();
    const year = today.getFullYear();
    const data = props.data ? props.data : {}
    const GwuLogo = data['data-theme'] && data['data-theme'] === 'light' ? hiveLogoLargeDark : hiveLogoLarge
    return (
        <>
        <div {...data} id="footer">
            <span style={{color: 'transparent', float: 'left'}}>2.2.8</span>
            <div style={{width:"700px", margin: "0px auto"}}>
                <div className="row footer-copyright">
                    © <span> 2014–{year}
                        <a href="/?cmd=main"> High-performance Integrated Virtual Environment (HIVE)</a>
                    </span>
                </div>

                <div className="row" style={{display: "inline-block"}}>
                    <div className="footer-adress">
                        <p>The Department of Biochemistry &amp; Molecular Medicine,</p>
                        <p> The School of Medicine and Health Sciences,</p>
                        <p>The George Washington University, </p>
                        <p>Ross Hall 530, 2300 Eye St, NW </p>
                        <p>Washington, DC 20037 </p>
                    </div>
                    <div className="footer-logo">
                        <a href="http://www.gwu.edu"><img src={GwuLogo} alt="The George Washington University" title="The George Washington University" /></a>
                    </div>
                </div>
                <div className="row footer-ctrl">
                    <span align="center" style={{padding:'0px 70px'}}> <a href="/?cmd=about">About</a> | <a href="/?cmd=publication">Publications</a> | <a href="/?cmd=people">People</a> </span>
                    <span align="right"> <a href="/?cmd=main">Main</a> | <a href="/?cmd=privacy">Privacy/Legal</a> </span>
                </div>
            </div>
        </div>
        </>
    );
};

export default GwuFooter;