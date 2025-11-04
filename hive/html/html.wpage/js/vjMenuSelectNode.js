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

function selectOptionNode(opt, order, hasCols) {
    this.value;
    this.title;
    this.opTxt;
    this.order = order;
    this.disabled=false;
    this.hasCols = false;
    this.isok = function() {
        return true&&this.value;
    }

    var option_id_prefix = "option_";
    this.id = option_id_prefix + this.order;
    
    var that = this;
    function parse(opt, hasCols) {
        if( hasCols ) {
            opt = opt.cols;
        }
        if(!opt.length) 
            return ;    
        that.value=that.title=that.opTxt=opt[0];
        if(opt.length==2){
            that.title=that.opTxt=opt[1];
        }
        else if(opt.length>=3){
            that.title=opt[1];
            that.opTxt=opt[2];
            if (opt.length >=4 ) {
                that.disabled = opt[3];
            }
        }                
        return that;
    }
    
    this.drawOption = function() {
        var t="";
        t += "<li";
        t += " data-id='"+this.id+"'";
        t += " data-order='"+this.order+"'";
        t += " data-value='"+sanitizeElementAttr(this.value)+"' ";
        if(this.title)
            t += " title='"+sanitizeElementAttr(this.title)+"'";
        if( this.disabled )
            t += "class='disabled_li'";
        t += ">";
        t += this.opTxt;
        t += "</li>";
        return t;
    }
    
    return parse(opt, hasCols);
}

