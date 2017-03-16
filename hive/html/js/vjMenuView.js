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
/*
clickCallback
popupMenu
*/

// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ constructors
// _/
// _/_/_/_/_/_/_/_/_/_/_/

function vjMenuView(viewer) {
    vjDataViewViewer.call(this, viewer); // inherit default behavior of the DataViewer

    if (!this.className) this.className = "MENU_table";
    if (!this.classNameTop) this.classNameTop = this.className + "_top";
    if (!this.classNameHighlight) this.classNameHighlight = this.className + "_highlight";

    if (!this.hideTimeout) this.hideTimeout = 10000;
    if (!this.iconSize) this.iconSize = 0; //alerJ(this.iconSize,this);
    if (!this.icons) {
        this.iconSizeimg = Math.round(parseInt(this.iconSize) * 0.6);
        this.icons = { expand: "<img width="+this.iconSizeimg+" border=0 src=\'img/recItem.gif\'>" };
    }

    if (!this.maxMenuDepthOpen) this.maxMenuDepthOpen = 10;
    if (!this.rootMenuStyle) this.rootMenuStyle = "horizontal";
    //    if (!this.lastSearch) this.lastSearch = '';
    //    if (!this.lastSubmit) this.lastSubmit =[{name:'search',value:''}];
    if (!this.allowSearch) this.allowSearch = true;

    if (!this.selectedCounter) this.selectedCounter = 20;
    if (!this.rowStart) this.rowStart = 0;
    if (!this.rowsShown) this.rowsShown = 0;
    if (!this.rowCnt) this.rowCnt = "unknown";
    if (!this.showPaginator) this.showPaginator = false;
    if (!this.showInfo) this.showInfo = false;
    if (!this.rowSpan) this.rowSpan = 1;
    if (!this.excludedDataRefresh) this.excludedDataRefresh = [0];
    if (this.isNrefreshOnLoad===undefined) this.isNrefreshOnLoad=true;          //Don't refresh panels
    if (!this.urlExchangeParameter) this.urlExchangeParameter = urlExchangeParameter;
    if (!this.computeSubmitableValue) this.computeSubmitableValue = docLocValue;
    if ( this.startArg === undefined ) this.startArg = "start";
    if ( this.cntArg === undefined ) this.cntArg = "cnt";

    this.activeNodeAtDepth = [];

    this.rebuildTree = function(viewer, content, tbl) {
        
        //alerJ(this.getData(1).data,viewer);
        if (!tbl) {
            tbl = new vjTable(this.getData(0).data, 0, vjTable_propCSV);
        }

        if (this.rows) tbl.customizeRows(this.rows, true, true);
        var rowsLength=tbl.rows.length;
        for(var ir=0; ir<rowsLength; ++ir){
            var row=tbl.rows[ir];
            if(row.data===undefined)continue;
            if(row.data.source===undefined)continue;
            if(row.data.source.indexOf('eval')==0)row.data.source=eval(row.data.source.substring(5));
            var rowDataTbl=new vjTable(this.dataSourceEngine[row.data.source].data, 0, vjTable_propCSV);

            if (row.type == "select" && row.addMoreRowsToOptionsMenuFromData == true){
                if (!row.options) {
                    row.options = new Array();
                    if (row.defaultOptionsValue){
                        row.options.push([row.defaultOptionsValue.name,row.defaultOptionsValue.name,row.defaultOptionsValue.title]);
                    }
                    else {
                        row.options.push(["none","none","none"]);
                    }
                }

                for (var ii=0;ii<rowDataTbl.rows.length; ii++){
                    row.options.push([rowDataTbl.rows[ii][row.data.name],rowDataTbl.rows[ii][row.data.name],rowDataTbl.rows[ii][row.data.title]]);
                }
                continue;
            }
            var addRows=new Array();

            var dlength=rowDataTbl.rows.length?rowDataTbl.rows.length:1;
            var value=0, drow=0,dname='',dpath='/';
            for(var idr=0 ; idr<dlength ; ++idr){
                var addrow=cpyObj(row);

                if(!rowDataTbl.rows.length){
                    value=0;
                }
                else{
                    drow=rowDataTbl.rows[idr];
                }

                var idc=0;
                for(idc in row.data){
                    if(idc=="source")continue;
                    if(drow) value=drow[row.data[idc]];
                    if(idc=='name'){
                        if(!drow)value="Not found";
                        dname=value;
                    }
                    else if (idc=='path'){
                        if(!drow){
                            if(row.data[idc])value=row.data[idc];
                            else value='';
                        }
//                        if(value===undefined)
                        dpath=value===undefined?row.data[idc]:value;
                    }
                    else {
                        if(drow)
                            addrow[idc]=value;
                        else
                            addrow[idc]="Not Found";
                    }

                }
                addrow['name']=dname;
                addrow['path']=dpath+dname;
                if(this.getData(1) && this.getData(1).url.indexOf("static")!=0 ) {
                    var t_url = this.getData(1).url;
                    var valueEl = this.computeSubmitableValue(row.name,"0",t_url);
                    if(valueEl.indexOf(addrow['name'])>=0) {
                        addrow['value'] ="1";
                    }
                    else
                        addrow['value'] ="0";
                }
                else
                    addrow['value'] ="0";
                if(!drow)addrow['type']='';
                addRows.push(addrow);
            }
            tbl.rows.splice(ir,1);
            --rowsLength;
            --ir;
            tbl.customizeRows(addRows,true);
        }
        tbl.enumerate(function(this_, tbl, ir) {
            var node = tbl.rows[ir];
            if (!node.nonAutoNode) {
                delete node.children;
                delete node.childrenCnt;
            }

//            if (node.response && node.url) {
//                if (!node.target)
//                    node.target = null;
//
//                node.url='javascript:vjObjEvent("linkActionResponse","'+this_.objCls+'","'+node.url+'","'+node.target+'")';
//                delete node.target;
//            } else if (node.target && node.url) {
//                node.url='javascript:linkURL("'+node.url+'","'+node.target+'")';
//                delete node.target;
//            }
        }, this);

//        tbl.enumerate("if(node.target && node.url) {node.url='javascript:linkURL(\"'+node.url+'\",\"'+ node.target+'\")'; delete node.target;}");
        this.tree = new vjTree(tbl);

        if (this.precompute)
            tbl.enumerate(this.precompute, this);

        for( var ids = 1 ; ids < this.data.length ; ++ids) {
            if (this.dataSourceEngine && this.getData(ids) && this.getData(ids).url.indexOf("static")!=0) {
                this.tree.enumerate(function(this_, node) {
                    if (node.isSubmitable) {
                        node.value = this_.computeSubmitableValue(node.name, isok(node.value) ? node.value : "", this_.getData(ids).url);
                    }
                }, this);
            }
        }

        if (this.allowSearch) {
            this.tree.enumerate("if(node.name=='search'){node.type='search';}");
            /*if (this.formObject) {
                alert("captured " + this.formObject.name);

                this.formObject.onsubmit = function(e) {
                    return false;
                };
            }*/
        }

        this.tree.root.menuHorizontal = (this.rootMenuStyle == "horizontal") ? true : false;

        // for pagenators
        var tblArr = new vjTable((this.data.length > 1 && this.dataSourceEngine) ? this.getData(1).data : "id,name,path,value\n", 0, vjTable_propCSV);
        if(this.exclusionObjRegex)tblArr.mangleRows(this.exclusionObjRegex, "delete");

        if (tblArr.rows.length > 0) {

            for (var ii = 0; ii < tblArr.rows.length && tblArr.rows[ii].id != "info"; ++ii);  // look for info
            this.rowsShown = Int(tblArr.rows.length / this.rowSpan);
            if (ii == tblArr.rows.length) {
                var isNotNewStart = this.computeSubmitableValue(this.startArg, this.rowStart, this.getData(1).url);
                if (isNotNewStart == "0") this.rowStart = 0;
                if (typeof this.LastIndexCol !== 'undefined')
                    this.queryStart = parseInt(tblArr.rows[tblArr.rows.length - (this.rowSpan ? this.rowSpan : -1)].cols[this.LastIndexCol]) + 1;
                this.rowCnt = 'unknown'; // couldn't find info block .. presume the count
            } else {
                this.rowCnt = parseInt(tblArr.rows[ii].total);
                this.rowStart = parseInt(tblArr.rows[ii].start);
                if (isNaN(this.rowCnt) || isNaN(this.rowStart)) { // the info comes NOT in object format
                    this.rowCnt = parseInt(tblArr.rows[ii].cols[2]);
                    this.rowStart = parseInt(tblArr.rows[ii].cols[1]);
                }
                this.getData(1).dataSize=this.rowCnt;
                --this.rowsShown;
            }

            // read the counter
           /* if (this.formObject && this.formObject.elements[this.container + "_pageCounter"])
                this.selectedCounter = this.formObject.elements[this.container + "_pageCounter"].value;
            else*/
                this.selectedCounter = this.computeSubmitableValue(this.cntArg, this.selectedCounter, this.getData(1).url);

            //this.rowCnt=this.selectedCounter;
        }
        else {
            this.rowsShown = 0;
            if(this.dataSourceEngine && this.getData(1)) {
                var u_start = this.computeSubmitableValue(this.startArg,0,this.getData(1).url);

                if(u_start && u_start!=="0") {
                    this.rowCnt = 'unknown';
                    this.rowStart =parseInt(u_start);
                }
                else{
                    this.rowCnt = 0;
                    this.rowStart = 0;
                }
            }
            else{
                this.rowCnt = 0;
                this.rowStart = 0;
            }
        }
    };

    this.composerFunction = function (viewer, content) {
        this.rebuildTree(viewer, content);
        this.redrawMenuView(this.tree.root);

        var t = "vjObjEvent(\"onHide\", \"" + this.objCls + "\",0,0,true);";
        if (gOnDocumentClickCallbacks.indexOf(t) == -1) gOnDocumentClickCallbacks += t;
    };
    this.refresh = function () {
        this.redrawMenuView ();
    };
    this.isVertical = function(node) {
        // depth 0 is root, depth 1 is top-level menu bar
        return node.menuHorizontal===undefined ? node.depth > 0 : !node.menuHorizontal;
    };
    this.redrawMenuView = function (node) {
        if (!node) node = this.tree.root;
        if (this.div != document.getElementById(this.container)) {
            // this.div may have become invalid because a parent div got re-rendered
            this.div = document.getElementById(this.container);
        }
        if (!this.div) {
            return;
        }
        gCreateFloatingElements(this.createFloatingLayers(this.maxMenuDepthOpen));
        //alert(this.div.id + " --  " + this.outputNode(this.tree.root,node.menuHorizontal ? false : true ))
        var div;
        if (node.path == this.tree.root.path) {
            div = this.div;
        } else if (this.activeNodeAtDepth[node.depth] == node.path) {
            div = gObject(this.container+"-level"+node.depth);
        }

        if (div) {
            div.innerHTML = this.outputNode(node, this.isVertical(node));
        }
        var javaScriptEngine 
        if (!javaScriptEngine && typeof(vjJS)!=="undefined"){
            javaScriptEngine = vjJS["undefined"];
        }
        
        if( javaScriptEngine )javaScriptEngine.evalCode;
    };

    this.executeUrl = function(url, node, comp) {
        var target = node.target;
        if(!target) {
            funcLink(url,this,node,comp);
        }
        else {
            if( target == "ajax" ) {
                if(url.indexOf("http://")<0){
                    url = "http://"+url;
                }
                this.actions_DS.reload(url,true);
            }
            else {
                linkSelf(url,target);
            }
        }
    };

    this.enumerate = function (operation, params, ischecked, leaforbranch, node) {
        if (!this.tree || !this.tree.root) return;
        return this.tree.enumerate(operation, params, ischecked, leaforbranch, node);
    };

    this.find = function (checker, collector, params, ischecked, leaforbranch, node) {
        if (!this.tree || !this.tree.root) return;
        return this.tree.findByOperation(checker, collector, params, ischecked, leaforbranch, node);
    };


    // _/_/_/_/_/_/_/_/_/_/_/
    // _/ event handlers

    this.onClickMenuNode = function (container, nodepath) {
        var node = this.tree.findByPath(nodepath);
        this.nodeHide(this.container,nodepath);


        if (this.callerObject && this.callerObject.onClickMenuNode) {
            this.callerObject.onClickMenuNode(container, node, this);
            return;
        }

        var res = true;
        if (node.confirmation) {
            res = confirm("Are you sure you want to " + node.title + " selected objects");
            if (!res) return;
        }


        //document.location=node.url;
        var func = this.clickCallback;
        if (!func) func = node.clickCallback;
        if (!func) {
            func = node.url;
            if (typeof (func) == 'string')
                func = func.replace(/&amp;/g, "&");
        }

        //var comp= {ids:new Array () };
        var comp = cpyObj(node);

        var prompt_res = false;
        if (node.prompt) {
            prompt_res = prompt(node.prompt, "type here");
            if (!prompt_res)
                return;
            comp.prompt_res = prompt_res;
        }
//        comp.dcls = "\""+this.objCls+"\"";
        comp.dataname=this.getData(1)?this.getData(1).name:"";
        comp.dataurl=this.getData(1)?this.getData(1).url:"";
        var _dataUrl_sep=comp.dataurl.indexOf("//");
        if(_dataUrl_sep>=0)
            comp.dataurl=comp.dataurl.substr(_dataUrl_sep+2);
        comp.ids = new Array();
        comp.types = new Array();
        comp.objs=new Array();
        if (this.evalVariables) {
            for ( var i in this.evalVariables) {
                comp[i] = this.evalVariables[i];
            }
        }
        if(this.objType)comp.objType=this.objType;
        if (this.objectsIDependOn) {

            for (var im = 0; im < this.objectsIDependOn.length; ++im) {
                comp.objs = comp.objs.concat(this.objectsIDependOn[im].accumulate("node.checked>0 && node.id", "node", 0));
                comp.ids = comp.ids.concat(this.objectsIDependOn[im].accumulate("node.checked>0 && node.id", "node.id", 0));
                comp.types = comp.types.concat(this.objectsIDependOn[im].accumulate("node.checked>0 && node.id", "node._type", 0));
            }
        }
        comp.type=comp.types.join(",");comp.ids=comp.ids.join(",");comp.objs=comp.objs.join(",");
        var srchEl = undefined;
        if (this.formObject && this.formObject.elements) srchEl = this.formObject.elements[this.container + "_search"];
        comp.search = srchEl ? srchEl.value : "";

        //alert( func  + " ="+evalVars(func,"$(",")",comp) )
        func = evalVars(func, "$(", ")", comp);
        if(!func)return ;

        if(node.refreshDelayCallback) {
            node.refreshDelayCallback = evalVars(node.refreshDelayCallback,"$(",")",comp);
        }

//        if( (node.response || node.backend || node.url_datasource) && func) {
//            this.actions_DS.reload(func,true);
//        }
//        else {
//            funcLink(func, this, node, comp);
//        }

        this.executeUrl(func, node, comp);

        if(node.redraw){
            if(node.redraw===true)
                this.redrawMenuView();
            else
                funcLink(node.redraw,this,node);
        }

        if (node.refreshDelay) {
            if(node.refreshDelayCallback) {
                setTimeout(node.refreshDelayCallback,node.refreshDelay);
            }
            else {
                var refreshnode = this.tree.findByName("refresh");
                //alerJ('found',node);
                if (refreshnode)
                    setTimeout("vjObjEvent('onClickMenuNode','" + this.objCls + "','" + refreshnode.path + "');", node.refreshDelay);
            }
        }
    };

    explorerValues = [];
    this.onClickExplorerNode = function (container, nodepath, eltname) {
        var node = this.tree.findByPath(nodepath);
        var element = this.formObject.elements[eltname];

        this.initExplorer(node);
        var that = this; // closure
        this.explorer.onSubmitObjsCallback = function(viewer, explorer_nodelist) {
//            if (node.multiSelect)
//            {
//                explorerValues.push (explorer_node.id);
//                if (gKeyCtrl)
//                    return;
//            }
//            if (explorerValues.length == 0)
//                node.value = explorer_node;
//            else
            var explorer_nodelistString = "";
            if( explorer_nodelist && typeof(explorer_nodelist)=="object") {
                //explorer_nodelistString = Object.keys(explorer_nodelist).join();
                for (var i = 0; i < explorer_nodelist.length; i++){
                    explorer_nodelistString += explorer_nodelist[i].id + ",";
                }
                explorer_nodelistString = explorer_nodelistString.substring (0, explorer_nodelistString.length-1);
            }
                node.value = explorer_nodelistString;
            
            if (element ) {
                element.value = explorer_nodelistString;
            }
//            else if (element)
//                element.value = explorerValues.toString();
            
            gObjectSet(that.container + "_explorer", "-", "-", "-", "hide", "-", "-");

            that.onChangeElementValue(container, nodepath, eltname);
        };
        gObjectSet(this.container + "_explorer", "-", "-", "-", "show", "-", "-");
        var pos = gObjPos(element);
        gObjectPositionShow(this.container + "_explorer", pos.x, pos.y + pos.cy, 1000 + node.depth + 1);
        this.explorer.render();
        this.explorer.load();
    };

    this.onHide = function (container, from, to, hideTopLevel, forceHide)
    {
        if(!forceHide && gMenuOver && gMenuOver.indexOf(this.objCls)==0 )
            return ;

        if (!from) from = 1;
        if (!to) to = this.maxMenuDepthOpen;
        for (var i = from; i <= to; ++i) {
            gObjectSet(this.container + "-level" + i, '-', '-', '-', 'hide', '-', '-');
            this.activeNodeAtDepth[i] = null;
        }

        var dat = new Date();
        if (this.popupMenu && ((dat.getTime() - this.menuConstructed.getTime())>500) && hideTopLevel) {
            gObjectSet(this.popupMenu, '-', '-', '-', 'hide', '-', '-');
            this.activeNodeAtDepth[0] = null;
        }

    };

    this.nodeHide = function (container,nodepath){
        if(!container)container=this.container;
        if(!nodepath)
            return this.onHide(container);
        var node = this.tree.findByPath(nodepath);
        if(gMenuOver!=(this.objCls+nodepath))
            this.onHide(container,node.depth,0,undefined,true);
    };

    this.mouseLeave = function(container , nodepath, element){
        if(!gMenuOver){
            this.nodeHide(container);
            return;
        }
        if(gMenuOver.indexOf(this.objCls)!=0)
            this.nodeHide(container);
    };
    this.mouseEnter = function(container , nodepath, element){
        if(gMenuOver!=(this.objCls+nodepath))return;
        var node = this.tree.findByPath(nodepath);
        if (this.lastHighlightElement != element && element) {
            this.lastHighlightElementClass = element.className;
            //element.className += " " + this.classNameHighlight;
            this.lastHighlightElement = element;
            $("#" + element.id).addClass(this.classNameHighlight);
        }

        if (!node)
            return; 
        
        this.onHide(this.container, node.depth, this.maxMenuDepthOpen,undefined,true);
        if (node.children.length == 0) return;

        var menuName = this.container + "-level" + node.depth;

        var o = gObject(menuName); if (!o) return;
        o.innerHTML = this.outputNode(node, this.isVertical(node));
        this.activeNodeAtDepth[node.depth] = node.path;
        gObjectSet(menuName, '-', '-', '-', 'show', '-', '-');

        var pos = gObjPos(element);
        gObjectPositionShow(menuName, pos.x + (node.depth == 1 ? 0 : pos.cx), pos.y + (node.depth == 1 ? pos.cy + 2 : 0), 2000 + node.depth);


        node.expanded = true;
    };
    this.onMouseOutNode = function (container, nodepath, element) {  //!Now onMouseOver is triggered on mouseLeave
        if (this.lastHighlightElement == element) {
            //this.lastHighlightElement.className = this.lastHighlightElementClass;
            this.lastHighlightElement = "";
            $("#" + element.id).removeClass(this.classNameHighlight);
        }
    };

    this.onMouseOutList = function (container) {
        gMenuOver=null;
        var that=this;
        setTimeout(function(){that.mouseLeave.call(that,container);},600);
    };

    this.onMouseOver = function (container, nodepath, element) //!Now onMouseOver is triggered on mouseEnter
    {
        gMenuOver=this.objCls ? (this.objCls + nodepath): null;
        var that=this;
        setTimeout(function(){that.mouseEnter.call(that,container,nodepath,element);},200);//});
    };

    this.getPageCounterElement = function()
    {
        return this.formObject.elements[this.container + "_pageCounter"];
    };

    this.getCurrentPageCounterValue = function()
    {
        var el = this.getPageCounterElement();
        var currentPageCounter = el ? parseInt(el.value) : this.selectedCounter;
        if (isNaN(currentPageCounter)) currentPageCounter = 1000000;
        return currentPageCounter;
    };
    
    this.onUpdate = function (container, page) {
        var el = this.getPageCounterElement();
        var currentPageCounter = this.getCurrentPageCounterValue();


        this.needsUpdate = false;
        for (var d = 1; d < this.data.length; ++d) {
            if (!this.getData(d)) continue;
            var newurl = this.cmdUpdateURL ? this.cmdUpdateURL : this.getData(d).url;

            if (d < 2) {
                var tempst=this.rowStart;
                if (page == 1)
                    this.rowStart += this.rowsShown;
                else if (page == -1)
                    this.rowStart -= this.selectedCounter;
                else this.rowStart = 0;
                if (this.selectedCounter != currentPageCounter)
                    this.rowStart = 0;
                if (this.rowStart < 0)
                    this.rowStart = 0;
                if (page != 1 || this.queryStart<tempst+parseInt(this.selectedCounter))
                    this.queryStart = this.rowStart;
            }

            this.selectedCounter = currentPageCounter;
            var skip = false;
            for (var k = 1; k < this.excludedDataRefresh.length; ++k) {
                if (this.excludedDataRefresh[k] == d)
                    skip = true;
            }
//            var submObj;
            var paramNodes = this.tree.accumulate("node.isSubmitable", "node");
            for (var i = 0; i < paramNodes.length; ++i) {
                if (!skip || paramNodes[i].forceUpdate) {
                    if (paramNodes[i].isNskip) skip = false;
                    //                    var paramEl = this.formObject.elements[paramNodes[i].name];
                    //                    if (!paramEl)
                    //                        paramEl = this.formObject.elements[this.container + "_" + paramNodes[i].name];
                    //                    //                if ((!paramNodes[i].value || !paramNodes[i].value.length) && paramEl.value.length) paramNodes[i].value = paramEl.value;
                    //                    var lastSubmitableIndex = -1;

                    //var parVal = this.computeSubmitableValue(paramNodes[i].name, 0, this.getData(d).url);
                    //if (paramNodes[i].value != '.history')
                    parVal = paramNodes[i].value; // ? paramNodes[i].value : (parVal ? parVal : '');
                    //                    for (var j = 0; j < this.lastSubmit.length; ++j) { if (this.lastSubmit[j].name == paramNodes[i].name) { lastSubmitableIndex = j; break; } }
                    //                    if (lastSubmitableIndex >= 0) {

                    //                        submObj = this.lastSubmit[lastSubmitableIndex];
                    //                        if (paramNodes[i].forceUpdate && (!submObj.value || submObj.value != parVal)) skip = false;
                    //                        submObj.value = parVal; //(paramEl.type == 'checkbox') ? paramEl.checked : paramEl.value;
                    //                    }
                    //                    else {
                    //                        this.lastSubmit.push({ 'name': paramNodes[i].name, value: parVal }); //(paramEl.type == 'checkbox') ? paramEl.checked : paramEl.value });
                    //                        submObj = this.lastSubmit[this.lastSubmit.length - 1];
                    //                    }
                    //                    if (paramNodes[i].forceUpdate && (!submObj.value || submObj.value != parVal)) skip = false;
                    if (!paramNodes[i].parentDependant || paramNodes[i]===undefined){
                        if (parVal === true) parVal = '1'; else if (parVal === false) parVal = '0';

                        newurl = this.urlExchangeParameter(newurl, paramNodes[i].name, (parVal && parVal.length) ? (!paramNodes[i].isRgxpControl ? parVal : (paramNodes[i].rgxpOff ? parVal.replace(/([.?*+^$[\]\\(){}|-])/g, "\\$1") : parVal)) : '-');
                    }
                    if (paramNodes[i].parentDependant){ // when the element got clicked, it'll be shown like "parent=element" in the newUrl in stead of "element=1"

                        var fieldElement = this.computeSubmitableValue(paramNodes[i].parent.name,null,newurl);
                        parVal = (parVal!="0" || parVal==true) ? paramNodes[i].name : "0";

                        if (!fieldElement)
                            newurl = this.urlExchangeParameter(newurl, paramNodes[i].parent.name, parVal.length ? (!paramNodes[i].isRgxpControl ? parVal : (paramNodes[i].rgxpOff ? parVal.replace(/([.?*+^$[\]\\(){}|-])/g, "\\$1") : parVal)) : '-');
                        else {
                            fieldElement = fieldElement.split(";");
                            var indexEl = fieldElement.indexOf(paramNodes[i].name);
                            if (indexEl!=-1) (parVal!="0") ? 1 :fieldElement.splice(indexEl,1);
                            else (parVal!="0") ? fieldElement.push(paramNodes[i].name) : 0;
                            fieldElement=fieldElement.join(";");
                            newurl = this.urlExchangeParameter(newurl, paramNodes[i].parent.name, fieldElement);
                        }
                    }
                }
            }

            if(this.menuUpdateCallback) {
                newurl=funcLink(this.menuUpdateCallback,this,newurl);
                if( !newurl )
                    return;
            }

            if (!skip) {
                if(!this.isNpagerUpdate) {
                    newurl = this.urlExchangeParameter(newurl, "pageRevDir", page == -1 ? '1' : '-');
                    newurl = this.urlExchangeParameter(newurl, this.startArg, this.queryStart ? this.queryStart : this.rowStart);
                    newurl = this.urlExchangeParameter(newurl, this.cntArg, this.selectedCounter);
                }
                //if (submObj) newurl = this.urlExchangeParameter(newurl, "prop_val", submObj.value.length ? submObj.value : '-');

                //alert("from Menu  " + newurl )
                if (newurl != this.getData(d).url) {
                    if(this.datastack){
                        if(!isok(this.getData(d).urlStack)){
                            this.getData(d).urlStack=new Array();
                        }
                        this.getData(d).urlStack.push(this.getData(d).url);
                    }
                    this.getData(d).url = newurl;
                    this.getData(d).load();
                }
            }
        }
        return;
    };

    this.displayNode = function(node,actList){
        var display=true;
        if (node.is_forced_action) {
            if (node.is_obj_action &&  actList.checkCnt == 0)
                return false;
        }
        else {
            if (node.is_obj_action && (!actList[node.name] || actList[node.name] != actList.checkCnt))
                return false;
        }
        if (node.single_obj_only && actList.checkCnt != 1)
            return false;
        if (node.type == "hidden" || node.hidden == true)
            return false;
        if(!node.nonAutoNode){
            display=false;
            for(var i=0 ; i<node.children.length ; ++i){
                if(this.displayNode(node.children[i],actList)){
                    display=true;
                    break;
                }
            }
        }
        return display;
    };

    this.isNodeInactive=function(node,displayed){
        if(!node.inactive)
            return false;
        if(typeof(node.inactive)=='string')
            return funcLink(node.inactive,node,this);
        else {
            if( node.inactive===true && displayed)
                return true;
            return false;
        }
    };

    // _/_/_/_/_/_/_/_/_/_/_/
    // _/ generators

    this.outputNode = function (node, isvertical) {

        //document.getElementById("DV_dvTable_Result_0_cell").parentElement.offsetWidth
        var minWidth = -1; 
        if (this.tab && this.tab.parent.frame!="notab" && this.tab.parent.container != "dvMenu" && this.tab.name != "menu" && document.getElementById(this.tab.container) &&document.getElementById(this.tab.container).offsetParent && document.getElementById(this.tab.container).offsetParent.offsetParent)
        {
            var viewerEl = document.getElementById(this.tab.container).offsetParent.offsetParent.offsetParent;
            var brdrsEl = __getBorderWidth(viewerEl);
            var viewerWidth = Int(__getCurrentComputedStyle(viewerEl,'width')) - brdrsEl.left - brdrsEl.right ;
            minWidth = (window.innerWidth - 75) < viewerWidth ? (window.innerWidth - 75) : viewerWidth;
        }
        
        var clsTbl = " class='" + (isvertical ? this.className : this.classNameTop) + "' ";
        var clsCell = " class='" + (isvertical ? this.className : this.classNameTop) + "_td' ";
        var clsSepar = " class='" + (!isvertical ? this.className : this.classNameTop) + "_separator' ";
        var t = "";
        t += "<table " + (isvertical ? "" : "width=" + (minWidth > -1 ? minWidth : "100%")) + " " + clsTbl + " onmouseout='mouseLeave(event,this,function(){vjObjEvent(\"onMouseOutList\", \"" + this.objCls + "\")})'>";

        if (!isvertical) t += "<tr>";
        //else if(this.showPopupTitle) {
        //    t+="<tr><td colspan=4 class='MENU_table_title' >"+(node.title ? node.title: node.name )+"</td></tr>";
        // }

        var children = node.children.sort(function (a, b) {

            if (a.align > b.align) return 1; else if (a.align < b.align) return -1;
            if(a.order===undefined)a.order=10;
            if(b.order===undefined)b.order=10;
            if (parseFloat(a.order) > parseFloat(b.order)) return 1; else if (parseFloat(a.order) < parseFloat(b.order)) return -1;

            else return 0;
        });

        // check the action list for optional items
        var actList = new Object({ checkCnt: 0 });
        if (this.objectsIDependOn) {
            for (var im = 0; im < this.objectsIDependOn.length; ++im) {
                var depObj = this.objectsIDependOn[im];
                depObj.enumerate("if(node.checked>0 " + ((depObj.tree && !depObj.actionsOnLeaves) ? " && node.leafnode" : "") + " ){var act=isok(node._action) ? node._action.split(',') : new Array();for(var iv=0; iv<act.length; ++iv){ if(node._actionCancel && node._actionCancel[act[iv]]) continue; if(!params[act[iv]])params[act[iv]]=1;else ++params[act[iv]];} ++params.checkCnt;}", actList);
            }
        }
        // alerJ("objectsIDependOn " , actList);


        var side = "left";
        var spaceOccupied = 0;
        for (var ic = 0; ic < children.length; ++ic) {
            var nc = children[ic];
            //            var lastSubmitableIndex = -1;
            //            for (var i = 0; i < this.lastSubmit.length; ++i) {
            //                if (nc.name == this.lastSubmit[i].name) {
            //                    lastSubmitableIndex = i;
            //                    break;
            //                }
            //            }
            var nodeDisplay=this.displayNode(nc,actList);
            var nodeActive=!this.isNodeInactive(nc,nodeDisplay);
            if(nc.inactive===undefined && !nodeDisplay){
                continue;
            }

            var realVal = nc.title ? nc.title : nc.name;

            if (realVal && realVal.indexOf("eval:") == 0)
                realVal = eval(realVal.substring(5));

            var a = "vjObjEvent(\"onClickMenuNode\", \"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\")";



            if (isvertical || nc.newLine ) t += "<tr>";
            if (isvertical && nc.name == "-") t += "<hr/>";
            if (!isvertical && nc.align == "right" && side == "left") {
                t += "<td style='border:0px;cursor:default' width='" + (100 - spaceOccupied) + "%'></td>"; side = "right";
                //t += "<td style='border:0px' width='" + (100 - children.length) + "%'></td>"; side = "right";
            }
            t += "<td " + clsCell + " id=" + (nc.name ? nc.name : "") + "_menuButton " ;
            if( nodeActive && nodeDisplay){
                if (nc.url && nc.type != 'search' && nc.type != 'text')
                    t += " onclick='" + a + "' ";
                else if(!nc.type && nc.isSubmitter){
                    t += "onclick='javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls + "\")'";
                }
//                else
//                    alerJ('node:',nc);
                if (nc.type != 'empty' && nc.type != 'separator') {
                    t += " onmouseover='var that=this;mouseEnter(event,this,function(){vjObjEvent(\"onMouseOver\", \"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\", that)})' ";
                    t += "onmouseout='var that=this;mouseLeave(event,this,function(){vjObjEvent(\"onMouseOutNode\", \"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\", that)})' ";
                }


            }
            else{
                if (nc.type != 'search' && nc.type != 'text'){
                    t += " style='opacity: 0.7;filter:alpha(opacity=70);cursor:default'";
                }
            }

            var tooltipValue= nc.description;
            if(!tooltipValue && nc.type != 'empty' && nc.type != 'separator') {
                tooltipValue = nc.title;
            }

            if( tooltipValue && !(tooltipValue == nc.title && (this.showTitles || nc.showTitle || !nc.icon) ) ) {
                t += " title='" + sanitizeElementAttr(textContent(tooltipValue)) + "' ";
            }

            if (nc.size)
                t += " width='" + nc.size + "' ";
            if (nc.size && !isNaN(parseInt(nc.size))) spaceOccupied += parseInt(nc.size);
            else spaceOccupied += 1;

            t += " >";


            //if(this.titlesOnBottom)t += "<span >";
            //else
                t += "<span style='white-space:nowrap;'>";
            if (nc.icon && nc.type != 'search' && nc.type != 'pager' && nc.type != 'select') {
                t += "<table border=0><tr><td ";
                if (nc.type == 'text') t += " onclick='" + a + "' ";
                t += "valign=center>";
                t += this.formDataValue(nc.icon, "icon", nc, undefined, nc.icon_srcset ? {srcset: nc.icon_srcset} : undefined);
                t += "</td><td>";
            }
            //t += "<table border='0'><tr><td valign='center'><img border='0' width='" + this.iconSize + "' height=" + this.iconSize + " src='img/" + nc.icon + ".gif' ></td><td>";
            if(nc.type=='select'){

                var optarr;
                if(nc.options instanceof Array){
                    optarr=nc.options;
                } else {
                    var optTabl=new vjTable(nc.options,0,vjTable_collapsePropFormat);
                    optarr=optTabl.rows;
                }
                var rr="";
                for(var ioR=0;ioR<optarr.length;++ioR){
                    var opCols=(nc.options instanceof Array) ? optarr[ioR] : optarr[ioR].cols;
                    if(!opCols.length)continue;
                    var value,title,opTxt;
                    value=title=opTxt=opCols[0];
                    if(opCols.length==2){
                        title=opTxt=opCols[1];
                    }
                    else if(opCols.length>=3){
                        title=opCols[1];
                        opTxt=opCols[2];
                    }
                    rr += "<option value='" + sanitizeElementAttr(value) + "' title='"+sanitizeElementAttr(title)+"' ";
                    if (value == (nc.selectedCounter ? nc.selectedCounter : nc.value))
                        rr += " selected ";
                    rr += " >"+ opTxt+ "</option>";
                }
                t += "<table border=0 ><tr>";
                if (rr.length) {
                    var select_title = "<span style='white-space:nowrap;' >";
                    if (!nc.icon) select_title += nc.title;
                    select_title += "</span>";
                    if (nc.icon) {
                        select_title += "<td><img border=0 width=" + this.iconSize + " height=" + this.iconSize + " src='img/" + nc.icon + ".gif' ></td>";
                    }
                    t += "<td valign=center >";
                    if (nc.isNAlignRight) {
                        t += select_title + "&nbsp;";
                    }
                    t+="<select name='" + this.objCls + "_" + sanitizeElementId(nc.name) + "' ";
                    t += "onchange='vjObjEvent(\"onChangeElementValue\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\",\"" + sanitizeElementAttrJS(nc.value) + "\")' ";
                    if(!nodeActive) {
                        t += " disabled ";
                    }
                    t +=">";
                    t += rr;
                    t += "</select>";
                    if(!nc.isNAlignRight) t += "&nbsp;"+select_title;
                    t += "</td>";
                }
                if (nc.icon){
                    t += "<td><img border=0 width=" + this.iconSize + " height=" + this.iconSize + " src='img/" + nc.icon + ".gif' ></td>";
                }
                t += "</tr></table>";
            }
            else if (nc.type == 'search') {
                t += "<table border='0'><tr>";
                var searchVal = this.getData(1) ? this.computeSubmitableValue(nc.name, '', this.getData(1).url) : this.computeSubmitableValue(nc.name);
                searchVal = nc.rgxpOff ? (nc.value ? nc.value : '') : searchVal;
                var searchName = sanitizeElementId(this.objCls + "_" + (nc.name ? nc.name : "search"));
                t += "<td valign='center' align='left'>";
                if (nc.prefix) t += nc.prefix;
                t += "<input type='text' size='" + (nc.size ? nc.size : 10) + "' valign='center' name='" + searchName + "' value='" + sanitizeElementAttr(searchVal) + "'"
                  + " onchange='vjObjEvent(\"onChangeElementValue\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\",\"" + sanitizeElementAttrJS(nc.value) + "\")'"
                  + " oninput='vjObjEvent(\"onInput\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\",\"" + sanitizeElementAttrJS(nc.value) + "\")'"
                  + " onkeypress='return vjObjEvent(\"onKeyPress\",\"" + this.objCls + "\",event,\"" + sanitizeElementAttrJS(nc.path) + "\",\"" + sanitizeElementAttrJS(nc.value) + "\")'";
                  if(!nodeActive) {
                      t += " disabled ";
                  }
                  t += "/></td>";
                if (this.iconSize) {
//                    t += "</tr></table></td></tr></table></span><span style='white-space:nowrap;'><table border=0><tr><td valign=center></td><td><table border='0'><tr>";
                    t += "<td valign='center' aligh='left'>";
                    t += "<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls + "\"); stopDefault(event);'>";
                    t += "<img src='img/search.gif' border='0' width='" + this.iconSize + "' height='" + this.iconSize + "' />";
                    t += "</button></td>";
                    if (nc.isRgxpControl) {
                        t += "<td valign='center' aligh='left'><button class='linker' type='button' onclick='javascript: vjObjEvent(\"onChangeElementValue\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\",\""
                        + searchName + "_rgxp\");stopDefault(event);'>"
                        + "<img src='img/" + (!nc.rgxpOff ? (nc.RxpIconOn ? nc.RxpIconOn : 'on_icon') : (nc.RxpIconOff ? nc.RxpIconOff : 'off_icon'))
                        + ".gif' id='" + searchName + "_rgxp' border='0' width='" + (this.iconSize / 2) + "' height='" + (this.iconSize / 2) + "' /></button></td>";
                    }
                }
                t += "</tr></table>";

            } else if(nc.type == 'submitter'){
                t += "<table><tr><td valign='center' aligh='left'>";
                t += "<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls + "\");stopDefault(event);'";
                if(!nodeActive) {
                    t += " disabled ";
                }
                t+=">";
                t += "<img src='img/search.gif' border='0' width='" + this.iconSize + "' height='" + this.iconSize + "' />";
                t += "</button></td></tr></table>";
            } else if (nc.type == 'pager' && (this.rowsShown>0 || this.rowStart != 0) ) {
                var rr = "", rr1 = "", rr2 = "", rr3 = "";
                //alerJ("pager", this)
                if (this.rowStart != 0) rr1 += "<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls + "\",-1);stopDefault(event);'><img border=0 valign=center width=" + this.iconSize + " src='img/previous.gif'/></button>";
                if ((this.rowCnt == 'unknown' || this.rowStart + this.rowsShown < this.rowCnt) && this.rowsShown) rr2 += "<button class='linker' type='button' onclick='javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls + "\",1);stopDefault(event);'><img border=0 valign=center width=" + this.iconSize + " src='img/next.gif'/></button>";
                if (this.showInfo) {
                    if( this.rowsShown ) {
                        rr3 += "<span style='white-space:nowrap;' >";
                        rr3 += "" + (this.rowStart + 1) + "-" + (this.rowStart + this.rowsShown);
                        if (this.rowCnt != 'unknown') rr3 += " of " + this.rowCnt;
                        rr3 += "</span>";
                    }
                    else {
                        rr3 += "<span style='white-space:nowrap;' > empty </span>";
                    }

                }
                var tmpCounter = this.selectedCounter;
                if (this.selectedCounter == 1000000) tmpCounter = "all";

                for (var ij = 0; ij < nc.counters.length; ++ij) {
                    //if( (nc.counters[ij]!="all" && this.rowCnt!="unknown" ) && nc.counters[ij]>this.rowCnt )
                    //    continue;
                    //if( nc.counters[ij]=="all" && this.rowsShown>=this.rowCnt)
                    //    continue;
                    rr += "<option value='" + sanitizeElementAttr(nc.counters[ij]) + "' ";
                    if (nc.counters[ij] == tmpCounter)
                        rr += " selected ";
                    rr += " >";
                    rr += nc.counters[ij];
                    rr += "</option>";
                }
                if (rr.length || rr1.length || rr2.length || rr3.length) {
                    t += "<table border=0 ><tr>";
                    if (rr1.length) t += "<td valign=center >" + rr1 + "</td>";
                    if (rr3.length) t += "<td valign=center >" + rr3 + "</td>";
                    if (rr2.length) t += "<td valign=center >" + rr2 + "</td>";
                    if (rr.length ) {
                        t += "<td valign=center ><select name='" + this.container + "_pageCounter' onchange='vjObjEvent(\"onUpdate\",\"" + this.objCls + "\")' >";
                        t += rr;
                        t += "</select>&nbsp;<span style='white-space:nowrap;' >";
                        if (!nc.icon) t += "per page";
                        t += "</span></td>";
                        if (nc.icon) t += "<td><img border=0 width=" + this.iconSize + " height=" + this.iconSize + " src='img/" + nc.icon + ".gif' ></td>";
                        //t+="<td><a href='javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls +"\")' > per page</a></td>";
                    }

                    t += "</tr></table>";
                }
            } else if (nc.type == "separator" ) {
                if( ic > 0 && ic < children.length-1 ){
                    if(isvertical) {
                        t += "<div height='1%' width='100%'  " + clsSepar + " id='tralala'></div>";
                    }
                    else {
                        t += "<table height='100%' width='" + (nc.size ? parseInt(parseInt(nc.size) / 2) : 5) + "' ><tr><td " + clsSepar + ">&nbsp;</td></tr></table>";
                    }
                }
            }
            else if (nc.type == "html") {
                t += realVal;
            }
            else if (nc.type == "explorer") {
                t += "<input type=\"text\" readonly ";
                var showTitle = this.showTitleForInputs || nc.showTitleForInputs;

                var gdata=this.getData(1);
                if (gdata && this.computeSubmitableValue(nc.name, 0, gdata.url)) {
                    t += " value=\"" + sanitizeElementAttr(this.computeSubmitableValue(nc.name, 0, gdata.url)) + "\" ";
                } else if (nc.value) {
                    t += " value=\"" + sanitizeElementAttr(nc.value) + "\" ";
                } else {
                    t += " value=\"\" placeholder=\"" + sanitizeElementAttr(realVal) + "\" ";
                    showTitle = false; // we already have the title in placeholder
                }
                var eltname = sanitizeElementId(nc.name ? this.objCls + "_" + nc.name : this.objCls + "-" + nc.type + "-" + i);
                t += " name=\"" + eltname + "\" ";
                var onclick = "vjObjEvent(\"onClickExplorerNode\", \"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\",\"" + eltname + "\")";
                t += " onclick='" + onclick + ";'";
                if(!nodeActive) {
                    t += " disabled ";
                }
                t+="/>";
                if (showTitle)
                    t += "<span onClick='" + onclick + "' >"+realVal+"</span>";
            }
            else if (nc.type == "help")
            {
                if (!nc.viewer) {
                    nc.viewer = new vjHelpView({data: nc.data, formObject: this.formObject});
                }
                var gdata=this.getData(nc.dataIndex);
                if (gdata) {
                    nc.viewer.container = sanitizeElementId(this.container + "_help_" + nc.path);
                    nc.viewer._rendered = false;
                    // override parent span's white-space:nowrap
                    t += "<div id='" + nc.viewer.container + "' style='white-space:normal;width:"+viewerWidth+"px'>" + nc.viewer.composeText(gdata.data) + "</div>";
                }
            }
            else if (nc.type && nc.type != 'empty' && nc.type != 'pager' && nc.type != 'select') {
                if (nc.prefix) t += nc.prefix;
                t += "<input type='" + (nc.type) + "' ";

                var showTitle = this.showTitleForInputs || nc.showTitleForInputs;

                var gdata=this.getData(1);
                if (gdata && this.computeSubmitableValue(nc.name, 0, gdata.url)) {
                    t += " value='" + sanitizeElementAttr(this.computeSubmitableValue(nc.name, 0, gdata.url)) + "' ";
                } else if (nc.value) {
                    t += " value='" + sanitizeElementAttr(nc.value) + "' ";
                } else {
                    if (nc.type == "text") {
                        t += " value='' placeholder='" + sanitizeElementAttr(realVal) + "' ";
                        showTitle = false; // we already have the title in placeholder
                    } else {
                        t += " value='" + sanitizeElementAttr(realVal) + "' ";
                    }
                }
                if (nc.name) t += " name='" + sanitizeElementId(this.objCls + "_" + nc.name) + "' ";
                else t += " name='" + this.objCls + "-" + nc.type + "-" + i + "' ";
                if (nc.size) t += " size=" + nc.size + " ";
                if (nc.readonly) t += " readonly disabled ";
                //                t+=" onChange='vjObjEvent(\"onElementChange\",\"" + this.objCls + "\",\""+nc.path+"\",this)' ";
                // if somebody presses "Enter" while in a text box, we want to react
                if (nc.type == 'text') t+= " onkeypress='vjObjEvent(\"onKeyPress\",\"" + this.objCls + "\",event,\"" + sanitizeElementAttrJS(nc.path) + "\",\"" + sanitizeElementAttrJS(nc.value) + "\")' oninput='vjObjEvent(\"onInput\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\",\"" + sanitizeElementAttrJS(nc.value) + "\")'";
                t += " onchange='vjObjEvent(\"onChangeElementValue\",\"" + this.objCls + "\",\"" + sanitizeElementAttrJS(nc.path) + "\",\"" + sanitizeElementAttrJS(nc.value) + "\");"+(nc.onChange ? nc.onChange : "")+"' ";
                if (nc.type == 'checkbox' && nc.value=='1') t += " checked "; // (lastSubmitableIndex >= 0 ? this.lastSubmit[lastSubmitableIndex].value : nc.value)) t += " checked />";
                else if (nc.type == "color" && nc.readonly) 
                    t+= " onClick='"+a+"' ";
                
                if(!nodeActive) {
                    t += " disabled ";
                }
                if(nc.type == "color" && nc.readonly) t+=">";
                else t+="/>";
                
                if (nc.type == 'checkbox')
                    t += realVal;
                else if (showTitle)
                    t += "<span onClick='"+a+"' >"+realVal+"</span>";
                
               if (nc.type == "color" && nc.readonly) t+= "</input>";
            }
            else if (this.showTitles || nc.showTitle || !nc.icon) {
                //if(this.titlesOnBottom)
                    //t+="<br/>";
                t += "<span style='white-space:nowrap;"+(nc.bgColor ? ";background-color:"+nc.bgColor+";" : "")+"' >";
                //if(nc.url)t += "<a href='"+nc.url+"' >";
                t += realVal;
                //if(nc.url)t+="</a>";
                t += "</span>";
            }

            t += "</span>";
            //t+="<span style='white-space:nowrap;'>"+nc.title+"</span>";
            //if(isvertical && nc.children.length && isok(this.icons.expand))
            //    t+="&nbsp;"+this.icons.expand;
            //t+=" /// "+nc.order  + "-"+nc.align;
            //if(nc.icon)
            if (nc.icon && nc.type != 'search' && nc.type != 'pager')
                t += "</td></tr></table>";
            t += "</td>";


            if (isvertical)
                t += "<td width=1>" + ((nc.children.length && isok(this.icons.expand)) ? this.icons.expand : "") + "</td>";

            if (isvertical || nc.newLine) t += "</tr>";


        }
        if (!isvertical && side == "left") t += "<td style='border:0px' width='" + (100 - children.length) + "%'></td>"; side = "right";
        if (!isvertical) t += "</tr>";
        t += "</table>";

        return t;
    };

    this.findElement = function(name, container) {
        if(!this.formObject)return null;
        var el = this.formObject.elements[this.objCls + "_" + name];
        if (!el)
            el = document.forms['form-floatingDiv'].elements[container + "_" + name];
        return el;
    };

    this.onKeyPress = function(container, e, nodepath, elname) {
        var node = this.tree.findByPath(nodepath);
        var el = this.findElement(node.name, container);
        
        if(this.onKeyPressCallback)
            this.onKeyPressCallback(container, e, nodepath, elname);
        
        if (node.type == 'search' || node.type == 'text') {
            var ev = e || event;
//            alert("HERE" + ev.keyCode || ev.which || 0);
            /* If enter is pressed */
            if ((ev.keyCode || ev.charCode || ev.which || 0) == 13) {
                if (el) node.value = (el.type == 'checkbox' ? el.checked : el.value);
                vjObjEvent('onUpdate', container);
                if (this.data.length <= 1 && node.url)
                    this.onClickMenuNode(container, nodepath);
                return false;
            }
        }
        return true;
    };

    this.onInput = function(container, nodepath, elname) {
        var node = this.tree.findByPath(nodepath);
        var el = this.findElement(node.name, container);

        if (el && node.forceUpdate && node.value != (el.type == 'checkbox' ? el.checked : el.value)) node.isNskip = true;
        if (el) node.value = (el.type == 'checkbox' ? el.checked : el.value);
    };

    this.onChangeElementValue = function (container, nodepath, elname)
    {

        var node = this.tree.findByPath(nodepath);
        var el = this.findElement(node.name, container);
        if (el && node.forceUpdate && node.value != (el.type == 'checkbox' ? el.checked : el.value)) node.isNskip = true;
        if (el) node.value = (el.type == 'checkbox' ? el.checked : el.value);
        //alerJ(el.value,node)

        if (node.type == 'search') {
            if (node.isRgxpControl && elname.indexOf("rgxp") >= 0) {
                node.rgxpOff = node.rgxpOff ? 0 : 1;
                node.description = "Regular expressions are currently " + (!node.rgxpOff ? 'on' : 'off');
                el.parentElement.parentElement.parentElement.parentElement.parentElement.parentElement.title = node.description;
                var elrgxp = gObject(elname);
                elrgxp.src = "img/" + (node.rgxpOff ? (node.RxpIconOff ? node.RxpIconOff : 'off_icon') : (node.RxpIconOn ? node.RxpIconOn : 'on_icon')) + ".gif";
            }
        }

        if (node.onChangeCallback) {
            funcLink(node.onChangeCallback, this,node);
        }
        if (node.isSubmitter) {
            vjObjEvent('onUpdate', container);
        }
    };

    this.createFloatingLayers = function (count) {
        var floating_ids = {};
        var t = "";
        for (var i = 0; i < count; ++i) {
            var id=this.container + "-level" + i;
            var elt_t = "<span id='" + id + "' ";
            if (this.parentVIS) {
                elt_t += " onMouseOver='vjObjEvent(\"mouseOverDiv\", \""
                    + this.parentVIS.obj.objCls + "\",\"" + id
                    + "\",1);' onMouseOut='vjObjEvent(\"mouseOverDiv\", \""
                    + this.parentVIS.obj.objCls + "\",\"" + id + "\",0);' ";
            }
            elt_t += " style='position:absolute; visibility:hidden;' ";
            elt_t += " ></span>";
            var elt = gObject(id);
            if (elt) {
                // don't duplicate a previously created span with same id
                elt.outerHTML = elt_t;
            } else {
                t += elt_t;
            }
            floating_ids[id] = 1;
        }

        var explorer_id = this.container + "_explorer";
        var explorer_container_id = explorer_id + "_container";
        var elt_t = "<span id=\"" + explorer_id + "\" style=\"position:absolute; visibility:hidden;\">";
        elt_t += "<table border=0 class=\"VISUAL_popup\">";
        elt_t += "<tr>";
        elt_t += "<td bgcolor=\"white\" align=\"right\" valign=\"middle\"><a href=\"#\" onclick=\"gObjectSet('" + explorer_id + "','-','-','-','hide','-','-');return false;\">";
        elt_t + "<small>close</small>";
        elt_t += "</a></td>";
        elt_t += "<td bgcolor=\"white\" width=\"1\"><a href=\"#\" onclick=\"gObjectSet('" + explorer_id + "','-','-','-','hide','-','-');return false;\">";
        elt_t += "<img src=\"img/delete.gif\" height=\"16\" border=0 />";
        elt_t += "</a></td>";
        elt_t += "</tr>";
        elt_t += "<tr>";
        elt_t += "<td colspan=2 bgcolor=\"white\"><span id=\"" + explorer_container_id + "\">";
        elt_t += "</span></td></tr></table>";
        elt_t += "</span>";

        var elt = gObject(explorer_id);
        if (elt) {
            // don't duplicate a previously created span with same id
            elt.outerHTML = elt_t;
        } else {
            t += elt_t;
        }

        floating_ids[explorer_id] = 1;

        if (this.parentVIS) {
            if (!this.parentVIS.flV.adoptedFloatingLayers) {
                this.parentVIS.flV.adoptedFloatingLayers = [];
            }
            var id;
            for (var i=0; i<this.parentVIS.flV.adoptedFloatingLayers.length; i++) {
                id = this.parentVIS.flV.adoptedFloatingLayers[i];
                floating_ids[id] = 0;
            }
            for (id in floating_ids) {
                if (floating_ids[id]) {
                    this.parentVIS.flV.adoptedFloatingLayers.push(id);
                }
            }
        }

        return t;
    };

    this.initExplorer = function(node) {
        var subTablesAttrs = [{
            tabname: "All",
            multiSelect: node.multiSelect ? node.multiSelect : false,
            tabico: "folder-apps",
            url: { type: "-", prop: "id,_brief,created" }
        }];

        if (node.explorerObjType) {
            subTablesAttrs[0].url.type = encodeURIComponent(node.explorerObjType);
        }

        if (node.explorerFileExt) {
            subTablesAttrs[0].url.prop_name = "orig_name,name";
            subTablesAttrs[0].url.prop_val = "." + encodeURIComponent(node.explorerFileExt) + "$";
        }

        this.explorer = new vjExplorerBaseView({
            container: this.container + "_explorer_container",
            formFolders: "form-floatingDiv",
            formTables: "form-floatingDiv",
            formSubmit:"form-floatingDiv",
            preselectedFolder: "/All",
            subTablesAttrs: subTablesAttrs,
            folders_DV_attributes: {
                width: 200,
                height: 400,
                frame:"notab",
                hideDV: true
            },
            tables_DV_attributes: {
                width: 700,
                height: 400,
                frame:"notab",
                maxtabs: 8
            },
            submit_DV_attributes : {
                width : "100%",
                height : 60,
                frame : "notab",
                isok:true
            },
            isNdisplay_Previews: true,
            autoexpand: 0,
            drag: false,
            isNShowactions:true,
            isSubmitMode:true,
            isok: true
        });
        this.explorer.init();
    };

    this.linkActionResponse = function(objCls, url, target) {
        var this_ = this;
        linkURL(url + '&raw=1', target, function(unused_param, text, page_request) { this_.onActionResponse(text, page_request); });
    };

    this.actionResponseHandlers = {};
    this.actionResponseGlobalHandlers = [];

    this.onActionResponse = function(text, page_request) {
        var response;
        try {
            response = JSON.parse(text);
        } catch (e) {
            return false;
        }

        for (var ih=0; ih<this.actionResponseGlobalHandlers.length; ih++) {
            var handler = this.actionResponseGlobalHandlers[ih];
            funcLink(handler.callback, handler.obj, response, handler.params);
        }

        var responsePerSig = {};

        for (var id in response) {
            if (!responsePerSig[response[id].signal])
                responsePerSig[response[id].signal] = {};

            responsePerSig[response[id].signal][id] = response[id].data;

            var handlers = this.actionResponseHandlers[response[id].signal];
            if (!handlers)
                continue;

            for (var ih=0; ih<handlers.length; ih++) {
                if (!handlers[ih].perId)
                    continue;

                funcLink(handlers[ih].callback, handlers[ih].obj, parseInt(id), response[id].data, handlers[ih].params);
            }
        }

        for (var sig in responsePerSig) {
            var handlers = this.actionResponseHandlers[response[id].signal];
            if (!handlers)
                continue;

            for (var ih=0; ih<handlers.length; ih++) {
                if (handlers[ih].perId)
                    continue;

                funcLink(handlers[ih].callback, handlers[ih].obj, responsePerSig[sig], handlers[ih].params);
            }
        }
        return true;
    };

    this.addActionResponseCallback = function(callback, obj, params) {
        this.actionResponseGlobalHandlers.push( { callback: callback, obj: obj, params: params, perId : false } );
    };

    this.addActionResponseCallbackPerSignal = function(signal, callback, obj, params) {
        if (!this.actionResponseHandlers[signal])
            this.actionResponseHandlers[signal] = [];

        this.actionResponseHandlers[signal].push( { callback: callback, obj: obj, params: params, perId : false } );
    };

    this.addActionResponseCallbackPerId = function(signal, callback, obj, params) {
        if (!this.actionResponseHandlers[signal])
            this.actionResponseHandlers[signal] = [];

        this.actionResponseHandlers[signal].push( { callback: callback, obj: obj, params: params, perId : true } );
    };

    var that = this;
    this.actions_DS = vjDS.add("","ds_"+this.objCls+"_actions","static://",function(unused_param,text,page_request){that.onActionResponse(text,page_request);});
}


