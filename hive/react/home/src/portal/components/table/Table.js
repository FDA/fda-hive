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
import { formatTitle, onlyLeaves } from '../../core/utils';
import { createTable, createTable1 } from './table.tamplate';
import {$} from '../../core/dom'

import { Toolbar } from '../../components/toolbar/Toolbar'

export class Table extends PortalComponent {
  static className = 'portal__table'

  constructor($root, options) {
    super($root, {
      name: 'Table',
      listeners: [],
      ...options
    })
    this.options = options
    this.$root = $root
    this.data = onlyLeaves(options.data)
  }

  getToolbar() {
    const $el = $.create('div', Toolbar.className)

    this.toolbar = new Toolbar($el, this.options)

    $el.html(this.toolbar.toHTML())

    this.$root.append($el)

    return $el.getHtml('outer')
  }

  toHTML() {
    const displayCloud = localStorage.getItem('displayCloud') == null || localStorage.getItem('displayCloud') === 'true';

    const $arrow = $.create('div');

    $arrow.html(`<div style="display: ${ displayCloud? 'none': 'block'}; margin-left: -16px " id="divTableArrow"
      onclick=" localStorage.setItem('displayCloud', 'true');
          if(!initDisplayCloud) window.location.reload(); else document.getElementById('divCloud').style.display = 'block';
          document.querySelector('.portal__tree').style.display = 'block';
          this.style.display = 'none';

    ">
        <i aria-label="icon: left" class="anticon anticon-left" style="font-size: 15px; color: rgb(0, 0, 0); "><svg viewBox="64 64 896 896" focusable="false" class="" data-icon="left" width="1em" height="1em" fill="currentColor" aria-hidden="true"><path d="M724 218.3V141c0-6.7-7.7-10.4-12.9-6.3L260.3 486.8a31.86 31.86 0 0 0 0 50.3l450.8 352.1c5.3 4.1 12.9.4 12.9-6.3v-77.3c0-4.9-2.3-9.6-6.1-12.6l-360-281 360-281.1c3.8-3 6.1-7.7 6.1-12.6z"></path></svg></i>
      </div>`)
    this.$root.append($arrow);

    this.getToolbar()

    this.$table_list = $.create('div', 'portal__table-list')

    this.$table_list.html(createTable1(this.data, 'alph'))

    this.$root.append(this.$table_list)

    this.$root.append( $.create('div').html('<div id="portalFooter" style="margin-top: 40px"/>'));
    return this.$root
  }

  handleSearchInTableList(text) {
    const data = JSON.parse( JSON.stringify(this.data) )
    this.data = findInTable1(data, text)
    this.$table_list.html(createTable1(this.data, 'alph'))

    const clearSearchIcon = document.getElementById("clearSearch");
    if (text !=="")
      clearSearchIcon.style.display = 'block';
    else
      clearSearchIcon.style.display = 'none';
  }

  init() {
    this.toolbar.init()
    super.init()
    this.$on('toolbar:input', this.handleSearchInTableList.bind(this) )
  }
}

function findInTable( data, text) {
  const list = Object.keys(data)
  text = text.toLowerCase()
  for (let i = 0; i < list.length; i++ ) {
    const item = list[i]
    if (!text || text === '') {
      data[item].hidden = false
      data[item].search = null
    } else if ( formatTitle( data[item].path.toLowerCase() ).indexOf(text) >= 0 || (data[item].title && data[item].title.toLowerCase().indexOf(text) >= 0) ) {
      // Title has text
      data[item].hidden = false
      data[item].search = text
    } else {
      data[item].hidden = true
      data[item].search = null
    }

    if (data[item].children) {
      let hasVissible = false
      for (let j = 0; j < data[item].children.length; j++) {
        const child = data[item].children[j]
        if ( formatTitle( child.name.toLowerCase() ).indexOf(text) >= 0 || (child.description && child.description.toLowerCase().indexOf(text) >= 0) || (child.title && child.title.toLowerCase().indexOf(text) >= 0) ) {
          hasVissible = true
          break;
        }
      }
      data[item].children.forEach( (child) => {
        if (!text || text === '') {
          child.hidden = false
          child.search = null
        } else if ( formatTitle( child.name.toLowerCase() ).indexOf(text) >= 0 || (child.description && child.description.toLowerCase().indexOf(text) >= 0) || (child.title && child.title.toLowerCase().indexOf(text) >= 0)) {
          data[item].hidden = false
          child.hidden = false
          child.search = text
        } else {
          child.hidden = data[item].search ? false
                          : !data[item].hidden && !hasVissible ? false
                          : true
          // all chldren of the list that doesnot match the search will be visible if title was matched and no sibling mathced
          child.search = null
        }
      });
    }
  }
  return data
}

function findInTable1( data, text) {
  const list = Object.keys(data)
  text = text.toLowerCase()
  for (let i = 0; i < list.length; i++ ) {
    const item = list[i]
    if (!text || text === '') {
      data[item].hidden = false
      data[item].search = null
    } else if ((data[item].base_app_name && data[item].base_app_name.toLowerCase().indexOf(text) >= 0) ) {
      // Title has text
      data[item].hidden = false
      data[item].search = text
    } else if ((data[item].base_app_description && data[item].base_app_description.toLowerCase().indexOf(text) >= 0) ) {
      // description has text
      data[item].hidden = false
      data[item].search = text
    } else {
      data[item].hidden = true
      data[item].search = null
    }

    if (data[item].children) {
      let hasVissible = false
      for (let j = 0; j < data[item].children.length; j++) {
        const child = data[item].children[j]
        if ( formatTitle( child.name.toLowerCase() ).indexOf(text) >= 0 || (child.description && child.description.toLowerCase().indexOf(text) >= 0) || (child.title && child.title.toLowerCase().indexOf(text) >= 0) ) {
          hasVissible = true
          break;
        }
      }
      data[item].children.forEach( (child) => {
        if (!text || text === '') {
          child.hidden = false
          child.search = null
        } else if ( formatTitle( child.name.toLowerCase() ).indexOf(text) >= 0 || (child.description && child.description.toLowerCase().indexOf(text) >= 0) || (child.title && child.title.toLowerCase().indexOf(text) >= 0)) {
          data[item].hidden = false
          child.hidden = false
          child.search = text
        } else {
          child.hidden = data[item].search ? false
                          : !data[item].hidden && !hasVissible ? false
                          : true
          // all chldren of the list that doesnot match the search will be visible if title was matched and no sibling mathced
          child.search = null
        }
      });
    }
  }
  return data
}
