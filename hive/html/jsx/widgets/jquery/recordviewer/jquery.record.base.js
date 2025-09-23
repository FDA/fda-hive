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
    $.widget("recordviewer.record_base", {
        options: {
            spec:{
                name: null,
                type: null,
            },
            preset:{
                
            },
            path: null,
            validation: null
        },

        _create: function () {
            var oThis = this;

            this._prepareData(function() {
                oThis._buildField();
            });
        },

        _prepareData: function(callback) {
            if(callback != null)
                callback();
        },

        _buildField: function() {
            var oThis = this;

            if(this.options.spec._limit){
                var ret = this.manageConstraints();
                
                if(!ret)
                    this.field = this.generateField();
                else this.field = ret; 
            }
            else
                this.field = this.generateField();
            
            if(this.options.spec._default){
                this.setValue(this.options.spec._default);
            }

            this.field.focusin(function() {
                    oThis.focusInField();
                    if(!$(this).data('initial-value')) {
                        oThis.setInitialValue();
                    }
                })
                .change(function() {
                    $(this).addClass('changed');
                    $(this).trigger('field-changed');
                })
                .on('field-undo', function() {
                    oThis.undoField();
                    $(this).removeClass('changed');
    
                    return false;
                })
                .on('field-redo', function() {
                    oThis.redoField();
                    $(this).addClass('changed');
    
                    return false;
                })
                .hover(function(){
                })
                .keypress(function(){
                })
                .focusout(function(){
                    oThis.focusOutField();
                    if(oThis.validateField()){
                        console.log("field passed validation");
                    }
                    else{
                        oThis.generateErrorField();
                        console.log("filed did not pass validation");
                    }
                        
                    
                });

            this.element.append(this.field);

            if(this.options.spec._plural)
                this.multiField();
        },
        
        generateField: function(){            
            return $(document.createElement('span')).html('<i>Not supported type yet: ' + this.options.spec.type + '</i>' );
        },
        
        setValue: function(value){
            $(this.field).data({ 'field-value': value });
            
            this.field.val(value);
        },
        
        setInitialValue: function() {
            $(this.field).data({ 'initial-value': this.options.spec.value });
        },

        resetInitialValue: function() {
            this.setValue($(this.field).data('initial-value'));
            $(this.field).removeAttr('data-changed');
        },
        
        destroyField: function(){
            this.field.remove();            
        },
        
        focusInField: function(){
            $(this.field).data({"old-value": this.element.val()});
            return true;
        },
        
        focusOutField: function(){
            this.curValue = this.field.val();
            return true;
        },
        
        saveField: function(){
            console.log("save for " + this.options.spec.type + "type is still not supported");
            return false;
        },
        
        undoField: function(){
            console.log("undo for " + this.options.spec.type + "type is still not supported");
            return false;
        },
        
        redoField: function(){
            console.log("redo for " + this.options.spec.type + "type is still not supported");
            return false;
        },
        
        validateField: function(){
            console.log("validation for " + this.options.spec.type + "type is still not supported");
            return false;
        },
        
        generateErrorField: function(){
            console.log("error message for " + this.options.spec.type + "type is still not supported");
            return false;
        },
        
        multiField: function(){            
            if(this.options.spec.is_readonly || !this.element.is('.field-container') || !this.options.spec._plural)
                return;

            var oThis = this;

            var multiToolbar = $(document.createElement('div'))
                .addClass('toolbar')
                .append(
                    $(document.createElement('button'))
                    .addClass('btn btn-default btn-xs pull-right')
                    .attr({ type: 'button' })
                    .append(
                        $(document.createElement('span'))
                            .addClass('glyphicon glyphicon-plus')
                            .attr({ 'aria-hidden': true })
                    )
                    .append( $(document.createTextNode(' Add')) )
                    .click(function() {
                        oThis.element.prepend(oThis.generateField());
                    })
                )
                .appendTo(this.element);
        },
        
        manageConstraints: function(){
            var constraint = this.options.spec._limit;
            if(constraint["choice"]){
                return  this.generateChoice(constraint["choice"]);
            }
            if(constraint["choice+"]){
                return  this.generateChoicePlus(constraint["choice+"]);
            }
            
            return false;
        },
        
        generateChoice: function (generateFrom){
            var field = $(document.createElement("select"))
                .addClass(this.options.spec.type)
                .data("spec", this.options.spec)
                .attr({
                    id: this.options.spec.path + "-field",
                    name: this.options.spec.tmpObjName,
                    type: 'choice'
                });
            
            for(var i = 0; i < generateFrom.length; i++){
                field.append($(document.createElement("option"))
                        .attr("value", generateFrom[i].value)
                        .text(generateFrom[i].title)
                );
            }
            
            return field;
        },
        
        generateChoicePlus: function (generateFrom){
            var field = $(document.createElement("select"))
                .addClass(this.options.spec.type)
                .data("spec", this.options.spec)
                .attr({
                    id: this.options.spec.path + "-field",
                    name: this.options.spec.tmpObjName,
                    type: 'choice'
                });
            
            var i = 0;
            for(; i < generateFrom.length; i++){
                var title = generateFrom[i].title ? generateFrom[i].title : generateFrom[i];
                var value = generateFrom[i].value ? generateFrom[i].value : i;

                field.append($(document.createElement("option"))
                        .attr("value", value)
                        .text(title)
                );
            }
            field.append($(document.createElement("option"))
                    .attr("value", i)
                    .text("Other...")
            );
            
            field.data("maxValue", i);
            field.on("change", function(event, data){
                if(parseInt(this.value) != NaN && parseInt(this.value) == $(this).data("maxValue"))
                    $(this).next().css("visibility", "");
                else
                    $(this).next().css("visibility", "hidden");
            });
            
            var div = $(document.createElement("div"))
                    .append(field)
                    .append($(document.createElement('input'))
                            .addClass('field')
                            .attr("placeholder", "Please Specify Value")
                            .css("visibility", "hidden")
                    );
            
            return div;
        }
    });
}(jQuery));
