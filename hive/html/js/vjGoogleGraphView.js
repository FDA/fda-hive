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

function vjGoogleGraphView( viewer )
{

    vjTableView.call(this,viewer);
    this.refreshAsTable=this.refresh;
    this.originalSeries = [];

    if(this.parsemode === undefined)
        this.parsemode = vjTable_propCSV;
    if(this.cntHeaderRows === undefined)
        this.cntHeaderRows = 1;

    this.switchToColumnMode=30;
    this.switchToTextMode=2;
    if(!this.type)this.type='scatter';
    if (!this.scaleTo) this.scaleTo = 0;
    this.delayedEventHandler=1;

    this.exclusionObjRegex = verarr(this.exclusionObjRegex);
    if (this.exclusionObjRegex.length == 0) {
        this.exclusionObjRegex.push({ info: /^info*/ });
        this.exclusionObjRegex.push({ id: /^info*/ });
    }
    if(this.maxRowsToAvoidRendering==undefined)
        this.maxRowsToAvoidRendering=4;

    if (!this.downloadLinkText) {
        this.downloadLinkText = "Download graph as SVG file";
    }
    if (this.downloadLinkManage === undefined) {
        this.downloadLinkManage = true;
    }
    
    if (viewer.nonSticky === undefined) this.nonSticky = false;

    this.defaultInitTblArr = function(content, parsemode) {
        this.tblArr = new vjTable(content, 0, parsemode, undefined, undefined, undefined, undefined, this.cntHeaderRows);
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

    if (!this.initTblArr || !viewer.initTblArr) {
        this.initTblArr = this.defaultInitTblArr;
    }

    this.composerFunction=function( viewer , content, ds )
    {
        if(this.debug)
            alert(this.getData(0).url + " =\n" + content);
        if (!content && ds && (typeof ds == "string"))
            content = vjDS[ds].data;
        else if (!content && viewer.data && viewer.data[0])
            content = vjDS[viewer.data[0]].data;
        if(!content || !content.length || content.indexOf("error:")==0) {
            delete this.tblArr;
            delete this.graphData;
            delete this.obj;
            gObject(this.container).innerHTML=content;
            return;
        }

        this.initTblArr(content, this.parsemode);
        if(this.tblArr){
            this.tblArr.mangleRows(this.exclusionObjRegex, "delete");
        }
        
        if (this.rows)
        {
            var tmpRows = [];
            
            for (var i = 0; i < this.tblArr.rows.length; i++)
            {
                if (this.rows.indexOf(i) > -1)
                {
                    tmpRows.push(this.tblArr.rows[i])
                }
            }
            
            this.tblArr.rows = tmpRows;
        }



        if(this.precompute)
            this.tblArr.enumerate(this.precompute);

        if(this.rows)
            this.tblArr.customizeRows(this.rows);


        this.refresh();
    };

    this.refresh = function ()
    {
        if(!this.tblArr || !this.tblArr.rows)return;
        this.minRenderedValue=1e+100;
        this.maxRenderedValue=-1e+100;

        if (this.onRefresh) {
            funcLink(this.onRefresh, this, this.tblArr);
        }

        if(!this.options)
            this.options=new Object();
        if(!this.options.vAxis )
            this.options.vAxis=new Object();

        {
            if(this.logGraph)
                this.options.vAxis.logScale=true;
            else
                this.options.vAxis.logScale=false;
        }



        this.graphData = new google.visualization.DataTable();
        this.graphData2tblArr_row = [];

        var maxRows=this.tblArr.rows.length; if(this.maxRowPerSeries)maxRows=this.maxRowPerSeries;


           var islabel;
           var firstColumnLabel = (this.type== "column-area") ?  (maxRows<this.switchToColumnMode ? true : false) : (this.series && this.series.length > 0) ? this.series[0].label : undefined;



        for ( var ic=0; ic< this.series.length; ++ic ) {
            if(!this.series[ic].name && this.tblArr.hdr && this.tblArr.hdr.length>0){
                this.series[ic].name=this.tblArr.hdr[parseInt(this.series[ic].col)].name;
                this.series[ic].tmpName = true;
            }
        }
        for ( var ic=0; ic< this.series.length; ++ic ) {
            var title=this.series[ic].title ? this.series[ic].title : this.series[ic].name;
            if(title=='none')title='';

            islabel= (ic==0 ? firstColumnLabel : this.series[ic].label);
            var coltype = this.series[ic].type;
            if (!coltype) {
                coltype = islabel ? "string" : "number";
                this.series[ic].type = coltype;
            }
            var role = this.series[ic].role ? this.series[ic].role : "";
            if (role.length > 0 && role != "interval"){
                this.series[ic].type = "string";
                this.graphData.addColumn({
                                type:"string"
                                ,role: role
                                , p: {'html': true}
                });
            }
            else if(role == "interval"){
                this.series[ic].type = "number";
                this.graphData.addColumn({ type:"number", role: "interval", p: {'html': true} , color:"black"});
            }
            else this.graphData.addColumn(coltype, title);
        }

        var iRR=0;
        var cumulator=new Array();
        for ( var ic=0; ic< this.series.length; ++ic ){
            cumulator.push(0.);
        }
         if (this.doHistogram) {
            var tblNew=new Array();;
                var XFirst = new Object;
                for ( var ir=0; ir< maxRows ; ++ir ) {
                    
                    var realXRow=ir;
                    
                    for ( var ic=0; ic< this.series.length; ++ic ){
                        var value;
                        var shiftRow=(this.series[ic].shiftRow) ?this.series[ic].shiftRow : 0 ;
                        if(this.series[ic].eval){
                            var node=this.tblArr.rows[ir+shiftRow];
                            var row=node;
                            value="" + eval(this.series[ic].eval);
        
                        }
                        else value=this.tblArr.rows[ir+shiftRow][this.series[ic].name];
        
                        if(ic==0) {
                            if( XFirst[value]===undefined ){
                                XFirst[value]=ir;
                                realXRow=ir;
                            }
                            else {
                                this.tblArr.rows[ir].hidden=true;
                                realXRow=XFirst[value];
                            }
                        }    
                        else { 
                            if(ir!=XFirst[value])
                                var yValue = this.tblArr.rows[realXRow][this.series[ic].name];
                                yValue = parseFloat(yValue);
                                if (isNumber(yValue)) {
                                    yValue+=parseFloat(value);
                                    this.tblArr.rows[realXRow][this.series[ic].name] = yValue;
                                }
                        }
                    }
                }
                
        }
        
        var totSum=0;
        for (var ir = 0; ir < maxRows; ++ir){
            if(this.scaleTo) {
                for ( var ic=0; ic< this.series.length; ++ic ){
                    islabel= (ic==0 ? firstColumnLabel : this.series[ic].label);
                    if( islabel )continue;
                    if (this.series[ic].type=="string") continue;
                    if(this.series[ic].notScaled)continue;
                    var shiftRow=(this.series[ic].shiftRow) ?this.series[ic].shiftRow : 0 ;
                    totSum+=parseFloat(this.tblArr.rows[ir+shiftRow][this.series[ic].name]);
                }
            }
        }
        
        
        for ( var ir=0; ir< maxRows ; ++ir ) {

            if(this.tblArr.rows[ir].hidden)
                continue;

            var posSum=0;
            if(this.positionalScaleTo) {
                for ( var ic=0; ic< this.series.length; ++ic ){
                    islabel= (ic==0 ? firstColumnLabel : this.series[ic].label);
                    if( islabel )continue;
                    if (this.series[ic].type=="string") continue;
                    if(this.series[ic].notScaled)continue;
                    var shiftRow=(this.series[ic].shiftRow) ?this.series[ic].shiftRow : 0 ;
                    posSum+=parseFloat(this.tblArr.rows[ir+shiftRow][this.series[ic].name]);
                }
            }



            if( this.type== "stepped-area") {
                this.graphData.addRows(1);
                this.graphData.addRows(1);
            } else if (this.type == "stepped-area-right" && iRR) {
                this.graphData.addRows(1);
            }
            this.graphData.addRows(1);





            for ( var ic=0; ic< this.series.length; ++ic ){

                var shiftRow=(this.series[ic].shiftRow) ?this.series[ic].shiftRow : 0 ;
                var value;
                if(this.series[ic].eval){
                    var node=this.tblArr.rows[ir+shiftRow];
                    var row=node;
                    value = eval(this.series[ic].eval);
                    if (typeof value != "number" && !(value instanceof Date)) {
                        value = "" + value;
                    }
                }
                else value=this.tblArr.rows[ir+shiftRow][this.series[ic].name];

                
                if(!value){
                    value= (this.series[ic].type == "number") ? 0 : "";
                }
                if(this.series[ic].type=="number")
                    value=parseFloat(value);

                if(this.scaleTo && (this.series[ic].type=="number") && (!this.series[ic].notScaled) ) {

                    value=totSum ? (this.scaleTo*value)/totSum : 0 ;
                    value=Number(Math.round(parseFloat(value) + "e+2") + "e-2")
                }
                
                if(this.positionalScaleTo && (this.series[ic].type=="number") && (!this.series[ic].notScaled) ) {

                    value=posSum ? (this.positionalScaleTo*value)/posSum : 0 ;
                    value=Number(Math.round(parseFloat(value) + "e+2") + "e-2")
                }
                if(this.doIntegral && ic!=0 && (this.series[ic].type=="number")) { 
                    cumulator[ic]+=value;
                    value=cumulator[ic];
                }
                
                
                    
                var irshift=0;
                if( this.type== "stepped-area") {
                    irshift=1;
                    this.graphData.setValue(iRR, ic , ic ? 0 : value );
                    this.graphData.setValue(iRR+2, ic , ic ? 0 : value );

                    if (!this.graphData2tblArr_row[iRR]) this.graphData2tblArr_row[iRR] = [];
                    this.graphData2tblArr_row[iRR][ic] = ir + shiftRow;
                    if (!this.graphData2tblArr_row[iRR + 2]) this.graphData2tblArr_row[iRR + 2] = [];
                    this.graphData2tblArr_row[iRR + 2][ic] = ir + shiftRow;
                } else if (this.type == "stepped-area-right" && iRR) {
                    irshift = 1;
                    this.graphData.setValue(iRR, ic , ic ? this.graphData.getValue(iRR - 1, ic) : value);
                    this.graphData.setFormattedValue(iRR, ic, "");

                    if (!this.graphData2tblArr_row[iRR]) this.graphData2tblArr_row[iRR] = [];
                    this.graphData2tblArr_row[iRR][ic] = ir + shiftRow;
                }
                
                this.graphData.setValue(iRR+irshift, ic , value);
                if(this.series[ic].formatter) {
                    this.graphData.setFormattedValue(iRR+irshift, ic, this.series[ic].formatter.call(this, value, this.tblArr.rows[ir+shiftRow], ic));
                }
                if(this.series[ic].type=="number" && ic!=0) {
                    if(this.maxRenderedValue<value)this.maxRenderedValue=value;
                    if(this.minRenderedValue>value)this.minRenderedValue=value;
                }

                if (!this.graphData2tblArr_row[iRR+irshift]) this.graphData2tblArr_row[iRR+irshift] = [];
                this.graphData2tblArr_row[iRR+irshift][ic] = ir + shiftRow;
            }

            if( this.type== "stepped-area") {
                iRR+=2;
            } else if( this.type == "stepped-area-right" && iRR) {
                iRR++;
            }
            ++iRR;
        }

        if (this.onGraphDataReady) {
            funcLink(this.onGraphDataReady, this);
        }

        var useType=this.type;
        if( (this.minRowsToAvoidRendering && this.tblArr.rows.length<this.minRowsToAvoidRendering) || (this.minimumChangeAllowed && Math.abs(this.maxRenderedValue-this.minRenderedValue<=this.minimumChangeAllowed ))){
            if (this.cols)
                  this.tblArr.customizeColumns(this.cols);
            this.sortIndex = new Array();

            var maxOrder = 0;
            for ( var is = 0; is < this.tblArr.hdr.length; ++is) {
                if (this.tblArr.hdr[is].order) {
                    if (maxOrder < this.tblArr.hdr[is].order) {
                        maxOrder = this.tblArr.hdr[is].order;
                    }
                }
            }
            for ( var is = 0; is < this.tblArr.hdr.length; ++is) {
                this.sortIndex.push({
                    idx : is,
                    order : (this.tblArr.hdr[is].order ? this.tblArr.hdr[is].order : maxOrder + is),
                    name : this.tblArr.hdr[is].name
                });
            }

            this.sortIndex.sort(function(a, b) {
                return (a.order - b.order);
            });

            if (this.rows)
                this.tblArr.customizeRows(this.rows);
            useType='table';
        }

        for (var ik = 0; ik < this.series.length; ik++)
        {
            if (this.series[ik].tmpName)
                delete this.series[ik].name;                
        }
        
        this.div.innerHTML = "<div></div>"
        if (this.downloadLinkManage) {
            var download_name = this.getTitle() + ".svg";
            this.div.innerHTML += "<a id='"+this.container+"-svg-download' href='#' download='" + sanitizeElementAttr(download_name) + "'>" + "<img width=16 height=16 src=./img/download.gif style='vertical-align:middle;'/>&nbsp;<small>" + this.downloadLinkText + "</small></a>";
            this.downloadLink = gObject(this.container+"-svg-download");
        }

        this.chart_div = this.div.firstChild;
        
        if (this.title) this.options.title = this.title;

        if(this.type== "column-area") {
            if(maxRows<this.switchToColumnMode)this.obj = new google.visualization.ColumnChart(this.chart_div);
            else this.obj  = new google.visualization.AreaChart(this.chart_div);
        }

        if(useType== "column") {
            this.obj = new google.visualization.ColumnChart(this.chart_div);
        }else if(useType== "scatter") {
            this.obj  = new google.visualization.ScatterChart(this.chart_div);
        }else if(useType== "line") {
            this.obj  = new google.visualization.LineChart(this.chart_div);
        }else if(useType== "area") {
            this.obj  = new google.visualization.AreaChart(this.chart_div);
        }else if(useType== "pie") {
            this.obj  = new google.visualization.PieChart(this.chart_div);
        }else if(useType== "candlestick") {
            this.obj  = new google.visualization.CandlestickChart(this.chart_div);
        }else if(useType== "stepped-area") {
            this.obj  = new google.visualization.AreaChart(this.chart_div);
        }else if(useType== "stepped-area-right") {
            this.obj  = new google.visualization.AreaChart(this.chart_div);
        }else if(useType== "map") {
            this.obj  = new google.visualization.Map(this.chart_div);
        } else if (useType == "timeline") {
            this.obj = new google.visualization.Timeline(this.chart_div);
        }
        else if(useType== "table") {
            if (this.downloadLinkManage && this.downloadLink) {
                var a = (typeof(this.downloadLink) === "string") ? gObject(this.downloadLink) : this.downloadLink;
                a.parentElement.removeChild(a);
                this.downloadLink = null;
            }
            this.refreshAsTable();
            return;
        }

        this.obj.draw(this.graphData, this.options);

        if (this.downloadLink) {
            var a = (typeof(this.downloadLink) === "string") ? gObject(this.downloadLink) : this.downloadLink;
            var svg = this.chart_div.getElementsByTagName("svg")[0];
            if (!svg)
                return;
            svg.setAttribute("xmlns", "http:
            svg.setAttribute("version", "1.1");
            svg.setAttribute("xmlns:xlink", "http:
            a.href = "data:image/svg+xml," + encodeURIComponent(svg.parentNode.innerHTML);
        }

        var funcname=vjObjEventGlobalFuncName("onSelectHandler",this.objCls);
        window[funcname]= eval(vjObjEventGlobalFuncBody("onSelectHandler",this.objCls, this.delayedHandler ));
        google.visualization.events.addListener( this.obj , 'select', eval(funcname));
        return;

    };
    
    this.selectCaptureCallback=this.onSelectHandler;
    this.onSelectHandler = function ()
    {
        var irow = -1;
        var icol = -1;
        var node = null;
        var funcCB = null ;
        if (this.obj.getSelection().length != 0) {
            icol = parseInt(this.obj.getSelection()[0].column);
            if (isNaN(icol)) icol = -1;
            irow = this.graphData2tblArr_row[this.obj.getSelection()[0].row][icol >= 0 ? icol : 0];
            node = this.tblArr.rows[irow];
            if (node) {
                funcCB = node.url;
            }
        }

        if(!node)return ;

        if (!funcCB && icol >= 0 && icol < this.tblArr.hdr.lengh) funcCB = this.tblArr.hdr[icol].url;
        if (!funcCB) funcCB = this.selectCallback;

        if (funcCB)
            funcLink(funcCB, this, node, irow);

    };

    var _super_getTitle = this.getTitle;
    this.getTitle = function() {
        if (this.title) {
            return this.title;
        }
        if (this.options && this.options.title) {
            return this.options.title;
        }
        if (this.options && this.options.vAxis && this.options.vAxis.title) {
            var title = this.options.vAxis.title;
            if (this.options && this.options.hAxis && this.options.hAxis.title) {
                title += " vs " + this.options.hAxis.title;
            }
            return title;
        }
        return _super_getTitle.call(this);
    }
}

function vjGoogleGraphPanelView(viewer)
{
    vjPanelView.call(this, viewer);
    this.iconSize=32;
    this.showTitles=true;

    this.rows=verarr(this.rows).concat(
            [{ name: 'scale', icon: 'img/maximize.gif', title: 'scale', path:'/scale', align:'left', description: "Scaling method" }
            ,{ name: 'scalenormal', title: 'normal scale', path:'/scale/normal', align:'left', description: 'Normal scale', url: "javascript:thisObj.graphObject.logGraph=undefined;thisObj.graphObject.refresh();" }
            ,{ name: 'scalelog10', title: 'log 10 scale', path:'/scale/log10', align:'left', description: 'Log 10 scale', url: "javascript:thisObj.graphObject.logGraph=10;thisObj.graphObject.refresh();" }
            ]
        );
    var possibleGraphs=[
        { name: 'column', icon: 'img/graph2.gif', title: 'Column chart', path:'/type/column', align:'left', description: "Column chart",  url: "javascript:thisObj.graphObject.type=args[1].name;thisObj.graphObject.refresh();"}
        ,{ name: 'scatter', icon: 'img/scatter.gif', title: 'Scatter rplot', path:'/type/scatter', align:'left', description: "Scatter plot",  url: "javascript:thisObj.graphObject.type=args[1].name;thisObj.graphObject.refresh(); "}
        ,{ name: 'line', icon: 'img/line.gif', title: 'Line diagram', path:'/type/line', align:'left', description: "Line diagram" ,  url: "javascript:thisObj.graphObject.type=args[1].name;thisObj.graphObject.refresh();"}
        ,{ name: 'area',icon: 'img/area.gif', title: 'Area chart', path:'/type/area', align:'left', description: "Area chart" ,  url: "javascript:thisObj.graphObject.type=args[1].name;thisObj.graphObject.refresh();"}
        ,{ name: 'pie', icon: 'img/pie.gif', title: 'Pie chart', path:'/type/pie', align:'left', description: "Pie chart" ,  url: "javascript:thisObj.graphObject.type=args[1].name;thisObj.graphObject.refresh();"}
        ,{ name: 'map', icon: 'img/map.png', title: 'Map', path:'/type/map', align:'left', description: "Map" ,  url: "javascript:thisObj.graphObject.type=args[1].name;thisObj.graphObject.refresh();"}
        ,{ name: 'candlestick', icon: 'img/graph2.gif', title: 'Candlestick diagram', path:'/type/candlestick', align:'left', description: "Candlestick diagram" ,  url: "javascript:thisObj.graphObject.type=args[1].name;thisObj.graphObject.refresh();"}
        ,{ name: 'stepped-area', icon: 'img/graph2.gif', title: 'Stepped area', path:'/type/stepped-area', align:'left', description: "Stepped area" ,  url: "javascript:thisObj.graphObject.type=args[1].name;thisObj.graphObject.refresh();"}
        ];


    var howmanyadded=0;
    for( var i=0; i<possibleGraphs.length;++i){
        if(viewer.possibleGraphs && viewer.possibleGraphs.indexOf(possibleGraphs[i].name)==-1)
            continue;
        if(howmanyadded==0)
            this.rows.push( { name: 'type', icon: 'img/graph2.gif', title: 'Type', path:'/type', align:'left', description: "Graph type" } );
        this.rows.push(possibleGraphs[i]);
        ++howmanyadded;
    }

    if (viewer.dataHelp) {
        this.rows.push(
        {
            name: "help",
            icon: "img/help.png",
            title: "Help",
            align: "left",
            description: "Graph explanation",
        },
        {
            name: "help_popup",
            path: "/help/popup",
            type: "help",
            dataIndex: viewer.data.indexOf(viewer.dataHelp)
        });
    }
}








function vjGoogleGraphControl(viewer)
{

    this.googleViewer = new vjGoogleGraphView(viewer);
    if(viewer.noToolbar)return  [this.googleViewer];


    this.panelViewer = new vjGoogleGraphPanelView({
        data: viewer.dataHelp ? ["dsVoid", "dsVoid", viewer.dataHelp] : ["dsVoid"],
        width:this.width,
        formObject: viewer.formObject,
        possibleGraphs:viewer.possibleGraphs,
        graphObject:this.googleViewer,
        dataHelp: viewer.dataHelp
    });

    return [this.panelViewer,this.googleViewer];
}





