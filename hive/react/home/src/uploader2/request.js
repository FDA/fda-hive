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
import axios from "axios";
import modals from '../hivelib/modal/modal_collector';

const { url_modal } = modals;

// Find folder
const filterDuplicatesArr = (arr) =>{
    return arr.filter((item, index) => arr.indexOf(item) === index);
}

const getProjectParam = () => {
    if (sessionStorage.getItem("projectID")) {
        return {
            params: {
                projectID: sessionStorage.getItem("projectID")
            }
        };
    } else {
        return {};
    }
}

export const findFolderPath = (key) =>
    new Promise(resolve => {
            const id = Number(key);

            const hrefSearch = [{name:'cmd',value:'objQry'},
                                {name:'raw',value:'1'},
                                {name:'qry',value: `y=[];f=function(xid,pid){x=alloftype("^directory$+",{ child: pid });if(x as int){x.foreach({f(xid,this)}); }else{ z = xid as obj;y.push({id: xid,name: z.name});}z = pid as obj;y.push({id: pid ,name: z.name});return y;};return(f(${id},${id}));`},
                               ]

            let url = url_modal.getPrefix();
            let query = "?";
            hrefSearch.forEach((item) => {
                query = url_modal.addURLParam(query,item.name,item.value);
            });

            (async () => {
                await axios.get(url + query, getProjectParam())
                .then(json =>{
                    if(json.status === 200 ){
                        return json.data;
                    }
                }).then(arr =>{
                    let expandedKeys = []
                    arr.forEach((item,i,ar) => {
                        if(id !== item.id){
                            expandedKeys.push(item);
                        }else if( i === 0){
                            return null;
                        }else{
                            expandedKeys.push(item);
                        }
                    });
                    expandedKeys = filterDuplicatesArr(expandedKeys).map(item => item.name.toString());
                    resolve(expandedKeys);
                })
            })().catch(error => console.error);
});

export const findFolderPathKey = (key) =>
    new Promise(resolve => {
            const id = key;

            const hrefSearch = [{name:'cmd',value:'objQry'},
                                {name:'raw',value:'1'},
                                {name:'qry',value: `y=[];f=function(xid,pid){x=alloftype("^directory$+",{child:pid});if(x as int) {x.foreach({f(xid,this)});}else{y.push(xid);} y.push(pid);return y;};return(f(${id},${id}));`},
                               ]

            let url = url_modal.getPrefix();
            let query = "?";
            hrefSearch.forEach((item)=>{
                query = url_modal.addURLParam(query,item.name,item.value);
            });

            (async () => {
                await axios.get(url + query, getProjectParam())
                .then(json =>{
                    if(json.status === 200 ){
                        return json.data;
                    }
                }).then(arr =>{
                    let expandedKeys = []

                    arr.forEach((item,i,ar) => {
                        if(key.toString() !== item.toString()){
                            expandedKeys.push(item);
                        }else if( i === 0){
                            return null;
                        }else{
                            expandedKeys.push(item);
                        }
                    });
                    expandedKeys = filterDuplicatesArr(expandedKeys).map(item => item.toString());
                    resolve(expandedKeys);
                })
            })().catch(error => console.error);
});

export const allUsers = () => {
    return(
        new Promise(resolve => {
                const hrefSearch = [{name:'cmdr',value:'usrList'},
                                    {name:'active',value:'1'},
                                    {name:'grp',value:'0'},
                                    {name:'primaryGrpOnly',value:'1'},
                                   ]

                let url = url_modal.getPrefix();
                let query = "?";
                hrefSearch.forEach((item,i,arr)=>{
                    query = url_modal.addURLParam(query,item.name,item.value);
                });

                (async () => {
                    await axios.get(url + query, getProjectParam())
                    .then( json =>{
                        if(json.status === 200 ){
                            return json.data;
                        }
                    }).then(list =>{
                        list = list.split(/\r?\n/);
                        resolve(list);
                    })
                })().catch(error => console.error);
        })
    )
}
