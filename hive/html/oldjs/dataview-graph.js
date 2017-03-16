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
// _/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Graph viewer
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/
    
vDV.generateGraphView=function(view, txt )
{

    if(!txt || txt.length==0)return ;
    var rowstart=0;
    //alert(view.coldef+"\n"+view.txtHdr+"\n"+txt)
    if(txt.indexOf("error:")==0) {
        gObject(view.container).innerHTML="";
        return;
    }
    //alert(view.container+"="+view.callback);
    if (view.callback){// && view.callback.length>0 ) {
        if(view.callback=="alert") {alert(txt);}
        else {if( view.callback(view, txt)==0) return ;}
    }    
    var rows=txt.split("\n");
        
    var hdrs; // get the headers
    if(view.txtHdr && view.txtHdr.length) hdrs=view.txtHdr.split(",");
    else hdrs=rows[rowstart++].split(",");
        
    var fndCols=new Array();
    var colSeries=new Array();
    var coldef=view.coldef.split(",");
    var scaleTo=parseFloat(view.scaleTo);
        
    // determine which columns are to be shown
    var data = new google.visualization.DataTable();

    for ( var col=0; col< coldef.length; ++col ) {
        var fnd;
        var coldeftype=coldef[col].substring(0,1);
        var nm=coldef[col].substr(2);
        var NMpos=nm.indexOf(":");
        var NM=nm;
        if(NMpos!=-1){
            NM=nm.substr(NMpos+1);
            nm=nm.substr(0,NMpos);
        }
        if(nm.indexOf("#")==0){
            nm=hdrs[parseInt(nm.substr(1))];
            NM=nm;
        }
            
        for ( fnd =0 ; fnd < hdrs.length && hdrs[fnd]!=nm; ++fnd );
        if(fnd==hdrs.length)continue;
        fndCols[fndCols.length]=col; // this column is to be outputed
        fndCols[fndCols.length]=fnd; // this column is to be outputed
            
        data.addColumn(coldeftype=="i" ? "number" : "string", NM);
    }
    if(!fndCols.length) {
        var o = gObject(view.container);
        if(o)o.innerHTML=txt; // "<img src='img/graph.gif' border=0 width=64 ><p/>";
        return ;
    }
        
        
        
    for ( var row=rowstart; row< rows.length; ++row ) {
        var cols=rows[row].split(",");
        data.addRows(1);
        if(cols.length<2)continue;
            
        var totSum=0;
        if(scaleTo) {
            for ( var col=1; col*2< fndCols.length ; ++col ){
                var dcol=fndCols[2*col+0];
                if(coldef[dcol].substring(0,1)!="i" )continue;

                var icol=fndCols[2*col+1];
                var value=cols[icol];
                value=parseFloat(value);
                totSum+=value;
            }
        }

        for ( var col=0; col*2< fndCols.length ; ++col ){
            var dcol=fndCols[2*col+0];
            var icol=fndCols[2*col+1];
            var value=cols[icol];
                
            if(coldef[dcol].substring(0,1)=="i" ){
                value=parseFloat(value);
                if(scaleTo && col>0){
                    value=totSum ? (scaleTo*value)/totSum : 0 ;
                    value=parseFloat(value.toPrecision(2));
                }
            }
                        
            data.setValue(row-rowstart, col , value) ;
        }
            
    }
        
    var o=document.getElementById(view.container);
    var obj;
    if(view.type== "column") { 
        obj = new google.visualization.ColumnChart(o);
    }else if(view.type== "scatter") {
        obj  = new google.visualization.ScatterChart(o);
    }else if(view.type== "line") {
        obj  = new google.visualization.LineChart(o);
    }else if(view.type== "area") {
        obj  = new google.visualization.AreaChart(o);
    }else if(view.type== "pie") {
        obj  = new google.visualization.PieChart(o);
    }else if(view.type== "candlestick") {
        obj  = new google.visualization.CandlestickChart(o);
    } 
    //obj.draw(data, view.options);
    obj.draw(data, view.options);
          
    if(view.select_handler) { 
        google.visualization.events.addListener( obj , 'select', view.select_handler );
        }
    view.obj=obj;
    view.graphData = data;
    obj.name='okokok';
    obj.lo='lo';
    return obj;
}

// _/_/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Registration
// _/
// _/_/_/_/_/_/_/_/_/_/_/_/
vDV.registerViewer( "graphview", vDV.generateGraphView) ;



