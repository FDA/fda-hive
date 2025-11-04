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
import  * as csv_modal from "./csv_modal"
import  * as data_modal from "./data_modal"
import  * as html_modal from "./html_modal"
import  * as menuitems_modal from "./menuitems_modal"
import  * as request_modal from  './request_modal'
import  * as sort_filter_modal from  './sort-filter_modal'
import  * as text_modal from  './text_modal'
import  * as time_modal from  './time_modal'
import  * as url_modal from  './url_modal'


const modals = {
    csv_modal: csv_modal,
    data_modal: data_modal,
    html_modal: html_modal,
    menuitems_modal: menuitems_modal,
    request_modal: request_modal,
    sort_filter_modal: sort_filter_modal,
    text_modal:text_modal,
    time_modal:time_modal,
    url_modal: url_modal
}

export default modals;