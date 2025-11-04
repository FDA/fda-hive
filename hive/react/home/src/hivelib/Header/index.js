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
import React,{ PureComponent } from 'react';
import "./css/header.css";
import "antd/dist/antd.css"
import hiveLogoSmall from "./../img/hive-hexagon.gif";
import hiveLogoLarge from "./../img/HIVE_logo_transparent_small_reduced.png";
import { getPrefix , getPrefixPlain} from "../modal/url_modal";
import { filterPaths, sortByOrder } from "./../modal/menuitems_modal";
import { RequestConstructor } from "../modal/request_modal";

import { SettingsContainer } from './components/settings_container';

import {Menu} from "./menu_container.js"
import { Pubsub } from './pubsub';

import fdaLogo from "../img/FDA_new_logo_blue.png";
import gwuLogo from "../img/logos/gwu/gwu_smhs_white_logo_45.png";

// map of logo type returned by server to image path
const logoMap = {
    HiveFdaLogos: fdaLogo,
    HiveGwuLogos: gwuLogo,
};

export class HeaderContainer extends PureComponent{
    constructor(props) {
        super(props);
        this.state = {
            store: {
                right: [],
                left: [],
            },
            size: 'small',
            logoType: null,
        };
        this.left_list_width = 0;
        this.right_list_width = 0;
        this.header_pubsub = new Pubsub();
    }

    componentDidMount(){
        window.addEventListener('resize', this.headerAutoCorrect);

        this.getMenus()

        this.getComponents()
    }

    getMenus = () => {
        let parameters = {
            cmdr: "objList" ,
            mode: "json" ,
            type: 'menuitem'
        }

        let request = new RequestConstructor({parameters});

        request.handleFetch()
        .then(response => response.json())
        .then(json => json.objs)
        .then(newData => {
            newData = filterPaths(newData);
            newData = sortByOrder(newData);
            const leftMenu = [];
            const rightMenu = [];

            // Adjust URLs //
            newData.forEach( item => {

                let itemPathName = encodeURIComponent(item.name);
                if(!item.url){
                    item.url = `?cmd=menu&root=${itemPathName}`
                }
                item.url =  modalURL(item.url);

                item.children.forEach(subitem => {
                    let subitemPathName = encodeURIComponent(subitem.name);
                    if(!subitem.url){
                        subitem.url = `?cmd=menu&root=${itemPathName}#${subitemPathName}`
                    }
                    subitem.url = modalURL(subitem.url);
                })

                item.align === "left" ? leftMenu.push(item) : rightMenu.push(item) ;

            })
            this.setState((state, props) => ({ store: {right: rightMenu, left: leftMenu} }) )
        })
        .catch(error => console.error('HeaderContainer: '   + error))
    }

    headerAutoCorrect =() => {
        try{
            let list = document.querySelector(".menu-list-container");
            let header = document.querySelector(".header");
            const left_list = document.querySelector(".menu-list.right");
            const right_list = document.querySelector(".menu-list.left");
            let diff = header.offsetWidth - left_list.offsetWidth - right_list.offsetWidth - 50;
            //register the first time it's the largest
            if (diff < 0) {
                this.left_list_width = left_list.offsetWidth || 0;
                this.right_list_width = right_list.offsetWidth || 0;
            }
            if(this.left_list_width && this.right_list_width){
                diff = header.offsetWidth - this.left_list_width - this.right_list_width - 50;
            }

            if (!header.classList.contains("header-autocorrect") && diff < 0) {
                header.className += " header-autocorrect";
            }
            if(diff < 0) {
                let top;
                if(list.classList.contains("responsive")){ top = '0px'; }
                else {top = `-${list.clientHeight + 10}px`;}
                list.style.top = top;

            } else if(header.classList.contains("header-autocorrect") && diff >=0) {
                header.classList.remove("header-autocorrect");
            }
        } catch(error){ console.error(error)}



    }
    componentDidUpdate = () => {
        this.headerAutoCorrect()
    }
    getComponents = () => {
        let parameters = {
            cmdr: "file" ,
            filename: "json/header_footer_components.json"
        }

        let request = new RequestConstructor({parameters});
        request.handleFetch()
            .then(response => response.json())
            .then(json => {
                if (!Object.keys(logoMap).includes(json['header'][0])) {
                    throw 'Header logo type returned by server not recognized';
                }
                this.setState({ logoType: json['header'][0] });
            })
            .catch((err)=>{
                console.error('Problem with custom logo: ', err);
            });
    }

    render() {
        const {store,size} = this.state;
        return (
            <>
            <div className="header" style={{backgroundColor: '#334061'}}>
                <div id="header" className={size}>
                    <>
                        <img src={hiveLogoLarge} className="logo__large" alt="hive logo" />
                        {this.state.logoType &&
                            <img src={logoMap[this.state.logoType]} className="logo__large" alt="blue fda logo" />
                        }
                    </>
                    <Menu logos={{hive:{small: hiveLogoSmall}}}
                            menuitems={store}
                            menuSize={size}
                            setMenuSize={ (size) => this.setState({size})}
                            header_pubsub={this.header_pubsub}
                            tabSelected={this.props.selected}
                    />
                </div>
            </div>
            <SettingsContainer header_pubsub={this.header_pubsub}/>
            </>
        );
    }
}

function modalURL(url) {
    if (url.toLowerCase().indexOf('http') === 0 || url.toLowerCase().indexOf('www') === 0) return url
    if (url.toLowerCase().indexOf('javascript') === 0) return null
    if (url.toLowerCase().indexOf('r/') === 0 || url.toLowerCase().indexOf('/r/') === 0) return `${getPrefixPlain()}/${url}`
    return getPrefix() + url
}
