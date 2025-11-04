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
    $.widget("layout.requestProjectPortal", $.layout.layoutmanager, {

        options: {
        },

        _onBeforeInit: function() {
            this.options.config = {
                layout: {
                    allowResize: false,
                      items:[
                      {
                          id: 'start',
                          top: '0',
                          left: '15%',
                          right: '85%',
                          bottom: '200',
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
            
            this.options.container = "requestProjectPortal" + parseInt(Math.random() * 100000);
        },
        _onAfterInit: function(){
            this.element.children().attr("id", this.options.container);
            
            var whereToAppend = $("#"+this.options.container).find(".panel-body");
            whereToAppend.append($(document.createElement("h2"))
                        .text(this.options.title ? this.options.title : "Project Requests")
                )
                .append($(document.createElement("button"))
                        .text ("Browse Existing Projects")
                        .attr ("name", "browseExisting")
                        .click(function(){
                            $(this).addClass('btn-active');
                            $("[name='createNew']").removeClass('btn-active');
                        })
                )
                .append($(document.createElement("button"))
                        .text ("Add a New Project")
                        .attr ("name", "createNew")
                        .click(function(){
                            $(this).addClass('btn-active');
                            $("[name='browseExisting']").removeClass('btn-active');
                        })
                );
             this._on (false,  $( "[name='browseExisting']"), {click: "_browseExisting"});
             this._on (false,  $( "[name='createNew']"), {click: "_createNew"});
        },
        _browseExisting: function(){            
            var manager = $.getRequestProjectManager();
            if (this.options.browseAdded){
                manager.hide("addObj");
                manager.show("objectTabs");
                manager.show("moreInfoTabs");
            }
            else{
                vjDS.add("infrastructure: Loading record specification information", "dsRecordSpec", "static:
                vjDS.add("infrastructure: Loading record data","dsRecordValues", "static:
                vjDS.add("Table query result", "dsObjList", "http:
                
                manager.hide("addObj");
                manager.append({
                      layout: {
                          items:[{
                              id: 'objectTabs',
                              top: '200',
                              left: '15%',
                              right: '50%',
                              bottom: '100%',
                              allowMaximize: false,
                              allowResize: false,
                              tabs: {
                                  items: [{
                                    active: true,
                                    title: 'Objects',
                                    name: 'objects',
                                    view: {
                                        name: 'dataview',
                                        options: {
                                            dataViewer: 'vjTableView',
                                            dataViewerOptions: {
                                                data:"dsObjList",
                                                bgColors:['#f2f2f2','#ffffff'],
                                                cols: [ { name:'id', title:'Identifier', order: 1 },    
                                                        { name:'title', title:'Title', order: 2 },
                                                        { name:'created', title:'Date of Submission', order: 5 , type: 'datetime'},
                                                        { name:'center', title:'Center', order: 3 },    
                                                        { name:'projectStat', title:'Project Status', order: 4 },    
                                                        ],
                                                typeOfObjectsToBeShown:this.options.objectType,
                                                selectCallback: this._onSelectObj,
                                                formObject: document.forms[this.options.formName]
                                            }
                                        }
                                    }
                                  }]
                              }
                          },
                           {
                              id: 'moreInfoTabs',
                              top: '200',
                              left: '50%',
                              right: '85%',
                              bottom: '100%',
                                allowMaximize: true,
                              allowResize: false,
                              tabs:{
                                  items: [{
                                      active: true,
                                      title: 'Preview',
                                      name: 'preview',
                                      class: 'preview',
                                      view: {
                                          name: 'dataview',
                                          options: {
                                              dataViewer: 'vjRecordView',
                                              dataViewerOptions: {
                                                  data: ['dsRecordSpec', 'dsRecordValues'],
                                                  constructionPropagateDown: 10,
                                                  showReadonlyInNonReadonlyMode: true,
                                                  readonlyMode: true,
                                                  objType:this.options.objectType,
                                                  formObject: document.forms[this.options.formName]
                                              }
                                          }
                                      }
                                  },
                                  {
                                      active: false,
                                      title: 'Edit',
                                      name: 'edit',
                                      class: 'edit',
                                      view: {
                                          name: 'dataview',
                                          options: {
                                              dataViewer: 'vjRecordView',
                                              dataViewerOptions: {
                                                  data: ['dsRecordSpec', 'dsRecordValues'],
                                                  autoStatus: 3,
                                                  constructionPropagateDown: 10,
                                                  showReadonlyInNonReadonlyMode: false,
                                                  implementSaveButton: true,
                                                  objType:this.options.objectType,
                                                  readonlyMode: false,
                                                  formObject: document.forms[this.options.formName]
                                              }
                                          }
                                      }
                                  }]
                              }
                          }]
                      }
                });
                this.options.browseAdded = true;
            }

            event.preventDefault();
        },
        _onSelectObj: function(viewer, node){
            vjDS["dsRecordSpec"].reload("http:
            vjDS["dsRecordValues"].reload("http:
        },
        _createNew: function(){            
            var manager = $.getRequestProjectManager();
            if (this.options.createAdded){
                manager.hide("objectTabs");
                manager.hide("moreInfoTabs");
                manager.show("addObj");
            }
            else{
                vjDS.add("infrastructure: ", "dsObjSpec", "http:
                
                manager.hide("objectTabs");
                manager.hide("moreInfoTabs");
                manager.append({
                      layout: {
                          items:[{
                              id: 'addObj',
                              top: '200',
                              left: '15%',
                              right: '85%',
                              bottom: '100%',
                              allowMaximize: false,
                              tabs: {
                                  items: [{
                                    active: true,
                                    title: 'Object to Add',
                                    name: 'objectAdd',
                                    view: {
                                        name: 'dataview',
                                        options: {
                                            dataViewer: 'vjRecordView',
                                            dataViewerOptions: {
                                                  data: ['dsObjSpec'],
                                                  autoStatus: 3,
                                                  constructionPropagateDown: 10,
                                                  showReadonlyInNonReadonlyMode: false,
                                                  implementSaveButton: true,
                                                  readonlyMode: false,
                                                  objType:this.options.objectType,
                                                  ForSaveReload: this._ForSaveReload,
                                                  formObject: document.forms[this.options.formName]
                                            }
                                        }
                                    }
                                  }]
                              }
                          }]
                      }
                });
                this.options.createAdded = true;
            }
            
            event.preventDefault();
        },
        _ForSaveReload: function (hiveId) {
            newurl = "?cmd=projectRequest&type=" + this.objType;
            if(hiveId.charAt(0)!='-')alert("Record Created Successfully");
            else alert("Record Copied Successfully")
            window.location.href = newurl;
        }
    });

}(jQuery));

jQuery.getRequestProjectManager = function() {
    if($('.layout-manager').length == 0)
        console.log('ERROR: cannot find Layout Manager!');
    else
        return $('.layout-manager').first().requestProjectPortal('instance');
}