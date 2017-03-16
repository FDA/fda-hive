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
var latestCommand = -1;

if (!javaScriptEngine) var javaScriptEngine = vjJS["undefined"];
javaScriptEngine.include("js/vjPluggableToolbar.js");

function vjTableViewX2(viewer) {
    
     if (this.objCls)
            return;

    vjDataViewViewer.call(this, viewer); // inherit default behaviours of the
                                            // DataViewer

    if(this.isStickyHeader===undefined)
        this.isStickyHeader=true;
    if(this.isReOrderable)
        this.order_drag = this.dragDrop();
    if (!this.className)
        this.className = 'TABLEVIEW_table';
    if (!this.startAutoNumber)
        this.startAutoNumber = '';
    if (this.startAutoNumber != '')
        this.startAutoNumber = parseInt(this.startAutoNumber);
    if (!this.skipRows)
        this.skipRows = 0;
    if (!this.iconSize)
        this.iconSize = 0;
    if(!this.newSectionColumn)
        this.newSectionColumn=0;
    if(this.checkIndeterminateTitle === undefined)
        this.checkIndeterminateTitle = "Partially selected";
    if(this.checkIndeterminateHeaderTitle === undefined)
        this.checkIndeterminateHeaderTitle = "Partially selected; click to select all";

    if(this.drag)this.drag=new dragDrop();
    if(this.dropableAttrs===undefined)this.dropableAttrs=['dragDropable'];
    if(this.dropOnAttrs===undefined)this.dropOnAttrs=this.dropableAttrs;
    if(this.order_dropableAttrs===undefined)this.order_dropableAttrs=['order_dragDropable'];
    if(this.order_dropOnAttrs===undefined)this.order_dropOnAttrs=this.order_dropableAttrs;
    if(this.parsemode === undefined)
        this.parsemode = vjTable_propCSV;
    this.checkedNodes=new Array();
    this.selectedNodes=new Array();
    this.selectedCnt=0;
    this.requestedExtension=new Object();

    this.selectCategory=-1;
    this.rowsColored=new Array();
    this.colsColored=new Array();
    this.prvCategSelCol=-1;
    this.ColColor = new Array();
    this.rowsTotalSelected = 0;
    this.colsTotalSelected = 0;
    this.cellsTotalSelected = 0;
    this.tqsObj=new Array();

    if (!this.predefinedCategories)
        this.predefinedCategories = new Array();

    this.arrayOfCategories=new Array();

    this.exclusionObjRegex = verarr(this.exclusionObjRegex);
    if (this.exclusionObjRegex.length == 0) {
        this.exclusionObjRegex.push({
            info : /^info*/
        });
        this.exclusionObjRegex.push({
            id : /^info*/
        });
    }
    if (!this.geometry)
        this.geometry = {
            width : '100%'
        };

    this.setNodeExpandState = function (node) {
        if(this.expandarray && this.expandarray[node.treenode.path]!==undefined){
            node.treenode.expanded=this.expandarray[node.treenode.path];
        }
        else{
            node.treenode.expanded=(( node.treenode.depth <= this.autoexpand ) || this.autoexpand == 'all');
        }
    };
    // _/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Table viewer constructors
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/

    this.resizeBody = function(argsArr) {
        var html_el = argsArr[0];
        var index = html_el.cellIndex;
        var size = parseInt(html_el.style.width);
        this.resizeColumnBody(index, size);
    };
    this.composerFunction = function(viewer, content, page_request) {

        if (vjDS[this.data[0]].qpbg_params) this.metadata_reqId = vjDS[this.data[0]].qpbg_params.RQ.dataID;

        // use only data[0] for session id checks - matters for table viewers using additional non-tabular data sources (e.g. sharing_tmplt)
        var t_sessionID = this.getDataSessionID(0);
        //if (!(this.data && t_sessionID && t_sessionID == this.contentID && this.tblArr)) {
            this.tblArr = new vjTable(content, 0, this.parsemode);
            this.tblArr = new vjTable(content, 0, this.parsemode);
            if (!(this.parsemode & vjTable_hasHeader)) {
                var numCols = 0;
                for (var ir=0; ir<this.tblArr.rows.length; ir++)
                    numCols = Math.max(numCols, this.tblArr.rows[ir].cols.length);
                this.tblArr.hdr = [];
                for (var ic=0; ic<numCols; ic++)
                    this.tblArr.hdr.push({name: ""});
                for (var ir=0; ir<this.tblArr.rows.length; ir++)
                    if (this.tblArr.rows[ir].cols.length)
                        this.tblArr.rows[ir][""] = this.tblArr.rows[ir].cols[0];
            }
            this.contentID = t_sessionID;

            this.tblArr.mangleRows(this.exclusionObjRegex, "delete");
            if (this.maxRowsToAvoidRendering) {
                if (this.dim() > this.maxRowsToAvoidRendering) {
                    this.div.className = 'sectHid';
                    return;
                } else
                    this.div.className = 'sectVis';
            }

            if (this.inclusionObjRegex)
                this.tblArr.mangleRows(this.inclusionObjRegex, "delete", true);

            // append additional columns
            var appendCols = verarr(this.appendCols);
            for (var ic = 0; ic < appendCols.length; ++ic) {
                if (!this.appendCols[ic].header || !this.appendCols[ic].header.name)
                    alert("DEVELOPER ALERT: vjTableViewX2: invalid header field in appendCols");

                this.tblArr.hdr.push(this.appendCols[ic].header);
                for (var ir = 0; ir < this.dim(); ++ir) {
                    var vv = this.appendCols[ic].cell;
                    if (!vv)
                        continue;

                    if (vv.indexOf("eval:") == 0) {
                        node = this.row(ir);
                        vv = eval(vv.substring(5));
                        if (!vv)
                            continue;
                    }
                    this.row(ir)[this.appendCols[ic].header.name] = vv;
                    this.row(ir).cols[this.tblArr.hdr.length - 1] = vv;
                }
            }

            if (this.precompute) {
                this.enumerate(this.precompute, this);
            }
            if ( this.preEditTable ) {
                funcLink(this.preEditTable,this);
            }

            if (this.treeColumn) {
                this.tree = new vjTree(this.tblArr, this.treeColumn, 0, 1);
                var params = {
                    order : 1,
                    tbl : this.tblArr
                };
                this.tree.enumerate("node.order=params.order;++params.order;node.reqID=''+node.order;", params);
                this.enumerate("if(node.treenode)node.order=node.treenode.order;params.setNodeExpandState.call(params,node);", this);

                this.tblArr.sortTbl(0,false,this.sortTreeColumnFunc);
                if (this.treePrecompute)
                    this.tree.enumerate(this.treePrecompute, this);
            }

            // apply styles to rows
            // perform column specific configuration
            if (this.cols)
                this.tblArr.customizeColumns(this.cols);

            this.sortIndex = new Array();
            // turning unstable sort to stable by always assigning order
            var maxOrder = 0;
            for (var is = 0; is < this.tblArr.hdr.length; ++is) {
                if (this.tblArr.hdr[is].order) {
                    if (maxOrder < this.tblArr.hdr[is].order) {
                        maxOrder = this.tblArr.hdr[is].order;
                    }
                }
            }
            for (var is = 0; is < this.tblArr.hdr.length; ++is) {
                this.sortIndex
                        .push({
                            idx : is,
                            order : (this.tblArr.hdr[is].order ? this.tblArr.hdr[is].order
                                    : maxOrder + is),
                            name : this.tblArr.hdr[is].name
                        });
            }

            this.sortIndex.sort(function(a, b) {
                return (a.order - b.order);
            });

            if (this.rows)
                this.tblArr.customizeRows(this.rows);

            if (this.postcompute)
                this.enumerate(this.postcompute, this);
            if ( this.postEditTable ) {
                funcLink(this.postEditTable,this);
            }

            if (!this.dim())
                this.noContent = true;
            else if (this.noContent)
                this.noContent = false;
        //}

        /* // old color logic?
        for (var i = 0; i < this.tblArr.hdr.length; i++)
        {
            if (this.tblArr.hdr[i].name.indexOf("_")==0)
            {
                var tmp = new Array();

                var hdrName = this.tblArr.hdr[i].name;
                for (var ir = 0 ; ir < this.dim(); ir++)
                {
                    if (this.tblArr.rows[ir][hdrName] == "1")
                        tmp.push (ir);
                }

                var a = hdrName.indexOf("_",2);
                var b = hdrName.indexOf("_",a+1);
                var idx = hdrName.substr(a+1, b-a-1);
                var colName = hdrName.substr(b+1);//figure out column number here, put in the correct slot

                var ir;
                for (ir = 0; ir < this.ColColor.length; ir ++)
                {
                    if (this.ColColor[ir].index == idx)
                    {
                        this.ColColor[ir].col = colName;
                        this.ColColor[ir].array = tmp;
                    }
                }
                this.tblArr.hdr.splice(i, 1);
                i--;
            }
        }*/

        this.tblArr.syncCols();
        this.clearColors();
        
        for (var i = 0; i < this.ColColor.length; i++)
        {
            var extracted = this.ColColor[i];
            for (var ir = 0 ; ir < extracted.array.length; ir++)
            {
                this.colorRow(extracted.array[ir], 0, extracted.index);
                var  toPush = {op:"rowcategory", panel:"always", categ: extracted.index, arg: {method:"inrowlist", value: extracted.array[ir], name: ""+extracted.index+"_"+extracted.array[ir]}};
                this.tqsObj.push(toPush);
            }
        }

        this.refresh();
    };

    this.refresh = function()
    {
        if (!this.tblArr || !this.tblArr.rows) return;
        this.div.innerHTML = this.generateTableViewText(this.tblArr)+"<div id='"+this.container+"-controls'></div>";
        this.updateCheckmarks();
        if(this.isStickyHeader){
            this._reHeadered = false;
            this.stickyHeader();
        }

        //this.addColorCategorizer(null, true);
        
        if(this.drag)this.setDnDobjects();
    };

    this.findByDOM=function(obj){
        var node=new Array();
        var isArray=true;
        if(!(obj instanceof Array)){
            obj=verarr(obj);
            isArray=false;
        }
        for(var io=0 ; io<obj.length ; ++io){
            if(obj[io].id.indexOf(this.container)>=0){
                var sep="_";
                var icord=obj[io].id.split(sep);
                var sepcnt=this.container.match(new RegExp(sep,"g")).length;
                var ir=icord[sepcnt+1];
                var ic=icord[sepcnt+2];
                if(ir!=undefined && ic!=undefined)
                    node.push({value:this.row(ir).cols[ic],ir:ir,ic:ic});
            }
        }
        if(!node.length)node=null;
        if(!isArray && node)node=node[0];
        return node;
    };

    this.stickyHeader=function(){
        var tbl_Div=gObject(this.container+"_table_div");if(!tbl_Div)return;
        var tbl=gObject(this.container+"_table");if(!tbl)return;
        var tbl_Hdr=gObject(this.container + "_table_header");if(!tbl_Hdr)return;
        var tbl_Hdr_Div=gObject(this.container + "_table_header_div");if(!tbl_Hdr_Div)return;
        var divCont=tbl_Div.parentElement;
        var thead = tbl.getElementsByTagName('thead')[0];
        if(!thead)return ;
        var tbody = tbl.getElementsByTagName('tbody')[0];
        var tr = tbody.getElementsByTagName('tr')[0];
        if(!tr)return;
        var ths = tbl.getElementsByTagName('th');
        var tds = tr.children;

        var ttHeight= __getCurrentComputedStyle(divCont, 'max-height');
        if(!Int(ttHeight))return;
        var tblHeight= __getCurrentComputedStyle(tbl, 'height');
        var isVertBar=parseInt(ttHeight)>=parseInt(tblHeight)?false:true;
        if(!isVertBar)return; //no need to create sticky headers
        var ttWidth= __getCurrentComputedStyle(divCont, 'width');
        var tblWidth= __getCurrentComputedStyle(tbl, 'width');
        var isHorizBar=parseInt(ttWidth)>=parseInt(tblWidth)?false:true;

        var thHeight=__getCurrentComputedStyle(thead, 'height');
        var tbl_Width = parseInt(__getCurrentComputedStyle(tbl, 'width'))
                        +"px";
        var tbl_Div_Width = parseInt(__getCurrentComputedStyle(tbl_Div, 'width'))
                            + "px";

        if(!isok(ths) || !isok(tds))return ;
        if(ths.length > 0) {
            for(var i=0; i < ths.length; i++) {
                if(!isok(tds[i])) continue;
                var border=parseFloat(__getCurrentComputedStyle(ths[i], 'border-right-width'))+parseFloat(__getCurrentComputedStyle(ths[i], 'border-left-width'))
                -parseFloat(__getCurrentComputedStyle(tds[i], 'border-right-width'))-parseFloat(__getCurrentComputedStyle(tds[i], 'border-left-width'));
                var t_td_width=__getCurrentComputedStyle(tds[i], 'width');
                if(this.icon_ColumnIndex==i && parseFloat(t_td_width)<this.iconSize)t_td_width=this.iconSize +"px";
                ths[i].style.width =parseFloat(t_td_width)-border+"px";
                tds[i].style.width = parseFloat(t_td_width)+"px";//__getCurrentComputedStyle(tds[i], 'width');


            }
        }
        var barWidth=__gScrollBarWidth();
        tbl.style.width=tbl_Width;
        tbl_Hdr.style.width=tbl_Width;
        tbl_Div.style.width=tbl_Div_Width;
        var a=parseInt(ttHeight)-parseInt(thHeight)- (isHorizBar?barWidth:0)+"px";
        tbl_Div.style.maxHeight = a;
        tbl_Hdr_Div.style.width=parseInt(tbl_Div_Width)-(isVertBar?barWidth:0) + "px";

        tbl.removeChild(thead);
        tbl.style['table-layout']='fixed';
        tbl_Hdr.style['table-layout']='fixed';
        tbl_Hdr.appendChild(thead);
        divCont.style.overflow='';
        this._reHeadered = true;
      };

      this.resizeColumnBody = function (elIndex,size) {
          if(!this._reHeadered)return;
          var tbl=gObject(this.container+"_table");if(!tbl)return;
          var tbody = tbl.getElementsByTagName('tbody')[0];
          var tr = tbody.getElementsByTagName('tr')[0];
          if(!tr)return;
          var tds = tr.children;
          tds[elIndex].style.width = size+"px";
      };

      this.generateTableViewText = function(tbl) {
        var prfx = this.container + "_check_";

        var tt = "";
        if (this.prefixHTML && this.prefixHTML.length) {
            tt += this.prefixHTML;
        }

        var cls = "class='" + this.className + "'";
        var clsCell = "class='" + this.className + "_cell'";
        if(!this.isNheader && this.isStickyHeader){//} && !__isIE) {

            tt += "<div id='"+this.container+"_table_header_div' style='overflow-x:hidden;' ";
            tt+=" >"
            +"<table id='"+this.container+"_table_header' "
            + cls
            + " "
            + (this.geometry && this.geometry.width ? "width='"
                    + this.geometry.width + "'" : "")
            + " "
            + (this.geometry && this.geometry.height ? "height='"
                    + this.geometry.height + "'" : "") + " style='overflow:hidden;";
            if(__isIE)
                tt+="min-height:0%;";
            tt+="'></table></div>";



            tt+="<div id='"+this.container+"_table_div' style='overflow:auto";
            if(__isIE)tt+=";min-height:0%";
            tt+="' onscroll='var ht=gObject(\""+this.container+"_table_header_div\");ht.scrollLeft=this.scrollLeft;'>";
        }
        tt += "<table id='"+this.container+"_table'"//+(this.isStickyHead?"_header' style='position:absolute'":"'")+" "
                + cls
                + " "
                + (this.geometry && this.geometry.width ? "width='"
                        + this.geometry.width + "'" : "")
                + " "
                + (this.geometry && this.geometry.height ? "height='"
                        + this.geometry.height + "'" : "") + " >";
        // -----------------------------
        // Headers
        if (!this.isNheader) {
        tt+="<thead>";


        tt += "<tr>";
        if (this.dim() > 0) {
            if (this.checkable) {
                tt += "<th align=center ";
                tt += " id='" + this.container + "_header_checkbox' ";
                tt += " width='1' ";
                tt += ">";
                tt += "<input name='"
                        + prfx
                        + "all' type=checkbox onClick='vjObjEvent(\"onCheckmarkSelected\", \""
                        + this.objCls + "\",this,-1);' >";
                tt += "</th>";
            }
            if (this.iconSize) {
                tt += "<th ";
                tt += " id='" + this.container + "_header_icon' ";
                tt += " onMouseEnter='gShowResizer(\""+this.container + "_header_icon\",null,\"r\",\"javascript:vjObj."+this.objCls+".resizeBody(args)\");' ";
                tt += " ></th>";
            }
            if (this.startAutoNumber != '') {
                tt += "<th ";
                tt += " id='" + this.container + "_header_autonumber' ";
                tt += " onMouseEnter='gShowResizer(\""+this.container + "_header__header_autonumber\",null,\"r\",\"javascript:vjObj."+this.objCls+".resizeBody(args)\");' ";
                tt += " ><small>#</small></th>";
            }
            for ( var ic = 0; ic < tbl.hdr.length; ++ic) {
                var col = tbl.hdr[isok(this.sortIndex) ? this.sortIndex[ic].idx : ic];
                if (col.hidden)
                    continue;
                tt += "<th ";
                if(!this.columnsCanWrap)tt+= " style='white-space:nowrap;' ";
                tt += " id='" + this.container + "_header_" + ic + "' ";
                tt += " onClick='vjObjEvent(\"onClickCell\", \"" + this.objCls + "\",-1," + ic + ");' ";
                tt += " onMouseEnter='gShowResizer(\""+this.container + "_header_" + ic +"\",null,\"r\",\"javascript:vjObj."+this.objCls+".resizeBody(args)\");' ";
                if (col.align)
                    tt += " align=" + col.align + " ";
                if(!this.isNSortEnabled && !col.isNSortEnabled){
                    tt += " onMouseOver='vjObjEvent(\"onMouseOver\", \""
                        + this.objCls + "\",\"header\"," + ic + ",event);' ";
                    tt += " onMouseOut='vjObjEvent(\"onMouseOut\", \""
                            + this.objCls + "\",\"header\"," + ic + ",event);' ";
                }

                tt += ">";
                var colval = (col.title ? col.title : col.name);
                if (this.dataTypeAppliesToHeaders){
                    colval = this.formDataValue(colval, col.type, {}, col);
                }
                if (typeof(colval) == "string" && !colval.length) {
                    colval = "&nbsp;";
                }

                var withSortIcon=false;
                if(!this.isNSortEnabled && !col.isNSortEnabled){
                    var imgsrc="img/arrow_sort_"+(col.isNdesc?"up":"down")+ (!isok(col.sorted)?'':'_highlighted') +".gif";
                    //tt+="<table><tr><td>";
                    tt+="<img height='12' id='"+this.container + "_header_" + ic+"_sorter' src='"+imgsrc+"'";
                    tt += " onClick='vjObjEvent(\"onHeaderSort\", \""
                        + this.objCls + "\","+ic+");' ";
                    tt+=" class='"+(!isok(col.sorted)?"sectHid":"sectVis")+"' >";
                }

                tt += colval;

                if(withSortIcon) {
                    tt+="</td></tr></table>";
                }
                tt += "</th>";


            }
        } else {
            tt += "<th align=center ";
            tt += " id='" + this.container + "_header' ";
            tt += " width='100%' ";
            tt += ">";
            tt += this.defaultEmptyText ? this.defaultEmptyText
                    : "nothing to show";
            tt += "</th>";
        }
        tt += "</tr>";
        tt+="</thead>";
          }

        // -----------------------------
        // Body
         tt+="<tbody>";
        this.checkedCnt = 0;
        var allRowsCount=this.dim();
        for ( var ir = this.skipRows; ir < allRowsCount ; ++ir) {

            var row=this.row(ir,true);

            if(!row) {
                continue;
            }

            if(row.treenode && row.treenode.parent && !(row.treenode.parent.expanded) ) {
                row.treenode.expanded=0;
                continue;
            }
            if (row.hidden)
                continue;

            var cellColorer;
            var cc_drawn=0;
            var tag = "td";
            var isNewSection = false;
            if (isok(this.newSectionWord)
                    && row.cols[this.newSectionColumn].indexOf(this.newSectionWord) == 0) {

                tag = "th";
                isNewSection = true;
            }

            tt += "<tr id='" + this.container + "_" + ir + "' >";

            var spRow = 0;
            if (this.rowSpan)
                spRow = (ir - this.skipRows) % this.rowSpan;

            if (this.checkable && row.styleHasCheckmark != -1) {

                cellColorer = this.checkColors ? this.checkColors
                        : this.bgColors;

                tt += "<" + tag + " align=center ";
                if (cellColorer)
                    tt += "bgcolor=" + cellColorer[ir % cellColorer.length];
                tt += " width='1' ";
                tt += " id='" + this.container + "_" + ir + "_checkbox' ";

                tt += " >";

                if (this.rowSpan && spRow != Math.round(this.rowSpan / 2) - 1)
                    row.styleNoCheckmark = true;

                if (!row.styleNoCheckmark) {
                    tt += "<input name='" + (prfx + (ir)) + "' type=checkbox ";
                    if (row.checked > 0) {
                        tt += "checked ";
                        ++this.checkedCnt;
                    }
                    if (!this.checkDisabled)
                        tt += " onClick='vjObjEvent(\"onCheckmarkSelected\", \""
                                + this.objCls + "\",this," + ir + ");' ";
                    if (this.checkDisabled)
                        tt += " disabled ";
                    tt += " >";
                }
                tt += "</" + tag + " >";
                ++cc_drawn;
            }

            var icon = row.icon;
            if (!isok(icon))
                icon = this.defaultIcon;
            if (this.iconSize) {
                cellColorer = this.iconColors ? this.iconColors : this.bgColors;
                var width =this.iconWidth===undefined? "": (" width='"+this.iconWidth+"' ");

                tt += "<" + tag;
                if (!isNewSection) {
                    tt += " height='" + this.iconSize + "'" + width;
                    if (cellColorer)
                        tt += " bgcolor=" + cellColorer[ir % cellColorer.length];
                }

                tt += " id='" + this.container + "_" + ir + "_icon' ";

                if (!isNewSection) {
                    tt += " onClick='vjObjEvent(\"onClickCell\", \"" + this.objCls
                        + "\"," + ir + ",-1);' style='cursor:pointer;' ";
                }

                tt += " >";
                if (icon && !isNewSection)
                    tt += this.formDataValue(icon, "icon", row, null); 
                tt += "</" + tag + ">";
                this.icon_ColumnIndex = cc_drawn;
            }

            if (this.startAutoNumber != '') {
                tt += "<" + tag;
                tt += " id='" + this.container + "_" + ir + "_autonumber' ";
                tt += " ><small>" + (this.startAutoNumber + ir - this.skipRows)
                        + "</small></" + tag + ">";
            }
            var maxCols = row.cols.length;
            if (tbl.hdr.length > maxCols)
                maxCols = tbl.hdr.length;
            if(maxCols>tbl.hdr.length)
                maxCols = tbl.hdr.length;
            for ( var iCC = 0; iCC < maxCols; ++iCC) {

                ic = isok(this.sortIndex) ? this.sortIndex[iCC].idx : iCC;

                var col = tbl.hdr[ic];

                if(!cellColorer)
                    cellColorer = this["col" + ic + "Colors"];
                if (!cellColorer && ic < tbl.hdr.length)
                    cellColorer = this["col" + tbl.hdr[ic].name + "Colors"];
                if (!cellColorer)
                    cellColorer = this.bgColors;

                if (tbl.hdr[ic].hidden)
                    continue;

                var realText = ic < row.cols.length ? row.cols[ic] : null;

                if (!realText)
                    realText = "";
                if (realText instanceof Array)
                    realText = realText.join(",");
                var tooltip = this.getToolTip(row, ic, this.formDataValue(realText, col.type, row, col, {ellipsize: false, textonly: true}));
                var dataValueText = this.formDataValue(realText, col.type, row, col);

                tt += "<"
                        + tag
                        + " "
                        + (row.bgcolor ? "bgcolor='" + row.bgcolor+"'" : (cellColorer ? "bgcolor=" + cellColorer[ir % cellColorer.length] : ""))
                        + " " + (col.align ? "align=" + col.align : "") + " ";

                tt += " id='" + this.container + "_" + ir + "_" + ic + "' ";
                tt += row.selected ? (" class='" + this.className + "_selected"
                        + row.selected + "'") : clsCell;

                tt += " onClick='vjObjEvent(\"onClickCell\", \"" + this.objCls
                        + "\"," + ir + "," + ic
                        + ");' style='cursor:pointer;' ";
                tt += " onMouseOver='vjObjEvent(\"onMouseOver\", \""
                        + this.objCls + "\"," + ir + "," + ic + ");' ";
                tt += " onMouseOut='vjObjEvent(\"onMouseOut\", \""
                        + this.objCls + "\"," + ir + "," + ic + ");' ";

                tt += " title='" + tooltip;
                tt += "' ";
                tt += " >";

                if (!col.wrap)
                    tt += "<span style='white-space:nowrap;' >";

                tt += dataValueText;

                if (!col.wrap)
                    tt += "</span>";
                tt += "</" + tag + ">";
            }

            tt += "</tr>";
        }
         tt+="</tbody>";
        tt += "</table>";
        tt += "</div>";

        if (this.appendHTML && this.appendHTML.length) {
            tt += this.appendHTML;
        }


        return tt;
    };

    this.getToolTip = function (node, ic,defaultVal) {
        var col = this.tblArr.hdr[ic];
        var tooltip = "";
        if( this.tooltip ) {
            tooltip=funcLink(this.tooltip,this,node,ic);
        }
        else if ( col && col.tooltip ) {
            tooltip=funcLink(col.tooltip,this,node,ic);
        }
        else
            tooltip = textContent(defaultVal);
        return sanitizeElementAttr(tooltip);
    };

    this.row = function ( ir , givetreenode ) {
        return this.tblArr.rows[ir];
    };

    this.dim = function() {
        if (!this.tblArr) return;
        return this.tblArr.rows.length;
    };

    this.enumerate = function(operation, params) {
        if (!this.tblArr || this.tblArr.rows.length == 0)
            return;
        return this.tblArr.enumerate(operation, params);
    };

    this.accumulate = function(checker, collector, params, checked) {
        return this.tblArr.accumulate(checker, collector, params, checked);
    };


    this.doSelect = function(ir, renderAll) {
        if (ir < 0)
            return false;
        this.selectedCnt = 0;
        this.selectedNodes = [];
        var spanMax = this.rowSpan ? this.rowSpan : 1;

        var oldShift = ir;
        var onSelected = false;
        if (!this.multiSelect)  // || !gKeyShift
            this.oldShift = undefined;
        if(this.multiSelect ){
            if (this.oldShift != undefined && gKeyShift){
                oldShift = this.oldShift;
                if(oldShift>ir){
                    oldShift=ir;
                    ir=this.oldShift;
                }
            }
            else
                this.oldShift = ir;
        }

        if (renderAll) {
            for ( var ivr = 0; ivr < this.dim(); ++ivr) {
                if (((!this.multiSelect || !gKeyShift) && (ir >= ivr && ir < ivr
                        + spanMax))
                        || (this.multiSelect && gKeyShift)
                        && (ivr >= oldShift && ivr < ir + spanMax)) {
                    onSelected = true;
                } else
                    onSelected = false;
                var crivr = ivr;
                for (ivr; ivr < crivr + spanMax; ++ivr) {
                    var node = this.row(ivr);
                    if (onSelected) {
                        if(gKeyShift)
                            node.selected=1;
                        else
                            node.selected ^= 1;
                    } else if (!this.multiSelect || !gKeyCtrl){
                        node.selected = 0;
                    }
                    if (node.selected) {
                        ++this.selectedCnt;
                        this.selectedNodes.push(node);
                    }
                    var maxCols = node.cols.length;
                    if (this.tblArr.hdr.length > maxCols)
                        maxCols = this.tblArr.hdr.length;

                    for ( var ik = 0; ik < maxCols; ++ik) {

                        if (ik >= this.tblArr.hdr.length
                                || this.tblArr.hdr[ik].hidden)
                            continue;
                        var o = gObject(this.container + "_" + ivr + "_" + ik);
                        if(!o)continue;
                        if ((!node.selected && (o.className.indexOf('cell') >= 0 || o.className.indexOf('selected0') >= 0))
                                || (node.selected && o.className.indexOf('selected1') >= 0))
                            break;
                        else if (node.selected)
                            o.className = this.className + "_selected"+node.selected;
                        else
                            o.className = this.className + "_cell";
                    }
                }
                --ivr;
            }
        }
        return this.selectedCnt;
    };

    // _/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Table viewer Event Handlers
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/

    this.formCheckSwitchTriState = function(checknam, state, indet_checked_state, indet_title) {
        var o = gObject(checknam);
        if (!state || state > 0) {
            o.checked = state;
            o.indeterminate = false;
            o.removeAttribute("title");
        } else {
            o.checked = indet_checked_state;
            o.indeterminate = true;
            if (indet_title) {
                o.title = indet_title;
            } else {
                o.removeAttribute("title");
            }
        }
    };

    this.updateCheckmarks = function(cursel, ischecked) {
        var prfx = this.container + "_check_";

        var cntchk = 0;

        for (var is = 0; is < this.dim(); ++is) {
            var o = gObject(prfx + is);
            if (!o)
                continue;

            if (cursel === null || cursel === undefined) {
                this.formCheckSwitchTriState(prfx + is, this.row(is).checked, true, this.checkIndeterminateTitle);
            } else if (cursel == -1) {
                this.formCheckSwitchTriState(prfx + is, ischecked, true, this.checkIndeterminateTitle);
            }

            if (o.checked) {
                ++cntchk;
            }
        }
        // alert(cntchk + "=="+this.dim()+"///" + cursel )

        if (gObject(prfx + "all")) {
            if (cursel == -1) {
                this.formCheckSwitchTriState(prfx + "all",
                        (ischecked ? true : false), false,
                        this.checkIndeterminateHeaderTitle);
            } else {
                // elems[prfx+"all"].checked=(cntchk==this.dim()-1);
                this.formCheckSwitchTriState(prfx + "all",
                        cntchk == 0 ? 0
                                : (cntchk == this.dim() ? 1 : -1), false,
                                this.checkIndeterminateHeaderTitle);
            }
        }

        if (cursel !== null && cursel !== undefined && cursel != -1)
            this.formCheckSwitchTriState(prfx + cursel,
                    (ischecked ? true : false), true,
                    this.checkIndeterminateTitle);
    };

    this.onCheckmarkSelected = function(container, thisobject, cursel) {
        var elems = this.formObject.elements;
        var prfx = this.container + "_check_";

        var ischecked = elems[prfx + (cursel == -1 ? "all" : cursel)].checked;

        this.updateCheckmarks(cursel, ischecked);

        this.checkedCnt = 0;
        this.checkedNodes=[];
        for ( var is = 0; is < this.dim(); ++is) {
            if (!elems[prfx + is])
                this.row(is).checked = false;
            else
                this.row(is).checked = elems[prfx + is].checked ? true
                        : false;
            if (this.row(is).checked){
                ++this.checkedCnt;
                this.checkedNodes.push(this.row(is));
            }
        }

        var node = (cursel != -1) ? this.row(cursel) : null;
        if (this.checkCallback) {
            if (this.rowSpan) {
                var firstSpRow = cursel
                        - ((cursel - this.skipRows) % this.rowSpan);
                node = (firstSpRow != -1) ? this.row(firstSpRow)
                        : null;
                funcLink(this.checkCallback, this, node, firstSpRow, -1);
            } else
                funcLink(this.checkCallback, this, node, cursel, -1);
        }

        this.updateDependents();
    };

    this.mimicCheckmarkSelected = function(ir, checked)
    {
        if (!this.tblArr || ir>=this.dim())
            return;
        var row = this.row(ir);
        //alerJ("row in vjTableView",row)
        if (!this.checkable || row.styleHasCheckmark === -1 || row.styleNoCheckmark)
            return;

        var prfx = this.container + "_check_";
        this.formCheckSwitchTriState(prfx + ir, checked, ir >= 0, ir >= 0 ? this.checkIndeterminateTitle : this.checkIndeterminateHeaderTitle);
        return this.onCheckmarkSelected(this.container, gObject(prfx + ir), ir);
    };

    // ##########
    this.colorCell=function(ir, ic,color)
    {
        var node = this.row(ir);
        
        if (color == -1)
            color = "white";

        var cell_cords = "" + ir + "-" + ic;
        var isExisted = this.cellsColored.indexOf(cell_cords);
        if ( isExisted != -1) // if existed => discolor the cell
        {
            var cell_element = gObject(this.container + "_" + ir + "_" + ic);
            if(!cell_element)
                return;
            this.cellsTotalSelected--;
            this.cellsColored.splice(isExisted, 1);
            cell_element.bgColor="white";
            var extracolors = [];
            if (this.rowsColored[ir] != undefined && this.rowsColored[ir].array != undefined)
                extracolors=extracolors.concat(this.rowsColored[ir].array);

            if(this.colsColored[ic] != undefined && this.colsColored[ic].array !=undefined)
            {
               var toColor=extracolors.concat(this.colsColored[ic].array);
               this.makeStripedBackground(cell_element, toColor);
            }
            return;
        }
        
        // not existed => color cell
        this.cellsColored.push(cell_cords);
        this.cellsTotalSelected++;
        
        var cell_toColor = gObject(this.container + "_" + ir + "_" + ic);
        if(!cell_toColor)
            return;
        var extracolors=new Array();

        if(this.colsColored[ic] !=undefined && this.colsColored[ic].array !=undefined)
        {
            extracolors=this.colsColored[ic].array;
        }
        
        if (this.rowsColored[ir] !=undefined  && this.rowsColored[ir].array != undefined ){
            extracolors = extracolors.concat(this.rowsColored[ir].array);
        }
        if (extracolors.length){
            this.makeStripedBackground(cell_toColor, extracolors);
            madeStriped ++;
        }
        else {
            cell_toColor.bgColor= color;
        }

    };
 
    this.colorRow=function(ir, ic,color) {
        var node = this.row(ir);
        
        if (color == -1) color = "white";
        if (!this.rowsColored[ir]) this.rowsColored[ir]=({array:new Array()});
        
        
        if (this.rowsColored[ir].array.indexOf(color) > -1)
        {
            this.rowsColored[ir].array.pop(color);
            this.rowsTotalSelected--;
            if (this.rowsColored[ir].array.length == 0)
                this.rowsColored[ir].array=new Array();
        }
        else{
            this.rowsColored[ir].array.push(color);
            this.rowsTotalSelected++;
        }
        
        var maxCols = node.cols.length;
        if (this.tblArr.hdr.length > maxCols)
            maxCols = this.tblArr.hdr.length;

        for ( var ik = 0; ik < maxCols; ++ik) {
            if (ik >= this.tblArr.hdr.length || this.tblArr.hdr[ik].hidden) continue;

            var o = gObject(this.container + "_" + ir + "_" + ik);

            if(!o) continue;
            o.style.backgroundImage = "";
            
            var extracolors;
            if (!this.rowsColored[ir])
                extracolors=[];
            else
                extracolors=this.rowsColored[ir].array;
            
            if (extracolors.length == 0){
                o.bgColor = "white";
            }else if (extracolors.length == 1){
                o.bgColor = extracolors[0];
            }else{
                this.makeStripedBackground(o, extracolors);
            }

            if(this.colsColored[ik])
            {
               var toColor=extracolors.concat(this.colsColored[ik].array);
               this.makeStripedBackground(o, toColor);
            }
        }

        return;

    };

    this.colorCol=function(ir, ic, color)
    {
        var node = this.tblArr.hdr[ic];

        if (color == -1) color = "white";
        if (!this.colsColored[ic]) this.colsColored[ic]=({array:new Array()});
        if (this.colsColored[ic].array.indexOf(color) > -1)
        {
            this.colsColored[ic].array.pop(color);
            this.colsTotalSelected--;
            if (this.colsColored[ic].array.length == 0)
                this.colsColored[ic].array=new Array();
        }
        else{
             this.colsColored[ic].array.push(color);
             this.colsTotalSelected++;
        }
        
        for (var r=0; r < this.dim(); r++){
            var o = gObject(this.container + "_" + r + "_" + ic);             

             if(!o) continue;
             o.style.backgroundImage = "";

             var extracolors;
             if (!this.colsColored[ic])
                 extracolors=[];
               else
                 extracolors=this.colsColored[ic].array;

             if (extracolors.length == 0){
                 o.bgColor = "white";
             }else if (extracolors.length == 1){
                 o.bgColor = extracolors[0];
             }else {
                 this.makeStripedBackground (o, extracolors);
             }             
             
             if(this.rowsColored[r]) {
                var toColor=extracolors.concat(this.rowsColored[r].array);
                 this.makeStripedBackground(o, toColor);
             }
        }
    };

    this.addFuncCall = function (name, params) {
        this.functionCalls.push ({fnName: name, fnparams:params, fnPanel: this.actualBigPanel.currentPlugin});
    };

    this.findInTqs = function (where, lookFor) {
        var toReturn = where.slice();
        
        for (var k = 0; k < lookFor.length; k++) {
            for (var i = 0; i < toReturn.length; i++) {
                if (toReturn[i][lookFor[k][0]] != lookFor[k][1]) {
                    toReturn.splice(i,1);
                    i--;
                }
            }
        }
        
        return toReturn;
    };
    
    this.findInTqsPos = function (where, lookFor) {
        var toReturn = [];
        
        for (var k = 0; k < lookFor.length; k++) {
            for (var i = 0; i < where.length; i++) {
                if (where[i][lookFor[k][0]] == lookFor[k][1] && toReturn.indexOf(i) < 0)
                    toReturn.push(i);
                else if (where[i][lookFor[k][0]] != lookFor[k][1] && toReturn.indexOf(i) >= 0)
                    toReturn.splice(toReturn.indexOf(i), 1);
            }
        }
        
        return toReturn;
    };

    this.onClickCell = function(container, ir, ic){

        if(gDragEvent){
            gDragEvent=false;
            return ;
        }

        var node;

        if(ir>=0) {

            node = this.row(ir);
            
            if (this.actualBigPanel && this.actualBigPanel.currentColor && this.actualBigPanel.currentPlugin && this.actualBigPanel.currentSelector && this.actualBigPanel.currentType)
            {
                if(this.actualBigPanel.currentType == "color row" )
                {
                    var startRow=ir,endRow=ir;
                    if(this.multiSelect && this.prvCategSelRow>=0 && gKeyShift ) {
                        startRow = this.prvCategSelRow>ir ? ir : this.prvCategSelRow;
                        endRow=this.prvCategSelRow>ir ? this.prvCategSelRow : ir;
    
                    }
    
                    for( var isr=startRow; isr<=endRow; ++isr) {
                        if(isr==this.prvCategSelRow && this.multiSelect && gKeyShift)
                            continue;
                        var nodeCur=this.row(isr);
    
                        if(!nodeCur.categorizerCategory)nodeCur.categorizerCategory=new Array();
                        nodeCur.categorizerCategory[""+this.selectCategory]=true;
    
                        var rowCat = this.findInTqs(this.tqsObj, [["op","rowcategory"],["panel",this.actualBigPanel.currentPlugin], ["categ", this.actualBigPanel.currentSelector]]);
                        
                        if (rowCat.length == 0)
                            this.tqsObj.push({op:"rowcategory", panel:this.actualBigPanel.currentPlugin, categ: this.actualBigPanel.currentSelector, color: this.actualBigPanel.currentColor, arg: {method:"inrowlist", values: [isr], name: ""+this.actualBigPanel.currentSelector+"_"+this.actualBigPanel.currentColor}});
                        else
                        {
                            var posInValues = rowCat[0].arg.values.indexOf(isr); 
                            if (posInValues > -1)
                                rowCat[0].arg.values.splice(posInValues, 1);
                            else
                                rowCat[0].arg.values.push(isr);
                        }
                        
                        var position = this.findInTqs(this.tqsObj, [["op", this.actualBigPanel.currentPlugin]]);
                        if (position.length == 0)
                        {
                            position = [{op: this.actualBigPanel.currentPlugin, arg:{}}];
                            this.tqsObj.push(position[0]);
                        }
                        if (!position[0].arg[this.actualBigPanel.currentSelector])
                        {
                            var curSel = this.actualBigPanel.currentSelector;
                            position[0].arg[curSel] = new Array(); 
                        }
                        
                        var index = position[0].arg[this.actualBigPanel.currentSelector].indexOf(isr);
                        if (index > -1)
                            position[0].arg[this.actualBigPanel.currentSelector].splice(index,1);
                        else
                            position[0].arg[this.actualBigPanel.currentSelector].push(isr);
    
                        this.actualBigPanel.currentColor = this.actualBigPanel.currentColor;
                        this.colorRow(isr,ic,this.actualBigPanel.currentColor);
                    }
                    this.prvCategSelRow=ir;
                }
                else if(this.actualBigPanel.currentType == "color col" && ic<this.tblArr.hdr.length && ic>=0  )
                {
                    var startCol = ic, endCol=ic;
    
                    if (this.multiSelect && this.prvCategSelCol>=0 && gKeyShift)
                    {
                        startCol = this.prvCategSelCol>ic ? ic : this.prvCategSelCol;
                        endCol = this.prvCategSelCol>ic ? this.prvCategSelCol : ic;
                    }
    
    
                    for (var isc=startCol ; isc <= endCol; isc ++)
                    {
                        if(isc==this.prvCategSelCol && this.multiSelect && gKeyShift)
                            continue;
                        var nodeCur=this.tblArr.hdr[isc];
    
                        if(!nodeCur.categorizerCategory)nodeCur.categorizerCategory=new Array();
                        nodeCur.categorizerCategory[""+this.selectCategory]=true;
                        
                        var rowCat = this.findInTqs(this.tqsObj, [["op","colcategory"],["panel",this.actualBigPanel.currentPlugin], ["categ", this.actualBigPanel.currentSelector]]);
                        
                        if (rowCat.length == 0)
                            this.tqsObj.push({op:"colcategory", panel: this.actualBigPanel.currentPlugin, categ: this.actualBigPanel.currentSelector, color: this.actualBigPanel.currentColor, arg: {method:"incollist", values: [isc], name: ""+this.actualBigPanel.currentSelector+"_"+this.actualBigPanel.currentColor}});
                        else
                        {
                            var posInValues = rowCat[0].arg.values.indexOf(isc); 
                            if (posInValues > -1)
                                rowCat[0].arg.values.splice(posInValues, 1);
                            else
                                rowCat[0].arg.values.push(isc);
                        }
                        
                        var position = this.findInTqs(this.tqsObj, [["op", this.actualBigPanel.currentPlugin]]);
                        if (position.length == 0)
                        {
                            position = [{op: this.actualBigPanel.currentPlugin, arg:{}}];
                            this.tqsObj.push(position[0]);
                        }
                        if (!position[0].arg[this.actualBigPanel.currentSelector])
                        {
                            var curSel = this.actualBigPanel.currentSelector;
                            position[0].arg[curSel] = new Array(); 
                        }
                        
                        var index = position[0].arg[this.actualBigPanel.currentSelector].indexOf(isc);
                        if (index > -1)
                            position[0].arg[this.actualBigPanel.currentSelector].splice(index,1);
                        else
                            position[0].arg[this.actualBigPanel.currentSelector].push(isc);
    
                        this.actualBigPanel.currentColor = this.actualBigPanel.currentColor;
                        this.colorCol(ir,isc,this.actualBigPanel.currentColor);
                    }
                    this.prvCategSelCol=ic;
                }
                // ###############
                else if(this.actualBigPanel.currentType == "color cell" && ic<this.tblArr.hdr.length && ic>=0  )
                {
                    var nodeCur=this.row(ir);
                    var cell_cords = "" + ir + "-" + ic;
                    var cell_val = this.tblArr.rows[ir].cols[ic];
                    //var take_cell_cords = 0;
                    if (this.actualBigPanel.takeCellCords != undefined) {
                        cell_val = cell_cords;
                    }
                    
                    if(!nodeCur.categorizerCategory) nodeCur.categorizerCategory=new Array();
                    nodeCur.categorizerCategory[""+this.selectCategory]=true;
                 
                    var position = this.findInTqs(this.tqsObj, [["op", this.actualBigPanel.currentPlugin]]);
                    if (position.length == 0)
                    {
                        position = [{op: this.actualBigPanel.currentPlugin, arg:{}}];
                        this.tqsObj.push(position[0]);
                    }
                    if (!position[0].arg[this.actualBigPanel.currentSelector])
                    {
                        var curSel = this.actualBigPanel.currentSelector;
                        position[0].arg[curSel] = new Array(); 
                    }
                    
                    var index = position[0].arg[this.actualBigPanel.currentSelector].indexOf(cell_val); // isc 
                    if (index > -1)
                        position[0].arg[this.actualBigPanel.currentSelector].splice(index,1);
                    else
                        position[0].arg[this.actualBigPanel.currentSelector].push(cell_val); // isc

                    this.actualBigPanel.currentColor = this.actualBigPanel.currentColor;
                    this.colorCell(ir,ic,this.actualBigPanel.currentColor);
                    // check other element 
                    
                }

            } 
            if (this.rowSpan)
                ir = ir - ((ir - this.skipRows) % this.rowSpan);
            if (!this.userCannotSelect)
                this.doSelect(ir, true);

            if(this.selectInsteadOfCheck){
                this.enumerate("if(node.selected)node.checked=1;else node.checked=0;");
                this.updateDependents();
            }

        }else
            node=this.tblArr.hdr[ic];

        var func;
        func = this.selectCaptureCallback;
        if (!func && node.url&& !this.notOpenUrl)
            func = node.url;
        if (!func && ic>=0 && this.tblArr.hdr[ic].url&& !this.notOpenUrl)
            func = this.tblArr.hdr[ic].url;
        if (!func && this.selectCallback)
            func = this.selectCallback;

        funcLink(func, this, node, ir, ic, ic>=0  ? this.tblArr.hdr[ic] : ic );
    };

    this.mimicClickCell = function(ir, ic, colName) {

        if(!this.tblArr)
            return ;
        if (colName)
            ir = this.tblArr.accumulate("node." + colName + "==" + ir,
                    "node.irow")[0];
        if (this.dim() > 0)
            this.onClickCell(this.container, ir, ic);
    };

    this.order_dragCallonStart=function(dragE){
        var node=this.findByDOM(dragE);

        return funcLink(this.dragStartCallback, node);
    };
    this.order_dragCallonCancel=function(dragE){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragCancelCallback, node);
    };
    this.order_dragCallonMove=function(dragE){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragMoveCallback, node);
    };
    this.order_dragCallonTarget=function(dragE,dropE){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragTargetCallback, node);
    };
    this.order_dragCallonDrop=function(dragE,dropE,copy){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragDropCallback, node, dropE,copy);
    };
    this.order_dragCallonStop=function(dragE){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragStopCallback, node);
    };
    this.order_setClonedObject=function(dragE){
        var node=this.findByDOM(dragE);
        var clElem=null;
        clElem=funcLink(this.dragSetCloneCallback, this , dragE, node.value, Int(node.ir) , Int(node.ic), this.tblArr.hdr[node.ic]);
        return clElem;
    };


    this.dragCallonStart=function(dragE){
        var node=this.findByDOM(dragE);

        return funcLink(this.dragStartCallback, node);
    };
    this.dragCallonCancel=function(dragE){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragCancelCallback, node);
    };
    this.dragCallonMove=function(dragE){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragMoveCallback, node);
    };
    this.dragCallonTarget=function(dragE,dropE){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragTargetCallback, node);
    };
    this.dragCallonDrop=function(dragE,dropE,copy){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragDropCallback, node, dropE,copy);
    };
    this.dragCallonStop=function(dragE){
        var node=this.findByDOM(dragE);
        return funcLink(this.dragStopCallback, node);
    };
    this.setClonedObject=function(dragE){
        var node=this.findByDOM(dragE);
        var clElem=null;
        clElem=funcLink(this.dragSetCloneCallback, this , dragE, node.value, Int(node.ir) , Int(node.ic), this.tblArr.hdr[node.ic]);
        return clElem;
    };
    this.setDnDobjects = function() {
        var rowCnt = this.dim();
        var colCnt = rowCnt ? this.row(0).cols.length : 0;
        var oDnDList = new Array();

        if (!this.selectDnDObjects) {
            for ( var ir = 0; ir < rowCnt; ++ir) {
                for ( var ic = 0; ic < colCnt; ++ic) {
                    oDnDList.push( gObject(this.container + "_" + ir + "_" + ic) );
                }
            }
        }
        else{
            oDnDList=funcLink(this.selectDnDObjects);
        }

        this.drag.dropableAttrs = this.dropableAttrs;
        this.drag.dropOnAttrs = this.dropOnAttrs;
        this.drag.setClonedObject={func:this.setClonedObject,obj:this};

        for ( var io = 0; io < oDnDList.length; ++io) {
            o = oDnDList[io];
            if (!o)
                continue;
            this.drag.initDragElement(o, {
                func : this.dragCallonDrop,
                obj : this
            }, {
                func : this.dragCallonStart,
                obj : this
            }, {
                func : this.dragCallonMove,
                obj : this
            }, {
                func : this.dragCallonTarget,
                obj : this
            }, {
                func : this.dragCallonStop,
                obj : this
            }, {
                func : this.dragCallonCancel,
                obj : this
            });
            this.drag.initDropElement(o);
        }
    };
    this.setReOrderDnDobjects = function() {
        var hdrCnt = rowCnt ? this.tblArr.hdr.length : 0;
        var oDnDList = new Array();

        if (!this.selectReOrderDnDObjects) {
            for ( var ic = 0; ic < hdrCnt; ++ic) {
                oDnDList.push( gObject(this.container + "_" + ir + "_" + ic) );
            }
        }
        else{
            oDnDList=funcLink(this.selectReOrderDnDObjects);
        }

        this.order_drag.dropableAttrs = this.order_dropableAttrs;
        this.order_drag.dropOnAttrs = this.order_dropOnAttrs;
        this.order_drag.setClonedObject={func:this.setClonedObject,obj:this};

        for ( var io = 0; io < oDnDList.length; ++io) {
            o = oDnDList[io];
            if (!o)
                continue;
            this.drag.initDragElement(o, {
                func : this.order_dragCallonDrop,
                obj : this
            }, {
                func : this.order_dragCallonStart,
                obj : this
            }, {
                func : this.order_dragCallonMove,
                obj : this
            }, {
                func : this.order_dragCallonTarget,
                obj : this
            }, {
                func : this.order_dragCallonStop,
                obj : this
            }, {
                func : this.order_dragCallonCancel,
                obj : this
            });
            this.order_drag.initDropElement(o);
        }
    };


    this.onMouseMove = function(event) {
        var deb=gObject("debug");

        gX=_dragOffsetX+event.clientX-_startX;
        gY=_dragOffsetY+event.clientY-_startY ;
        deb.innerHTML="gX:"+parseInt(event.clientX)+"   --  gY:"+parseInt(event.clientY) +
        "  -- style:" +_draggable.style.position +  " --//-- element.left="+_draggable.style.left + "element.top="+_draggable.style.top ;

        deb.innerHTML+="\t\t\tTarget:"+_draggable.id;

        _draggableOldStyle=_draggable.style;

        _draggable.style.cursor = 'move';
        _draggable.style.left=parseInt(event.clientX)+"px";
        _draggable.style.top=parseInt(event.clientY)+"px";

    };

    this.onClickExpandNode=function(container, nodepath, expanded )
    {

        var treenode=this.tree.findByPath(nodepath);
        if(!treenode)return;
        treenode.expanded=treenode.expanded ? 0 : 1;
        if(!this.expandarray)this.expandarray=new Object();
        this.expandarray[treenode.path]=treenode.expanded;
        this.refresh();
        stopEventPropagation(event);
    };

    this.onMouseOver = function(container, ir, ic,event) {
        var o = gObject(this.container + "_" + ir + "_" + ic);
        if(!o)return;


        if(ir=='header'){
            if( this.enableTableSorter()){
                var sO=gObject(this.container + "_" + ir + "_" + ic+"_sorter");
                if(sO)
                    sO.className='sectVis';
            }
            return;
        }

        o.className = this.className + "_highlighted";

        if(this.mouseOverCallback){
            this.lastMouseOverCellText=o.innerHTML;
            funcLink(this.mouseOverCallback, this, this.row(ir), ir, ic, ic>=0  ? this.tblArr.hdr[ic] : ic );
        }

    };

    this.onMouseOut = function(container, ir, ic,event) {

        if(ir=='header'){
            if(this.enableTableSorter()){
                var sO=gObject(this.container + "_" + ir + "_" + ic+"_sorter");
                if(sO){
                    var pSO=gObject(this.container + "_" + ir + "_" + ic);
                    if(!eventOrigins(pSO,event))return;
                    var tric=isok(this.sortIndex) ? this.sortIndex[ic].idx : ic;
                    if(!isok(this.tblArr.hdr[tric].sorted))
                        sO.className='sectHid';
                }
            }

            return;
        }



        var node = this.row(ir);
        var maxCols = node.cols.length;
        if (this.tblArr.hdr.length > maxCols)
            maxCols = this.tblArr.hdr.length;

        for ( var ic = 0; ic < maxCols; ++ic) {
            if (ic < this.tblArr.hdr.length && this.tblArr.hdr[ic].hidden)
                continue;
            var o = gObject(this.container + "_" + ir + "_" + ic);
            if (o){
                o.className = node.selected ? this.className + "_selected"
                        + node.selected : this.className + "_cell";
            }
        }
    };


    this.onHeaderSort = function(container,ic){

        var tric=isok(this.sortIndex) ? this.sortIndex[ic].idx : ic;
        var hdcol=this.tblArr.hdr[tric];

        if(hdcol.isNdesc===undefined)
            hdcol.isNdesc=false;
        else{
            hdcol.isNdesc=~hdcol.isNdesc;
        }
        for(var ih=0;ih<this.tblArr.hdr.length;++ih){
            var thcol=this.tblArr.hdr[ih];
            if(ih==tric)
                thcol.sorted=true;
            else
                thcol.sorted=false;

        }

        this.tblArr.sortTbl(tric, hdcol.isNdesc);
        this.refresh();
    };

    this.onClickMenuNode = function(container, node, menuobject) {
        alert("requested to " + node.name);
    };

    this.enableTableSorter = function (){
        if(this.localSortDisable)
            return 0;
        var dataS=this.dataSourceEngine[this.data];
        var dataurl=dataS.url;
        var datacnt=docLocValue("cnt",0,dataurl);
        var datastart=docLocValue("start","0",dataurl);
        if(datacnt!="1000000" || datacnt==0){                //if there is no pager or 'all' (all=1000000) has been selected then enable sort
            if(dataS.dataSize!==undefined){                    //if there pager and dataSize provided from the menu
                if(dataS.dataSize>this.dim() )    //if datasize > than the displayed number of rows don't enable sorter
                    return 0;
            }
            else if( parseInt(datastart)!=0){                //if datasize not provided but start position in url is provided and
                return 0;                                        //non-zero then don't enable sorter
            }
            else if(this.dim()==parseInt(datacnt))    //Finally if start is not provided and number of the rows displayed
                return 0;                                            //are less than the counter then enable sorter.
        }
        return 1;
    };

    this.clearColors = function()
    {
        for (var ir = 0; ir < this.dim(); ir++)
        {
             for ( var ic = 0; ic < this.tblArr.hdr.length; ic ++)
             {
                 if (ic >= this.tblArr.hdr.length || this.tblArr.hdr[ic].hidden)
                     continue;

                 var o = gObject(this.container + "_" + ir + "_" + ic);

                 if(!o)
                     continue;

                 o.bgColor="white";
                 o.style.backgroundImage = null;
             }
        }

        this.rowsColored = new Array();
        this.colsColored = new Array();
        this.cellsColored = new Array();
        this.rowsTotalSelected = 0;
        this.colsTotalSelected = 0;
        this.cellsTotalSelected = 0;
    }

    this.makeStripedBackground = function(o, colors)
    {
        if (!o)
            return false;

        var stripe = 10;
        var width = stripe * colors.length;
        var svgText = '<?xml version="1.0" standalone="no"?><svg width="' + stripe * colors.length + '" height="' + stripe * colors.length + '" version="1.1" xmlns="http://www.w3.org/2000/svg">';
        for (var ic=0; ic<colors.length; ic++) {
            var x1 = ic * stripe;
            var x2 = (ic + 1) * stripe;
            svgText += '<polygon points="' + x1 + ' 0, ' + x2 + ' 0, 0 ' + x2 + ', 0 ' + x1 + '" stroke="' + colors[ic] + '" fill="' + colors[ic] + '"/>';
            svgText += '<polygon points="' + width + ' ' + x1 + ', ' + width + ' ' + x2 + ', ' + x2 + ' ' + width + ', ' + x1 + ' ' + width + '" stroke="' + colors[ic] + '" fill="' + colors[ic] + '"/>';
        }
        svgText += '</svg>';

        // Note: requires IE-10 (or any version of Firefox or Chrome)
        // non-base64 svg data uris fail in Firefox
        o.style.backgroundImage = "url('data:image/svg+xml;base64," + window.btoa(svgText) + "')";
        return true;
    };
}

