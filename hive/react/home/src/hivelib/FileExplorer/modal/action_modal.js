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
import { gClip , linkURL } from "./../controller/buttonUrlController";
import { getPrefix, getPrefixPlain } from './../../modal/url_modal';
import { CustomRequest as RequestConstructor } from './../../modal/request_modal';
////////////////////////////////
/// Constructing Action Data ///
////////////////////////////////
const calcActionData = (actionData,actionList) => {
        //Filter out only actionData that is actionList

        const actionList_keys = Object.keys(actionList);
        const treeObject = { name: '/', subitems: [] };

        actionData.forEach( el => {

            let item = {};

            if(actionList_keys.includes(el.name)) {

                let x = treeObject;
                if (!el.path){
                    item['align'] = 'left';
                    item['disabled'] = actionList[el.name];
                    item['subitems'] = [];

                    for (let key in el){
                        const newKey = key.indexOf('_') === 0 ? key.substring(1) : key;
                        item[newKey] = el[key];
                    }

                    x.subitems.push(item);


                }else{

                    // 1.Split path
                    const path = el.path.slice(1).split('/');

                    // 2b.Loop over each item of the path cheching whether it's a part of the tree
                    for (let i = 0; i < path.length; i++) {
                        item = x.subitems.find(item => item.name === path[i]);

                        if (!item) {

                            const item1 = {};
                            if (i === path.length - 1) {

                                item1['align'] = 'left';
                                item1['disabled'] = actionList[el.name];
                                item1['name'] = path[i];
                                item1['subitems'] = [];

                                for(let key in el){
                                    const newKey = key.indexOf('_') === 0 ? key.substring(1) : key;
                                    item1[newKey] = el[key];
                                }

                                x.subitems.push(item1);
                                x = item1;
                            } else {
                                item1['name'] = path[i];
                                item1['subitems'] = [];
                                item1['path'] = `/${path.slice(0,i+1).join('/')}`

                                x.subitems.push(item1);
                                x = item1;
                            }
                        } else {
                            x = item;
                        }
                    }
                }
            }

        });
        // get order
        treeObject.subitems.forEach( sub => {
            for( let i = 0 ; i < actionData.length; i++){
                if (actionData[i].path === sub.path){
                    let order = actionData[i].order || Number(actionData[i].order) === 0 ?  Number(actionData[i].order) : 1000
                    sub['order'] = order
                    break;
                }
            }
        })
        treeObject.subitems.sort(function (a, b) {
            return a.order - b.order;
          });
       return treeObject.subitems;
}

export const setUpActionList = (actionsData,selectedItems) => {
        if(selectedItems.length > 0){
            const actionList_bycount = {} ;
            selectedItems.forEach(el => {
                if (el._action){
                    el._action.forEach( action => {
                        if(actionList_bycount[action]){
                              actionList_bycount[action] ++;
                        }else{
                              actionList_bycount[action] = 1;
                        }
                    })
                }
            })

            //   actionList_bycount = {cut:23 , edit:20 , delete: 15 , share: 23}
            for( let action in actionList_bycount) {
                actionList_bycount[action] = actionList_bycount[action] < selectedItems.length ? 'disabled' : null;
            }
            //   actionList_bycount = {cut: null , edit: null , delete: 'disable' , share: null}
            return Object.entries(actionList_bycount).length > 0  ? calcActionData( actionsData , actionList_bycount ) : null;

        }else{
            return null;
        }
}

//////////////////////////////////////////
/// Constructing URL for Action Button ///
//////////////////////////////////////////
export const hrefConstruct = async (url, info,target) => {
    // 4.
    let ts , isString=false , isPrompt=false , isFunc = false, doEval = true ;
    const evalSingleVar = (variable) => {
        if (url.indexOf("javascript:") === 0 || url.indexOf("function:") === 0) {
            isFunc = true;
        }
        if (variable.substring(0,2)==="s:") {
            isString=true;
            variable = variable.substring(2);
        } else if (variable.substring(0,2)==="?:") {
            isPrompt=true;
            variable = variable.substring(2);
        }

        let expr = info[variable] || info[variable] === 0 ? info[variable] : '' ;
        ts = expr;

        if(isPrompt){

            // Find vars ${'var'}
            //Temp for prompt
            let {str, single} = exctractSingleVar(variable)

            ts = prompt(str, single || 'New Item');

            if(ts === null){
                doEval = false;
                return null;
            }

            ts = encodeURI(ts);
        }
        else if (isString) {
            ts = "\""+expr+"\"";
        }
        return ts;
    }

    // 3. Replace all variables with ones from info object
    const replaceVar = (el) => {
        el = el.slice(2, el.length - 1);
        // check whether el has s: ?:

        el = evalSingleVar(el)

        return el;
    }

    // 2. Find all variable in href
    const findVar = (url) => {
        // begins "$("   ends ")"
        const regex = /\$\(.+?\)/g;
        return url.replace(regex, el => replaceVar(el));
    }

    // 1. Distilles urls from functions
    const funcLink = func => {
            if(func.includes('javascript:')){
               func = func.replace('javascript:','') ;
            }else if(func.includes('function:')){
               func = func.replace('function:','') ;
            }
            return func;
    }

    //! Temp
    const exctractSingleVar = (str) =>{
       // begins "${"   ends "}"
       const regex = /\$\{.+?\}/;
       let el = str.match(regex)
       //Temp for prompt
       if(el){
        str = str.replace(el[0], '')
       }
       let single = el ? el[0].slice(2, el[0].length - 1) : null;
       single = single !== null &&  (info[single] || info[single] === 0) ? info[single] : single;
       return {single, str};
    }

    url = funcLink(findVar(url));
    if(!doEval){
        return null;
    }else if (isPrompt) {
        return await linkURL( url , 'ajax');
    }else if(isFunc) {
        let gClipRes = await new Function('gClip','return ' + url)(gClip);
        return gClipRes;
    }else if(url.indexOf("/r/") === 0){
        window.open(getPrefixPlain() + url, '_blank');
    }else if(target === "ajax"){
        url = url.indexOf('cmdr') === -1 ? url.replace('cmd','cmdr') : url;
        let request = new RequestConstructor({url_parameters:url})
        let result = {} // We need unified way of receiving back info
        await request.handleFetch()
                .then(response =>{
                    result['status'] = response.status
                    return (response.json() || response.text())
                })
                .then((json) => {
                    result['data'] = json
                })
        return result;
    }else{
        //downloads does not open new window , opens new window on export
        window.open(getPrefix() + url, '_blank');
    }
}