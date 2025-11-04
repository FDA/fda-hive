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
import { $ } from '../../core/dom';
import modals from '../../../hivelib/modal/modal_collector';
// eslint-disable-next-line camelcase
const { url_modal } = modals;

function createListRecord(record) {
  if (record.hidden) return ''
  let name = record.title || formatTitle(record.name) || ''
  let description = record.description || ''

  if (record && record.search) {
    const reg = new RegExp(record.search, 'gi');
    name = name.replaceAll( reg, (m) => $.create('span', 'highlight').text(m).getHtml('outer'))
    description = description.replaceAll( reg, (m) => $.create('span', 'highlight').text(m).getHtml('outer'))
  }
  return `
    <li class="portal__list-record">
        <div class="portal__table__name"><a href="${url_modal.getPrefix() + record.url}" target="_blank">${name}</a> </div>
        <p class="portal__table__description" >${description}</p>
    </li>
  `
}
function createListRecord1(record) {
  if (record.hidden) return ''
  let name = record.title || formatTitle(record.name) || ''
  let description = record.description || ''

  if (record && record.search) {
    const reg = new RegExp(record.search, 'gi');
    name = name.replaceAll( reg, (m) => $.create('span', 'highlight').text(m).getHtml('outer'))
    description = description.replaceAll( reg, (m) => $.create('span', 'highlight').text(m).getHtml('outer'))
  }
  return `
    <li class="portal__list-record">
        <div class="portal__table__name"><a href="${url_modal.getPrefix() + record.url}" target="_blank">${name}</a> </div>
        <p class="portal__table__description" >${description}</p>
    </li>
  `
}

function createListTitle(name, path, item ) {
  let pathArr = path.split('/');
  pathArr = pathArr.map((name)=>formatTitle(name))
  path = pathArr.join(' > ')
  name = formatTitle(name)
  if (item && item.search) {
    const reg = new RegExp(item.search, 'gi');
    name = name.replaceAll( reg, (m) => $.create('span', 'highlight').text(m).getHtml('outer'))
    path = path.replaceAll( reg, (m) => $.create('span', 'highlight').text(m).getHtml('outer'))
  }

  return `
    <p data-list-title="name">${name}</p>
    <span class="portal__list-title--path"> ${path}</span>
  `
}

function createListTitle1(name, search ) {

  if (search) {
    const reg = new RegExp(search, 'gi');
    name = name.replaceAll( reg, (m) => $.create('span', 'highlight').text(m).getHtml('outer'))
  }

  return name;
}

function createList(list) {
  if (list.hidden || !list.path) return ''
  return `
    <ul class="portal__list" id="${list.path.replace(/\//g, '-')}">
      <li class="portal__list-title">  ${createListTitle1((list.title || list.name), list.path, list)}</li>
      ${list.children.map( (record) => createListRecord1(record) ).join('')}
    </ul>
  `
}
function createList1(list) {
  if (list.hidden) return ''

  let toBeReplaced = '-';

  return `
    <div class="portal__lists-content">
      <span class="item0"><a href="${url_modal.getPrefix() + list.base_app_url}" target="_blank">${createListTitle1(list.base_app_name.replace(new RegExp(toBeReplaced, 'g'), ' '), list.search)}</a></span>
      <span class="item1" style="margin-left: 30px"}}>${list.base_app_version ?? ''} </span>
      <span class="item2">${createListTitle1(list.base_app_description ?? '', list.search)} </span>
    </div>
  `
}

export function createTable(tree = {}, sort) {
  let listNames = Object.keys(tree)
  if (sort === 'alph') listNames = listNames.sort()

  return `
    <div class="portal__lists-header">
        <span>
            Name
        </span>
        <span class="portal__table__description">
            Description
        </span>
    </div>
    <div class="portal__lists">
    ${listNames.map((list) => createList(tree[list])).join('')}
    </div>
  `
}

export function createTable1(tree = {}, sort) {
  let listNames = Object.keys(tree)

  if (sort === 'alph') tree.sort((a, b) => {
    let nameA = a.base_app_name.toLowerCase();
    let nameB = b.base_app_name.toLowerCase();
    if (nameA < nameB) {
      return -1;
    }
    if (nameA > nameB) {
      return 1;
    }
    return 0;
  });

  return `
    <div class="portal__lists-header-new">
        <span class="item0">
            Name
        </span>
        <span class="item1">
            Version
        </span>
        <span class="portal__table__description item2">
            Description
        </span>
    </div>
    <div class="portal__lists">
    ${listNames.map((list) => createList1(tree[list])).join('')}
    </div>
  `
}
