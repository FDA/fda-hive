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
//Action Creator
export const progressView = (progressStat) => {
    //Return an action
    return {
        type: 'PROGRESS_VIEW',
        payload: progressStat
    };
};

export const handleProgress = (uploadState) => {
    // Handles Buttons showed
    return {
        type: 'HANDLE_PROGRESS',
        payload: uploadState
    };
};

export const clearUpload = (clear) => {
    return {
        type: 'CLEAR_UPLOAD',
        payload: clear
    }
}

export const loadingFiles = (loading) =>{
    return {
        type: 'LOADING_FILES',
        payload: loading
    }
}

export const getFileList = (filesList) => {
    return {
        type: 'ADD_FILE_LIST',
        payload: filesList
    };
};

export const removeFileList = (index) => {
    //Return an action
    return {
        type: 'REMOVE_FILE',
        payload: index
    };
};

export const removeKeyFileList = (keyarr) => {
    //Return an action
    return {
        type: 'REMOVE_FILES_BY_KEY',
        payload: keyarr
    };
};
export const removeFileListByPath = (keyarr) => {
    //Return an action
    return {
        type: 'REMOVE_FILES_BY_PATH',
        payload: keyarr
    };
};


// Can receive either an empy array to clear
// the whole files list or fully new array without concatenation
export const subFileList = (arr) => {
    //Return an action
    return {
        type: 'SUB_FILE_LIST',
        payload: arr
    };
};

export const addModalLocalFiles = (files) => {
    //Files for modal window local state
    return {
        type: 'ADD_MODAL_LOCAL_FILES',
        payload: files
    };
};

//Form Archiver is an object, receives objects
export const addToFormArchiver = (objs) => {
    return {
        type: 'ADD_FORM_ARCHIVER',
        payload: objs
    };
}
export const recordSVCArchiverId = (id) => {
    return {
        type: 'SVC_ARCHIVER_ID',
        payload: id
    };
}
export const recordPipelineLink = (link) =>{
    return {
        type: 'SVC_PIPELINE_LINK',
        payload: link
    };
}

export const handleIsOnline = (state) => {
    return {
        type: 'SWITCH_ON_OFFLINE',
        payload: state
    }
}