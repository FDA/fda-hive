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
 sampleTableView={

 checkable
 newSectionWord: appearance of these words in the first column makes them to be rendered as th ... sections

 className: 'DV_table',
 startAutoNumber: '',
 skipRows: 0,
 prefixHTML: 'text', // prefix text for table
 appendHTML: 'text', // psotfix tex for the table
 allowVariableReplacement: false, in the values
 appendCols : [{header: name, cell: text }]
 clickLink: javascript: or url
 exclusionObjRegex: {file-name: null, id: /![0-9]/g}
 iconSize: 24
 defaultIcon : 'rec'
 this: "dsMenuAct",
 defaultEmptyText: "" ,   //      'no element to show'
 maxTxtLen: 32,        // max number of text will be shown in a cell, the rest is interpreted by ...
 isStickyHeader:true    //puts the header into seperate div (and table) so scrolling doesn't affect header.
 isReOrderable : false  //columns can be reordered

 cols:[   // serial number of the column to be customized
      { name: name of the column to be customized,
        link: url or javascript,
        align: right | left,
        type: largenumber, percent,
        wrap: true|false,
        hidden: true|false
      }],

 rows:[ { //serial number of the row to customized
       checked,
       styleColor,
       styleNoCheckmark,
       url: url or javascript
       }]
  selectCallback: function,         //   "function:vjObjFunc('onSelectedItem','" + this.objCls + "')"
  checkCallback: function,          //   "function:vjObjFunc('onCheckedItem','" + this.objCls + "')"
  callbackRendered: function,       //    "function:vjObjFunc('onLoadedItem','" + this.objCls + "')"
  precompute: "if(node.id==0 || node.id=='+')node.styleNoCheckmark=true;",
 };



 */

