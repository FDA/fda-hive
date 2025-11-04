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
    function renderVideo(props = 'none'){
        this.name = props.name;
        this.source = props.source;
        this.insert_loc = props.insert_loc;
        
        this.render = function(){
            if(!this.insert_loc || !(this.insert_loc instanceof HTMLElement)) {
               this.errorDisplay('Video insertion location is not defined');
               return;
            }
            if(!this.source){
               this.errorDisplay('source of the video is not defined' , 'ui');
               return;
            }
            this.getName();
            let video_name = `<h1 class="hv-video--title">${sanitizeInnerHTML(this.name)}</h1>`;
            this.insert_loc.insertAdjacentHTML('beforeend', video_name);
            this.constructVideoUI();
        }
        
        this.constructVideoUI = function(){
            let video_container = document.createElement('video');
            video_container.setAttribute("src", getBaseUrlNoCGI() + this.source);
            video_container.setAttribute("class","hv-video");
            video_container.setAttribute("width", "100%");
            video_container.setAttribute("height", "auto");
            video_container.setAttribute("controls",true);
            video_container.setAttribute("preload", "auto");
            this.insert_loc.appendChild( video_container);             
        }
        
        this.getName = function(){
            if(!this.name){
                let src_path = this.source.split('/');
                this.name = src_path[ src_path.length - 1 ];
            }
            this.name = this.name.split('_').join(' ');
        }
        
        this.errorDisplay = function(msg,where = 'console'){
            if(this.insert_loc && where === 'ui'){
               let errorDiv = msg ?  `<h1>: ( Sorry, ${sanitizeInnerHTML(msg)} </h1>` : `<h1>: ( Sorry, something went wrong</h1>`;
               this.insert_loc.insertAdjacentHTML('beforeend', errorDiv); 
            }else if(where = 'console'){
                console.error(msg)
            }else{
                console.error(msg)
            }
        }
        
        
    }    