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
    var oThis;
    editableTable = undefined;
    editableTableContent = undefined;
    $.widget("view.tableEditorWidget", {
        options:{
            data: "dsVoid",
            dataCounter: 0,
            colsForInput:[]
        },
        _create: function () {
            oThis = this;
            
            oThis.options.container = "treeWidgetDiv" + parseInt(Math.random() * 100000);
            oThis.element.append($(document.createElement("div")).attr("id", oThis.options.container));
            
            oThis.options.data = verarr(oThis.options.data);
            $(oThis.options.data).each(function(i, dsName){
                if (vjDS[dsName].state == "done"){
                    oThis.allReady ({}, vjDS[dsName].data);
                    return;
                }
                
                vjDS[dsName].register_callback(oThis.allReady);
                vjDS[dsName].load();
            });
        },
        allReady: function(viewer, content){
            if (++oThis.options.dataCounter < oThis.options.data.length)
                return;
            
            oThis._constructTable();
        },
        _constructTable: function (){
            var allContent = "";
            
            for (var i = 0; i < oThis.options.data.length; i++){
                allContent += vjDS[oThis.options.data[i]].data;
            }
            
            var table = new vjTable (allContent, vjTable_propCSV);
            var htmlToPut = "<input id='saveFileName' placeholder='Enter File Name'></input>" +
                    "<button id='saveTableButton'>Save Table</button>" +
                    "<button id='archiveTableButton'>Archive Table</button>";
            htmlToPut += "<table id='tableEditor" + oThis.options.container + "' class='display' cellspacing=0 width='100%'>";
            
            var htmlFoot = "<tfoot>";
            htmlToPut += "<thead>";
            if (table.rows.length <= 0) return;
            htmlToPut += "<tr>";
            htmlFoot += "<tr>"
            for (var t = 0; table.rows[0].cols && t < table.rows[0].cols.length; t++){
                htmlToPut += "<th>" + table.rows[0].cols[t] + "</th>";
                htmlFoot += "<th>" + table.rows[0].cols[t] + "</th>";
            }
            htmlFoot += "</tr></tfoot>";
            htmlToPut += "</tr></thead>" + htmlFoot;
            
            htmlToPut += "<tbody>";
            for (var t = 1; table.rows[0].cols && t < table.rows[0].cols.length; t++){
                htmlToPut += "<tr>";
                for (var d = 0; d < table.rows[t].cols.length; d++){
                    if (oThis._contains (oThis.options.colsForInput, table.rows[0].cols[d], d)){
                        htmlToPut += "<td><input type='text' value='"+table.rows[t].cols[d]+"' row='"+t+"' col='"+d+"'></td>";
                    }
                    else
                        htmlToPut += "<td row='"+t+"' col='"+d+"'>" + table.rows[t].cols[d] + "</td>";
                }
                htmlToPut += "</tr>";
            }
            htmlToPut += "</tbody>";
            
            htmlToPut += "</table>";
            
            $("#" + oThis.options.container).append(
                    $(document.createElement ("div"))
                            .attr("id", oThis.options.container + "-table")
                            .html (htmlToPut)
            );
            
            editableTable = $("#" + oThis.options.container + "-table").children("table").DataTable();
            editableTableContent = table;
            
            $("#tableEditor" + oThis.options.container).find("input").on ("change", function(eventData) {
                var row = $(this).attr("row");
                var col = $(this).attr("col");
                
                editableTableContent.rows[row].cols[col] = this.value;
            });
            
            $("#saveTableButton").click(function (eventData){
                eventData.preventDefault();
                
                var csvStr = editableTableContent.arrayToCSV();
                var blob = new Blob ([csvStr]);
                var a = window.document.createElement("a");
                a.href = window.URL.createObjectURL(blob, {type: "text/plain"});
                a.download = $("#saveFileName")[0].value + ".csv";
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
            });
            $("#archiveTableButton").click(function (eventData){
                eventData.preventDefault();
                
                var csvStr = editableTableContent.arrayToCSV();
                var blob = new Blob ([csvStr], { type: 'text/csv;charset=utf-8;' });
                
                
                var formData = new FormData ();
                formData.append ("file", blob,  $("#saveFileName")[0].value + ".csv");
                formData.append ("chkauto_Downloader", "-1");
                formData.append ("Idx_Downloader", "1");
                formData.append ("QC_Downloader", "1");
                formData.append ("screen_Downloader", "1");
                
                var request = new XMLHttpRequest();
                request.open("POST", "dmUploader.cgi");
                request.send(formData);
                
                alert("Your upload request is being processed. Please check your inbox.");
            });
        },
        _contains: function(array, elementName, elementNum){
            for (var i = 0; i < array.length; i++){
                if (array[i] == elementName || array[i] == elementNum) return true;
            }
            
            return false;
        }
    });
}(jQuery));
