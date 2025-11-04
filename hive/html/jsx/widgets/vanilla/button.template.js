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
loadCSS(`css/hv/buttons.css`)

function createButton(button) {
    let data = button.event ? `data-event="${button.event}"` : ''
    if(button.data) data += generateDataAtt(button.data)
    let _button = button.name || button.icon 
                    ? `<button 
                            class="hv-btn ${button.classes || ''}" 
                            ${data}
                        >
                            ${button.icon || ''}
                            ${button.name || ''}
                        </button>`
                    :''
    if (button.prepend) _button = `<div class="hv-btn-bundle"> ${button.prepend} ${_button}</div>`
    return _button;
}

function createFakeButton(button){
    let data = button.event ? `data-event="${button.event}"` : ''
    if(button.data) data += generateDataAtt(button.data)

    let _button = button.name || button.icon 
                    ? `<span class="hv-btn ${button.classes || ''}" ${data}>${button.name || ''}</span>`
                    :''
    return _button;
}

function createButtonSet(buttons){
    if(!buttons || buttons.length === 0) return ''
    let _buttons =''
    buttons.forEach( function(button){
        _buttons += createButton(button)
    });
    return `<div class="hv-button-set">${_buttons}</div>`
}

function generateDataAtt(data){
    let dataAtt = ''
    Object.keys(data).forEach(function(key){
        dataAtt += ` data-${key}="${data[key]}"`
    })
    return dataAtt
}