function outputNode_Select(menu_obj, current_node, nodeActive) {
    this.menu_obj = menu_obj;
    this.my_node = current_node;
    this.multiSelect = this.my_node.multiSelect;
    this.nodeActive = nodeActive;
    this.separator = ",";
    
    var _myCls = "";
    var _myCls = (Math.random()+"").replace(".","");
    
    var _options_cnt = 0;
    var _selectedOptions = {};
    var _options = [];
    
    function generateEmptyPlaceHolderNode(my_id){
        return new selectOptionNode(["&#8203"], my_id);
    }
    
    function isAnEmptyPlaceHolderNodeByText(txt) {
        return txt==="&#8203";
    }
    
    function isAnEmptyPlaceHolderNodeByElement(el) {
        return el.innerHTML==="\u200b";
    }
    
    function getSelectedLUT(value, sep) {
        function getNodeValueArray(value,sep) {
            if(!value)
                return [];
            return value.split(sep);
        }
        
        var value_array = getNodeValueArray(value,sep);
        var selected_lut = {};
        if( value_array ) {
            value_array.forEach(function(el){
                selected_lut[el]=true;
            });
        }
        return selected_lut;
    }
    
    this.resetOption = function() {
        var node_value_lut = getSelectedLUT(this.my_node.value,this.separator);
        
        _selectedOptions = {};
        _options_cnt = 0;
        
        var optarr = [];        
        if(this.my_node.options instanceof Array){
            optarr=this.my_node.options;
        } else {
            var optTabl=new vjTable(this.my_node.options,0,vjTable_collapsePropFormat);
            optarr=optTabl.rows;
        }
        
        _options = {};
        var are_all_disabled = true;
        var hasCols = !(this.my_node.options instanceof Array);
        for(var i= 0 ; i<optarr.length; ++i) {
            var cur_node = new selectOptionNode(optarr[i], i+1, hasCols);
            _options[cur_node.id] = cur_node;
            if( node_value_lut[cur_node.value] ) {
                cur_node.disabled = true;
                this.toggleSelectedNode(cur_node.id);
            }
            if( !cur_node.disabled ) {
                are_all_disabled = false;
            }
        }
        _options_cnt = Object.values(_options).length;
        if( !are_all_disabled && !this.multiSelect ) {
            var cur_node = generateEmptyPlaceHolderNode(0)
            _options[cur_node.id] = cur_node;
        }
    }
    
    this.cntOfOptions = function() {
        return _options_cnt;
    }
    
    function getOptionsStableOrder(options) {
        return Object.values(options).sort(function(x,y){return x.order-y.order;});
    }
    
    function generateOptions(optarr,node_value_array) {
        var t="";
        var selected_lut = getSelectedLUT(node_value_array);
        getOptionsStableOrder(optarr).forEach(function(opt){
            if( opt.isok()) {
                if(selected_lut[opt.value])
                    opt.disabled = true;
                t += opt.drawOption();
            }
        });
        return t;
    }
    
    function generateSelectedOptions(optarr,node_value_array) {
        var t="";
        Object.values(optarr).forEach(function(opt){
            if(opt.isok() && opt.disabled) {
                opt.disabled = false;
                t += opt.drawOption();
                opt.disabled = true;
            }
        });
        return t;
    }

    this.toggleSelectedNode = function(current_id) {
        if(!this.multiSelect)
            _selectedOptions = {};
        
        var selected = true;
        if(current_id in _selectedOptions) {
            delete _selectedOptions[current_id];
            selected = false;
        } else {
            _selectedOptions[current_id]=_options[current_id];
            
        }
        return selected;
    }
    
    this.updateSelectedNodes = function(current_id) {
        var selected = this.toggleSelectedNode(current_id);
        
        this.updateInputTag();
        return selected;
    }
    this.updateInputTag = function() {
        this.my_node.value = Object.values(_selectedOptions).filter(x => x.value).map(x => x.value).join(this.separator);
        
        var element = this.menu_obj.findElementByName(this.element_name);
        var old_value = element.value; 
        element.value = this.my_node.value;
        if ( element.value != old_value )
            this.menu_obj.onChangeElementValue(this.menu_obj.objCls,this.my_node.path, this.element_name);
    }
    
    function disableElement(el) {
        if ( !el.classList.contains("disabled_li") )
            el.classList.add("disabled_li");
    }
    function enableElement(el) {
        if ( el.classList.contains("disabled_li") )
            el.classList.remove("disabled_li");
    }
    
    this.onSelectNodes = function(el) {
        var disabled_els = Array.from(el.parentElement.getElementsByClassName("disabled_li"));
        
        var selected = this.updateSelectedNodes(el.dataset.id);
        if ( selected && !isAnEmptyPlaceHolderNodeByElement(el) ) {
            disableElement(el);
        } else if ( !selected ){                
            enableElement(el);
        }
        
        if( !this.multiSelect ) {
            disabled_els.forEach(function(it){
                enableElement(it);
            });
        } else {
            this.addDisplayNode(el);
        }

        this.updateTitle();
    }

    this.generateTitle = function() {
        var t = "";
        if( _selectedOptions && Object.keys(_selectedOptions).length ) {
            if(this.multiSelect) {
                var cntSelected = Object.keys(_selectedOptions).length;
                if ( cntSelected == 1 ) {
                    t = Object.values(_selectedOptions)[0].title;
                } else if ( cntSelected ) {
                    t = cntSelected + " out of "+this.cntOfOptions()+" selected";
                }
            } else {
                t = Object.values(_selectedOptions)[0].title;
            }            
        }
        
        if( !(typeof(t)!=="undefined" && t && !isAnEmptyPlaceHolderNodeByText(t)) ) {
            t = this.my_node.title;
        } 
        
        return t;
    }
    this.updateTitle = function() {
        var txt = this.generateTitle();
        var title_el = gObject(this.controllerTitleID);
        title_el.innerHTML = txt;
    }
    
    this.onClickDisplayNode = function(el) {
        var ul_el = gObject(this.drop_down_selection_id);
        var cur_el = ul_el.querySelector("[data-id='"+el.dataset.id+"']");
        enableElement(cur_el);
        this.removeDisplayNode(el);
        
        this.updateTitle();
    }
    
    this.addDisplayNode = function(el) {
        var li = document.createElement("li");
        li.innerHTML = el.innerHTML;
        li.dataset.id = el.dataset.id;
        var ul_el = gObject(this.drop_down_display_id);
        ul_el.appendChild(li);
        if (ul_el.childElementCount) {
            this.showDisplayNodeList()
        }
    }
    this.removeDisplayNode = function(el) {
        var ul_el = el.parentElement;
        ul_el.removeChild(el);
        this.updateSelectedNodes(el.dataset.id);
        if (!ul_el.childElementCount) {
            this.hideDisplayNodeList();
        }
    }
    
    this.showDisplayNodeList = function(el) {
        if (typeof(el)==="undefined")
            el = gObject(this.drop_down_display_id).parentElement;
        if (el &&  el.classList.contains("hide_selected") ) {
            el.classList.remove("hide_selected");
        }
    }
    this.hideDisplayNodeList = function(el) {
        if (typeof(el)==="undefined")
            el = gObject(this.drop_down_display_id).parentElement;
        if (el && !el.classList.contains("hide_selected") ) {
            el.classList.add("hide_selected");
        }
    }
    
    this.onClickSelectMenuNode = function() {
        var el = gObject(this.controllerID);
        if(el &&  el.classList.contains("with_drop_down") ) {
            this.hideSelectMenuNodeList(this.controllerID);
        } else {
            this.showSelectMenuNodeList(this.controllerID);
        }
        return;
    }
    
    this.hideSelectMenuNodeList = function() {
        var el = gObject(this.controllerID);
        if (el &&  el.classList.contains("with_drop_down") ) {
            el.classList.remove("with_drop_down");
        }
    }
    this.showSelectMenuNodeList = function() {
        var el = gObject(this.controllerID);
        if (el && !el.classList.contains("with_drop_down") ) {
            el.classList.add("with_drop_down");
        }
    }
    
    this.draw = function() {
        this.resetOption();
        var t = "";
        var menu_node_title = this.my_node.title;
        t += "<span style='display:none'>";
        this.element_name =  this.menu_obj.objCls + "_" + sanitizeElementId(this.my_node.name);
        t += "<input type='text' name='" + this.element_name + "'";
        if(this.my_node.value)
            t += " value='"+this.my_node.value+"'";
        t += " ></input>"
        t += "</span>";
        
        if (this.my_node.isNAlignRight && this.my_node.title) {
            t += "<span style='vertical-align:middle;padding-right:1ch'>"+menu_node_title+"</span>";
        }
        
        this.controllerID = this.element_name+"_controller_"+_myCls;
        this.controllerTitleID = this.controllerID+"_title";
        t += "<div id="+this.controllerID+" class="+this.menu_obj.className+"_selectnew>";
        t += "<a class="+this.menu_obj.className+"_selectnew_header"; 
        t += ">";
        t += "<span id="+this.controllerTitleID+">";
        t += this.generateTitle();
        t += "</span>";
        t += "<div><b></b></div>";
        t += "</a>";
        
        var drop_down_id = this.controllerID+"_drop_down_div";
        t += "<div id='"+drop_down_id+"' class="+this.menu_obj.className+"_selectnew_list_drop_down>";
        this.drop_down_selection_id = drop_down_id + "_selection";
        
        t += "<ul id='"+this.drop_down_selection_id+"'>";
        t += generateOptions(_options);
        t += "</ul>";
        
        
        if(this.multiSelect) {
            this.drop_down_display_id = drop_down_id + "_display"; 
            var li = generateSelectedOptions(_options);
            t += "<div class='"+this.menu_obj.className+"_selectnew_list_drop_down_selected";
            if( !li || !li.length )
                t +=" hide_selected";
            t += "'>";
            
            t += "<ul  id='"+this.drop_down_display_id+"' >";
            t += "</ul>";
            
            t += "</div>";            
        }
        
        t += "</div>";
        
        t += "</div>";

        if ( typeof(GlobalNode_listeners)==="undefined")
            GlobalNode_listeners = {};
        
        var that = this;
        if( !(this.controllerID in GlobalNode_listeners) ) {
            document.addEventListener("click", function(event) {
                var hideList=false;
                var target = getEventTarget(event);
                if ( target.closest("#"+that.controllerID) ) {
                    if ( target.closest("#"+that.controllerID+" > a")) {
                         return that.onClickSelectMenuNode();
                    } else if( target.closest("#"+that.drop_down_selection_id) ) {
                        if( target.tagName == "LI" ) {
                            that.onSelectNodes(target);
                        }    
                    } else if (that.multiSelect && target.closest("#"+that.drop_down_display_id)) {
                        that.onClickDisplayNode(target);
                    }
                    if( target.closest("#"+drop_down_id ) ) {
                        if( !that.multiSelect )
                            hideList = true;
                    }
                } else {
                    hideList = true;
                }
                if ( hideList ) {
                    that.hideSelectMenuNodeList(that.controllerID);
                }
            });
            GlobalNode_listeners[this.controllerID] = true;                
        }
        
        if (!this.my_node.isNAlignRight && this.my_node.title) {
            t += "<span style='vertical-align:middle;padding-left:1ch'>"+menu_node_title+"</span>";
        }
            
        return t;
    }
    return this;
}

