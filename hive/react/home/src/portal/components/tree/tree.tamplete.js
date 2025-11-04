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
import { formatTitle } from '../../core/utils'
import { hrefFriendlyPath } from '../../core/utils'

function createBranch(tree, sort) {
  if (sort === 'alph') {
    tree = tree.sort(function(a, b) {
      const pathA = a.path; // ignore upper and lowercase
      const pathB = b.path; // ignore upper and lowercase
      if (pathA < pathB) {
        return -1;
      }
      if (pathA > pathB) {
        return 1;
      }

      // names must be equal
      return 0;
    });
  }
  const branch = tree.map( ( sprout ) => {
    const title = formatTitle(sprout.name)
    const modPath = hrefFriendlyPath(sprout.path)
    const href = sprout.leaves ? `href="#${modPath}"` : ''
    const classes = sprout.leaves === 0 ? `class="tree-link_inactive"` : ''
    if (sprout.children.length) {
      return `
       <li role="treeitem">
            <span class="tree-switcher tree-switcher_close" data-switcher-branch="closed">
              <i aria-label="icon: down" class="tree-switcher-icon">
                <svg viewBox="64 64 896 896" focusable="false" class="" data-icon="down" width="1em" height="1em" fill="currentColor" aria-hidden="true">
                  <path d="M884 256h-75c-5.1 0-9.9 2.5-12.9 6.6L512 654.2 227.9 262.6c-3-4.1-7.8-6.6-12.9-6.6h-75c-6.5 0-10.3 7.4-6.5 12.7l352.6 486.1c12.8 17.6 39 17.6 51.7 0l352.6-486.1c3.9-5.3.1-12.7-6.4-12.7z"></path>
                </svg>
              </i>
            </span>

            <span title="${sprout.title || title}" class="tree-node-content-wrapper ">
              <a ${href} ${classes} data-tree-link="${modPath}">${title}</a>
            </span>

            <ul class="tree-child-tree tree-child-tree_hidden" data-expanded="true" role="group">
            ${createBranch(sprout.children, sort)}
            </ul>
        </li>
      `
    } else {
      return `
        <li class="tree-treenode-switcher-close" role="treeitem">
          ${tree.length > 1 ? '<span class="tree-switcher tree-switcher-noop"></span>' : ''}
          <span title="${sprout.title || title}" class="tree-node-content-wrapper">
            <a  ${href} ${classes} data-tree-link="${modPath}">${sprout.title || title}</a>
          </span>
        </li>
      `
    }
  })

  return branch.join('')
}

export function createTree(tree = [], sort) {
  const displayCloud = localStorage.getItem('displayCloud') == null || localStorage.getItem('displayCloud') === 'true';

  return `
  <div style=" display: ${ displayCloud? 'flex': 'none'}; justify-content: flex-end; margin-top: -20px;"
    onclick=" localStorage.setItem('displayCloud', 'false');
    document.querySelector('.portal__tree').style.display = 'none';
    document.getElementById('divTableArrow').style.display = 'block';
    "
  >
    <i aria-label="icon: left" class="anticon anticon-left" style="font-size: 15px; color: rgb(255, 255, 255); ">
    <svg viewBox="64 64 896 896" focusable="false" class="" data-icon="left" width="1em" height="1em" fill="currentColor" aria-hidden="true">
    <path d="M724 218.3V141c0-6.7-7.7-10.4-12.9-6.3L260.3 486.8a31.86 31.86 0 0 0 0 50.3l450.8 352.1c5.3 4.1 12.9.4 12.9-6.3v-77.3c0-4.9-2.3-9.6-6.1-12.6l-360-281 360-281.1c3.8-3 6.1-7.7 6.1-12.6z"></path></svg></i>
  </div>

  <div class="tree-header">App Portal
  <span style="position: relative; top: -5px; display: none"
  onclick=" let displayCloud = localStorage.getItem('displayCloud') == null || localStorage.getItem('displayCloud') === 'true';
  displayCloud = !displayCloud;  localStorage.setItem('displayCloud', displayCloud);
  if (displayCloud)
    if(!initDisplayCloud) window.location.reload();
    else document.getElementById('divCloud').style.display = 'block';
  else document.getElementById('divCloud').style.display = 'none';
    ">
  <i aria-label="icon: cloud" class="anticon anticon-cloud" style="font-size: 15px; color: rgb(255, 255, 255); ">
  <svg viewBox="64 64 896 896" focusable="false" class="" data-icon="cloud" width="1em" height="1em" fill="currentColor" aria-hidden="true">
  <path d="M811.4 418.7C765.6 297.9 648.9 212 512.2 212S258.8 297.8 213 418.6C127.3 441.1 64 519.1 64 612c0 110.5 89.5 200 199.9 200h496.2C870.5 812 960 722.5 960 612c0-92.7-63.1-170.7-148.6-193.3zm36.3 281a123.07 123.07 0 0 1-87.6 36.3H263.9c-33.1 0-64.2-12.9-87.6-36.3A123.3 123.3 0 0 1 140 612c0-28 9.1-54.3 26.2-76.3a125.7 125.7 0 0 1 66.1-43.7l37.9-9.9 13.9-36.6c8.6-22.8 20.6-44.1 35.7-63.4a245.6 245.6 0 0 1 52.4-49.9c41.1-28.9 89.5-44.2 140-44.2s98.9 15.3 140 44.2c19.9 14 37.5 30.8 52.4 49.9 15.1 19.3 27.1 40.7 35.7 63.4l13.8 36.5 37.8 10c54.3 14.5 92.1 63.8 92.1 120 0 33.1-12.9 64.3-36.3 87.7z">
  </path></svg></i></span>

  <div id="divCloud" /> </div>`
    + `<ul class="hive__tree tree-show-line" role="tree" unselectable="on"> ${createBranch(tree, sort)} </ul>`
}
