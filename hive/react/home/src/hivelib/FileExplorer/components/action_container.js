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
import React from "react";
import {
    Button,
    Menu,
    Dropdown,
    Icon
} from "antd";
import shortid from 'shortid';
import { handleNotifications } from "./../controller/notification_controller";
import { OpenNotificationWithIcon } from "./../view/notification_view";
import { hrefConstruct } from "./../modal/action_modal";
import { getPrefixPlain } from "./../../modal/url_modal";
const SubMenu = Menu.SubMenu;


const ButtonRowView = ({ actionList, selectedRowKeys, clearRowKeys, types, srcFolder, reload, hasUploader,submitter,selectedTitle, style = {} }) => {
        // ========================= //
        // Action Buttons
        const info = {
            "ids": selectedRowKeys ? selectedRowKeys.join(',') : '',
            "srcFolder": srcFolder >= 0 ? srcFolder : '',
            "type": types,
            "objCls": 'dcls',
            "dcls": 'dcls',
            "submitter": submitter,
            "title": selectedTitle || 'Item name',
            "projectID": sessionStorage.getItem("projectID")
        }

        const uploader = {
            name: 'Uploader',
            subitems: [
                {
                    title: 'Upload from Computer',
                    id: shortid.generate(),
                    url: '/r/home?tab=uploader2',
                    target: '_blank'
                },
                {
                    title: 'SRA Fastq Downloader',
                    id: shortid.generate(),
                    url: '?cmd=pipeline&type=svc-pipeline-sra-fastq',
                    target: '_blank'
                },
                {
                    title: 'SRA SAM Downloader',
                    id: shortid.generate(),
                    url: '?cmd=pipeline&type=svc-pipeline-sra-sam',
                    target: '_blank'
                },
                {
                    title: 'AWS S3',
                    id: shortid.generate(),
                    url: '?cmd=pipeline&type=svc-pipeline-download-s3',
                    target: '_blank'
                },
                {
                    title: 'HTTP Downloader',
                    id: shortid.generate(),
                    url: '?cmd=pipeline&type=svc-pipeline-download-http',
                    target: '_blank'
                },
                {
                    title: 'FTP Downloader',
                    id: shortid.generate(),
                    url: '?cmd=pipeline&type=svc-pipeline-download-ftp',
                    target: '_blank'
                },
                {
                    title: 'NCBI GenBank Downloader',
                    id: shortid.generate(),
                    url: '?cmd=pipeline&type=svc-pipeline-download-ncbi-genbank',
                    target: '_blank'
                },
                {
                    title: 'Import from HIVE Network',
                    id: shortid.generate(),
                    url: '/r/home?tab=network/',
                    target: '_blank'
                }

            ]
        }

        const iconConstruct = (icon) => {
            if (icon.charAt(0) === '/') {
                return <img  alt={icon} className="hive-btn__icon"  src={getPrefixPlain() + icon} /> ;
            }else {
                return <Icon type={ icon } /> ;
            }
        }
        const menuItem = (data, action, first) => {
            let ant_btn = first ? 'ant-btn' : '';
            return ( <Menu.Item
                           className = { ant_btn }
                           key = { action.id || shortid.generate()}
                           title = { action.description ||  action.title || action.name}
                           disabled = { action.disabled || (action.single_obj_only && selectedRowKeys.length > 1) ? true : false}
                           onClick = { (e) => executeURL(e, action) }
                     >
                         { action.icon ? iconConstruct(action.icon) : '' }
                         <a
                               className = "action-button"
                               target = "_blank"
                               rel = "noopener"
                         >
                               { action.title }
                         </a>
                     </Menu.Item>
            );
        }

        const renderMenuList = (data) =>
            data.map(action => {

                if (action.subitems && action.subitems.length > 0) {
                    return <SubMenu
                                title={ action.name }
                                key={ data.indexOf(action) }
                            >
                                { renderMenuList(action.subitems) }
                            </SubMenu>
                } else {
                    return menuItem(data, action);
                }
            })

        const menu = (data) => <Menu>{ renderMenuList(data) }</Menu>

        const renderDropDown = (action) => (
            <Dropdown
                    key={ shortid.generate() }
                    overlay={ menu(action.subitems) }
            >
                <Button>
                    { action.name }
                    <Icon type="down" />
                </Button>
            </Dropdown>
        )

        const mapDropdown = (data) => (
            data.map((action, i) => {
                if (action.subitems && action.subitems.length > 0) {
                    return renderDropDown(action);
                } else {
                    return menuItem(data, action, true);
                }
            })
        )

        const executeURL = (e, action) => {
            if(action.hasOwnProperty('note_duration') ){
                let notitication_pros = {
                   type: action.note_type ? action.note_type : 'warning',
                   message: `${action.title} Began` ,
                   description: action.description ,
                   key: info.ids,
                   icon: action.note_type ? null : (<Icon type="loading" />),
                   duration: action.note_duration
               }
               OpenNotificationWithIcon(notitication_pros);
            }
            if(action.target && action.target === "ajax"){
                 let notitication_pros = {
                    type: 'info' ,
                    message: `${action.title} Began` ,
                    description: action.description ,
                    key: info.ids,
                    icon: action.note_type ? null : (<Icon type="loading" />),
                    duration: action.note_duration
                }
                OpenNotificationWithIcon(notitication_pros);
            }

            hrefConstruct(action.url, info,action.target).then((result) => {
                if(result !== null){
                    handleNotifications(result, action.title , info.ids);
                    if (action.reload || action.target === "ajax") {
                        clearRowKeys(true); // Clear selection and reload table
                    }
                }
            })
        }

        const tableButtons = () => {
            if (hasUploader) {
                return(
                        <Dropdown
                                key={ shortid.generate() }
                                overlay={ menu(uploader.subitems) }
                        >
                                <Button
                                    title='Uploader'

                                >
                                    <Icon type = "cloud-upload" />
                                    { !actionList ? 'Upload' : null }
                                    <Icon type="down" />
                                </Button>
                            </Dropdown>
                        );
            }
        }

        return (
            <div style={style} className = "action-container" >
                <Menu mode = "horizontal" >
                    <Button
                        title='Reload'
                        onClick = {() => reload(true)}
                    >
                       <Icon type = "reload" />
                       {!actionList ? 'Reload' : null}
                    </Button>
                    { tableButtons() }
                    { actionList ? mapDropdown(actionList) : '' }
                </Menu>
            </div>
        )
}

export const ButtonRow = React.memo(ButtonRowView);