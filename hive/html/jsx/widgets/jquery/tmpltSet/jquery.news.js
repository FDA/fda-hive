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

$(function () {
    $.widget("layout.news", {

        options: {
            data:[]
        },
        
        _create: function(){
            var oThis = this;
            this.element.addClass("news__container");
            
            if(this.options.data && this.options.data.length > 0){
                for(var i = 0; i < this.options.data.length; i++){
                    this._appendElement(this.options.data[i], totalNewsCount - i);
                }
                this.element.on('click', function(event){
                    console.log(event.target.hasAttribute("data-preview"))
                    if(event.target.hasAttribute("data-preview")){
                        oThis._toggleInfo(event.target);
                     }
                })
            }else{
                let no_news = `<div class="news__none">
                                    <p> No News, yet</p>
                                    <p>=(</p>
                               </div>`;
                this.element.append(no_news)
            }
            
        },
        
        _appendElement: function (news_item, appCnt){
            var oThis = this;
            
            let date = news_item.activation_date && !isNaN(new Date(news_item.activation_date)) ? news_item.activation_date.substring(0, news_item.activation_date.indexOf("T"))  : news_item.creation_date && !isNaN(new Date(news_item.creation_date)) ? news_item.creation_date.substring(0, news_item.creation_date.indexOf("T")) : '';
            
            let desciption = news_item.text ? `<div class="news__description" >${news_item.text.replace(new RegExp("&lt;",'g'), "<").replace(new RegExp("&gt;", 'g'), ">")}</div>` : '';
            
            let news_item_div =`<div class="news__item ${appCnt > 0 ? 'unread' : ''} ${news_item.importance ? news_item.importance : ''}">
                               <figure>
                                    <div class="header accordion" data-header="true">
                                        <h4 class="news__title ${news_item.importance ? news_item.importance : ''} ${news_item.importance === "high" ? "highlight highlight__red " : ''}"> ${news_item.title ? news_item.title : ''}</h4>
                                        <div>
                                            <p class="news__date">${date}</p>
                                            ${news_item.text ? '<i class="view__description rv-eye"  data-preview="true"></i>' : ''}
                                        </div>
                                    </div>
                                    ${desciption}
                                </figure>
                            </div>`;
            
           this.element.append(news_item_div);            
        },
        
        _toggleInfo: function(elem){
            $(elem).closest(".unread").removeClass("unread");
            $(elem).toggleClass("rv-eye-no rv-eye");
            
            var description = $(elem).closest("div[data-header]").siblings(".news__description");
            if (description.css("maxHeight") && parseInt(description.css("maxHeight")) != 0){
                description.css("maxHeight", "0px");
                description.css("display", 'none');
            } else {
                description.css("display", 'block');
                description.css("maxHeight", description[0].scrollHeight + "px");
            }
        }
    });

}(jQuery));