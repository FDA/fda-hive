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
function ProjectListDialog (props)
{
        this.onSubmit = props.submit;
        this.cur_submission_project = props.cur_submission_project
        var tThis = this;
        
        vjDS.add("", "dsProjectList", "static:
        vjDS.add("", "dsComputationProjectList", "static:
        
        var bothBack = false;
        var callbackFunc = function(){
            if(!bothBack){
                bothBack = true;
                return;
            }
            
            let csv = CSVToArray(vjDS.dsProjectList.data);
            csv = csv.filter(function(item){ return csv[0].length === item.length})
            tThis.projectJson = ArrayToObjArray(csv);
            
            if(!tThis.projectJson || tThis.projectJson.length < 1){
                tThis.onSubmit(-1)
                return null;
            }
            
            var computationData = vjDS.dsComputationProjectList.data;
            var compRows = new vjTable(computationData, undefined, vjTable_propCSV);
            
            var allIds=[];
            for(var i=0; i < compRows.rows.length; i++){
                var row = compRows.rows[i];                
                var id = parseInt(row.submission_project);
                
                if(allIds.indexOf(id) > -1) continue;
                else allIds.push(id);
            }
            
            tThis.projectJson.sort(function(a,b){
                if(allIds.indexOf(a.id) >= 0 || allIds.indexOf(b.id) >= 0) return -1;
                
                return a._brief.localeCompare(b._brief);
            });
            
            var selectOptions = $(document.createElement("datalist"))
                    .attr("id", 'select_project')
                    .attr("name",'select_project')
                    .css('width','100%')
                    .text("Select a Project")
                    .append($(document.createElement("option"))
                        .attr("value", "-1" + " - Personal Computation (assign no project)")
                    );

            for(var i = 0; i < tThis.projectJson.length; i++){
                if(this.cur_submission_project !== undefined && parseInt(tThis.projectJson[i].id) === parseInt(this.cur_submission_project)){
                     this.cur_submission_project ={id: tThis.projectJson[i].id, value: tThis.projectJson[i].id + ` - ${tThis.projectJson[i]._brief} | (${tThis.projectJson[i].project_status})`}
                }
                selectOptions.append($(document.createElement("option"))
                        .attr("value", tThis.projectJson[i].id + ` - ${tThis.projectJson[i]._brief} | (${tThis.projectJson[i].project_status})`)
                );   
                
            }
            
            let hasDialog =  document.querySelector('#dialog-project');
            if(hasDialog === null){
                $("body").append(
                    $(document.createElement("div"))
                        .attr("id", "dialog-project")
                        .append (
                                $(document.createElement("h4"))
                                .attr('class','hv-dialog-container-title')
                                .text("Please select a project.")
                        )
                        .append (
                                $(document.createElement("p"))
                                .attr('class','hv-dialog-container-dscpt')
                                .text("Please select project to be associated with this download. To view full dropdown list of projects, please, delete the text.")
                        )
                        .append($(document.createElement("form"))
                                .append($(document.createElement("fieldset"))
                                        .append($(document.createElement("label"))
                                            .text("Select an Active Project")
                                            .attr("for", 'select_project')
                                    )
                                    .append($(document.createElement("input"))
                                            .attr('id' , 'select_project_input')
                                            .attr('list','select_project')
                                            .attr("name",'select_project')
                                            .attr('title', 'delete the text to view full list of projects')
                                            .css('width','100%')
                                            .attr('autocomplete',"off")
                                            .attr("value", this.cur_submission_project && this.cur_submission_project.hasOwnProperty('value') ? this.cur_submission_project.value : '')
                                    )
                                    .append(selectOptions)
                                )
                        )
                    );

                $("#dialog-project").dialog({
                    modal: true,
                    width: 600,
                    buttons: {
                        OK: function() {
                            $(this).dialog("close");
                            let value = parseInt($(this).find("#select_project_input").val());
                            let subid = value ? parseInt( value ) : -1 ;
                            
                            tThis.onSubmit(subid);
                        }
                    },
                    open: function() {
                        $(this).closest(".ui-dialog")
                        .find(".ui-dialog-titlebar-close")
                        .addClass("ui-button ui-widget ui-state-default ui-corner-all ui-button-icon-only ui-dialog-titlebar-close")
                        .html("<span class='ui-button-icon-primary ui-icon ui-icon-closethick'></span>");
                    },
                    close: function() {
                        $(this).dialog("close");
                        $(this).dialog("destroy").remove();
                    }
                });
            }
        };
  
        vjDS.dsProjectList.unregister_callback(callbackFunc);
        vjDS.dsProjectList.register_callback(callbackFunc);
        vjDS.dsComputationProjectList.unregister_callback(callbackFunc);
        vjDS.dsComputationProjectList.register_callback(callbackFunc);

        vjDS.dsProjectList.reload("http:
        
        vjDS.dsComputationProjectList.reload("http:

        
};