function vjMenuPopup(viewer, content, pos) {

    var popupMenuName = viewer.popupMenuName ? viewer.popupMenuName : "dvMenuPopup";
    var popupViewer = viewer.dataViewEngine.find(popupMenuName, "menu", 0);

    var ds = undefined; if (popupViewer && popupViewer.data) ds = viewer.dataSourceEngine[popupViewer.data];
    if (!popupViewer || !ds) {
        alert("DEVELOPER ALERT: trying to open menu without appropriate popup menu " + popupMenuName + " infrastructure set up");
        return;
    }
    popupViewer.callerObject = viewer;
    popupViewer.popupMenu = popupMenuName;
    popupViewer.menuConstructed = new Date();
    //alert("opening " + popupMenuName + " - " +popupViewer.menuConstructed);

    if (!pos) pos = { x: gMoX, y: gMoY, cx: 0, cy: 0 }; //gObjPos(element);
    gObjectPositionShow(popupMenuName, pos.x + pos.cx, pos.y + pos.cy, 1000 + 1);
    //gObjectSet(popupViewer.container,'-','-','-','show','-','-');

    ds.url = "static://" + content;
    ds.load();
}



function vjPanelView(viewer) {
    if (!this.className) this.className = "PANEL_table";
    this.allowSearch = true;
    this.showInfo = true;
    this.hideViewerToggle=true;
    if(viewer.rows!==undefined){
        for( var ir=0 ; ir < viewer.rows.length ; ++ir){
            var row=viewer.rows[ir];
            if(row.data===undefined)continue;
            if(row.data.source===undefined)continue;
            viewer.data.push(row.data.source);
        }
    }
    vjMenuView.call(this, viewer); // inherit default behaviours of the MenuViewer

}

