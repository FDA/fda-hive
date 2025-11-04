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
export class FileModal {
    constructor(props){
        this.props = props;

        this.hasFiles = {
            dir: new Set(),
            ext: new Set(),
            filename: new Set()
        }

        this.counter = 0;
        this.fileCounter = 0
        this.allFiles = []

        this.currentDir = new Set();
    }

    loopFiles = (files , allFiles = []) => {
        this.allFiles = [...allFiles];
        for (let i = 0 ; i < files.length; i++){
          let file = files[i];
          // check if file name or path is dublicate
          this.addParToFile(file);
        }
        this.counter ++
        this.currentDir.clear()
        return {files};
    }

    addParToFileB = (file) => {
        let name = file.name;
        let path;
        const isFile = file instanceof File;
        if((isFile && file.webkitRelativePath === "") || file.fullPath === ""){
             path = ""
        } else if(isFile) {
             path = file.webkitRelativePath
        }else {
             path = file.fullPath
        }
        var regName = new RegExp(name);
        path = path.indexOf('/') === 0 ? path.replace(regName,'').slice(1,path.length) : path.replace(regName,'');
        let uploadPath =`${path}${name}`
        if(this.allFiles.length > 0) {
            let duplicate = this.checkDuplicates(path, name);
            path = duplicate.path
            name = duplicate.name
            uploadPath = duplicate.uploadPath
        }else{
            if(path === ""){
                let ext = this.getFileExtension(name);
                //this.hasFiles.ext.push(ext)
                this.hasFiles.ext.add(ext)
                //this.hasFiles.filename.push(name)
                this.hasFiles.filename.add(name)
            }else{
                let fs = uploadPath.indexOf('/');
                let dir = path.slice(0,fs);
                // dir not recorded
                if(!this.hasFiles.dir.has(dir)) this.hasFiles.dir.add(dir)
                if(!this.currentDir.has(dir)) this.currentDir.add(dir)
            }
        }
        file['namepath'] = {name,path}
        file['key'] = `file://${uploadPath}`
        this.fileCounter ++;
        file['id'] = this.fileCounter;
        file['uploadPath'] = uploadPath // `${this.counter}${path}${name}` => this.counter messedup path comparison
        return file;
    }

    clearAll = () => {
        this.hasFiles.dir.clear()
        this.hasFiles.ext.clear()
        this.hasFiles.filename.clear()

        this.counter = 0;
        this.fileCounter = 0
        this.allFiles = []
        this.currentDir.clear()
    }

    removeFileFromSet = (fileName) => {
        if(this.hasFiles.filename.has(fileName)){
            this.hasFiles.filename.delete(fileName)
        }
    }
    removeDirFromSet = (dirName) => {
        if(this.hasFiles.dir.has(dirName)){
            this.hasFiles.dir.delete(dirName)
        }
    }

    addParToFile = (file) => {
        let name = file.name;
        let path;
        const isFile = file instanceof File;
        if((isFile && file.webkitRelativePath === "") || file.fullPath === ""){
             path = ""
        } else if(isFile) {
             path = file.webkitRelativePath
        }else {
             path = file.fullPath
        }
        //var regName = new RegExp(name);
        // path = path.indexOf('/') === 0 ? path.replace(regName,'').slice(1,path.length) : path.replace(regName,'');
        path = path.indexOf('/') === 0 ? path.replace(name,'').slice(1,path.length) : path.replace(name,'');
        let uploadPath =`${path}${name}`
        if(this.allFiles.length > 0) {
            let duplicate = this.checkDuplicates(path, name);
            path = duplicate.path
            name = duplicate.name
            uploadPath = duplicate.uploadPath
        }else{
            if(path === ""){
                let ext = this.getFileExtension(name);
                //this.hasFiles.ext.push(ext)
                this.hasFiles.ext.add(ext)
                //this.hasFiles.filename.push(name)
                this.hasFiles.filename.add(name)
            }else{
                let fs = uploadPath.indexOf('/');
                let dir = path.slice(0,fs);
                // dir not recorded
                if(!this.hasFiles.dir.has(dir)) this.hasFiles.dir.add(dir)
                if(!this.currentDir.has(dir)) this.currentDir.add(dir)
            }
        }
        file['namepath'] = {name,path}
        file['key'] = `file://${uploadPath}`;
        this.fileCounter ++;
        file['id'] = this.fileCounter;
        file['uploadPath'] = uploadPath // `${this.counter}${path}${name}` => this.counter messedup path comparison
        return file;
    }

    checkDuplicates = (path,name) => {
        // No path means no need to look at dir repeat only ext and file name.
        //1a. If no path see if extension been already used if so check filename duplicate.
        //   Otherwise record extension and filename for further reference
        //1b. If there is path, compare to existing first directories. Modify duplicate dir accordingly.
        //    Record used dir for further reference
        if (path === "") {
            let ext = this.getFileExtension(name);
            // handle null !!!
            if(ext && this.hasFiles.ext.has(ext)){
                // check with file names
                let i = 0;
                let checkName = name;
                while(this.hasFiles.filename.has(checkName)){
                    // cut off extention
                    i++
                    checkName = name.replace(ext, `(${i})${ext}`)
                }
                name = checkName
                this.hasFiles.filename.add(name)
            }else if(ext && !this.hasFiles.ext.has(ext)){
                this.hasFiles.ext.add(ext)
                this.hasFiles.filename.add(name)
            }
        } else if (path && path.indexOf('/') > 0){
            const fs = path.indexOf('/');
            let dir  = path.slice(0,fs);
            if(this.hasFiles.dir.has(dir) && !this.currentDir.has(dir)){
                let i = 0;
                let checkDir = dir;
                while(this.hasFiles.dir.has(checkDir) && !this.currentDir.has(checkDir)){
                    i++
                    checkDir = `${dir}(${i})`
                }
                dir = checkDir
                path = dir + path.slice(fs,path.length)
            }
            if (!this.currentDir.has(dir)){
                this.currentDir.add(dir)
            }
            if (!this.hasFiles.dir.has(dir)){
                this.hasFiles.dir.add(dir)
            }

        }
        let uploadPath = `${path}${name}`;

        return {path,name,uploadPath}
    }
    getFileExtension = (name) => {
        let p = name.indexOf('.')
        if(p > -1){
            return name.slice(p,name.length)
        }
        return null
    }

    get getExtensions(){
        return this.extensions;
    }
}
