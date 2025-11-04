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
function DialogComponent(props) {
    loadCSS(`css/dialog.css`)
    
    let d_this = this
    this.dialogTitle = props.dialogTitle || 'HIVE Dialog'
    this.dialogContent = props.dialogContent || '<img border="0" width="24" src="img/progress.gif"> Loading ...'
    
    this.dialogId = props.dialogID

    this.updateDialogContent = function(html){
        $(`#${d_this.dialogId}`)
            .html(html)
    }

    this.updateDialogTitle = function(title){
        $(`#${d_this.dialogId}`)
            .dialog({title: title})
    }

    let hasDialog =  document.querySelector(`#${this.dialogId}`);
    if(hasDialog === null){
        $("body").append(
            $(document.createElement("div"))
                .attr("id", this.dialogId)
                .append (
                   this.dialogContent
                )
            );
        $(`#${this.dialogId}`).dialog({
            modal: true,
            title: this.dialogTitle,
            classes:{'ui-dialog-titlebar': 'hv-dialog-titlebar'},
            height: props.height || 650,
            minWidth: props.width || 600,
            width: props.width || 900,
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

}