function vjSubmitPanelView(viewer)
{

    if (this.rows === undefined) this.rows= new Array();
    this.rows=verarr(this.rows);
    if (this.empty === undefined) this.empty= true;
    if (this.empty == true){
        this.rows=this.rows.concat( [
                {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
                {name:'pager', icon:'page' , title:'per page',order:2, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']}
//                {name: 'search', align: 'right', type: ' search',order:10, isSubmitable: true, title: 'Search', description: 'search sequences by ID',order:'1', url: "?cmd=objFile&ids=$(ids)" }
                ]);
    }

    this.iconSize=24;
    this.hideViewerToggle=true;
    if(this.formName!==undefined)this.formObject=document.forms[this.formName];

    vjPanelView.call(this,viewer);
    this.rows = this.rows.concat([
                {name:'update', align: 'right', type: "button",  order:10, value:'Update', description:'Update content', url: "javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls + "\")"}]);
};

function vjBasicPanelView(viewer)
{

    if (this.rows === undefined) this.rows= new Array();
    this.rows=verarr(this.rows);
    if (this.empty === undefined) this.empty= true;
    if (this.empty == true){
        this.rows=this.rows.concat( [
                {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
                {name:'pager', icon:'page' , title:'per page',order:2, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                { name: 'search', align: 'right', type: ' search',order:10, isSubmitable: true, title: 'Search', description: 'search sequences by ID',order:'1', url: "?cmd=objFile&ids=$(ids)" }
                ]);
    }

    this.iconSize=24;
    this.hideViewerToggle=true;
    if(this.formName!==undefined)this.formObject=document.forms[this.formName];

    vjPanelView.call(this,viewer);
};

//# sourceURL = getBaseUrl() + "/js/vjMenuView.js"