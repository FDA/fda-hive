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
export function capitalize(str) {
  if (typeof str !== 'string') {
    return ''
  }
  return str.charAt(0).toUpperCase() + str.slice(1)
}

export function formatTitle(str) {
  return str.trim().replace(/_/g, ' ')
}

export function cutOffLeaves(tree) {
  tree['leaves'] = 0;
  if (!tree.children) return [];

  if (tree.children.length) {
    tree.children = tree.children.filter(( child ) =>{
      if (child.children.length ) {
        return cutOffLeaves(child)
      } else {
        tree['leaves'] ++
      }
    })
  }
  return tree
}

export function onlyLeaves(tree, list = {}) {
  if (!tree.children) return tree;
  const leaves = tree.children.filter( (child) => {
    if (!child.children || !child.children.length) {
      if (!list[tree.path]) list[tree.path] = tree

      return true;
    } else {
      list = onlyLeaves(child, list )
      return false
    }
  })
  if (leaves.length) list[tree.path]['children'] = leaves

  return list
}

// receives array with paths
export function pathToTree(array = []) {
  if (!Array.isArray(array)) {
    // throw new Error('No array passed into pathToTree')
    array = []
  }

  const data = {
    root: {
      path: '/',
      children: []
    }
  }

  let obj;

  array.forEach( (item) => {
    obj = data.root
    // some don't have a title
    if (!item.path) {
      return;
      throw new Error('path is not defined')
    }

    const addToData = (location, item) => {
      location.children.push(item)
    }

    const checkChildren = (children, kname) => {
      for (let i = 0; i < children.length; i++) {
        const kid = children[i]
        if (kid.name === kname) {
          return i;
        }
      }
      return null;
    }
    const pathArr = item.path.charAt(0) === '/' ? item.path.split('/').slice(1) : item.path.split('/')

    for (let i = 0; i < pathArr.length; i++) {
      const location = checkChildren(obj.children, pathArr[i])
      if ( location !== null) {
        obj = obj.children[location]
      } else {
        const path = pathArr.slice(0, i+1).join('/')
        let itemToAdd = {name: pathArr[i], path: path, children: []}
        if (i === pathArr.length - 1 ) {
          itemToAdd = Object.assign( itemToAdd, item )
          itemToAdd['path'] = path
        }
        addToData(obj, itemToAdd)
        obj = obj.children[obj.children.length-1]
      }
    }
  })

  return data
}

export function hrefFriendlyPath(path) {
  return path.replace(/\//g, '-')
}