function vjTableControlX2(viewer)
{
     if (this.objCls)
            return;
     
    if (viewer.container)
        this.container = viewer.container;
     vjTableViewX2.call(this, viewer);
    
    if(viewer.formObject===undefined)viewer.formObject= document.forms[viewer.formName];
    this.objCls=("cls"+Math.random()).replace(".","");
    vjObj.register(this.objCls,this);
    var objcls=""+this.objCls;
    this.predefinedCategories = verarr(viewer.predefinedCategories);
    this.additionalPanels = verarr(viewer.additionalPanels);
    var that = this;
    this.oldHeaderSelected;
    this.oldHeaderSelectedNum;
    this.tableInfo={};
    if (!viewer.graph)  this.graph = new vjGoogleGraphView();
    else this.graph = viewer.graph;
    this.vData = viewer.data;
    if(!viewer.menuUpdateCallbacks) this.menuUpdateCallbacks = [];
    else this.menuUpdateCallbacks = viewer.menuUpdateCallbacks;
    if (!viewer.noPanelOpen) this.noPanelOpen = [];
    else this.noPanelOpen=viewer.noPanelOpen;
    oldUrl = "";
    if (viewer.keyWords) this.keyWords = viewer.keyWords;
    else this.keyWords = ["*"];
    if (viewer.notToOpenArgs) this.notToOpenArgs = viewer.notToOpenArgs;
    else this.notToOpenArgs = {};
    if (viewer.onClearAllCallback) this.onClearAllCallback = viewer.onClearAllCallback;

    
    this.graphDS = "ds_graph_" + this.objCls;
    vjDS.add("Graph data", this.graphDS, "static://");
    
    viewer.isStickyHeader = true;
    this.tableViewer = new vjTableViewX2(viewer);
    this.tableViewer.selectCallback="function:vjObjFunc(\"onClickCell\", \"" + objcls+ "\",'general');";

    this.tableViewer.callbackRendered = "function:vjObjFunc(\"onTableRendered\", \"" + objcls+ "\");";


    this.onTableRendered = function(content, page_request) {
        for (var i=0; i<this.onTableRenderedCallbacks.length; i++) {
            this.onTableRenderedCallbacks[i](content, page_request, this);
        }
    }

    //when the table gets rendered we need to request the metadata file (this tells us about the types of columns and what rows/columns need to be colored, how many of each there are)
    //the tqs file that is requested are the current tqs commands
    this.tableRenderedCallCount = 0;
    this.onTableRenderedCallbacks= new Array();
    this.onTableRenderedCallbacks.push (
         function(content, page_request, that) {
            var url = "?cmd=-qpData&req="+that.tableViewer.metadata_reqId+"&raw=1&grp=1&dname=metadata.json&default=error:%20"+that.tableViewer.metadata_reqId+"%20metadata.json%20not%20found";
            //alert (url);
            ajaxDynaRequestPage(url, {objCls: objcls, callback:'metadata_parser'}, vjObjAjaxCallback);
         }
    );
    this.onTableRenderedCallbacks.push (
            function(content, page_request, that) {
               var url = "?cmd=-qpData&req="+that.tableViewer.metadata_reqId+"&raw=1&grp=1&dname=tqs.json&default=error:%20"+that.tableViewer.metadata_reqId+"%20tqs.json%20not%20found";
               //alert (url);
               ajaxDynaRequestPage(url, {objCls: objcls, callback:'tqs_parser'}, vjObjAjaxCallback);
            }
    );
    
    var urlToPut = "http://?cmd=objQry&qry=alloftype('plugin_tblqry').map({{visualization:.visualizationArr,argument:.argumentsArr,tqsToUseOnApply:.tqsToUseOnApply,panelDesc:.panelDesc,keyWords:.keyWordArr,panelIcon:.panelIcon,panelName:.panelName,panelTitle:.panelTitle,panelUrl:.panelUrl,panelObjQry:.panelObjQry,panelPath:.panelPath,toPrint:.toPrint,notUpdatePage:.notUpdatePage}})&raw=1";
    vjDS.add("Table query result", "plugins", urlToPut);

    
    var generalRows =[
                      {name:'load', order:-3.01 ,title: 'Load New Data' , icon:'upload' , description: 'load new data' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'load');" },
                      {name:'source', order:-3 ,title: 'Data Source' , icon:'database' , description: 'select Source', path:"/source", hidden: true},
                      {name:'plus', order:-2 ,title: 'New Column' , icon:'plus' , description: 'add new column' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'newcontrol');", path:"/plus" },
                      {name:'graphs', order:-1 ,title: 'Graphs' , icon:'pie' , description: 'select graphic to build', path:"/graphs"},
                      {name:'diagrams', order:1 ,title: 'Diagrams' , description: 'select graphic to build', menuHorizontal:false, path:"/graphs/diagrams"},
                          {name:'column', order:-1 ,title: 'Column' , description: 'select column graph', menuHorizontal:false, path:"/graphs/diagrams/column" ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'colgraph');" },
                          {name:'line', order:-1 ,title: 'Line' , description: 'select line to build',menuHorizontal:false, path:"/graphs/diagrams/line" ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'linegraph');" },
                          {name:'pie', order:-1 ,title: 'Pie' , description: 'select pie to build', menuHorizontal:false, path:"/graphs/diagrams/pie" ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'piegraph');" },
                          {name:'scatter', order:-1 ,title: 'Scatter' , description: 'select scatter to build', menuHorizontal:false, path:"/graphs/diagrams/scatter" ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'scattergraph');" },
                      
                      {name:'pager', icon:'page' , align:'right',order:19, title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000], value0:30},
                      {name:'search', align:'right',order:20, isSubmitable: true, prohibit_new: true },
                      {name:'analyze', align:'left', order:0, icon:'graph' , title:'Analysis', path: '/analyze' },
                      {name:'glueControl', align:'left', order:1, icon:'copy' , title:'Glue Tables', path: '/glue'  ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'glueControl');"},
                      {name:'reset', align:'left', order:4, icon:'refresh' , title:'Reset Table', path: '/reset'  ,  url: "javascript:vjObjEvent(\"onResetTable\", \"" + objcls+ "\");javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")"},
                      {name:'download', align:'left', order:2, icon:'save' , title:'Download the table', path: '/download' },
                      {name:'partOfTable', align:'left', order:1, title:'Download visible part of the table', path: '/download/partOfTable', url: "javascript:vjObjEvent(\"onPartOfTable\", \"" + objcls+ "\");" },
                      {name:'entireTable', align:'left', order:2, title:'Download the entire table', path: '/download/entireTable', url: "javascript:vjObjEvent(\"onEntireTable\", \"" + objcls+ "\");"},
                      {name:'archiveTable', align:'left', order:3, title:'Archive the entire table', path: '/download/archiveTable', url: "javascript:vjObjEvent(\"onArchiveTable\", \"" + objcls+ "\");"}
                      ];   
    var generalRowsToPush;
    
    if (!this.dissableBtns)
        generalRowsToPush = generalRows;
    else
    {
        generalRowsToPush = [];
        
        for (var i = 0; i < generalRows.length; i++)
        {
            if (this.dissableBnts.indexOf(generalRows[i].name) >= 0)
                continue;
            else
                generalRowsToPush.push(generalRows[i]);
        }
    }
    
    var plugginsToPush= {
        load: {
             rows: [
                    {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                    {name:'objs',  order:1 ,title: 'Object ID' , description: 'object ID of the table source', type:"explorer", explorerObjType: "u-file+,table+,process+,!svc-download,!svc-archiver,!image", size: 8, isSubmitable:true, multiSelect: true },
                    {name:'tbl',  order:2 ,title: 'Table Name', description: "table name", size: 8, showTitleForInputs:true, path: "/tbl/" },
                    {name:'tqsId',  order:3 ,title: 'TQS File Object ID', description: "table name", type:"explorer", size: 16, explorerObjType: "u-tqs", explorerFileExt: "json", isSubmitable:true },
                    {name:'objQry',  order:4 ,title: 'Query', description: 'object query of the table source', type:"text", size: 64, isSubmitable:true },
                    {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', url: "javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
             icon: "upload", 
             title: "Load New Data"
        },
           general: {rows: generalRowsToPush},
           colgraph: {
               rows:[
                     {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                     {name:'colorx', path:"/colorx", order:0, value:'#FF85C2', type:'color', readonly:true , showTitleForInputs: true , title: "Select x", url: "javascript:vjObjEvent(\"onGraph\",\""+objcls+"\",\"x\", 'colgraph')" },
                     {name:'colory', path:"/colory", order:2, value:'#6699FF', type:'color', readonly:true , showTitleForInputs: true , title: "Select y", url: "javascript:vjObjEvent(\"onGraph\",\""+objcls+"\",\"y\", 'colgraph')" },
                     {name:'namex', path:"/namex", order:1, showTitleForInputs: false , title: "Name for x-axis", type: "text" },
                     {name:'doHistogram', order:1, showTitleForInputs: false , value: false, title: "Histogram mode", type: "checkbox" },
                     {name:'doIntegral', order:2, showTitleForInputs: false , value: false, title: "Integral mode", type: "checkbox" },
                     {name:'clear', icon:'refresh', order:50, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"" + objcls+ "\",'colgraph');" },
                     {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGraph\", \"" + objcls+ "\",\"generate\", 'colgraph');javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
               title: "Column Graph",
               toPrint: "Built Column Graph"
           },
           glueControl: {
               rows:[
                     {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                  {name:'obj', title:"Select object for right table", showTitleForInputs:true, order:3, type:"explorer", explorerObjType: "u-file+,table+,process+,!svc-download,!svc-archiver,!image", size: 8, multiSelect: true},
                     {name:'req', order:4, type:'text', showTitleForInputs: true , title: "Enter request ID for table"},
                     {name:'tbl', order:5, showTitleForInputs: false , title: "Table from Object/Request", type: "text" },
                     {name:'rhs', order:6, showTitleForInputs: false , title: "Formula for Right Hand Side", type: "text" },
                     {name:'hidecol', order:7, showTitleForInputs: false , title: "Column to hide", type: "text" },
                     {name:'lhs', order:1, title:'Formula for Left Hand Side', type:"text" },
                     {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGlue\", \"" + objcls+ "\");javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
               title: "Glue tables together",
               toPrint: "Glued 2 tables together"
           },
           linegraph:{
               rows:[
                     {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                     {name:'colorx', path:"/colorx", order:0, value:'#99FF66', type:'color', readonly:true , showTitleForInputs: true , title: "Select x", url: "javascript:vjObjEvent(\"onGraph\",\""+objcls+"\",\"x\",'linegraph')" },
                     {name:'colory', path:"/colory", order:2, value:'#A3D1FF', type:'color', readonly:true , showTitleForInputs: true , title: "Select y", url: "javascript:vjObjEvent(\"onGraph\",\""+objcls+"\",\"y\",'linegraph')" },
                     {name:'namex', path:"/namex", order:1, showTitleForInputs: false , title: "Name for x-axis", type: "text" },
                       {name:'doHistogram', order:1, showTitleForInputs: false , value: false, title: "Histogram mode", type: "checkbox" },
                        {name:'doIntegral', order:2, showTitleForInputs: false , value: false, title: "Integral mode", type: "checkbox" },
                     {name:'clear', icon:'refresh', order:50, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"" + objcls+ "\",'linegraph');" },
                     {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGraph\", \"" + objcls+ "\",\"generate\", 'linegraph');javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
            title: "Line Graph",
               toPrint: "Built Line Graph"
           },
           piegraph: {
               rows:[
                 {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                 {name:'colorx', path:"/colorx", order:0, value:'#FFFF99', type:'color', readonly:true , showTitleForInputs: true , title: "Select Category", url: "javascript:vjObjEvent(\"onGraph\",\""+objcls+"\",\"x\",'piegraph')" },
                 {name:'colory', path:"/colory", order:1, value:'#DBB8FF', type:'color', readonly:true , showTitleForInputs: true , title: "Select Measurement", url: "javascript:vjObjEvent(\"onGraph\",\""+objcls+"\",\"y\",'piegraph')" },
                 {name:'clear', icon:'refresh', order:50, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"" + objcls+ "\",'piegraph');" },
                   {name:'doHistogram', order:1, showTitleForInputs: false , value: false, title: "Histogram mode", type: "checkbox" },
                    {name:'doIntegral', order:2, showTitleForInputs: false , value: false, title: "Integral mode", type: "checkbox" },
                 {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGraph\", \"" + objcls+ "\",\"generate\", 'piegraph');javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
            title: "Pie Graph",
                 toPrint: "Built Pie Graph"
           },
           scattergraph: {
               rows:[
                 {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                 {name:'colorx', path:"/colorx", order:0, value:'#D685FF', type:'color', readonly:true , showTitleForInputs: true , title: "Select x", url: "javascript:vjObjEvent(\"onGraph\",\""+objcls+"\",\"x\",'scattergraph')" },
                 {name:'colory', path:"/colory", order:2, value:'#B5FFDA', type:'color', readonly:true , showTitleForInputs: true , title: "Select y", url: "javascript:vjObjEvent(\"onGraph\",\""+objcls+"\",\"y\",'scattergraph')" },
                 {name:'namex', path:"/namex", order:1, showTitleForInputs: false , title: "Name for x-axis", type: "text" },
                   {name:'doHistogram', order:1, showTitleForInputs: false , value: false, title: "Histogram mode", type: "checkbox" },
                    {name:'doIntegral', order:2, showTitleForInputs: false , value: false, title: "Integral mode", type: "checkbox" },
                 {name:'clear', icon:'refresh', order:50, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"" + objcls+ "\",'scattergraph');" },
                 {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGraph\", \"" + this.objCls+ "\",\"generate\", 'scattergraph');javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
            title: "Scatter Graph",
            toPrint: "Built Scatter Graph"
           },
           newcontrol: {
               rows:[
                 {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                 {name:'newname' , path:"/newname", order: 1, type: "text", url: "javascript:vjObjEvent(\"onAddNewColumn\", \"" + objcls+ "\");", title:'Column Name' },
                 {name:'newformula', path:"/newformula", order: 2, description: 'add formula for new column' , type:'text', title:'Formula' },
                 //{name:'colType', path:"/colType", order: 3, type:'select', options:[['-1',' '], ['0','String'],['1','Integer'],['2','Real']], title: 'Choose Column Type (Optional)'},
                 {name:'search', align:'right',order:10, isSubmitable: true, prohibit_new: true },
                 {name:'apply', order:8 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onAddNewColumn\", \"" + objcls+ "\");vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
            title: "New Column",
            icon: "plus"
           },
           editcontrol: {
               rows:[
                 {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                 {name:'sort', order: 2, title: '   &darr;   Sort' , path: "/sort"},
                 {name:'sortType', order: 2, title: 'Chose Type of Sorting' , path: "/sort/sortType", value:0, options: [[0,""], [1,"Ascending"],[2,"Descending"]], type:'select'},
                 {name:'filterOnCol', order:0, title:'Select Column to Filter on', description: 'Select Column to Filter on and Hide Rows'},
                 //{name:'colType', order:3, hidden:true, title:'Change Column Type', description: 'Change the type of the column', type:'select', options:[['0','Integer'],['1','String']], showTitleForInputs:true, url: "javascript:vjObjEvent(\"onChgColType\", \"" + objcls+ "\")"},
                 {name:'stringExpression',  order:1, hidden:true, size:'40', type: "text", path: "/stringExpressionBtn/stringExpression", title: "Expression"},
                 {name:'stringExpressionBtn',  order:4, hidden:true, title:' &darr;   Filter', showTitleForInputs: true, url: "javascript:vjObjEvent(\"onExpHide\", \"" + objcls+ "\");"},
                 {name:'stringExpressionNegationCheck',  order:1, hidden:true, type: "checkbox", title: "Check for Negation", path: "/stringExpressionBtn/stringExpressionNegationCheck"},
                 {name:'stringExpressionRegCheck',  order:2, hidden:true, type: "checkbox", title:"Check for Regular Expressions", path: "/stringExpressionBtn/stringExpressionRegCheck"},
                 {name:'stringExpressionCaseSens',  order:3, hidden:true, type: "checkbox", title:"Check for Case Sensitive", path: "/stringExpressionBtn/stringExpressionCaseSens"},
                 {name:'intExpression',  order:6, hidden:true, title: '  &darr;   Filter', showTitleForInputs: true, url: "javascript:vjObjEvent(\"onExpHide\", \"" + objcls+ "\");"},
                 {name:'intFromExpression',  order:4, hidden:true, title: 'From', showTitleForInputs:true, size:'5', type: "text", path: "/intExpression/intFromExpression"},
                 {name:'intToExpression',  order:5, hidden:true, title: 'To', showTitleForInputs:true, size:'5', type: "text", path: "/intExpression/intToExpression"},
                 {name:'intExclusiveExpression',  order:6, hidden:true, title: 'Exclusive', showTitleForInputs:true, size:'5', type: "checkbox", path: "/intExpression/intExclusiveExpression"},
                 {name:'stringColoration',  order:1, hidden:true, size:'40', type: "text", path:"/stringColorationBtn/stringColoration", title: "Expression"},
                 {name:'stringColorationBtn',  order:5, hidden:true, title:'   &darr;   Color', showTitleForInputs:true, url: "javascript:vjObjEvent(\"onColorizeRows\", \"" + objcls+ "\");"},
                 {name:'stringColorationNegationCheck',  order:5, hidden:true, type: "checkbox", title: "Check for Negation", path: "/stringColorationBtn/stringColorationNegationCheck"},
                 {name:'stringColorationRegCheck',  order:6, hidden:true, type: "checkbox", title:"Check for Regular Expressions", path: "/stringColorationBtn/stringColorationRegCheck"},
                 {name:'intFromColoration',  order:7, hidden:true, title: 'From', showTitleForInputs:true, size:'5', type: "text", path: "/intColoration/intFromColoration"},
                 {name:'intToColoration',  order:8, hidden:true, title: 'To', showTitleForInputs:true, size:'5', type: "text", path: "/intColoration/intToColoration"},
                 {name:'intExclusiveColor',  order:9, hidden:true, title: 'Exclusive', showTitleForInputs:true, size:'5', type: "checkbox", path: "/intColoration/intExclusiveColor"},
                 {name:'intColoration',  order:9, hidden:true, title:'  &darr;   Color', showTitleForInputs:true,  url: "javascript:vjObjEvent(\"onColorizeRows\", \"" + objcls+ "\");"},
                 {name:'intFiltcolor', order:10, hidden:true, value:"#ff9999", title: 'Choose color' ,  type:'color', showTitleForInputs: true , title: "Select color", path:"/intColoration/intFiltcolor"},
                 {name:'stringFiltcolor', order:10, hidden:true, value:"#ff9999", title: 'Choose color' ,  type:'color', showTitleForInputs: true , title: "Select color", path:"/stringColorationBtn/stringFiltcolor"},
                 {name:'rename', order: 14, title: ' Rename' , showTitleForInputs:true, type: "text", url: "javascript:vjObjEvent(\"onRenameHeader\", \"" + objcls+ "\");" },
                 {name:'del', order: 15, icon:'delete' , title:'Delete Column', description: 'hide the column', url: "javascript:vjObjEvent(\"onDeleteColumn\", \"" + objcls+ "\");vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" },
                 {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }
                 ]
           }
       };
    //this.tableViewer.actualBigPanel.rebuildPluginTrees();
    
    
    var tbl_data = "ds_%PANNELCLASS%_tbl_data";

    //this.tableViewer.actualBigPanel._original_onChangeElementValue = this.tableViewer.actualBigPanel.onChangeElementValue;
    myonChangeElementValue = function(container, nodepath, elname) {
        //this._original_onChangeElementValue(container, nodepath, elname);
        var node = this.tree.findByPath(nodepath);
        if (nodepath == "/objs") {
            vjDS[this.tbl_data].reload("http://?cmd=propget&files=%2A.{csv,tsv,tab}&ids="+node.value+"&mode=csv", true);
        } else if (nodepath == "/tbl/tbl-*") {
            var tblNode = this.tree.findByPath("/tbl");
            if (tblNode && tblNode.children) {
                for (var i=0; i<tblNode.children.length; i++) {
                    tblNode.children[i].value = node.value;
                }
                this.redrawMenuView(tblNode);
            }
        } else if (nodepath.indexOf("/tbl/tbl-") == 0 && !parseBool(node.value)) {
            this.tree.findByPath("/tbl/tbl-*").value = "0";
            this.redrawMenuView(this.tree.findByPath("/tbl"));
        }
    };
    
    myTblDataCallback = function(param, content) {
        var rows = this.rows;
        var newrows = [];
        for (var i=0; i<rows.length; i++) {
            //if (!rows[i].path || rows[i].path.indexOf("/tbl/") != 0) {
                newrows.push(rows[i]);
            //}
        }
        var fileTbl = new vjTable(content, 0, vjTable_propCSV);
        newrows.push({name: "tbl-*", order: 100, title: "<em>All files</em>", path: "/tbl/tbl-*", value: "", type: "checkbox", isSubmitable:true, noMatch:true})
        if (fileTbl.rows.length && fileTbl.rows[0]._file) {
            var files = verarr(fileTbl.rows[0]._file).sort(cmpCaseNatural);
            for (var i=0; i<files.length; i++) {
                var tblName = files[i];
                newrows.push({name: "tbl-" + tblName, order: i + 101, title: tblName, path: "/tbl/tbl-" + tblName, value: tblName, type: "checkbox", isSubmitable:true, noMatch:true});
            }
        }
        var treeValues = {};
        this.tree.enumerate(function(viewer, node) {
            treeValues[node.path] = node.value;
        }, this);
        this.plugins[this.currentPlugin].rows = newrows;
        this.rebuildTree();
        this.tree.enumerate(function(viewer, node) {
            if (treeValues[node.path] != undefined)
                node.value = treeValues[node.path];
        }, this);
        this.redrawMenuView(this.tree.findByPath("/tbl"));
   };
    
    this.tableViewer.actualBigPanel = new vjPluggableToolbar({
        data: ["plugins", viewer.data],
        iconSize : 24,
        showTitles: true,
        showTitleFOrInputs: true,
        selectedCounter: viewer.selectedCounter,
        maxTxtLen: viewer.maxTxtLen,
        name: "theMainOne",
        selectOnClick: true,
        menuUpdateCallback:"function:vjObjFunc(\"menuUpdateCallback\", \"" + objcls+ "\");",
        formObject: viewer.formObject,
        currentPlugin: "general",
        panelOpenCallback: "function:vjObjFunc(\"onPanelOpen\", \"" + objcls+ "\");",
        clearAllCallback: "function:vjObjFunc(\"onClearAll\", \"" + objcls+ "\");",
        keyWords: this.keyWords,
        container: "pluggin"+Math.floor(Math.random()*1111111),
        plugins: plugginsToPush,
        tbl_data: tbl_data,
        tbl_dataCallback: myTblDataCallback,
        onChangeElementValueCallback: myonChangeElementValue,
        notUpdateCallback: "function:vjObjFunc(\"notUpdate\", \"" + objcls+ "\");",
        notToOpenArgs: this.notToOpenArgs,
        isok: true
    });
    
    //notToOpenArgs: {"editcontrol": ["rename", "del"], "load": [], "graphs":[]},

    //remember all of the panels created that need to be attached
    this.arrayPanels=[this.tableViewer.actualBigPanel];

    //these parse the metadata and the tqs files when they are loaded
    this.metadata_parser = function (params,data)
    {
        if (data.indexOf("error: ") != 0) {
            try {
                this.tableInfo = JSON.parse(data);
            } catch(E) {};
        }
        if (this.selections.tree && this.tableInfo && this.tableInfo.input)
        {
            this.selections.tree.findByPath("/tRows").title = this.tableInfo.input.rows.data + " total rows";
            this.selections.tree.findByPath("/tCols").title = this.tableInfo.input.columns.data + " total columns";
            this.selections.refresh();
        }
        this.tryCallbackRendered();
    };

    this.tqs_parser = function (params,data)
    {
        if (data.indexOf("error: ") != 0) {
            try {
                this.tableViewer.tqsObj = JSON.parse(data);
            } catch(E) {};
        }
        this.tryCallbackRendered();
    };
    
    this.tryCallbackRendered = function() {
        if (++this.tableRenderedCallCount >= this.onTableRenderedCallbacks.length && viewer.callbackRendered) {
            viewer.callbackRendered.call(this, null);
            this.tableRenderedCallCount = 0;
        }
        this.tableViewer.actualBigPanel.onPanelOpen(objcls, 'general');
    };

    //manages the panel that needs to be opened. If a filtering column is opened the appropriate fields need to be set to visible (string vs int)
    //along with apropriate values need to be filled into the text boxes had values in them before loading
    this.onPanelOpen=function(view, name)
    {
        if (this.tableViewer.actualBigPanel.noPanelOpen && this.tableViewer.actualBigPanel.noPanelOpen.indexOf(name) > -1)
        {
            var tempViewer;
            
            for (var key in this.tableViewer.actualBigPanel.plugins)
            {
                if (key == name)
                {
                    tempViewer = this.tableViewer.actualBigPanel.plugins[key];
                    this.tableViewer.tqsObj.push({op: name});
                    this.tableViewer.actualBigPanel.onUpdate();
                    return;
                }
            }

        }

        this.tableViewer.clearColors();
        if (name == "editcontrol"){
            this.tableViewer.colorCol(0, this.oldHeaderSelectedNum, "#ffd180");
            this.tableViewer.lastPanelEdit = true;            
        }
        var tqs = this.tableViewer.tqsObj;

        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpression", "value", "");
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionNegationCheck", "value", 0);
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionCaseSens", "value", 0);
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromExpression", "value", "");
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToExpression", "value", "");
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intExclusiveExpression", "value", 0);
        
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColoration", "value", 0);
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationRegCheck", "value", 0);
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationNegationCheck", "value", 0);
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToColoration", "value", "");
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromColoration", "value", "");
        
        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "sortType", "value", 0);
        
        this.tableViewer.actualBigPanel.currentSelector = undefined;
        this.tableViewer.actualBigPanel.currentColor = undefined;
        this.tableViewer.actualBigPanel.currentType = undefined;

        //this loop actually fills the text boxes with appopriate values
        for (var i = 0; i < tqs.length; i++)
        {
            if (name == "editcontrol")
            {
                if (tqs[i].op == "filter" && tqs[i].arg.col.name == this.oldHeaderSelected)
                {
                    if (this.tableInfo.output.columns.types[this.oldHeaderSelectedNum] == "string")
                    {
                        if (tqs[i].arg.value)
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpression", "value", tqs[i].arg.value);
                        else{
                            var tmp = "";
                            for (var ii=0; ii < tqs[i].arg.values.length; ii++) tmp += tqs[i].arg.values[ii] + ";";
                            
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpression", "value", tmp.substring (0,tmp.length-1));
                        }
                        if ( tqs[i].arg.negate == true)
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionNegationCheck", "value", 1);
                        if ( tqs[i].caseSensitive == true)
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionCaseSens", "value", 1);
                    }
                    else
                    {
                        if (!(! tqs[i].arg.value.min))
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromExpression", "value", tqs[i].arg.value.min);
                        if (!(! tqs[i].arg.value.max))
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToExpression", "value", tqs[i].arg.value.max);
                        if ( tqs[i].arg.value.exclusive == true)
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intExclusiveExpression", "value", 1);
                    }
                }
                else if (tqs[i].op == "rowcategory" && (typeof(tqs[i].arg.value) == "string" && tqs[i].arg.value.indexOf(this.oldHeaderSelected) > 0))
                {
                    if (this.tableInfo.output.columns.types[this.oldHeaderSelectedNum] == "string")
                    {
                        var extracted =  tqs[i].arg.value.substring( tqs[i].arg.value.indexOf("~/")-1);
                        if (tqs.regex==true)
                        {
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColoration", "value", extracted);
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationRegCheck", "value", 1);
                        }
                        else
                        {
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColoration", "value", extracted.substring(3,extracted.length-2));

                            if (extracted[0] == "!")
                                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationNegationCheck", "value", 1);
                        }
                        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringFiltcolor", "value", tqs[i].color);
                    }
                    else
                    {
                        var part1 = tqs[i].arg.value.substring(tqs[i].arg.value.indexOf("}") + 1);
                        var is2parts = tqs[i].arg.value.indexOf("&&");

                        if (is2parts > 0)
                        {
                            var part2 = part1.substring(part1.indexOf("}") + 1);
                            var space = part1.indexOf(" ");

                            if (part1[0] == ">")
                                fromInt = parseInt(part1.substring(2,space));
                            else
                                toInt = parseInt(part1.substring(2,space));

                            if (part2[0] == ">")
                                fromInt = parseInt(part2.substring(2));
                            else
                                toInt = parseInt(part2.substring(2));

                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToColoration", "value", toInt);
                            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromColoration", "value", fromInt);
                        }
                        else
                        {
                            if (part1[0] == ">")
                                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToColoration", "value", parseInt(part1.substring(2,space)));
                            else
                                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromColoration", "value", parseInt(part1.substring(2,space)));
                        }
                        this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFiltcolor", "value", tqs[i].color);
                    }
                }
                else if (tqs[i].op == "sort" && tqs[i].col == this.oldHeaderSelected)
                {
                    this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "sortType", "value", tqs[i].arg.reverse ? 2 : 1);
                }
                else
                {

                }
            }
            else if (name == tqs.op)
            {
                for (var key in tqs.arg)
                {
                    var keyInTree = this.tableViewer.actualBigPanel.plugins[this.tableViewer.actualBigPanel.currentPlugin].tree.findByName(key); 
                    if (!keyInTree || !keyInTree.type || keyInTree.type == "color")
                        continue;
                    
                    this.tableViewer.actualBigPanel.plugins[this.tableViewer.actualBigPanel.currentPlugin].tree.findByName(key).value = tqs.arg[key];
                }
            }
            
        }

        if (!(!this.tableInfo.output||!this.tableInfo.output.rows.categories))
        {
             var rowCategs = this.tableInfo.output.rows.categories;
             for (var key in rowCategs)
             {
                 if (rowCategs[key].panel == "always" || rowCategs[key].panel == name)
                 {
                     for (var i = 0; i < rowCategs[key].rows.length; i++)
                         this.tableViewer.colorRow(rowCategs[key].rows[i], 0,rowCategs[key].color);
                 }
             }
        }

        //these 2 loops are responsible for coloring the rows and the columns appropriately depending of whether they apply to the panel opened
       

        if (!(!this.tableInfo.output||!this.tableInfo.output.columns.categories))
        {
            var colCategs = this.tableInfo.output.columns.categories;
            for (var key in colCategs)
            {
                if (colCategs[key].panel == "always" || colCategs[key].panel == name)
                {
                    for (var i = 0; i < colCategs[key].cols.length; i++)
                        this.tableViewer.colorCol(0, colCategs[key].cols[i], colCategs[key].color);
                }
            }
        }
    };

    //prepares the url approprately
    this.menuUpdateCallback=function(tempViewer, url)
    {
        var extCmd="";
        var maxCount = 0;
        
        if (url.indexOf("%26cnt%3D")!=-1 && tempViewer.selectedCounter !== undefined && tempViewer.selectedCounter>0){
            var new_value_count = "%26cnt%3D" + tempViewer.selectedCounter;
            url = url.replace(/%26cnt%3D[0-9]*/g,new_value_count);
        }
        
        if (url == "static://")
            url = "qpbg_tblqryx4://_.csv//";
        
        if (url.indexOf("objs=") >= 0 && (url.indexOf("objQry=") >=0 || url.indexOf("objList") >= 0))
        {
            if (latestCommand && latestCommand == "objs")
            {
                url = "qpbg_tblqryx4://_.csv//";
                url = urlExchangeParameter(url, "objQry", this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objQry").value);
                latestCommand = "objQry";
            }
            else
            {
                url = "qpbg_tblqryx4://_.csv//&cnt=100";
                url = urlExchangeParameter(url, "objs", this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objs").value);
                latestCommand = "objs";
            }
            this.tableViewer.tqsObj = [];
            
            return url;
        }
        
        this.onExpHide(objcls);
        this.onSort(objcls);
        this.onColorizeRows(objcls);
        this.onRenameHeader(objcls);
        this.onCollectInfo(objcls);
        
        var currentPlugin =  this.tableViewer.actualBigPanel.currentPlugin;
        if (tempViewer.plugins[currentPlugin].tqsToUse){
            var actualTqsToUse = tempViewer.plugins[currentPlugin].tqsToUse
            var tmpTqs = tempViewer.plugins[currentPlugin].tqsToUse;
            while (tmpTqs.indexOf("$[") >= 0 && tmpTqs.indexOf("]") > tmpTqs.indexOf("$[")){
                var startB = actualTqsToUse.indexOf("$[");
                var endB = actualTqsToUse.indexOf("]");
                var actualElem = actualTqsToUse.substring (startB+2, endB);
                
                //will go through the last TQS element to see if anythng matches actualElem
                var tblTqs = this.tableViewer.tqsObj;
                if (tblTqs[tblTqs.length-1].arg[actualElem]){
                    var element = tblTqs[tblTqs.length-1].arg[actualElem];
                    var elementToUse = "";
                    
                    if (element instanceof Array){
                        for (var jj = 0; jj < element.length; jj++){
                            if (typeof(element[jj]) == "number"){
                                elementToUse += (jj == 0 ? "" : " && ") + "$" + element[jj];
                            }
                            else if (element[jj] instanceof String)
                                elementToUse += (jj == 0 ? "" : " && ") + "${" + element[jj] + "}";
                        }
                    }
                    else if (typeof(element) == "number"){
                        elementToUse = "" + element;
                    }
                    else if (element instanceof String)
                        elementToUse = element;
                    
                    actualTqsToUse = actualTqsToUse.substring (0,startB) + elementToUse + actualTqsToUse.substring (endB+1);
                }
                tmpTqs = tmpTqs.substring(tmpTqs.indexOf("]"));
            }
            
            var jsonAT = JSON.parse (actualTqsToUse);
            
            this.tableViewer.tqsObj = this.tableViewer.tqsObj.concat (verarr(jsonAT));
        }
        
        // this is a temporary solution to col and row category. For now, all of the categories will be moved to the end. later, we will come up with something else
        var last = this.tableViewer.tqsObj[this.tableViewer.tqsObj.length-1];
        for (var i = 0; i < this.tableViewer.tqsObj.length; i++)
        {
            if (this.tableViewer.tqsObj[i] == last)
                break;
            
            if ((this.tableViewer.tqsObj[i].op == "rowcategory" || this.tableViewer.tqsObj[i].op == "colcategory") && this.tableViewer.tqsObj[i].color)
            {
                var extract = this.tableViewer.tqsObj[i];
                this.tableViewer.tqsObj.splice(i,1);
                this.tableViewer.tqsObj.push(extract);
                i--;
            }
        }
        

        var ds=new vjDataSource();
        if (this.tableViewer.actualBigPanel.plugins["load"]!=undefined && this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objs").value && this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objs").value.length && url == "static://"){
            url = "qpbg_tblqryx4://_.csv//&cnt=100&cols=0-1000";
            url = urlExchangeParameter(url, "objs", this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objs").value);
            latestCommand = "objs";
        }
        else if (this.tableViewer.actualBigPanel.plugins["load"]!=undefined && this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objQry") && this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objQry").value && this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objQry").value.length && url == "static://"){
            url = "qpbg_tblqryx4://_.csv//&cols=0-1000";
            url = urlExchangeParameter(url, "objQry", this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("objQry").value);
            latestCommand = "objQry";
        }

        if (this.tableViewer.actualBigPanel.plugins["load"]!=undefined && this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("tqsId").value && this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("tqsId").value.length){
            url = urlExchangeParameter(url, "tqsId", this.tableViewer.actualBigPanel.plugins["load"].tree.findByName("tqsId").value);
        }
        
        url = urlExchangeParameter(url, "extCmd", extCmd);
        url = urlExchangeParameter(url, "tqsCnt", "-");
        url = urlExchangeParameter(url, "tqs", ds.escapeQueryLanguage(JSON.stringify(this.tableViewer.tqsObj)));
        
        if (this.tableViewer.tqsObj.length > 1)
            url = urlExchangeParameter(url, "tqsId", "-");
        
        if (maxCount > parseInt(this.tableViewer.actualBigPanel.getCurrentPageCounterValue()))
        {
            for (var ip=0; ip<this.arrayPanels.length; ip++) {
                var v = this.arrayPanels[ip];
                if (v == this.selections) {
                    continue;
                }
                var elt = v.getPageCounterElement();
                if (elt) {
                    elt.value = maxCount+1;
                }
                if (v.selectedCounter != undefined) {
                    v.selectedCounter = maxCount+1;
                }
            }
            url = urlExchangeParameter(url, "cnt", maxCount+1);
        }
            

        var tblName = url.split("//")[1];
        if (this.tableViewer.actualBigPanel.plugins["load"]!=undefined){
            var tblNode = this.tableViewer.actualBigPanel.plugins["load"].tree.findByPath("/tbl");
            if (tblNode.children.length) {
                var tblNames = [];
                for (var i=0; i<tblNode.children.length; i++) {
                    if (tblNode.children[i].value != true || tblNode.children[i].name == "tbl-*")
                        continue;
                    tblNames.push(tblNode.children[i].name.replace(/^tbl-/, ""));
                }
                var tmpTblName = tblNames.sort(cmpCaseNatural).join("%0A"); 
                if (tmpTblName != "") tblName = tmpTblName;
            }
        }
        if (url.match(/^qpbg_[^:]+:\/\//)) {
            var spl1 = url.split("://");
            var spl2 = spl1.slice(1).join("://").split("//");
            url = spl1[0] + "://" + tblName + "//" + spl2.slice(1).join("//");
        } else {
            url = urlExchangeParameter(url, "tbl", tblName);
        }
        
        if(this.menuUpdateCallbacks) 
        {
            for (var i = 0; i < this.menuUpdateCallbacks.length; i++)
            {
                nurl=funcLink(this.menuUpdateCallbacks[i], this, url);
                if( !nurl )
                {
                    oldUrl = url;
                    latestReqID = -1;
                    return url;
                }
                else
                    url = nurl;
            }
        }
        
        if(this.colLimit)
            url = urlExchangeParameter(url, "cols", this.colLimit);
        else 
            url = urlExchangeParameter(url, "cols", "0-200");
        
        url = urlExchangeParameter(url, "mode", "-");
        url = urlExchangeParameter(url, "prop", "-");
        url = urlExchangeParameter(url, "type", "-");
        oldUrl = url;
        latestReqID=-1;
        return url;
    };
    
    this.onCollectInfo = function(tempViewer)
    {
        var currentPanel = this.tableViewer.actualBigPanel.currentPlugin;
        
        if (!currentPanel || currentPanel == "general" || currentPanel == "load")
            return;
        
        var currentTqs = this.tableViewer.findInTqs(this.tableViewer.tqsObj, [["op", currentPanel]]);
        
        if (!currentTqs || currentTqs.length == 0)
        {
            currentTqs = {op: currentPanel, arg:{}};
            this.tableViewer.tqsObj.push(currentTqs);
        }
        else
            currentTqs = currentTqs[0];
        
        if (this.tableViewer.actualBigPanel.plugins[currentPanel].toPrint)
            currentTqs.toPrint = this.tableViewer.actualBigPanel.plugins[currentPanel].toPrint;
        
        //this.tableViewer.actualBigPanel.plugins[currentPanel].tree.findByName(this.tableViewer.actualBigPanel.plugins[currentPanel].rows[k].name).value
        for (var k = 0; k < this.tableViewer.actualBigPanel.plugins[currentPanel].rows.length; k++)
        {
            var type = this.tableViewer.actualBigPanel.plugins[currentPanel].rows[k].type;
            if (!this.tableViewer.actualBigPanel.plugins[currentPanel].tree)
                continue;
            var value = this.tableViewer.actualBigPanel.plugins[currentPanel].tree.findByName(this.tableViewer.actualBigPanel.plugins[currentPanel].rows[k].name).value;
            if (!type || type == "color")
                continue;
            if (!value || value == "")
                continue;
            currentTqs.arg[this.tableViewer.actualBigPanel.plugins[currentPanel].rows[k].name] = value;
        }
    };
    
    this.onResetTable = function (obj){
        var tqs = this.tableViewer.tqsObj;
        var loadTqs = this.tableViewer.findInTqs(this.tableViewer.tqsObj, [["op", "load"]]);
        
        this.tableViewer.tqsObj = [loadTqs];
    };
    
    this.onPartOfTable = function (obj)
    {
         vjDS.add("Visible Part of table download", "dsTblPart", "static://");
         vjDS["dsTblPart"].reload(vjDS[this.data].url,true,"download");
    };
    
    this.notUpdate = function (viewer, panel)
    {
        if (this.notUpdate) {
            funcLink(this.notUpdateCallback, this, panel);
        }
    };
    
    this.onEntireTable = function (obj)
    {
        var nameTbl = window.prompt("Name of table to be downloaded: ","Tbl");
        
        vjDS.add("Full table download", "dsTblFull", "static://");
        if (oldUrl == "")
        {
            var fullUrl = vjDS[this.data].url;
            //fullUrl = urlExchangeParameter(fullUrl, "cnt", "-");
            // need to remove all count in plain text format and encoded format 
            if (fullUrl.indexOf("%26cnt%3D") !=-1){
                fullUrl = fullUrl.replace(/%26cnt%3D[0-9]*/g,"");
            }
            if (fullUrl.indexOf("&cnt=")!=-1 ){
                fullUrl = fullUrl.replace(/&cnt=[0-9]*/g,"");
            }
            else if (fullUrl.indexOf("cnt=")!=-1 ){
                fullUrl = fullUrl.replace(/cnt=[0-9]*/g,"");
            }

            var indx1 = fullUrl.indexOf("//");
            var short = fullUrl.substring(indx1 + 2);
            var indx2 = short.indexOf("//");
            fullUrl = fullUrl.substring(0, indx1+2) + short.substring(0, indx2) + ":::" + nameTbl + ".csv" + short.substring(indx2);
            fullUrl = urlExchangeParameter(fullUrl, "cols", "-");
            fullUrl = urlExchangeParameter(fullUrl, "start", "-");
            
            //fullUrl = urlExchangeParameter(fullUrl, "dsaveas", nameTbl + ".csv");
            vjDS["dsTblFull"].reload(fullUrl, true, "download");
            return;
        }
        var url = oldUrl;
        var ds=new vjDataSource();
        url = urlExchangeParameter(url, "tqs", ds.escapeQueryLanguage(JSON.stringify(this.tableViewer.tqsObj)));
        url = urlExchangeParameter(url, "start", "-");
        url = urlExchangeParameter(url, "cnt", "-");
        url = urlExchangeParameter(url, "cols", "-");
        // url = urlExchangeParameter(url, "dsaveas", nameTbl + ".csv");
        //url = url.replace(new RegExp('^(qpbg_[^:]+://([^/]*%0[aA][^/]*)?)//'), "$1:::"+nameTbl+".csv//");
        var indx1 = url.indexOf("//");
        var short = url.substring(indx1 + 2);
        var indx2 = short.indexOf("//");
        url = url.substring(0, indx1+2) + short.substring(0, indx2) + ":::" + nameTbl + ".csv" + short.substring(indx2);
        
        vjDS["dsTblFull"].reload(url, true, "download");
    };
    
    this.onArchiveTable = function (obj)
    {
        var nameTbl = window.prompt("Name of table to be downloaded: ","Tbl");
        
        vjDS.add("Full table download", "dsArchTbl", "static://");
        if (oldUrl == "")
        {
            var fullUrl = vjDS[this.data].url;
            //fullUrl = urlExchangeParameter(fullUrl, "cnt", "-");
            // need to remove all count in plain text format and encoded format
            if (fullUrl.indexOf("%26cnt%3D") !=-1){
                fullUrl = fullUrl.replace(/%26cnt%3D[0-9]*/g,"");
            }
            if (fullUrl.indexOf("&cnt=")!=-1 ){
                fullUrl = fullUrl.replace(/&cnt=[0-9]*/g,"");
            }
            else if (fullUrl.indexOf("cnt=")!=-1 ){
                fullUrl = fullUrl.replace(/cnt=[0-9]*/g,"");
            }

            fullUrl = urlExchangeParameter(fullUrl, "cnt", "-")
            fullUrl = urlExchangeParameter(fullUrl, "arch", "1");
            fullUrl = urlExchangeParameter(fullUrl, "arch_dstname", nameTbl + ".csv");
            
            var indx1 = fullUrl.indexOf("//");
            var short = fullUrl.substring(indx1 + 2);
            var indx2 = short.indexOf("//");
            fullUrl = fullUrl.substring(0, indx1+2) + short.substring(0, indx2) + ":::" + nameTbl + ".csv" + short.substring(indx2);
            fullUrl = urlExchangeParameter(fullUrl, "cols", "-");
            fullUrl = urlExchangeParameter(fullUrl, "start", "-");
            
            vjDS["dsArchTbl"].reload(fullUrl, true);
            return;
        }
        var url = oldUrl;
        var ds=new vjDataSource();
        url = urlExchangeParameter(url, "tqs", ds.escapeQueryLanguage(JSON.stringify(this.tableViewer.tqsObj)));
        url = urlExchangeParameter(url, "start", "-");
        url = urlExchangeParameter(url, "cnt", "-");
        url = urlExchangeParameter(url, "cols", "-");
        url = urlExchangeParameter(url, "arch", "1");
        url = urlExchangeParameter(url, "arch_dstname", nameTbl + ".csv");
        //url = url.replace(new RegExp('^(qpbg_[^:]+://([^/]*%0[aA][^/]*)?)//'), "$1:::all.csv//");
        
        var indx1 = url.indexOf("//");
        var short = url.substring(indx1 + 2);
        var indx2 = short.indexOf("//");
        url = url.substring(0, indx1+2) + short.substring(0, indx2) + ":::" + nameTbl + ".csv" + short.substring(indx2);
                
        vjDS["dsArchTbl"].reload(url, true);
    };

    //this clears all table color selections that apply to this particular panel
    this.onClearAll = function (tempViewer, panelName)
    {
        //remove all the coloring of rows and columns from the specified panel
        this.tableViewer.clearColors();
        this.tableViewer.tqsObj.push({op:"clearColors", panel: panelName});

        for (var i = 0; i < this.tableViewer.tqsObj.length; i++)
        {
            if (this.tableViewer.tqsObj[i].panel==panelName)
            {
                this.tableViewer.tqsObj.splice(i, 1);
                i--;
            }
        }

        for (var i = 0; i < this.tableViewer.tqsObj.length; i++)
        {
            if (this.tableViewer.tqsObj[i].panel=="always")
            {
                this.tableViewer.colorRow(this.tableViewer.tqsObj[i].arg.value, 0, -1);
            }
        }
        
        var arr = this.tableViewer.findInTqs(this.tableViewer.tqsObj, [["op", panelName]]);
        if (arr.length > 0)
        {
            var index = this.tableViewer.tqsObj.indexOf(arr[0]);
            this.tableViewer.tqsObj.splice(index, 1);
        }
        
        if (this.onClearAllCallback)
            this.onClearAllCallback(tempViewer, panelName);
    };
    
    this.onGlue = function (tempViewer)
    {
        var toPushTqs = {op: "glue", arg: { rhs:{}, lhs:{}}};
        
        var objToPush = this.tableViewer.actualBigPanel.plugins["glueControl"].tree.findByName("obj").value;
        var reqToPush = this.tableViewer.actualBigPanel.plugins["glueControl"].tree.findByName("req").value;
        var tblToPush = this.tableViewer.actualBigPanel.plugins["glueControl"].tree.findByName("tbl").value;
        var rhsFormulaToPush = this.tableViewer.actualBigPanel.plugins["glueControl"].tree.findByName("rhs").value;
        var hideColToPush = this.tableViewer.actualBigPanel.plugins["glueControl"].tree.findByName("hidecol").value;
        var lhsformulaToPush = this.tableViewer.actualBigPanel.plugins["glueControl"].tree.findByName("lhs").value;
        
        if (objToPush && objToPush != "" && objToPush.indexOf(",")>0)
            toPushTqs.arg.rhs["obj"] = "["+objToPush+"]";
        else if (objToPush && objToPush != "")
            toPushTqs.arg.rhs["obj"] = objToPush;
        
        if (reqToPush && reqToPush != "" && reqToPush.indexOf(",")>0)
            toPushTqs.arg.rhs["req"] = "["+reqToPush+"]";
        else if (reqToPush && reqToPush != "")
            toPushTqs.arg.rhs["req"] = reqToPush;
        
        if (tblToPush && tblToPush != "" && tblToPush.indexOf(",")>0)
            toPushTqs.arg.rhs["tbl"] = "["+tblToPush+"]";
        else if (tblToPush && reqToPush != "")
            toPushTqs.arg.rhs["tbl"] = tblToPush;
        
        if (rhsFormulaToPush && rhsFormulaToPush != "")
            toPushTqs.arg.rhs["formula"] = rhsFormulaToPush;

        if (lhsformulaToPush && lhsformulaToPush != "")
            toPushTqs.arg.lhs["formula"] = lhsformulaToPush;
        
        if (hideColToPush && hideColToPush != "")
        {
            if (!isNumber(hideColToPush))
            {
                if (hideColToPush.indexOf(",") > 0)
                    toPushTqs.arg.rhs["hidecol"] = "[" + hideColToPush +"]";
                else
                    toPushTqs.arg.rhs["hidecol"] = hideColToPush;
            }
            else
            {
                var arr = [];
                
                while (hideColToPush.indexOf(",") > 0)
                {
                    arr.push({name: hideColToPush.substring(0, hideColToPush.indexOf(","))});
                    hideColToPush = hideColToPush.substring(hideColToPush.indexOf(",") + 1);
                }
                
                arr.push({name: hideColToPush});
                
                toPushTqs.arg.rhs["hidecol"] = arr;
            }
        }
        
        this.tableViewer.tqsObj.push(toPushTqs);
    };
    

    //this builds a specified graph (line, pie, scatter, column)
    this.onGraph = function (tempViewer, category, graphType)
    {
        //record what graph is being built on what subset
        var color;
        
        if (category == "generate")
        {
            var allCategs = this.tableViewer.arrayOfCategories;
            var colx = -1;
            var coly = new Array();            
            var toPushTqs = this.tableViewer.findInTqs(this.tableViewer.tqsObj, [["op", "basicGraph"]]);
            
            if (!toPushTqs || toPushTqs.length == 0)
            {
                toPushTqs = {op: "basicGraph", arg:{}};
                this.tableViewer.tqsObj.push(toPushTqs);
            }
            else
                toPushTqs = toPushTqs[0];

            for (var i = 0; i < this.tableViewer.tqsObj.length; i++)
            {
                if (this.tableViewer.tqsObj[i].panel && this.tableViewer.tqsObj[i].panel == graphType)
                {
                    if (this.tableViewer.tqsObj[i].categ.indexOf("x") >=0 && this.tableViewer.tqsObj[i].arg.values && this.tableViewer.tqsObj[i].arg.values.length > 0)
                        colx = this.tableViewer.tqsObj[i].arg.values[0];
                    else if (this.tableViewer.tqsObj[i].categ.indexOf("y") >=0)
                        coly = this.tableViewer.tqsObj[i].arg.values;
                    
                }
            }

            if (colx == -1 || (coly.length < 1))
                return;
            
            toPushTqs.arg.rowSet = colx;
            toPushTqs.arg.colSet = coly;

            if (graphType == "colgraph")
            {
                var doHistogram=this.tableViewer.actualBigPanel.plugins["colgraph"].tree.findByName("doHistogram").value;
                var doIntegral=this.tableViewer.actualBigPanel.plugins["colgraph"].tree.findByName("doIntegral").value;
                 
                 
                this.graph.data = [this.vData];
                this.graph.name = "Column Graph";

                var namex=0;
                namex = this.tableViewer.actualBigPanel.plugins["colgraph"].tree.findByPath("/namex");
                
                if (namex != 0)
                    this.graph.options = {height:700, hAxis:{title:namex.value}};
                else
                    this.graph.options = {height:700};

                this.graph.series = [{
                            name : this.tableViewer.tblArr.hdr[colx].name,
                            label:true
                        }];

                for (var i = 0 ; i < coly.length; i++)
                    this.graph.series.push({name:this.tableViewer.tblArr.hdr[coly[i]].name});

                this.graph.type = 'column';
                this.graph.doHistogram=doHistogram;
                this.graph.doIntegral=doIntegral;
                
                toPushTqs.arg.graphType = "column";

                return;
            }
            else if (graphType == "linegraph")
            {
                var doHistogram=this.tableViewer.actualBigPanel.plugins["linegraph"].tree.findByName("doHistogram").value;
                var doIntegral=this.tableViewer.actualBigPanel.plugins["linegraph"].tree.findByName("doIntegral").value;
                
                this.graph.data = [this.vData];
                this.graph.name = "Line Graph";
                
                var namex=0;
                namex = this.tableViewer.actualBigPanel.plugins["linegraph"].tree.findByPath("/namex");

                if (namex != 0)
                    this.graph.options = {height:700, hAxis:{title:namex.value}};
                else
                    this.graph.options = {height:700};


                this.graph.series = [{
                            name : this.tableViewer.tblArr.hdr[colx].name
                        }];

                for (var i = 0 ; i < coly.length; i++)
                    this.graph.series.push({name:this.tableViewer.tblArr.hdr[coly[i]].name});

                this.graph.type = 'line';
                this.graph.doHistogram=doHistogram;
                this.graph.doIntegral=doIntegral;
                
                toPushTqs.arg.graphType = "line";

                return;
            }
            else if (graphType == "piegraph")
            {
                var doHistogram=this.tableViewer.actualBigPanel.plugins["piegraph"].tree.findByName("doHistogram").value;
                var doIntegral=this.tableViewer.actualBigPanel.plugins["piegraph"].tree.findByName("doIntegral").value;
                
                this.graph.data = [this.vData];
                this.graph.name = "Pie Graph";
                this.graph.options = {height:700};

                this.graph.series = [{label:true, name : this.tableViewer.tblArr.hdr[colx].name},{name:this.tableViewer.tblArr.hdr[coly[0]].name}];

                this.graph.type = 'pie';
                this.graph.doHistogram=doHistogram;
                this.graph.doIntegral=doIntegral;
                
                toPushTqs.arg.graphType = "pie";

                return;
            }
            else if (graphType == "scattergraph")
            {
                var doHistogram=this.tableViewer.actualBigPanel.plugins["scattergraph"].tree.findByName("doHistogram").value;
                var doIntegral=this.tableViewer.actualBigPanel.plugins["scattergraph"].tree.findByName("doIntegral").value;
                
                this.graph.data = [this.vData];
                this.graph.name = "Scatter Graph";
                
                var namex=0;
                namex = this.tableViewer.actualBigPanel.plugins["scattergraph"].tree.findByPath("/namex");
                
                if (namex != 0)
                    this.graph.options = {height:700, hAxis:{title:namex.value}};
                else
                    this.graph.options = {height:700};


                this.graph.series = [{
                            name : this.tableViewer.tblArr.hdr[colx].name
                        }];

                for (var i = 0 ; i < coly.length; i++)
                    this.graph.series.push({name:this.tableViewer.tblArr.hdr[coly[i]].name});

                this.graph.type = 'scatter';
                this.graph.doHistogram=doHistogram;
                this.graph.doIntegral=doIntegral;
                
                toPushTqs.arg.graphType = "scatter";

                return;
            }
        }

        var viewerToUse;
        //making the tables actually visible to the user
        if (graphType == "linegraph")
            viewerToUse = this.tableViewer.actualBigPanel.plugins["linegraph"];
        else if (graphType == "colgraph")
            viewerToUse = this.tableViewer.actualBigPanel.plugins["colgraph"];
        else if (graphType == "piegraph")
            viewerToUse = this.tableViewer.actualBigPanel.plugins["piegraph"];
        else if (graphType == "scattergraph")
            viewerToUse = this.tableViewer.actualBigPanel.plugins["scattergraph"];

        if (category == "x")
            color = viewerToUse.tree.findByPath("/colorx");
        else if (category == "y")
            color = viewerToUse.tree.findByPath("/colory");

        this.tableViewer.actualBigPanel.startColorCategorizer(this.tableViewer.actualBigPanel, graphType, category+color.value, color.value, "color col");
    };

    this.onChgColType = function (tempViewer)
    {
        //add a proper operation to the list.
        //for now, this is not being handled on the back end. can remove this functionality (for now)

        var newType;
        this.editColumnControl.enumerate(function(param, node) { if(node.name == "colType") newType = node; });
//        var ds=new vjDataSource();

        if (!newType.value)
            return;

        var toSet = "real";

        if (newType == 1)
            toSet = "string";

        //here, need to name sure on how to specify the new column type
//        query = ds.escapeQueryLanguage("setincoltype(\""+this.oldHeaderSelected+"\",\""+toSet+"\");");

        //this.prepareOut+=query;
    };
    
    this.onSort = function (tempViewer)
    {
        //add a proper filter operation depending on what type of input it is
        var sortNode;
        sortNode = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/sort/sortType");

        if (sortNode.value <= 0)
            return;
        
        var toPush = {op:"sort", arg:{formula: "$"+this.oldHeaderSelectedNum, reverse: sortNode.value == 1 ? false : true}, col: this.oldHeaderSelected, toPrint: "Sorted on column " + this.oldHeaderSelected};
        this.tableViewer.tqsObj.push(toPush);
    };

    //filter for what types of expressions need to stay visible (technecally the name of the function is wrong)
    this.onExpHide = function (tempViewer)
    {
        //add a proper filter operation depending on what type of input it is
        var newType,to,from;
        newType = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/stringExpressionBtn/stringExpression");
        to = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/intExpression/intToExpression");
        from = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/intExpression/intFromExpression");
        
        var ds=new vjDataSource();
        var query;
        
        if (this.tableInfo.output == undefined)
            return;

        if ((this.tableInfo.output.columns.types[this.oldHeaderSelectedNum] == "string" && !newType.value) ||
                (this.tableInfo.output.columns.types[this.oldHeaderSelectedNum] != "string" && (!to.value && ! from.value)))
            return;

        var toPush={op:"filter", arg:{col:{name:this.oldHeaderSelected}}, col:this.oldHeaderSelected};
        var toPrintStr = "Filtered on column " + this.oldHeaderSelected + " ";
        
        var oldObj = this.findInTqsPos(this.tableViewer.tqsObj, [["op", "filter"], ["col", this.oldHeaderSelected]]);

        if (this.tableInfo.output.columns.types[this.oldHeaderSelectedNum] == "string" && newType.value!= " ")
        {
            toPush.arg.negate = false;
            toPush.arg.caseSensitive = false;

            var negation="", regexp="", caseSens="";
            var neg = "!";
            negation = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/stringExpressionBtn/stringExpressionNegationCheck");
            regexp = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/stringExpressionBtn/stringExpressionRegCheck");
            caseSens = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/stringExpressionBtn/stringExpressionCaseSens");
            
            if (newType.value.indexOf(";") > 0){
                var allVals = [];
                var curVal = newType.value;
                
                while (curVal.indexOf(";") > 0){
                    var tmp = curVal.substring(0, curVal.indexOf(";"));
                    allVals.push(tmp);
                    curVal = curVal.substring(curVal.indexOf(";")+1);
                }
                allVals.push(curVal);
                
                if (regexp.value)
                {
                    toPush.arg.method = "regex";
                    toPush.arg.values = allVals;
                    toPrintStr += "using regular expression ";
                }
                else
                {
                    toPush.arg.method = "substring";
                    toPush.arg.values = allVals;
                    toPrintStr += "using key ";
                }
            }
            else{
                if (regexp.value)
                {
                    toPush.arg.method = "regex";
                    toPush.arg.value = newType.value;
                    toPrintStr += "using regular expression ";
                }
                else
                {
                    toPush.arg.method = "substring";
                    toPush.arg.value = newType.value;
                    toPrintStr += "using key ";
                }
            }
            
            if (negation.value)
            {
                toPush.arg.negate = true;
                toPrintStr += "not "
            }
            toPrintStr += newType.value;
            if(caseSens.value)
            {
                toPush.arg.caseSensitive = true;
                toPrintStr += " (case sensitive)"
            }
        }
        else
        {
            var fromInt = parseFloat (from.value);
            var toInt = parseFloat (to.value);
            var exclusive="";
            exclusive = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/intExpression/intExclusiveExpression");

            toPush.arg.method = "range";

            if (!(!toInt) && !(!fromInt))
            {
                toPush.arg.value = {min: fromInt, max: toInt};
                toPrintStr += "from " + fromInt + " to " + toInt;
            }
            else if (!toInt && !fromInt)
                return;
            else if (!fromInt)
            {
                toPush.arg.value = {max: toInt};
                toPrintStr += "to " + toInt;
            }
            else
            {
                toPush.arg.value = {min: fromInt};
                toPrintStr += "from " + fromInt;
            }

            if (exclusive.value)
            {
                toPush.arg.value.exclusive = true;
                toPrintStr += " exclusively";
            }

        }

        toPush.toPrint = toPrintStr;
        if (oldObj.length > 0)
            this.tableViewer.tqsObj[oldObj[0]] = toPush;
        else
            this.tableViewer.tqsObj.push (toPush);
    };

    //add an appropriate command to the tqs in order to colorize appropriate rows
    this.onColorizeRows = function (tempViewer)
    {
        //add a proper filter tag. with the correct color and the approprate options

        var newType,to,from;
        newType = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/stringColorationBtn/stringColoration");
        to = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/intColoration/intToColoration");
        from = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/intColoration/intFromColoration");
        var toPush = {};
        var toPrintStr = "Colored column " + this.oldHeaderSelected;
        
        var ds=new vjDataSource();
        var clr;
        var query;
        if (this.tableInfo.output == undefined)
            return;
        if ((this.tableInfo.output.columns.types[this.oldHeaderSelectedNum] == "string" && !newType.value) ||
                (this.tableInfo.output.columns.types[this.oldHeaderSelectedNum] != "string" && (!to.value && ! from.value)))
            return;
        
        var oldObj = this.findInTqsPos(this.tableViewer.tqsObj, [["op", "filter"], ["col", this.oldHeaderSelected]]);

        if (this.tableInfo.output.columns.types[this.oldHeaderSelectedNum] == "string" && newType.value!= " ")
        {
            clr = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/stringColorationBtn/stringFiltcolor");
            var negation="", regexp="";
            var neg = "=";
            negation = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/stringColorationBtn/stringColorationNegationCheck");
            regexp = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/stringColorationBtn/stringColorationRegCheck");

            if (negation.value)
                neg = "!";

            if (regexp.value)
            {
                toPush = { col: this.oldHeaderSelected, op: "rowcategory", arg: { name: "_color_"+clr+"_"+this.oldHeaderSelected, value: "${"+this.oldHeaderSelected+"}"+newType.value, method: "formula"}, group:"row", color: ""+clr.value, panel: "always"};
                toPrintStr += " on regular expression " + newType.value;
            }
            else
            {
                toPush = { col: this.oldHeaderSelected, op: "rowcategory", arg: { name: "_color_"+clr+"_"+this.oldHeaderSelected, value: "${"+this.oldHeaderSelected+"}"+neg+"~/"+newType.value+"/i", method: "formula"}, group:"row", color: ""+clr.value, panel: "always"};
                toPrintStr += " based on key " + (negation.value ? "not" : "") + " " + newType.value;
            }
        }
        else
        {
            clr = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/intColoration/intFiltcolor");

            var fromInt = parseFloat (from.value);
            var toInt = parseFloat (to.value);

            if (!(!toInt) && !(!fromInt))
            {
                toPush = { col: this.oldHeaderSelected, op: "rowcategory", arg: { name: "_color_"+clr+"_"+this.oldHeaderSelected, value: "${"+this.oldHeaderSelected+"}>="+fromInt+" && ${"+this.oldHeaderSelected+"}<="+toInt, method:"formula"}, group:"row", color: ""+clr.value, panel: "always"};
                toPrintStr += " from " + fromInt + " to " + toInt;
            }
            else if (!toInt && !fromInt)
                return;
            else if (!fromInt)
            {
                toPush = {col: this.oldHeaderSelected,  op: "rowcategory", arg: { name: "_color_"+clr+"_"+this.oldHeaderSelected, value: "${"+this.oldHeaderSelected+"}<="+toInt, method:"formula"}, group:"row", color: ""+clr.value, panel: "always"};
                toPrintStr += " up to " + toInt;
            }
            else
            {
                toPush = { col: this.oldHeaderSelected, op: "rowcategory", arg: { name: "_color_"+clr+"_"+this.oldHeaderSelected, value: "${"+this.oldHeaderSelected+"}>="+fromInt, method:"formula"}, group:"row", color: ""+clr.value, panel: "always"};
                toPrintStr += " from " + fromInt;
            }
        }
        toPush.toPrint = toPrintStr;
        if (oldObj.length > 0)
            this.tableViewer.tqsObj[oldObj[0]] = toPush;
        else
            this.tableViewer.tqsObj.push (toPush);
    };

    //will add the appropriate tqs command in order to delete a certain column in the table
    this.onDeleteColumn=function(viewerCls)
    {
        //add a proper option to the tqs, with the proper column name

        tempViewer = vjObj.find(viewerCls);
        var colName;
        var ds=new vjDataSource();
        var query;
        //colName = viewer.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/rename");
        

        this.tableViewer.tqsObj.push({ op: "hidecol", arg: {col: {name: this.oldHeaderSelected}}, toPrint: "Deleted column " + this.oldHeaderSelected});
    };

    //will rename a header of some column
    this.onRenameHeader=function(controlCls)
    {
        //add proper operation here, to rename the current column

        var control = vjObj.find(controlCls);
        //control.tableViewer.tblArr.hdr
        var rename;
        rename = this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/rename");

        if ((this.oldHeaderSelected == undefined) || !rename.value || rename.value == this.oldHeaderSelected || rename.value.indexOf(" ") == 0)
            return;

        this.tableViewer.tqsObj.push({ op: "renamecol", arg: { col: {name: this.oldHeaderSelected}, to: rename.value }, toPrint: "Renamed column " + this.oldHeaderSelected + " to " + rename.value});
    };

    //will add a new column to the table based on the formula specified
    this.onAddNewColumn = function (viewerCls)
    {
        tempViewer = vjObj.find(viewerCls);
        var colnode, fomulaNode, colType="";
        colnode = this.tableViewer.actualBigPanel.plugins["newcontrol"].tree.findByPath("/newname");
        formulaNode = this.tableViewer.actualBigPanel.plugins["newcontrol"].tree.findByPath("/newformula");
        //colType = this.tableViewer.actualBigPanel.plugins["newcontrol"].tree.findByPath("/colType");

        this.tableViewer.tqsObj.push({ op: "appendcol", arg: { name: colnode.value, formula: formulaNode.value }, toPrint: "Added new column " + "\"" + colnode.value + "\" based on formula " + formulaNode.value});
    };

    //overrides on click cell. if a header is selected, them the filtering panel will be opened
    this.onClickCell=function(viewer, node, ir, ic)
    {
        if(ir<0)
        {
            this.oldHeaderSelected=node.name;
            this.oldHeaderSelectedNum = ic;

            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "filterOnCol", "title", "Working with column " + node.name);
            this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "colType", "hidden", false);

            //here might need to change what is actually written in the text boxes of what is selected

            if (this.tableInfo.output.columns.types[ic] == "string" )
            {
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpression", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionBtn", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColoration", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationBtn", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringFiltcolor", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringFiltcolor", "readonly", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionNegationCheck", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionRegCheck", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionCaseSens", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationNegationCheck", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationRegCheck", "hidden", false);
                //this.editColumnControl.tree.findByPath("/intensity").readonly=false;

                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToExpression", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromExpression", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intExpression", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intColoration", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToColoration", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromColoration", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intExclusiveColor", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFiltcolor", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intExclusiveExpression", "hidden", true);
            }
            else if ((this.tableInfo.output.columns.types[ic] == "real" || this.tableInfo.output.columns.types[ic] == "integer"))
            {
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToExpression", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromExpression", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intExpression", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intColoration", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intToColoration", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFromColoration", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intExclusiveColor", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intExclusiveExpression", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFiltcolor", "hidden", false);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "intFiltcolor", "readonly", false);
                //this.editColumnControl.tree.findByPath("/intensity").hidden=false;

                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpression", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionBtn", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColoration", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationBtn", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringFiltcolor", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionNegationCheck", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionRegCheck", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringExpressionCaseSens", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationNegationCheck", "hidden", true);
                this.tableViewer.actualBigPanel.applyChangeRow ("editcontrol", "stringColorationRegCheck", "hidden", true);
                //this.editColumnControl.tree.findByPath("/intensity").readonly=true;
            }

            this.tableViewer.actualBigPanel.plugins["editcontrol"].rows[2].value=node.name;
            if(this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/rename"))
                this.tableViewer.actualBigPanel.plugins["editcontrol"].tree.findByPath("/rename").value = node.name;

            this.tableViewer.actualBigPanel.rebuildTree(this.tableViewer.actualBigPanel,this.tableViewer.actualBigPanel, "",new vjTable(null, 0, vjTable_propCSV),  "editcontrol");
            this.tableViewer.actualBigPanel.onPanelOpen(viewer, "editcontrol");
        }

        //will also update the total number of rows and columns currently selected under this pannel
        if (!(!this.selections.tree))
        {
            this.selections.tree.findByPath("/rows").title = this.tableViewer.rowsTotalSelected + " rows currently selected";
            this.selections.tree.findByPath("/cols").title = this.tableViewer.colsTotalSelected + " columns currently selected";
            this.selections.refresh();
        }
        
        if (this.onClickCellCallback)
            this.onClickCellCallback(viewer, node, ir, ic);
    };
    
    //creates the panel with total number of rows and columns selected
    this.selections= new vjPanelView({
        data: ["dsVoid", viewer.data ],
        iconSize: 16,
        showTitles:true,
        name:'allSelections',
        rows:[              
              {name:'tRows', title:'0 total rows', description: 'rows selected', align: "left"},
              {name:'tCols', title:'0 total columns', description: 'columns selected', align: "left"},                            
              {name:'rows', title:'0 rows currently selected', description: 'rows selected', align: "right"},
              {name:'cols', title:'0 columns currently selected', description: 'columns selected', align: "right"}
              ],
        formObject: viewer.formObject
    });


    //add all of the specified fields to the columns specified (by whoever creates this controller)
   
    this.arrayPanels=this.arrayPanels.concat([this.selections, this.tableViewer]);
    //return this.arrayPanels.concat([this.selections, this.graph, this.tableViewer]);
}

//# sourceURL = getBaseUrl() + "/js/vjTableViewX2.js"