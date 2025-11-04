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

// import debounce from 'lodash.debounce'

export class Toolbar extends PortalComponent {
  static className = 'portal__toolbar'
  constructor($root, options) {
    super($root, {
      name: 'Toolbar',
      listeners: ['input'],
      ...options
    })
  }
  // handler = debounce( (event, that) => {
  //   that.$emit('toolbar:input', $(event.target).text() ), 1000, {
  //     'leading': false,
  //     'trailing': true
  //   }
  // }
  // )

  handleClear = () => {
      document.getElementById("portalSearch").value ='';
  };

  onInput(event) {
    // this.handler(event, this)
    this.$emit('toolbar:input', $(event.target).text() )
  }

  toHTML() {
    return `
    <span class="ant-input-search ant-input-affix-wrapper" style="width: 200px; z-index:700">
      <input placeholder="Search" class="ant-input" type="text" value="" id="portalSearch">
      <span class="ant-input-suffix" style="z-index:700">
        <i id="clearSearch" style="display: none"
        onclick="document.getElementById('portalSearch').value ='';
        document.getElementById('clearSearch').style.display = 'none';
        document.getElementById('portalSearch').dispatchEvent(new Event('input', { bubbles: true }));"
        aria-label="icon: close-circle" role="button" tabindex="-1" class="anticon anticon-close-circle ant-input-clear-icon"><svg viewBox="64 64 896 896" focusable="false" class="" data-icon="close-circle" width="1em" height="1em" fill="currentColor" aria-hidden="true"><path d="M512 64C264.6 64 64 264.6 64 512s200.6 448 448 448 448-200.6 448-448S759.4 64 512 64zm165.4 618.2l-66-.3L512 563.4l-99.3 118.4-66.1.3c-4.4 0-8-3.5-8-8 0-1.9.7-3.7 1.9-5.2l130.1-155L340.5 359a8.32 8.32 0 0 1-1.9-5.2c0-4.4 3.6-8 8-8l66.1.3L512 464.6l99.3-118.4 66-.3c4.4 0 8 3.5 8 8 0 1.9-.7 3.7-1.9 5.2L553.5 514l130 155c1.2 1.5 1.9 3.3 1.9 5.2 0 4.4-3.6 8-8 8z"></path></svg></i>

        <i aria-label="icon: search" tabindex="-1" class="anticon anticon-search ant-input-search-icon">
          <svg viewBox="64 64 896 896" focusable="false" class="" data-icon="search" width="1em" height="1em" fill="currentColor" aria-hidden="true">
            <path d="M909.6 854.5L649.9 594.8C690.2 542.7 712 479 712 412c0-80.2-31.3-155.4-87.9-212.1-56.6-56.7-132-87.9-212.1-87.9s-155.5 31.3-212.1 87.9C143.2 256.5 112 331.8 112 412c0 80.1 31.3 155.5 87.9 212.1C256.5 680.8 331.8 712 412 712c67 0 130.6-21.8 182.7-62l259.7 259.6a8.2 8.2 0 0 0 11.6 0l43.6-43.5a8.2 8.2 0 0 0 0-11.6zM570.4 570.4C528 612.7 471.8 636 412 636s-116-23.3-158.4-65.6C211.3 528 188 471.8 188 412s23.3-116.1 65.6-158.4C296 211.3 352.2 188 412 188s116.1 23.2 158.4 65.6S636 352.2 636 412s-23.3 116.1-65.6 158.4z">
            </path>
          </svg>
        </i>
      </span>
    </span>
    `
  }
}
