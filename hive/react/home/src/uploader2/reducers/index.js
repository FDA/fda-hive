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
import { combineReducers } from 'redux';

const progressReducer = (progressStat = { progress:0 , uploading:false , error:null }, action) => {
    switch (action.type){
        case 'PROGRESS_VIEW':
            return Object.assign({},progressStat,action.payload);
        default:
            return progressStat
    }
}
//HANDLE_PROGRESS
const progressStateReducer = (uploadState = 'disabled' , action) => {
    switch(action.type){
        case 'HANDLE_PROGRESS':
            return action.payload
        default:
            return uploadState;
    }
}

const clearUploadReducer = (clear = false , action) => {
    switch(action.type){
        case 'CLEAR_UPLOAD':
            return action.payload;
        default:
            return clear;
    }
}
const loadingFilesReducer = (loading = false, action)=>{
    switch(action.type){
        case 'LOADING_FILES':
            return action.payload;
        default:
            return loading;
    }
}
// FilesList is full list of unstructured files to be uploaded
const filesReducer = (filesList = [], action) => {
    switch (action.type){
        case 'ADD_FILE_LIST':
            return [...filesList,...action.payload]
        case 'REMOVE_FILES_BY_KEY':
            let keyarr = action.payload;
            filesList = [...filesList].filter((item)=> !keyarr.includes(item.key))
            return filesList;
        case 'REMOVE_FILES_BY_PATH':
            let path = action.payload;
            filesList = [...filesList].filter((item)=> item.key.indexOf(path) !== 0)
            return filesList;
        case 'REMOVE_FILE':
            let index = action.payload;
            let list = filesList.slice(0,index).concat(filesList.slice(index+1));
            return list;
        case 'SUB_FILE_LIST':
            return action.payload;
        default:
            return filesList
    }
}

const formArchiverReducer = (form = {} , action) => {
    switch(action.type){
        case 'ADD_FORM_ARCHIVER':
            let fullForm = Object.assign({}, form , action.payload);
            return fullForm;
        default:
            return form;
    }
}

const svcArchiverIdReducer = (id = null, action ) => {
     switch(action.type){
        case 'SVC_ARCHIVER_ID':
            return action.payload;
        default:
            return id;
    }
}
const svcPipelineLinkReducer = (link = null, action ) => {
    switch(action.type){
       case 'SVC_PIPELINE_LINK':
           return action.payload;
       default:
           return link;
   }
}


const switchOnOfflineReducer = (online = true, action ) => {
    switch(action.type){
       case 'SWITCH_ON_OFFLINE':
           return action.payload;
       default:
           return online;
   }
}

export default combineReducers({
    progressStat: progressReducer,
    uploadState: progressStateReducer,
    filesList: filesReducer,
    clearUpload: clearUploadReducer,
    formArchiver: formArchiverReducer,
    svcArchiverId: svcArchiverIdReducer,
    svcPipelineLink:svcPipelineLinkReducer,
    isOnline: switchOnOfflineReducer,
    loadingFiles: loadingFilesReducer
})