function vjTableView(viewer) {

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
    if(!this.cellColorCallback)
        this.cellColorCallback=0;
    if(this.checkIndeterminateTitle === undefined)
        this.checkIndeterminateTitle = "Partially selected";
    if(this.checkIndeterminateHeaderTitle === undefined)
        this.checkIndeterminateHeaderTitle = "Partially selected; click to select all";
    if(this.overflowAuto === undefined)
        this.overflowAuto = true; // do we use overflow:auto in table's outer div? True by default, but must be set to false if e.g. inside overflow-auto container managed by jquery

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
    if(!this.dblClickDelay) this.dblClickDelay=250;
    // if (!this.rowSpan) this.rowSpan = 1;
    // if(!this.exclusionObjRegex)this.exclusionObjRegex=new Array();
    // if(!this.txtlen)this.txtlen=32;
    // if(!this.actionColumn)this.actionColumn="_action";
    // if(!this.popupMenuName) this.popupMenuName="dvMenuPopup";

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
    // alert(this.exclusionObjRegex.length)

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

    this.defaultInitTblArr = function(content, parsemode) {
        this.tblArr = new vjTable(content, 0, parsemode);
        if (!(parsemode & vjTable_hasHeader)) {
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
    };

    /* this.initTblArr must initialize this.tblArr to a vjTable */
    if (!this.initTblArr) {
        this.initTblArr = this.defaultInitTblArr;
    }

    this.composerFunction = function(viewer, content) {

        // if(this.debug)
        // alert(this.data+"\n"+content);
        // use only data[0] for session id checks - matters for table viewers using additional non-tabular data sources (e.g. sharing_tmplt)
        var t_sessionID = this.getDataSessionID(0);
        if (!(this.data && t_sessionID && t_sessionID == this.contentID && this.tblArr)) {
            this.initTblArr(content, this.parsemode);
            // this.myTable = new vjTable(vjDS["xxxxx"], 0, true);
            this.contentID = t_sessionID;

            // if(this.debug)this.enumerate("alerJ('777 '+node._type,node)");
            if (!this.tblArr) {
                alert("DEVELOPER ALERT: vjTableView: custom initTblArr() callback failed to create tblArr")
                return;
            }

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
                    alert("DEVELOPER ALERT: vjTableView: invalid header field in appendCols");

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
                        // alert(vv);
                    }
                    this.row(ir)[this.appendCols[ic].header.name] = vv;
                    this.row(ir).cols[this.tblArr.hdr.length - 1] = vv;
                }
            }

            this.tblArr.viewer=this;
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
                this.enumerate("if(node.treenode){node.order=node.treenode.order;params.setNodeExpandState.call(params,node);}if(node.tree_duplicate){node.order=node.tree_duplicate.order;}", this);

                this.tblArr.sortTbl(0,false,this.sortTreeColumnFunc);
                if (this.treePrecompute)
                    this.tree.enumerate(this.treePrecompute, this);
            }

            // apply styles to rows
            /*
             * for( var ir=0 ; ir< this.dim() ; ++ir) { var
             * row=this.tblArr.rows[ir]; if(this.bgColors &&
             * this.bgColors.length)
             * row.styleColor=this.bgColors[ir%this.bgColors.length]; }
             */
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
        }
        this.tblArr.syncCols();
        this.refresh();
    };

    this.refresh = function() {
        if (!this.tblArr || !this.tblArr.rows) return;

        var tbl_Div=gObject(this.container+"_table_div");
        if(tbl_Div){
            this.last_scroll = { left : tbl_Div.scrollLeft, top : tbl_Div.scrollTop };
        }

        if(!this.div){
            var containerDiv = gObject(this.container);
            containerDiv.innerHTML = "<div id=" + this.container + "_table_div> </div>";
            this.div = gObject(this.container+"_table_div");
        }
            
        this.div.innerHTML = this.generateTableViewText(this.tblArr);
        this.updateCheckmarks();
        if(this.isStickyHeader){
            this._reHeadered = false;
            this.stickyHeader();
        }

        tbl_Div=gObject(this.container+"_table_div");
        if(tbl_Div && this.last_scroll){
            tbl_Div.scrollLeft = this.last_scroll.left;
            tbl_Div.scrollTop = this.last_scroll.top;
        }

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

        var ttHeight= __getCurrentComputedStyle(divCont, 'height');
        if(!Int(ttHeight)) {
            ttHeight= __getCurrentComputedStyle(divCont, 'max-height');
        }
        if(!Int(ttHeight))return;
        var tblHeight= __getCurrentComputedStyle(tbl, 'height');
        var isVertBar=parseInt(ttHeight)>=parseInt(tblHeight)?false:true;
        if(!isVertBar)return; //no need to create sticky headers
        var ttWidth= __getCurrentComputedStyle(divCont, 'width');
        var tblWidth= __getCurrentComputedStyle(tbl, 'width');
        var isHorizBar=parseInt(ttWidth)>=parseInt(tblWidth)?false:true;

        var thHeight=__getCurrentComputedStyle(thead, 'height');
        var tbl_Width = parseInt(__getCurrentComputedStyle(tbl, 'width'));
        var tbl_Div_Width = parseInt(__getCurrentComputedStyle(tbl_Div, 'width'));

        if(!isok(ths) || !isok(tds))return ;
        if(ths.length > 0) {
            for(var i=0; i < ths.length; i++) {
                if(!isok(tds[i])) continue;
                var td_border_right_width = parseFloat(__getCurrentComputedStyle(tds[i], 'border-right-width'));
                td_border_right_width = td_border_right_width?td_border_right_width:0;
                var td_border_left_width = parseFloat(__getCurrentComputedStyle(tds[i], 'border-left-width'));
                td_border_left_width = td_border_left_width?td_border_left_width:0;
                var border=parseFloat(__getCurrentComputedStyle(ths[i], 'border-right-width'))+parseFloat(__getCurrentComputedStyle(ths[i], 'border-left-width'))
                -td_border_left_width-td_border_right_width;
                var t_td_width= __getCurrentComputedStyle(tds[i], 'width');
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

        // if(tbl.rows.length==0)return;
        // /var arr=tbl.arr;

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



            tt+="<div id='"+this.container+"_table_div' style='";
            if (this.overflowAuto) tt += "overflow:auto";
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
                tt += "<input id='"
                        + prfx
                        + "all' type=checkbox "
                if (this.isNCheckableHeader) {
                    tt += "disabled ";
                } else {
                    tt += "onClick='vjObjEvent(\"onCheckmarkSelected\", \"" + this.objCls + "\",this,-1);'";
                }
                tt += " >";
                tt += "</th>";
            }
            if (this.iconSize) {
                tt += "<th ";
                tt += " id='" + this.container + "_header_icon' ";
                tt += " onMouseEnter='gShowResizer(\""+this.container + "_header_icon\",null,\"r\",\"javascript:vjObj."+this.objCls+".resizeBody(args)\");' ";
                tt += "></th>";
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
                tt += " id='" + this.container + "_header_" + ic + "' ";
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
                    //tt+="</td><td>";
                    //withSortIcon=true;
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
        /*
         * tt+="</table>";
         *
         * tt+="<div style='height:200px; overflow:auto;'>"; tt+="<table "+cls +" ";
         * tt+=(this.geometry && this.geometry.width ?
         * "width='"+this.geometry.width+"'" : "" )+" "; tt+=(this.geometry &&
         * this.geometry.height ? "height='"+(this.geometry.height)+"'" : "" );
         * //tt+=" style='overflow:auto;'" tt+=" >";
         */

        // -----------------------------
        // Body
         tt+="<tbody>";// style='overflow:scroll;'>";
        this.checkedCnt = 0;
        //delete this.tree;
        var allRowsCount=this.dim();//this.tree ? this.tree.rows.length : tbl.rows.length;

        //var allRowsCount=tbl.rows.length;
        for ( var ir = this.skipRows,ivis=-1; ir < allRowsCount ; ++ir) {

            var row=this.row(ir,true);
            if(!row) {
                continue;
            }

            // var row = tbl.rows[ir];
            if(row.tree_duplicate)continue; // because it is the duplicate, dont need to create a new node
            if(row.treenode ) {
                if( row.treenode.parent && !(row.treenode.parent.expanded) ) {
                    row.treenode.expanded=0;
                    continue;
                }
                if( row.treenode.depth == 0 && !this.showRoot ) {
                    continue;
                }
            }
            if (row.hidden)
                continue;

            ++ivis;
            
            var cellColorer;
            // if( matchObj( row, this.exclusionObjRegex ) ) continue;
            var cc_drawn=0;
            var tag = "td";
            var isNewSection = false;
            // alerJ("row[0]="+row[0]+"/",row)
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
                    tt += "bgcolor=" + cellColorer[(!this.rowSpan?ivis:(ivis/this.rowSpan)) % cellColorer.length];
                tt += " width='1' ";
                tt += " id='" + this.container + "_" + ir + "_checkbox' ";

                tt += " >";

                if (this.rowSpan && spRow != Math.round(this.rowSpan / 2) - 1)
                    row.styleNoCheckmark = true;

                if (!row.styleNoCheckmark) {
                    tt += "<input id='" + (prfx + (ir)) + "' type=checkbox ";
                    if (row.checked) {
                        tt += "checked ";
                        ++this.checkedCnt;
                    }
                    if (!this.checkDisabled && !row.checkDisabled)
                        tt += " onClick='vjObjEvent(\"onCheckmarkSelected\", \""
                                + this.objCls + "\",this," + ir + ");' ";
                    if (this.checkDisabled || row.checkDisabled)
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
                        tt += " bgcolor=" + cellColorer[ivis % cellColorer.length];
                }

                tt += " id='" + this.container + "_" + ir + "_icon' ";

                if (!isNewSection) {
                    tt += " onClick='vjObjEvent(\"onDomClickCell\", \"" + this.objCls
                        + "\"," + ir + ",-1);' style='cursor:pointer;' ";
                }

                tt += " >";
                if (icon && !isNewSection)
                    tt += this.formDataValue(icon, "icon", row, null); // tt+="<img
                                                                        // src='img/"+icon+".gif'
                                                                        // width="+this.iconSize+"
                                                                        // />";
                tt += "</" + tag + ">";
                this.icon_ColumnIndex = cc_drawn;
            }

            // autonumbering
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
            // alerJ('c);
            for ( var iCC = 0; iCC < maxCols; ++iCC) {

                ic = isok(this.sortIndex) ? this.sortIndex[iCC].idx : iCC;

                var col = tbl.hdr[ic];


                cellColorer = this["col" + ic + "Colors"];
                if (!cellColorer && ic < tbl.hdr.length)
                    cellColorer = this["col" + tbl.hdr[ic].name + "Colors"];
                if (!cellColorer)
                    cellColorer = this.bgColors;

                if (tbl.hdr[ic].hidden)
                    continue;

                //if(this.debug)
                //    alerJ(ic,row);

                var realText = ic < row.cols.length ? row.cols[ic] : null;
                //if(tbl.hdr[ic].name=="path")
                //    alerJ("col="+tbl.hdr[ic].name+"\\ "+realText,row);


                if (!realText)
                    realText = "";
                if (realText instanceof Array)
                    realText = realText.join(",");
                var tooltip = this.getToolTip(row, ic, this.formDataValue(realText, col.type, row, col, {ellipsize: false, textonly: true, datetime:{absoluteOnly:true,forceDate:true,forceTime:true}}));
                var dataValueText = this.formDataValue(realText, col.type, row, col);
                //if(this.debug && col.type=='timespan')
                //    alerJ('hhh '+dataValueText,row);
                if (this.cellColorCallback){
                    row.bgcolor = funcLink(this.cellColorCallback,viewer,dataValueText, col, row.cols[ic]);
                }

                tt += "<"
                        + tag
                        + " ";
                if (tbl.hdr[ic].size) {
                    tt += "width=" + tbl.hdr[ic].size + " ";
                }
                tt += (row.bgcolor ? "bgcolor='" + row.bgcolor+"'" : (cellColorer ? "bgcolor=" + cellColorer[(this.rowSpan?(Int(ivis/this.rowSpan)):ivis) % cellColorer.length] : ""))
                        + " " + (col.align ? "align=" + col.align : "") + " ";
                // tt+="<"+tag+" "+ ( "" ) +" "+( col.align ? "align="+col.align
                // : "") +" ";
                tt += " id='" + this.container + "_" + ir + "_" + ic + "' ";
                tt += row.selected ? (" class='" + this.className + "_selected"
                        + row.selected + "'") : clsCell;

                // if( isok(row.url) || isok(col.url) ||
                // isok(this.selectCallback) )
                tt += " ondblclick='vjObjEvent(\"onDomDBLClickCell\", \"" + this.objCls
                + "\"," + ir + "," + ic
                + ");' ";
                tt += " onClick='vjObjEvent(\"onDomClickCell\", \"" + this.objCls
                        + "\"," + ir + "," + ic
                        + ");' style='cursor:pointer;' ";
                tt += " onMouseOver='vjObjEvent(\"onMouseOver\", \""
                        + this.objCls + "\"," + ir + "," + ic + ");' ";
                tt += " onMouseOut='vjObjEvent(\"onMouseOut\", \""
                        + this.objCls + "\"," + ir + "," + ic + ",event);' ";

//                if(this.drag) {
//                    if(!compatibility.draganddrop) {
//                        tt+=" draggable=\"true\"";
//                        //tt+= " onmousedown = 'vjObjEvent(\"onSetListeners\",\""+this.objCls+"\","+ir+","+ic+",event);'";
//                    }
//                    else {
//                        tt+" style='position:fixed'; position=fixed";
//                        tt+= " onmousedown = 'vjObjEvent(\"onmousedown\",\""+this.objCls+"\","+ir+","+ic+",event);'";
//                        tt+= " onmouseup = 'vjObjEvent(\"onmouseup\",\""+this.objCls+"\","+ir+","+ic+",event);'";
//                    }
//                }
                // alert(tt);



                if(tooltip && tooltip.length && !this.forceEmptyTooltip)
                    tt += " title='" + tooltip+"' ";
                // if(row.description)tt+=": "+row.description;
                tt += " >";

                if (!col.wrap)
                    tt += "<span style='white-space:nowrap;' >";

                // var maxlen=(col.maxTxtLen ? col.maxTxtLen : this.maxTxtLen);
                // if(maxlen && dataValueText.length>maxlen &&
                // dataValueText.indexOf(">")==-1 &&dataValueText.indexOf("<")==-1 ){
                // //alert(dataValueText+ "----"+dataValueText.substr(0,maxlen)+"...");
                // dataValueText=dataValueText.substr(0,maxlen)+"...";
                // }
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

    /*
     * this.adjustTableHeaderSize=function () { var nms=new Array ( "checkbox",
     * "autonumber", "icon" ); //var nms=new Array ( "autonumber"); var
     * maxCols=this.row(0).cols.length;
     * if(this.tblArr.hdr.length>maxCols) maxCols=this.tblArr.hdr.length;
     *
     * for( var it=0; it<1; ++it) {
     *
     * var Pos=new Array(); var totWidth=0; for ( var ic=0; ic<
     * nms.length+maxCols ; ++ic) { var col=(ic)<nms.length ? nms[ ic ] :
     * (ic-nms.length); var
     * s=gObject(this.container+"_"+this.skipRows+"_"+col);if(!s)continue; var
     * d=gObject(this.container+"_header_"+col);if(!d)continue; if(it%2){var
     * t=s;s=d;d=t;} var src=gObjPos(s); totWidth+=src.cx-2; Pos.push( { cx:
     * src.cx, x: src.x, s:s, d: d } ); }
     *
     * var allW=Pos[Pos.length-1].cx+Pos[Pos.length-1].x; for ( var ic=0; ic<Pos.length;
     * ++ic) { var maxW=parseInt(Pos[ic].cx*allW/totWidth);
     * Pos[ic].d.style.width=(maxW)+"px"; //var
     * maxW=parseInt(Pos[ic].cx*100/totWidth);
     * //Pos[ic].d.style.width=(maxW)+"%"; } }
     *  }
     */
    // _/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Table viewer actions
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/
    /*
     * this.doShowActions =function( whichone ,webelement ) { var from=
     * (whichone=="checked") ? 0 : whichone; var to= (whichone=="checked") ?
     * this.dim() : whichone;
     *
     *
     * var actLst=new Object(), allCnt=0, act; for ( var ir=from; ir<to; ++ir) {
     * ++allCnt; if(whichone=="checked" && !this.row(ir).checked )
     * continue;
     *
     * var actL=verarr(this.tblArr.rows[ir][this.actionColumn]); for ( var ia=0;
     * ia< actL.length; ++ia){ if(!actLst[actL[ia]])actLst[actL[ia]]=1; else
     * ++actLst[actL[ia]]; } }
     *
     * var t="",ic=0; for ( act in actLst) { //if(actLst[act]!=allCnt)continue;
     * t+="/"+act+","+act+","+actLst[act]+"\n" } if(!t.length) return ;
     *
     * t="path,url,act\n"+t;
     *
     * //vjMenuPopup(this,t,gObjPos(webelement)); return t;
     *  }
     */

    this.getToolTip = function (node, ic,defaultVal) {
        var col = this.tblArr.hdr[ic];
        var tooltip = "";
        if( this.tooltip ) {
            tooltip=funcLink(this.tooltip,this,node,ic);
        }
        else if ( col && col.tooltip ) {
            tooltip=funcLink(col.tooltip,this,node,ic);
        }
        else if( defaultVal )
            tooltip = textContent(defaultVal);
        if(tooltip==node.cols[ic] && !this.forceRepeatedTooltip){
            tooltip="";
        }
        return sanitizeElementAttr(tooltip);
    };

    this.row = function ( ir , givetreenode ) {
        return this.tblArr.rows[ir];
        /*
        var row;
        if( this.tree && this.tree.rows) {
            var row=this.tree.rows[ir];
            if( row && row.tablenode && givetreenode )
                return row.tablenode;
            return row;
        }
        else return this.tblArr.rows[ir];
        */
        //return this.tblArr.rows[ir];
    };

    this.dim = function() {
        //return ( this.tree && this.tree.rows) ? this.tree.rows.length : this.tblArr.rows.length;
        return this.tblArr.rows.length;
    };

    this.enumerate = function(operation, params) {
        if (!this.tblArr || this.tblArr.length == 0)
            return;
        return this.tblArr.enumerate(operation, params);
    };

    this.accumulate = function(checker, collector, params, checked) {
        return this.tblArr.accumulate(checker, collector, params, checked);
    };


    this.doSelect = function(ir, renderAll) {
        if (ir < 0)
            return false;
//        var previousCnt = this.selectedCnt;
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
            } else if (cursel == -1 && !this.row(is).checkDisabled) {
                this.formCheckSwitchTriState(prfx + is, ischecked, true, this.checkIndeterminateTitle);
            }

            if (o.checked) {
                ++cntchk;
            }
        }
        // alert(cntchk + "=="+this.dim()+"///" + cursel )

        if (gObject(prfx + "all")) {
            var all_state = ischecked;
            if (this.dim()) {
                if (cntchk == this.dim()) {
                    all_state = 1;
                } else if (cntchk) {
                    all_state = -1;
                } else {
                    all_state = 0;
                }
            }
            this.formCheckSwitchTriState(prfx + "all", all_state, false, this.checkIndeterminateHeaderTitle);
        }

        if (cursel !== null && cursel !== undefined && cursel != -1)
            this.formCheckSwitchTriState(prfx + cursel,
                    (ischecked ? true : false), true,
                    this.checkIndeterminateTitle);
    };

    this.onCheckmarkSelected = function(container, thisobject, cursel) {
        var elems = this.formObject.elements;
        var prfx = this.container + "_check_";
        var checkbox_elem = elems[prfx + (cursel == -1 ? "all" : cursel)];

        var ischecked = checkbox_elem.checked;
        if (ischecked && cursel == -1) {
            // if clicking on header checkbox, and before click it was (or should have been) indeterminate,
            // and all checkable rows are already checked, interpret this action as "uncheck all".
            var cnt_checkable_unchecked = 0;
            var cnt_checked = 0;
            for (var is = 0; is < this.dim(); ++is) {
                var o = gObject(prfx + is);
                if (!o)
                    continue;
                if (o.checked) {
                    cnt_checked++;
                }
                if (!this.row(is).checkDisabled && !o.checked) {
                    cnt_checkable_unchecked++;
                }
            }
            if (cnt_checkable_unchecked == 0 && cnt_checked > 0 && cnt_checked < this.dim()) {
                ischecked = false;
            }
        }

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
        // else this.doShowActions( "checked" , thisobject ) ;
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

    this.onDomClickCell = function(container, ir, ic) {
        var node = this.row(ir);
        that=this;
        if( node.dClickTimer ) {
            clearTimeout( node.dClickTimer );
            delete node.dClickTimer;
        }
        if( node.sClickTimer ) {
            clearTimeout( node.sClickTimer );
            delete node.sClickTimer;
        }
        node.sClickTimer = setTimeout(function() { that.onClickCell.call(that,container,ir,ic); }, this.dblClickDelay); 
    };

    this.onDomDBLClickCell = function(container, ir, ic) {
        var node = this.row(ir);
        that=this;
        if( node.dClickTimer ) {
            clearTimeout( node.dClickTimer );
            delete node.dClickTimer;
        }
        if( node.sClickTimer ) {
            clearTimeout( node.sClickTimer );
            delete node.sClickTimer;
        }
        var tnow=new Date();
        if( node.lastSClick && (tnow.getTime()-node.lastSClick.getTime()) > 2000 )delete node.lastSClick;
        if( !node.lastSClick || (node.lastSClick && (tnow.getTime()-node.lastSClick.getTime()) < this.dblClickDelay) ){
            node.dClickTimer = setTimeout(function() { that.ondbClickCell.call(that,container,ir,ic); }, this.dblClickDelay); 
        }
    };
    
    this.onClickCell = function(container, ir, ic)
    {

        if(gDragEvent){
            gDragEvent=false;
            return ;
        }
        var node = this.row(ir);
        node.lastSClick=new Date();
//        var isSelected=true;
        if (this.rowSpan)
            ir = ir - ((ir - this.skipRows) % this.rowSpan);
        if (!this.userCannotSelect)
            /*isSelected=*/this.doSelect(ir, true);

//        if(!isSelected)return;

        if(this.selectInsteadOfCheck){
            this.enumerate("if(node.selected)node.checked=1;else node.checked=0;");
            this.updateDependents();
        }


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
    
    this.ondbClickCell = function(container, ir, ic) {
        var node = this.row(ir);
        var func = this.dbClickCaptureCallback;
        if (!func && node.dbc_url && !this.notOpendbc_Url)
            func = node.dbc_url;
        if (!func && ic >= 0 && this.tblArr.hdr[ic].dbc_url
                && !this.notOpendbc_Url)
            func = this.tblArr.hdr[ic].dbc_url;
        if (!func && this.dbClickCallback)
            func = this.dbClickCallback;

        funcLink(func, this, node, ir, ic, ic >= 0 ? this.tblArr.hdr[ic] : ic);
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
        if(!node) return ;
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

        this.drag.dropableAttrs = this.dropableAttrs;//new Array('dragDropable');
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

        this.order_drag.dropableAttrs = this.order_dropableAttrs;//new Array('dragDropable');
        this.order_drag.dropOnAttrs = this.order_dropOnAttrs;
        this.order_drag.setClonedObject={func:this.setClonedObject,obj:this};

//        if(this.order_dropableAttrs) drableAttrs=verarr(this.order_dropableAttrs);
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

        //if (o)
        o.className = this.className + "_highlighted";

        if(this.mouseOverCallback){
            //this.mouseOverCallback(this,o,ir,ic);
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
        // alerJ((node.selected ? this.className+"_selected" :
        // this.className+"_cell" )+ ir+"/"+ic,node);
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
//        hdcol.sorted=true;

        this.tblArr.sortTbl(tric, hdcol.isNdesc);
        this.refresh();
    };

    this.onClickMenuNode = function(container, node, menuobject) {
        alert("requested to " + node.name);
    };

    this.enableTableSorter = function (){
        if (verarr(this.data).length != 1) {
            return 0;
        }
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

}

/*

function vjTableMobileView(viewer) {

    vjTableView.call(this, viewer); // inherit default behaviours of the
    this.generateTableViewText=function(tbl)
    {

        // row.cols ass an array
        // tbl.hdr - array of headers
        // tbl.hdr[i] - particular header cell
        // row.cols[i] - particular cell value

        for ( var ih = 0 ; ih< tbl.hdr.length; ++ih ) {
            var hdr=tbl.hdr[ih];

        }

        for ( var ir = 0 ; ir< this.dim(); ++ir ) {
            var row=this.row(i);

            for ( var ic = 0 ; ic< row.cols.length; ++ic ) {
                var cell=row.cols[ic];

            }
        }
    }
}
*/

//# sourceURL = getBaseUrl() + "/js/vjTableView.js"