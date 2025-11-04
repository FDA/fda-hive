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
import { PortalComponent } from '../../core/PortalComponent';
import { $ } from '../../core/dom';
import { cutOffLeaves } from '../../core/utils';
import { createTree } from './tree.tamplete';

export class Tree extends PortalComponent {
  static className = 'portal__tree'

  constructor($root, options) {
    super($root, {
      name: 'Tree',
      listeners: ['click', 'mousedown']
    })
    this.$root = $root
    this.data = Object.assign({}, options.data)
    this.data = cutOffLeaves(this.data)
  }

  onClick(e) {
    // event delegation
    const _target = e.target

    if ( _target.dataset.switcherBranch|| _target.closest('[data-switcher-branch]') ) {
      const _treeSwitch = _target.dataset.switcherBranch
                            ? _target
                            : _target.closest('[data-switcher-branch]')
      onSwitcher(_treeSwitch)
    }
    if (_target.dataset.treeLink) {
      onScroll(_target)
    }
  }
  onMousedown(event) {
    const $target = $(event.target)
    if ($target.data.resizer === 'tree') {
      const $parent = $target.closest('[data-type="resizable"]')
      const parCoords = $parent.getCoords()

      let delta;
      // let width;
      document.onmousemove = (e) => {
        delta = e.pageX - parCoords.left
        delta = e.pageX - $parent.$el.offsetLeft
        // width = parCoords.width + delta
        $parent.css({width: delta + 'px'})
      }
      document.onmouseup = (e) => {
        e.preventDefault()
        document.onmousemove = null
        document.onmouseup = null
      }
    }
  }

  toHTML() {
    const displayCloud = localStorage.getItem('displayCloud') == null || localStorage.getItem('displayCloud') === 'true';
    const displayTree = 'display:' + (displayCloud? 'block': 'none');

    const $resizer = $.create('span', 'portal__tree-resizer')
        .attr('data-resizer', 'tree')
        .getHtml('outer')
    this.$root.attr('data-type', 'resizable')
    this.$root.attr('style', displayTree);
    return createTree(this.data.children, 'alph') + $resizer
  }
}

function onScroll(_target) {
  document.querySelectorAll('[data-list-title="name"]').forEach( (_el) => _el.classList.remove('title-highlight') )
  const $target = $(`#${_target.dataset.treeLink}`)
  const name = $target.find('[data-list-title="name"]')
  name.classList.add('title-highlight')
  window.onscroll = null
}


function onSwitcher(_el) {
  // 1.rotate switch
  // 2.hid or reveal branches
  const _list = _el.parentNode.querySelector('ul')

  if (_el.dataset.switcherBranch === 'opened') {
    _el.classList.remove('tree-switcher_open')
    _el.classList.add('tree-switcher_close')
    _el.dataset.switcherBranch = 'closed'

    _list.classList.remove('tree-child-tree_visible');
    _list.classList.add('tree-child-tree_visuallyhidden');

    setTimeout( () => {
      _list.classList.add('tree-child-tree_hidden');
    }, 300)
  } else if (_el.dataset.switcherBranch === 'closed') {
    _el.classList.remove('tree-switcher_close')
    _el.classList.add('tree-switcher_open')
    _el.dataset.switcherBranch = 'opened'

    _list.classList.remove('tree-child-tree_hidden');
    _list.classList.add('tree-child-tree_visible');
    _list.classList.remove('tree-child-tree_visuallyhidden');
  }
}
