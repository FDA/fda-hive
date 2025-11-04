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
import {  Tooltip  , Avatar , Progress ,Badge } from 'antd';
import shortid from 'shortid';
import { getPrefixPlain , getPrefix } from './../../modal/url_modal'
import TypeIcons from "./../../modal/type_icons";
import { formatDatetime } from "./../../modal/time_modal";
import { stripHTMLtags } from "./../../modal/html_modal";
import{ TableButton } from "../view/table-button_view"

const hive_statuses = [ 'unknown', 'waiting', 'processing', 'running', 'suspended', 'done', 'killed', 'failure', 'error', 'unknown' ];

//////////////////////
/// Create Columns ///
//////////////////////
/// Loop trough selected type ARRAY and CREATE COLUMNS based on assosiated type
export const columnsConstruct = ( type_columns , character_cnt, column_names, column_orders ) => {
        let columns = [];
        if(type_columns && type_columns.length > 0){
            type_columns.forEach( (el,index) => {
                if(el !== 'submitter'){
                    const item = {
                            //fixed: el === '_id' ? 'left' : null,
                            /////
                            headerName: el === "_type" ? ' ' : column_names[el] ? column_names[el] :  el,
                            field: el,
                            // cart , biocompute dnaseq
                            // Make sure that at least one column has unset width to avoid table header and body misaligning
                            width: el === '_type' ? 100 : !character_cnt[el] ? 100 : (character_cnt[el]*7 + 30),
                            maxWidth: el === '_type' ? 100 : null,
                            minWidth: el === '_id' ? 100 : el === 'progress100' ? 120 : 95,
                            //el === 'created' ||  width: el === 'name' || el === '_brief' || !character_cnt[el] ? null :
                            cellRendererFramework:  el === '_type'  ? (props) => tableAvatarAg(props,column_orders) : el === 'created' ? properDateAg : el === '_brief' ? briefAG : el === '_id' ? runAG : el === 'progress100'? progressAG : el === 'status' ? statusAg : '',
                            //rowDrag:  el === '_type'  ? true : false,
                    }
                    columns.push(item);
                }
            })
        }
        return columns;
}

const statusAg = (props) => {

    let ant_statuses = {
        unknown: 'default',
        waiting: 'warning',
        processing: 'processing',
        running: 'processing',
        suspended: 'error',
        killed: 'error',
        failure: 'error',
        error: 'error',
        done: 'success',
    }
    if(props.value){
        let cur_stat_txt = hive_statuses[props.value]
        return (<Badge status={ant_statuses[cur_stat_txt]} text={cur_stat_txt} />);
    }
    return '';
}

const tableAvatarAg = (props,column_orders) => {
    let el = props.value;
    let title =  column_orders[el] && column_orders[el].title ? column_orders[el].title : el
    if(!TypeIcons[el] || !TypeIcons[el].icon){
        return(<>
            <Tooltip key={shortid.generate()} title={title} placement="right">
                <Avatar
                    size="small"
                    style={{marginLeft: '15px'}}
                    key={shortid.generate()}
                    onError={()=>true}
                >
                    {el.charAt(0).toUpperCase()}
                </Avatar>
            </Tooltip>
        </>
        );
    }
    let str = TypeIcons[el].icon;

    if(!/\//.test(str)){
        str = `img/${str}`
    }
    return(<>
        <Tooltip key={shortid.generate()} title={title} placement="right">
            <Avatar
                size="small"
                style={{marginLeft: '15px'}}
                src={`${getPrefixPlain()}/${str}`}
                key={shortid.generate()}
                onError={()=>true}
            />
        </Tooltip>
        </>
    );
}

const properDateAg = (props) => {
    let el = props.value;
    return  <Tooltip key={shortid.generate()} title={el} placement="right">{formatDatetime(el)}</Tooltip> ;
}

const briefAG = (props) => {
    let el = props.value;
    return <span>{stripHTMLtags(el)}</span>;
}

const runAG = (props) => {
    // icon="arrow-right"
//    <Tooltip key={shortid.generate()} title="open" placement="right">
//                        <Button href={`${getPrefix() }?cmd=${ props.data.submitter}&id=${props.data._id}`} type="primary" size="small" target="_blank" shape="round" style={{position: 'absolute',top:'-5px' , left: '70px' ,  padding: '0px 5px 0 7px' , height: '23px'}} icon="arrow-right"  />
//    </Tooltip>
    if(props.data.submitter) {
        return (<div style={{width: 100}}>

                        <TableButton
                            href={`${getPrefix() }?cmd=${ props.data.submitter}&id=${props.data._id}`}
                            style={{position: 'absolute',top:'6px' , left: '70px'}}
                            content="open"
                            rel="opener"
                        />
                    <span>{props.value} </span>
                </div>);
    }else{
        return <span>{props.value}</span>;
    }
}

const progressAG = (props) => {
    let item_stat = parseInt(props.data.status);
    let status = item_stat  === 2 || item_stat  === 3 ? 'active' : item_stat  === 4 || item_stat > 5 ? 'exception' : 'success';
    if(props.data.hasOwnProperty('progress100')){
        return (<div style={{ width: 95 }}>
                    <Progress percent={props.value} size="small" status={status}/>
                  </div>);
    }
       return '';
}