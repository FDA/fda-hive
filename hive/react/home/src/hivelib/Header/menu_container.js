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
import { Icon } from "antd";
import { DropDown } from "./dropdown_container.js";
import { CookieConstructor } from "../controller/cookie_controller";

export const Menu = (props) =>{
const {menuitems,menuSize,setMenuSize,logos, tabSelected} = props;

// Does li need class 'dropdown'
const hasSubitems = (item) => {
    return  item.children.length !== 0;
}

const dropDown = (isDropdown) =>{
    return isDropdown ? ` dropdown` : '';
}

const iconDropDown = (isDropdown) =>{
    if(isDropdown)
        return <Icon type="down" style={{fontSize:'10px', marginLeft:'3px'}}/>
}

// Get name from cookies
const getLogin = () => {
    let Cookie = new CookieConstructor()
    let user_cookie = Cookie.getCookie('user_name');

    return unescape(user_cookie);
}

const changeSize = (e) => {
    e.preventDefault();
    if(menuSize === 'small'){
        setMenuSize('large');
    }else{
        setMenuSize('small');
    }
}

const hamburgerState = (e) => {
    e.preventDefault();
    let x = document.getElementById('header');
    if (!x.classList.contains("responsive") ) {
            x.className += " responsive";
        } else {
            x.classList.remove("responsive");
        }
}

const strToClass = (str) => {
        if(typeof str === 'string'){
        return str.toLowerCase().replace(' ' , '_')
        }
        return 'item';
}

return(
        <>
            <div className="menu-icon__hamburger right" onClick={hamburgerState}> &#9776; MENU</div>
            <div className="menu-list-container">
                <ul className="menu-list left">
                <div className="menu-icon__logo"><img src={logos.hive.small} alt="hive logo"/></div>
                {menuitems.left.map(( item, i ) =>
                                        <li key={i} className={`level-one-${strToClass(item.name)}` + dropDown(hasSubitems(item))}>
                                            <a href={item.url} style={strToClass(item.name)==tabSelected? {color:'#e8d544'}:{}}>{strToClass(item.name) === 'profile' ? getLogin() : item.title}</a>
                                            <DropDown menuitem={item} header_pubsub={props.header_pubsub}/>
                                            {iconDropDown(hasSubitems(item))}
                                        </li>  )}
                </ul>
                <ul className="menu-list right">
                    <div onClick={changeSize} className="menu-icon__expand right"><Icon type={menuSize === 'small' ? "double-right" : "double-left"}/></div>
                    {menuitems.right.map(( item, i ) =>
                                        <li key={i} className={`level-one-${strToClass(item.title)}` + dropDown(hasSubitems(item))}>
                                            <a href={item.url}>{strToClass(item.name) === 'profile' ? getLogin() : item.title}{iconDropDown(hasSubitems(item))}</a>
                                            <DropDown menuitem={item} header_pubsub={props.header_pubsub}/>
                                        </li>  )}
                </ul>
            </div>

        </>

    );
}