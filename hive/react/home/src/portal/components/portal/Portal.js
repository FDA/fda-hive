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
import ReactDOM from 'react-dom';
import { HeaderContainer } from '../../../hivelib/Header/index';
import { ProjectToolbar } from "../../../hivelib/project/ProjectToolbar";
import renderMyComponent, {renderCustomComponent} from '../../../hivelib/controller/renderReactComponent';
import { RequestConstructor } from '../../../hivelib/modal/request_modal';
import Loader from '../../../hivelib/Loader/index';
import { FooterContainer } from '../../../hivelib/Footer/footer_container';
import {$} from '../../core/dom'
import TagCloud from '../../../hivelib/TagCloud/wordcloud2';

import { pathToTree } from '../../core/utils';
import { Emitter } from '../../core/Emitter';

export class Portal {
  wordList = [];
  constructor(selector, options) {
    this.$el = $(selector)
    this.components = options.components || []
    this.emitter = new Emitter()
    // eslint-disable-next-line new-cap
    this.loader = Loader()
  }

  async getRoot() {
    const $root = $.create('div', 'portal-wrap')
    const data = await this.getPortal()
    if (!data || data == 'error') {
      return 'error'
    }

    const wordCounts = {};

    data.forEach(item => {
      if (item.base_app_name){
        item.base_app_name.split(/[\s\-_]+/).forEach(word => {
            if (word) {
                wordCounts[word] = (wordCounts[word] || 0) + 1;
            }
        });
      }

      if (item.base_app_description){
        item.base_app_description.split(/[\s\-_]+/).forEach(word => {
            if (word) {
                wordCounts[word] = (wordCounts[word] || 0) + 1;
            }
        });
      }
    });

    this.wordList = Object.keys(wordCounts).map(word => [word, wordCounts[word]]);

    const componentOptions = {
      emitter: this.emitter
    }
    this.components = this.components.map( (Component) => {
      const $el = $.create('div', Component.className)
      componentOptions['data'] = JSON.parse( JSON.stringify(data) )
      const component = new Component($el, componentOptions )

      $el.html(component.toHTML())

      $root.append($el)

      return component
    });

    return $root
  }

  async getPortal() {
    const parameters = {
      cmdr: 'objList',
      mode: 'json',
      type: 'user_app_base+'
    }
    // eslint-disable-next-line camelcase
    const request_menuitems = new RequestConstructor({parameters});
    const menuitems = await request_menuitems.handleFetch()
        .then((response) => {
          if (response.status < 400) {
            return response.json()
          } else {
            return null
          }
        })
        .then((json) => {
          return json
        })
        .catch((e)=>{
          console.warn(e)
        })
    if (!menuitems) return 'error'

    const tree = pathToTree(menuitems.objs)
    const trRoot = findPortal(tree.root)
    if (trRoot) return trRoot;
    else return menuitems.objs;
  }

  handleChildClick = (item) => {
    let inputElement = document.getElementById('portalSearch');
    if (item && item.length>0) {
      inputElement.value = item[0];

      var event = new Event('input', {
        bubbles: true,
        cancelable: true,
      });

      inputElement.dispatchEvent(event);
    }
  };

  async render() {
    //export default function renderMyComponent( Component , element, selected, data, onClick) {
  //ReactDOM.render(<Component selected={selected} data={data}  onClick={onClick} />, element);
  console.log(this.$el);
  console.log(this.$el.node());
    renderCustomComponent(HeaderContainer, {element: this.$el.node(), selected: 'apps', child: ProjectToolbar});
    //renderCustomComponent(ProjectToolbar, {element: this.$el.child.node()});

    //ReactDOM.render(<HeaderContainer selected='apps' />, this.$el.node());

    const $portal = $.create('div', 'portal')
    this.$el.append($portal)

    $portal.html(this.loader)

    const $root = await this.getRoot()
    if ($root !== 'error') {
      $portal.clear().append($root)
      this.components.forEach( (component) => component.init());
    } else {
      $portal.clear().html(`<h1>ERROR</h1>`)
    }


    renderMyComponent(TagCloud,  document.getElementById('divCloud'), '', this.wordList, this.handleChildClick);

    let displayCloud = localStorage.getItem('displayCloud') == null || localStorage.getItem('displayCloud') === 'true';
    if (displayCloud) document.getElementById('divCloud').style.display = 'block';
    else document.getElementById('divCloud').style.display = 'none';

    renderMyComponent(FooterContainer,  document.getElementById('portalFooter'));
  }
}

function findPortal(tree) {
  const children = tree.children
  for (let i = 0; i < children.length; i++ ) {
    const child = children[i]
    if ( child.name.toLowerCase() === 'apps') {
      return child;
    } else {
      findPortal(child)
    }
  }
}