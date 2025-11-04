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
    $.widget("layout.jsonValidate", $.layout.layoutmanager, {

        options: {
        },

        _onBeforeInit: function() {
            this.options.config = {
                layout: {
                    allowResize: false,
                      items:[
                      {
                          id: 'input1',
                          top: '0',
                          left: '0%',
                          right: '50%',
                          bottom: '25%',
                          allowResize: false,
                          overflow: "auto",
                        view:{
                            name: "dataview",
                            options:{ 
                                
                            }
                        }
                  },{
                          id: 'input2',
                          top: '0',
                          left: '50%',
                          right: '100%',
                          bottom: '25%',
                          allowResize: false,
                          overflow: "auto",
                        view:{
                            name: "dataview",
                            options:{ 
                                
                            }
                        }
                  },{
                          id: 'output1',
                          top: '25%',
                          left: '0%',
                          right: '50%',
                          bottom: '100%',
                          allowResize: false,
                          overflow: "auto",
                        view:{
                            name: "dataview",
                            options:{ 
                                
                            }
                        }
                  },{
                          id: 'output2',
                          top: '25%',
                          left: '50%',
                          right: '100%',
                          bottom: '100%',
                          allowResize: false,
                          overflow: "auto",
                        view:{
                            name: "dataview",
                            options:{ 
                                
                            }
                        }
                  }]
               }
            };
            
            this.options.container = "jsonValidate" + parseInt(Math.random() * 100000);
        },
        
        _onAfterInit: function(){
            var oThis = this;
            
            this.element.attr("container", this.options.container);
            var input1 = $("#input1-panel");
            var input2 = $("#input2-panel");
            var output1 = $("#output1-panel");
            var output2 = $("#output2-panel");
            
            input1.append($(document.createElement("p"))
                    .text("Please paste first JSON text input into the textbox and click check"))
            .append($(document.createElement("div"))
                    .append($(document.createElement("textarea"))
                            .attr("id", "textArea1")
                            .css("width", input1.width() - 10 + "px")
                            .css("height", input1.height()-80 + "px")
                    ))
            .append($(document.createElement("button"))
                    .text("Check")
                    .attr("type", "button")
                    .on("click", function(){ oThis._onFirstCheck(); }));
            
            input2.append($(document.createElement("p"))
                    .text("Please paste second JSON text input into the textbox and click check"))
            .append($(document.createElement("div"))
                    .append($(document.createElement("textarea"))
                            .attr("id", "textArea2")
                            .css("width", input1.width() - 10 + "px")
                            .css("height", input2.height()-80 + "px")
                    ))
            .append($(document.createElement("button"))
                    .text("Check")
                    .attr("type", "button")
                    .on("click", function(){ oThis._onSecondCheck(); }));
            
            output1.append($(document.createElement("p")));
            output2.append($(document.createElement("p")));
        },
        
        _onFirstCheck:function(){
                var content = $("#textArea1").val();
                var parsed;
                var output1 = $("#output1-panel");
                
                try{
                    parsed = JSON.parse(content);
                }catch(e){
                    output1.children("p").text("Poorly formatted JSON");
                    console.log(e);
                    this.firstValid = false;
                    return;
                }
                
                var sortedParsed = this._objSort(parsed);
                
                this.firstValid = true;

                var toStr = JSON.stringify(sortedParsed, null, 4);
                toStr = replaceAll(toStr, " ", "&nbsp;");
                toStr = replaceAll(toStr, "\n", "<br />");
                this.firstPrint = toStr;
                
                if(this.firstValid && this.secondValid){
                    this._printBoth();
                    return;
                }
                
                output1.children("p").html(toStr);
        },
        
        _onSecondCheck:function(){
                var content = $("#textArea2").val();
                var parsed;
                var output2 = $("#output2-panel");
                
                try{
                    parsed = JSON.parse(content);
                }catch(e){
                    output2.text("Poorly formatted JSON");
                    console.log(e);
                    this.secondValid = false;
                    return;
                }
    
                var sortedParsed = this._objSort(parsed);
                
                this.secondValid = true;
                var toStr = JSON.stringify(sortedParsed, null, 4);
                toStr = replaceAll(toStr, " ", "&nbsp;");
                toStr = replaceAll(toStr, "\n", "<br />");
                
                this.secondPrint = toStr;
                if(this.firstValid && this.secondValid){
                    this._printBoth();
                    return;
                }
                
                output2.children("p").html(toStr);
        },
        
        _printBoth: function(){
                var firstSplit = this.firstPrint.split("<br />");
                var secondSplit = this.secondPrint.split("<br />");
                
                var toPrint = "";
                
                for(var i = 0; i < firstSplit.length && i < secondSplit.length; i++){
                    if(firstSplit[i] == secondSplit[i])
                        toPrint += firstSplit[i] + "<br />";
                    else{
                        toPrint += "> > > > > > > > > > >" + firstSplit[i] + "<br />";
                        toPrint += "< < < < < < < < < < <" + secondSplit[i] + "<br />";
                    }
                }
                $("#output1-panel").children("p").empty();
                $("#output1-panel").children("p").html(toPrint);
        },
          
        _objSort: function(obj){
            var toReturn = {};
            
            var sortedKeys = Object.keys(obj).sort();
            for (var i = 0; i < sortedKeys.length; i++){
                if(obj[sortedKeys[i]] instanceof Array)
                    toReturn[sortedKeys[i]] = this._arraySort(obj[sortedKeys[i]]);
                else if (obj[sortedKeys[i]] instanceof Object)
                    toReturn[sortedKeys[i]] = this._objSort(obj[sortedKeys[i]]);
                else 
                    toReturn[sortedKeys[i]] = obj[sortedKeys[i]];
            }
            
            return toReturn;
        },
        
        _arraySort: function(arr){
            var toReturn = [];
            
            var sortedArr = arr.sort();
            for(var i = 0; i < sortedArr.length; i++){
                if(sortedArr[i] instanceof Array)
                    toReturn[i] = this._arraySort(sortedArr[i]);
                else if (sortedArr[i] instanceof Object)
                    toReturn[i] = this._objSort(sortedArr[i]);
                else
                    toReturn[i] = sortedArr[i];
            }
            
            return toReturn;
        }
    });

}(jQuery));

jQuery.getJsonValidate = function() {
    if($('.layout-manager').length == 0)
        console.log('ERROR: cannot find Layout Manager!');
    else
        return $('.layout-manager').first().jsonValidate('instance');
}

function replaceAll(str, find, replace) {
    return str.replace(new RegExp(find, 'g'), replace);
}