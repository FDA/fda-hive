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
            vjDS.add("infrastructure: ", "dsObjSpec", "http:

            this.options.config = {
                layout: {
                    allowResize: false,
                      items:[
                      {
                          id: 'start',
                          top: '0',
                          left: '0',
                          right: '100%',
                          bottom: '100%',
                          allowResize: false,
                          overflow: "auto",
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
                                      objType:"HIVE-Development_Project_Request",
                                      ForSaveReload: this._ForSaveReload,
                                      saveButtonClass: 'hv-btn',
                                      ErrorOnSave: this._ErrorOnSave,
                                      setCallback: this._getEmail,
                                      formObject: document.forms[this.options.formName],
                                      widget: this
                                }
                            }
                        }
                  }]
               }
            };
            
            this.options.container = "requestProjectPortal" + parseInt(Math.random() * 100000);

            var that = this;
            
            vjDS.add("", "dsUserTree", "http:
            vjDS.dsUserTree.register_callback(function(obj, data){
                var tbl = new vjTable(data, vjTable_hasHeader|vjTable_propCSV);

                for(var i = 0; i < tbl.rows.length; i++){
                    if(tbl.rows[i].cols[2] == "Team"){
                        that.teamId = tbl.rows[i].cols[0];
                        return;
                    }
                }
            });
            vjDS.dsUserTree.reload("http:
        },

        _getEmail: function (container, path){
            
            this.widget.email = this.nodeTree.findByFieldName('email').value;

            return true;
        },
        
        _ForSaveReload: function (hiveId) {
            ajaxDynaRequestPage("?cmd=permset&ids=" + hiveId + "&groups=" + this.widget.teamId + "&perm=browse|read|download|write|exec|del|share|admin&flag=allow|down|active", {} ,vjObjAjaxCallback);
            let window_location = window.location.origin + window.location.pathname;
            let requester_name = this.nodeTree.findByFieldName('research_contacts_name').value.length > 0 ? ` by ${this.nodeTree.findByFieldName('research_contacts_name').value}` : '';
            let text = `Hello, \n A project request has been submitted at ${window_location}${requester_name}. Please look at request #${hiveId}. \n Preview Request: ${window_location}?cmd=record&ids=${hiveId} \n Create Project: ${window_location}?cmd=create_hive_project&ids=${hiveId}`;
            ajaxDynaRequestPage("?cmd=sendmail&message=" + vjDS.escapeQueryLanguage(text) + "&email=" + vjDS.escapeQueryLanguage(this.widget.email) + "&autoreply=projreq&reqid=" + hiveId, {} , vjObjAjaxCallback);

            $("body").append(
                $(document.createElement("div"))
                    .attr("id", "dialog")
                    .attr("title", "Information")
                    .append (
                            $(document.createElement("p")).text("Object number " + hiveId + " has been created. An email was sent to the HIVE team and you will be contacted soon.")
                    )
                );
            
            $("#dialog").dialog({
                modal: true,
                width: 500,
                buttons: {
                    Continue: function() {
                        $(this).dialog("close");
                        $(this).dialog("destroy").remove();
                    }
                },
                open: function() {
                    $(this).closest(".ui-dialog")
                    .find(".ui-dialog-titlebar-close")
                    .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                    .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                },
                close: function(){
                    $(this).dialog("destroy").remove();
                }
            });
        },
        _ErrorOnSave: function (error_msg) {
            $("body").append(
                $(document.createElement("div"))
                    .attr("id", "dialog")
                    .attr("title", "Error")
                    .append (
                            $(document.createElement("p")).text("\u274C An error occurred while creating a Project Request.")
                    )
                    .append (
                            $(document.createElement("code")).text(error_msg)
                    )
                );
            
            $("#dialog").dialog({
                modal: true,
                width: 500,
                buttons: {
                    Continue: function() {
                        $(this).dialog("close");
                        $(this).dialog("destroy").remove();
                    }
                },
                open: function() {
                    $(this).closest(".ui-dialog")
                    .find(".ui-dialog-titlebar-close")
                    .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                    .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                },
                close: function(){
                    $(this).dialog("destroy").remove();
                }
            });
        }
    });

}(jQuery));

jQuery.getRequestProjectManager = function() {
    if($('.layout-manager').length == 0)
        console.log('ERROR: cannot find Layout Manager!');
    else
        return $('.layout-manager').first().requestProjectPortal('instance');
}