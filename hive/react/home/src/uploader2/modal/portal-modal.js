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
export const addItem = (name,path, id, url, description, order , align = 'left', children = []) => {
    const item = {
        path: path ? path.splice(0,path.length-1) : path,
        name,
        id,
        url,
        description,
        order,
        align,
        children
    }
    return item;
}

export const filterPaths = (myData) => {
    const treeConstruct = {
        name: '/',
        children: []
    }
    // Structure Tree
    myData.forEach(el => {
        // 1.Split path
        let path = el.path.slice(1).split('/');

        // 2b.Loop over each item of the path cheching whether it's a part of the tree
        var x = treeConstruct;
        for (let i = 0; i < path.length; i++) {
            let item = x.children.find(item => item.name === path[i]);
            if (!item) {
                if (i === path.length - 1) {
                    const item1 = addItem( path[i], path, el._id, el.url, el.description, el.order,el.align);
                    x.children.push(item1);
                    x = item1;
                } else {
                    const item1 = addItem(path[i]);
                    x.children.push(item1);
                    x = item1;
                }
            } else {
                x = item;
                if (i === path.length - 1) {
                    //Edit path proporties
                    // x.align = x.align
                    x.description = el.description;
                    x.path = path.splice(0,path.length-1);
                    x.url = el.url;
                    x.id = el._id;
                    x.order = el.order;
                }

            }
        }
    });
    return treeConstruct.children;
}

export const sortByOrder = (array) => {
    array.sort( (a,b) => {
       if(a.order === b.order || (a.order === undefined && b.order === undefined)){
            if (a.name < b.name) {
                return -1;
            } else if (a.name > b.name) {
                return 1;
            } else if (a.name === b.name) {
               // names must be equal
                return 0;
            }
       }else{
          if(a.order === undefined){
              return 1;
          } else if (b.order === undefined){
              return -1;
          } else {
              return a.order - b.order;
          }
       }
   })
   return array;
}