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
//MenuItems

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
            let item1;
            if (!item) {
                if (i === path.length - 1) {
                   item1 = {
                        name: path[i],
                        title: el.title ? el.title : path[i].replaceAll('_', ' '),
                        path: path ? path.slice(0,path.length-1) : path,
                        id: el._id,
                        url: el.url,
                        description: el.description,
                        order: el.order,
                        align: el.align ? el.align : 'left' ,
                        children: []
                    }
                } else {
                    item1 = {
                        name: path[i],
                        title: el.title ? el.title : path[i].replaceAll('_', ' '),
                        align: el.align ? el.align : 'left' ,
                        children: []
                    }
                }
                if (el.target) item1.target = el.target;
                x.children.push(item1);
                x = item1;
            } else {
                x = item;
                if (i === path.length - 1) {
                    //Edit path proporties
                    x.title = el.title ? el.title : path[i].replaceAll('_', ' ');
                    x.description = el.description;
                    x.path = path.slice(0,path.length-1);
                    x.url = el.url;
                    x.id = el._id;
                    x.order = el.order;
                    x.align = el.align ?  el.align : x.align;
                    if (el.target) x.target = el.target;
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
          }
       }
       return a.order - b.order;
   })
   return array;
}