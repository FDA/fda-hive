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
function ProgressActionButton(props) {
    
    let icons = {
        'Suspend': "pause",
        'Resume': "play",
        'Kill': "stop",
        'Run': "resubmit"
    }
    let action = props.action;
    if(props.status == "Running") action = "Kill";
    else if (props.status == "Waiting") action = "Suspend";
    else if (props.status == "Suspended") action = "Resume";
    else if (props.status == "Killed" || props.status == "ProgError" || props.status == "SysError") action = "Run";
    else if (props.status == "Done") action = "Run";

    let data = props.data ? Object.keys(props.data).map(function(item){
        return `data-${item}="${props.data[item]}"`
    }).join(' ') : ''

    return `<div 
                style='
                       height:2em; 
                       width: 2em; 
                       border-radius: 50%; 
                       background-color: #f2f2f2; 
                       padding: 3px; 
                       box-shadow: 0px 1px 4px 0px #bfbfbf; 
                       margin: 2px 0px;
                       cursor: pointer;
                       '
                data-event="progress-action"
                data-action="${ action }"
                ${data}
            > 
                    <i 
                         style=' font-size:22px; ' 
                        class='rv-${ icons[action] }' 
                        title='${ action }' 
                        data-event="progress-action"
                        data-action="${ action }"
                        ${data}
                    /> 
            </div>`

}