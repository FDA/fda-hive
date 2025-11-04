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
$(function() {
    $.widget("cart.cart_graph", {
    
        _create: function () {
            $(this.element).append($("#canvas"));
            cartdata        = {};
            cartJson        = {};
            continiousTypes = ['int16', 'int32', 'int64', 'float16', 'float32', 'float64'];
            var colors = ['BLUE','RED','SILVER','GRAY','GREEN','BLACK','MAROON','AQUA','TEAL','NAVY','FUCHSIA','PURPLE','YELLOW']

            var color_map = {
                    'SILVER'  : '#C0C0C0' ,
                    'GRAY'    : '#808080' ,
                    'BLACK'   : '#000000' ,
                    'RED'     : '#FF0000' ,
                    'MAROON'  : '#800000' ,
                    'YELLOW'  : '#FFFF00' ,
                    'GREEN'   : '#008000' ,
                    'AQUA'    : '#00FFFF' ,
                    'TEAL'    : '#008080' ,
                    'BLUE'    : '#0000FF' ,
                    'NAVY'    : '#000080' ,
                    'FUCHSIA' : '#FF00FF' ,
                    'PURPLE'  : '#800080'
            }
            
            Plotly.setPlotConfig({ modeBarButtonsToRemove: ['sendDataToCloud'] });

            $("#graphButton").click(function () {
                $('#breakdowntable').empty();
                
                $.getLayoutManager().show('Graph');
                $('#center').remove();
                if($("#right").is(':visible')){
                    $("#right").hide();
                    $("#drawingConfig").animate({ "left": "+=300px" }, "fast" );
                }

                var num       = 1;
                var queryType = $("#queryArea" + num + " #queryType"+num).val();
                var queryJson = tableDict;

                var graphType = $("#graphType").val();
                var xvalueField    = $("#xvalueField"   ).val();
                var yvalueField    = $("#yvalueField"   ).val();
                
                var colorByField   = $("#colorByField"  ).val();
                var splitByField   = $("#splitByField"  ).val();
                var labelByField   = 'USUBJID';

                var fitField       = 'linear' ;

                var uniqueSplitValues = get_unique_split_values(queryJson , splitByField , queryType);

                if(uniqueSplitValues.length > 1){
                    var canvasAreaHeight=250*uniqueSplitValues.length;
                    var canvas = $('<div id="center"><div id="canvasArea" style="height:'+canvasAreaHeight+'px; margin: 25px"></div></div>');

                }else{
                    var canvas = $('<div id="center"><div id="canvasArea" style="height:98%"></div>');
                };


                $('#middle').append(canvas);

                $('#middle .enscroll-track').remove();
                $('#middle .corner2').remove();

                if (queryType == 'summary')
                {

                    var colorArray    = null;
                    var cur_mode      = null;
                    var cur_num_cols  = 2;
                    var time_var_map  = {};
                    
                    if (graphType === "automatic")
                    {
                        var xType = colTypeDict[xvalueField];
                        var yType = colTypeDict[yvalueField];
                        
                        var xIsContinuous;
                        var yIsContinuous;
                        
                        if (xType && yType)
                        {
                            xIsContinuous = continiousTypes.includes(xType);
                            yIsContinuous = continiousTypes.includes(yType);
                        }
                        
                        if (xIsContinuous && yIsContinuous)
                        {
                            graphType = "scatter";
                        }
                        else if (!xIsContinuous && !yIsContinuous)
                        {
                            graphType = "bar";
                        }
                        else
                        {
                            graphType = "box";
                        }
                    }
                    
                    if (graphType === "scatter")
                    {
                        cur_mode = 'markers';

                        if(fitField!=null && fitField!='' && fitField!='nofit')
                        {
                            cur_mode = 'fit';
                        }

                        draw_scatter_plot(queryJson, xvalueField, yvalueField,splitByField,labelByField,colorByField,fitField);
                    }
                    else if (graphType === "bar")
                    {
                        cur_mode = 'bar';
                        if(splitByField!=''&& splitByField!=null&&splitByField!='nosplit' && uniqueSplitValues.length>1)
                        {
                            draw_bar_plot(queryJson,xvalueField,yvalueField,splitByField);
                        }
                        else
                        {
                            draw_bar_plot(queryJson, xvalueField, yvalueField,splitByField);
                        }
                    }
                    else if (graphType === "box")
                    {
                        cur_mode = 'box';
                        var data_field;
                        var group_field;
                        var orientation = "v";
                        var xType = colTypeDict[xvalueField];
                        var yType = colTypeDict[yvalueField];
                        if (continiousTypes.includes(xType) && !continiousTypes.includes(yType))
                        {
                            orientation = "h";
                        }
                        else if (continiousTypes.includes(yType) && !continiousTypes.includes(xType))
                        {
                            orientation = "v"
                        }
                            
                        if (orientation === "h")
                        {
                            data_field = xvalueField;
                            group_field = yvalueField;
                        }
                        else if (orientation === "v")
                        {
                            data_field  = yvalueField;
                            group_field = xvalueField;
                        }
                        $('#canvasArea').empty();
                        draw_box_plot(queryJson, data_field , group_field , splitByField,orientation);
                    }
                }
                else if(queryType == 'timeSeries')
                {

                    var time_var_map = {};
                    var cur_num_cols = 1;

                    var yFields     = [];
                    var yFieldTypes = [];

                    $("#timeSeriesFieldCollection").find("input[name='field']").each(function (index, element) {
                        var variable = element.value;
                        if (variable != undefined && variable != null && variable!="") {
                            yFields.push(variable);
                        }
                    });

                    $("#timeSeriesFieldCollection").find("input[name='fieldType']").each(function (index, element) {
                        var variable = element.value;
                        if (variable != undefined && variable != null && variable!="") {
                            yFieldTypes.push(variable);
                        }
                    });

                    for(var i=0;i<yFields.length;i++){
                        if(!time_var_map.hasOwnProperty(yFields[i])){
                            time_var_map[yFields[i]]='';
                        }
                        time_var_map[yFields[i]]=yFieldTypes[i];
                    };

                    draw_timeseries_plot( queryJson , splitByField , cur_mode , cur_num_cols , time_var_map, xvalueField );
                };
            });


            function get_unique_split_values(cartJson , split_by , queryType) {

                var uniqueSplitValues = [];

                if(split_by!='' && split_by!=null && split_by!='nosplit' ){

                    if(queryType == 'summary'){
                        for(var i=0;i<cartJson[split_by].length;i++){
                            if(uniqueSplitValues.indexOf(cartJson[split_by][i])<0){
                                uniqueSplitValues.push(cartJson[split_by][i]);
                            };
                        };
                    } else if (queryType == 'timeSeries'){
                        var studies = Object.keys(cartJson);
                        for(var std_i=0;std_i<studies.length;std_i++){
                            if (split_by == 'STUDYID'){
                                uniqueSplitValues.push(studies[std_i]);
                            } else if(split_by == 'USUBJID'){
                                var pats = Object.keys(cartJson[ studies[std_i] ]);
                                for(var pat_i=0; pat_i<pats.length; pat_i++){
                                    uniqueSplitValues.push(pats[pat_i]);
                                };
                            };
                        };
                    };
                };

                return uniqueSplitValues;
            };

            function getRandomColor() {
                var letters = '0123456789ABCDEF';
                var color = '#';
                for (var i = 0; i < 6; i++) {
                    color += letters[Math.floor(Math.random() * 16)];
                }
                return color;
            }

            function filter_by_array(data_array, filter_array , filter_value ){
                var fltr_data = [] 
                if(data_array!=null ){
                    for(var i=0;i<filter_array.length;i++){
                        if(filter_array[i] == filter_value ){
                            fltr_data.push(data_array[i]);
                        }
                    }
                };
                return fltr_data;
            };

            function get_cross_tab_cnt(array_one, array_two, val_one,val_two){
                count = 0
                for(var i=0; i<array_one.length;i++){
                    if( (array_one[i] == val_one) && (array_two[i] == val_two) ){
                        count = count + 1;
                    };
                };
                return count;
            };
              

            function filter_by_null( array_one, array_two ){
                var array_one_res = [] ;
                var array_two_res = [] ;

                if (array_one.length !== array_two.length) {
                    throw "Error: arrays passed to filter_by_null must have equal size";
                };

                for(var i=0;i<array_one.length;i++){
                    if( ! Number.isNaN(array_one[i])  &&   ! Number.isNaN(array_two[i]) ){
                        array_one_res.push(array_one[i]);
                        array_two_res.push(array_two[i]);
                    };
                };

                return [array_one_res , array_two_res];
            };

            function gen_split_layout(subplot_names , num_cols , margin , mode , l_vars , r_vars , x_col_name){

                var num_plots = subplot_names.length;
                var plt_ncols = Math.min(num_cols,num_plots);
                var nrows     = Math.ceil(num_plots / plt_ncols);
                var y_domains = [];
                var x_domains = [];
                var y_incr    = 1 / plt_ncols;
                var x_incr    = 1 / nrows;
                var margin    = 0.1;


                var ynnc  = y_incr*(margin)
                for( var i=0 ; i<plt_ncols ; i++ ){
                    str = ((i  )*y_incr)+ynnc
                    end = ((i+1)*y_incr)-ynnc

                    x_domains.push([str,end])
                }

                var xnnc  = x_incr*(margin)
                for( var i=0 ; i<nrows ; i++ ){
                    str = ((i  )*x_incr)+xnnc
                    end = ((i+1)*x_incr)-xnnc

                    y_domains.push([str,end])
                }


                var layout = {};


                for( var sbplt_row_idx=0 ; sbplt_row_idx < nrows ; sbplt_row_idx++){
                    for( var sbplt_col_idx=0 ; sbplt_col_idx < plt_ncols ; sbplt_col_idx++){

                        var plt_num = ( sbplt_col_idx + 1 ) + (plt_ncols*sbplt_row_idx);

                        if(plt_num <= num_plots) {

                            var xax = plt_num;
                            if (plt_num == 1){
                                xax='';
                            };

                            layout['xaxis' + xax]={
                                    visible  : true          ,
                                    showline : true          ,
                                    domain     : x_domains[sbplt_col_idx],
                                    title      : (x_col_name==null)?(subplot_names[plt_num-1]==='All'?'':subplot_names[plt_num-1]):(x_col_name)

                            };

                            var yax = '';

                            var l_vars_len = (! l_vars ) ? 0 : l_vars.length ;
                            var r_vars_len = (! r_vars ) ? 0 : r_vars.length ;

                            nvars  = l_vars_len + r_vars_len
                            base   = ((plt_num-1)*(nvars))+1;

                            for( var l_i=0; l_i < l_vars_len ; l_i++){
                                yax =base+(l_i);
                                yaxis_label = 'yaxis' + yax

                                if(yax==1){
                                    yaxis_label = 'yaxis'
                                    cur_anchor  = 'x';
                                } else {
                                    if(l_i ==0){cur_anchor = 'x'+xax; layout['xaxis' + xax]['anchor']='y'+yax; } else {cur_anchor = 'free';};

                                    if(base==1){ovrly = 'y';} else {ovrly = 'y'+base;};
                                }

                                layout[yaxis_label]={
                                        visible  : true          ,
                                        showline : true          ,
                                        domain   : y_domains[sbplt_row_idx],
                                        title    : (subplot_names[plt_num-1]==='All'?'':l_vars[l_i])
                                };

                                layout[yaxis_label]['anchor']=cur_anchor;

                                if(base!=yax && yax!=1){
                                    layout[yaxis_label]['overlaying'] = ovrly;
                                };
                            };

                            for( var r_i=0; r_i < r_vars_len ; r_i++){
                                yax =base + r_i + l_vars_len
                                yaxis_label = 'yaxis' + yax
                                if(r_i==0 ){ cur_anchor = 'x'+xax }else{ cur_anchor = 'free';};
                                if(base==1){ ovrly = 'y';        } else{ ovrly = 'y'+base;   };

                                layout[yaxis_label]={
                                        visible  : true          ,
                                        showline : true          ,
                                        domain   : y_domains[sbplt_row_idx],
                                        anchor   : cur_anchor    ,
                                        side     : 'right'       ,
                                        title    : r_vars[r_i]
                                };


                                if(base!=yax){
                                    layout[yaxis_label]['overlaying']=ovrly;
                                }
                            };
                        }

                    };
                };

                if (mode === 'bar'){
                    layout['barmode']="stack";
                };
                if (mode === 'box'){
                    layout['title'  ]             = 'Box Plot Styling Mean and Standard Deviation';
                    layout['xaxis'  ]['title'   ] = x_col_name;
                    layout['xaxis'  ]['zeroline'] = false     ;
                    layout['yaxis'  ]['title'   ] = l_vars[0] ;
                    layout['yaxis'  ]['zeroline'] = false     ;
                    layout['boxmode']             = 'group'   ;
                };

                layout["title"]=subplot_names[0];
                return layout;
            }

            function cart_anova(x , grp) {

                grps_unq = [... new Set(grp)];
                grp_data = {};
                for (var i=0; i<grps_unq.length; i++){
                    grp_data[grps_unq[i]] = [];
                };

                for(var j=0; j < x.length; j++){
                    cur_x = parseFloat( x[j] )
                    grp_data[grp[j]].push(cur_x);
                };
                
                args = []
                for (var i=0; i<grps_unq.length; i++){
                    args.push( grp_data[grps_unq[i]] )
                };

                var total_sum_of_squares    = jStat.sumsqerr( x );
                var sum_of_squares_within_grps = 0
                for (var i=0; i<args.length; i++){
                    sum_of_squares_within_grps = sum_of_squares_within_grps + jStat.sumsqerr( args[i] );
                };
                var sum_of_squares_btn_grps = total_sum_of_squares - sum_of_squares_within_grps;
                var df1    = args.length -1;
                var df2    = x.length - args.length;
                var Fnum   = sum_of_squares_btn_grps / df1;
                var Fdnum  = sum_of_squares_within_grps / df2;
                var fscore = Fnum / Fdnum;
                
                p = jStat.ftest( fscore, df1, df2)

                return p;
            };


            function scatterPlot(x,y, labels , mode , color_array , color_map) {

                plt_colors=[]

                if(color_array!=null && color_map!=null){
                    for(var i=1;i<color_array.length;i++){
                        plt_colors.push( color_map[color_array[i]] )
                    }
                }else if(mode=='markers'){
                    plt_colors = '#0000FF'
                }

                var dataset = {
                        mode: mode ,
                        type: 'scatter',
                        x: x,
                        y: y,
                        text: labels,
                        marker: {
                            sizemode: 'area',
                            size    : 6     ,
                            sizeref : 2e5   ,
                            color   : plt_colors
                        }
                };
                return dataset
            }

            function draw_bar_plot(cartJson, x_col_name, y_col_name , splitByField) {
                $('#canvasArea').empty();

                var split_array = cartJson[splitByField];
                var x_array     = cartJson[x_col_name  ];
                var y_array     = cartJson[y_col_name  ];
                

                cross_tab_json = {};

                var split_unqs = [];
                if(splitByField!='' && splitByField!=null && splitByField!='nosplit' ){
                    split_unqs = new Set(split_array);
                    split_unqs = Array.from(split_unqs).sort().filter(Boolean)
                };
                split_unqs.unshift('All');

                for(var i = 0; i < split_unqs.length; i++){
                    var cur_split = split_unqs[i];
                    cross_tab_json[ cur_split ] = {};

                    if(cur_split == 'All'){
                        var x_filtered_by_split  = x_array;
                        var y_filtered_by_split  = y_array;
                    }else{
                        var x_filtered_by_split  = filter_by_array(x_array , split_array , cur_split);
                        var y_filtered_by_split  = filter_by_array(y_array , split_array , cur_split);
                    };

                    var cur_y_unqs = new Set(y_filtered_by_split)
                    var cur_y_unqs = Array.from(cur_y_unqs).sort()
                    cur_y_unqs.unshift('All')

                    var cur_x_unqs = new Set(x_filtered_by_split)
                    var cur_x_unqs = Array.from(cur_x_unqs).sort()
                    cur_x_unqs.unshift('All')
                    for(var j = 0; j < cur_y_unqs.length; j++){
                        var cur_y_grp = cur_y_unqs[j];
                        cross_tab_json[cur_split][ cur_y_grp ] = {};

                        for(var k = 0; k < cur_x_unqs.length; k++){
                            var cur_x_grp = cur_x_unqs[k];
                            var res_cnt = 0;
                            if(cur_y_grp == 'All'){
                                if(cur_x_grp == 'All'){
                                    res_cnt = x_filtered_by_split.length;
                                }else{
                                    res_cnt =  x_filtered_by_split.filter(x => x===cur_x_grp).length; 
                                };
                            }else{
                                if(cur_x_grp == 'All'){
                                    res_cnt =  y_filtered_by_split.filter(x => x===cur_y_grp).length;
                                }else{
                                    res_cnt =  get_cross_tab_cnt( x_filtered_by_split , 
                                                                  y_filtered_by_split ,
                                                                  cur_x_grp,
                                                                  cur_y_grp ); 
                                };
                            };
                            cross_tab_json[cur_split][ cur_y_grp ][cur_x_grp] = res_cnt;
                        };

                    };
                };


                pvals = {};
                
                for(var i = 0; i < split_unqs.length; i++){
                    var sum = 0;
                    var cur_split = split_unqs[i];

                    if(cur_split == 'All'){
                        var x_filtered_by_split  = x_array;
                        var y_filtered_by_split  = y_array;
                    }else{
                        var x_filtered_by_split  = filter_by_array(x_array , split_array , cur_split);
                        var y_filtered_by_split  = filter_by_array(y_array , split_array , cur_split);
                    };
                    var cur_y_unqs = new Set(y_filtered_by_split)
                    var cur_y_unqs = Array.from(cur_y_unqs).sort()

                    var cur_x_unqs = new Set(x_filtered_by_split)
                    var cur_x_unqs = Array.from(cur_x_unqs).sort()
                    for(var j = 0; j < cur_y_unqs.length; j++){
                        var cur_y_grp = cur_y_unqs[j];
                        for(var k = 0; k < cur_x_unqs.length; k++){
                            var cur_x_grp = cur_x_unqs[k];

                            var xtotal = cross_tab_json[cur_split]['All'][cur_x_grp];
                            var ytotal = cross_tab_json[cur_split][cur_y_grp]['All'];
                            var N      = cross_tab_json[cur_split]['All']['All']
                            var actual = cross_tab_json[cur_split][cur_y_grp][cur_x_grp];

                            var expected = (ytotal / N) * xtotal;
                            var err      = Math.abs( (actual - expected) );
                            if(degrees_of_freedom == 1){
                                err = err - 0.5;
                            }
                            sum = sum + (  ( Math.pow( err,2) )/expected  );
                        };
                    };
                    var degrees_of_freedom = (cur_y_unqs.length - 1)*(cur_x_unqs.length - 1)
                    var cur_pval = 1 - jStat.chisquare.cdf(sum, degrees_of_freedom );
                    pvals[cur_split]=cur_pval;
                };


                for(var i=0;i<split_unqs.length;i++){
                    var graphData    = [];
                    var annotation   = [];
                    cur_split = split_unqs[i];

                    if(cur_split == 'All'){
                        var x_filtered_by_split  = x_array;
                        var y_filtered_by_split  = y_array;
                    }else{
                        var x_filtered_by_split  = filter_by_array(x_array , split_array , cur_split);
                        var y_filtered_by_split  = filter_by_array(y_array , split_array , cur_split);
                    };
                    var cur_y_unqs = new Set(y_filtered_by_split)
                    var cur_y_unqs = Array.from(cur_y_unqs).sort()

                    var cur_x_unqs = new Set(x_filtered_by_split)
                    var cur_x_unqs = Array.from(cur_x_unqs).sort()

                    for (var j = 0; j < cur_y_unqs.length; j++) {
                        yfield = cur_y_unqs[j];

                        yps = []
                        xps = []
                        ycnt = []
                        for (var k = 0; k < cur_x_unqs.length; k++){
                            xfield = cur_x_unqs[k];
                            if( ( yfield in cross_tab_json[cur_split]) && ( xfield in cross_tab_json[cur_split][yfield]) ){
                                xps.push( xfield );
                                yps.push( cross_tab_json[cur_split][yfield][xfield] / cross_tab_json[cur_split]['All'][xfield] );
                                ycnt.push( cross_tab_json[cur_split][yfield][xfield] );
                            };
                        };
                        var dataset = {
                                x      : xps   ,
                                y      : yps   ,
                                text   : ycnt.map(String),
                                textposition: 'auto'     ,
                                textfont: {
                                    family: 'sans serif',
                                    size: 16,
                                    color: 'rgb(0,0,0)'
                                  },
                                type   : "bar" ,
                                name   : yfield,
                                opacity: 0.5   ,
                                marker : {
                                    color: color_map[colors[j]]
                                }
                        };
                        graphData.push(dataset);
                    };

                    annotation.push({
                        text      : 'p-value= '+pvals[cur_split].toFixed(2)+'' ,
                        showarrow : false                           ,
                        arrowhead : 4                               ,
                        font      : {size : 16 , bold:true}         ,
                        xref      : 'paper' ,
                        yref      : 'paper' ,
                        xanchor   : 'right' ,
                        yanchor   : 'bottom',
                        x         : 0.8     ,
                        y         : 0.95 
                    });
    
    
                    var subID="sub"+i;
                    if(split_unqs.length==1){
                        var tmp="<div id='"+subID+"' class='singleplot'></div>"
                    }else{
                        var tmp="<div id='"+subID+"' class='subplot'></div>"
                    }
    
                    $("#canvasArea").append(tmp)

                    var subLayout = {
                            barmode: "group",
                            showlegend: true,
                            yaxis: {
                                title: y_col_name,
                                zeroline: false,
                                showline: true
                            },
    
                            xaxis: {
                                title: x_col_name,
                                showline: true,
                                zeroline: false
                            },
                            
                            title       : cur_split, 
                            annotations : annotation
                    };

                Plotly.plot('sub'+i, graphData, subLayout, {showLink: false});
            };
            };

            function regression_fit_trace(xarr,yarr){ 
                var cur_data  = [];
                var cur_x_arr = [];
                var cur_y_arr = [];
                for(var j=0; j < xarr.length; j++){
                    cur_x = parseFloat( xarr[j] )
                    cur_y = parseFloat( yarr[j] )
                    if (  !isNaN(cur_x) &&  !isNaN(cur_y) ){
                        cur_data.push( [ cur_x , cur_y ] );
                        cur_x_arr.push( cur_x );
                        cur_y_arr.push( cur_y );
                    };
                };

                res = [];
                if( (cur_x_arr.length>1) && (cur_y_arr.length>1) ){
                    var Scorr = jStat.spearmancoeff(cur_x_arr, cur_y_arr);
                    const result = regression( method='linear' , data=cur_data );
                    idx = result.points.length - 1
                    const anotx = result.points.reduce((max, p) => p[0] > max ? p[0] : max, result.points[0][0]);
                    const anoty = result.points.reduce((max, p) => p[1] > max ? p[1] : max, result.points[0][1]);
                    var fitted_y = [];
                    for(var j=0; j < result.points.length; j++){
                        fitted_y.push( result.points[j][1] )
                    };
    
                    const slope     = result.equation[0];
                    const intercept = result.equation[1];
    
                    ant_push1 = {
                        x         : anotx * 1.3,
                        y         : anoty * 1.1,
                        text      : 'R2='+result.r2.toFixed(2)+' ',
                        showarrow : false        ,
                        arrowhead : 4            ,
                        font      : {size : 12 , bold:true}
                    };
                    ant_push2 = {
                        x         : anotx * 1.3,
                        y         : anoty * 1.3,
                        text      : 'S='+Scorr.toFixed(2)+' ',
                        showarrow : false        ,
                        arrowhead : 4            ,
                        font      : {size : 12 , bold:true}
                    };
    
                    var line_trace = scatterPlot( 
                            cur_x_arr ,
                            fitted_y  ,
                            null      ,
                            'lines'   ,
                            null      ,
                            null
                    );
                    line_trace['showlegend'] = false;
                    line_trace['line']={'dash':'dot' , 'width':1 , 'color':'black'};

                    res = [ line_trace , ant_push1 , ant_push2]
                };
                return res;
            };

            function draw_scatter_plot(cartJson    , 
                                       x_col_name  , 
                                       y_col_name  ,
                                       splitByField,
                                       labelByField,
                                       colorByField,
                                       fitField
            ){


            var split_array = cartJson[splitByField];
            var label_array = cartJson[labelByField];
            var color_array = cartJson[colorByField];
            var x_array     = cartJson[x_col_name  ].map(parseFloat);
            var y_array     = cartJson[y_col_name  ].map(parseFloat);

            var split_unqs = []
            if(splitByField!='' && splitByField!=null && splitByField!='nosplit' ){
                split_unqs = new Set(split_array)
                split_unqs = Array.from(split_unqs).sort().filter(Boolean)
            }
            split_unqs.unshift('All')

            var col_by_unqs = []
            if(colorByField!='' && colorByField!=null && colorByField!='nocolorby' ){
                col_by_unqs = new Set(color_array)
                col_by_unqs = Array.from(col_by_unqs).sort().filter(Boolean)
            }else{
                col_by_unqs = ['All']
            }

            for(var i=0;i<split_unqs.length;i++){
                var graphData  = [];
                var annotation = [];
                var cur_split  = split_unqs[i];
                if (cur_split == 'All'){
                    var cur_x_arr = x_array, cur_y_arr = y_array , cur_col_array = color_array , cur_label_array = label_array
                }else{    
                    var cur_x_arr       = filter_by_array(x_array     , split_array , cur_split);
                    var cur_y_arr       = filter_by_array(y_array     , split_array , cur_split);
                    var cur_col_array   = filter_by_array(color_array , split_array , cur_split);
                    var cur_label_array = filter_by_array(label_array , split_array , cur_split);
                }
                for(var j=0;j<col_by_unqs.length;j++){
                    var cur_col_by  = col_by_unqs[j];

                    if (cur_col_by == 'All'){
                        var cur_x_arr2 = cur_x_arr, cur_y_arr2 = cur_y_arr , cur_col_array2 = cur_col_array , cur_label_array2 = cur_label_array
                    }else{    
                        var cur_x_arr2       = filter_by_array(cur_x_arr       , cur_col_array , cur_col_by);
                        var cur_y_arr2       = filter_by_array(cur_y_arr       , cur_col_array , cur_col_by);
                        var cur_col_array2   = filter_by_array(cur_col_array   , cur_col_array , cur_col_by);
                        var cur_label_array2 = filter_by_array(cur_label_array , cur_col_array , cur_col_by);
                    }

                    var scatter_trace = {
                            mode: 'markers',
                            type: 'scatter',
                            x: cur_x_arr2,
                            y: cur_y_arr2,
                            text: cur_label_array2,
                            marker: {
                                sizemode: 'area',
                                size    : 6     ,
                                sizeref : 2e5   ,
                                color   : color_map[colors[j]]
                            }
                        };
                    if(cur_col_by!='All'){
                        scatter_trace['name']=cur_col_by;
                    }else{    
                        scatter_trace['showlegend'] = false;
                    };
                    graphData.push(scatter_trace);
                };

                if (fitField=='linear') {
                    if( (cur_x_arr.length>1) && (cur_y_arr.length>1) ){
                        if( (!cur_x_arr.every(isNaN))  &&  (!cur_y_arr.every(isNaN)) ){
                            reg_res = regression_fit_trace(cur_x_arr,cur_y_arr) 
                            if ( reg_res.length == 3 ) {
                                line_trace = reg_res[0] 
                                annotation.push(reg_res[1]) 
                                annotation.push(reg_res[2])
                                graphData.push(line_trace);
                            };
                        };
                    };
                };


                var subID="sub"+i;
                if(split_unqs.length==1){
                    var tmp="<div id='"+subID+"' class='singleplot'></div>"
                }else{
                    var tmp="<div id='"+subID+"' class='subplot'></div>"
                }

                $("#canvasArea").append(tmp)

                var subLayout={
                        yaxis: {
                            title    : y_col_name,
                            zeroline : false     ,
                            showline : true
                        },

                        xaxis: {
                            title    : x_col_name,
                            showline : true      ,
                            zeroline : false
                        },

                        title       : split_unqs[i],
                        annotations : annotation
                };

                if(color_array!=null){
                    subLayout['legend'] = {'x':1.2 , 'y':1.0 , 'orientation':'v'};
                };

                Plotly.plot('sub'+i, graphData, subLayout, {showLink: false});

            };

            };

            function draw_box_plot(    cartJson       , 
                                       data_col_name  , 
                                       group_col_name ,
                                       splitByField   ,
                                       orientation    ,
            ){


            var split_array = cartJson[splitByField  ];
            var data_array  = cartJson[data_col_name ].map(parseFloat);
            var group_array = cartJson[group_col_name];

            var split_unqs = []
            if(splitByField!='' && splitByField!=null && splitByField!='nosplit' ){
                split_unqs = new Set(split_array)
                split_unqs = Array.from(split_unqs).sort().filter(Boolean)
            }
            split_unqs.unshift('All')


            group_by_unq = new Set(group_array)
            group_by_unq = Array.from(group_by_unq).sort().filter(Boolean)

            for(var i=0;i<split_unqs.length;i++){
                var graphData  = [];
                var annotation = [];
                var cur_split  = split_unqs[i];
                if (cur_split == 'All'){
                    var cur_data_arr = data_array, cur_group_arr = group_array
                }else{    
                    var cur_data_arr  = filter_by_array(data_array  , split_array , cur_split);
                    var cur_group_arr = filter_by_array(group_array , split_array , cur_split);
                }
                var res_filter = filter_by_null( cur_data_arr, cur_group_arr )

                var cur_data_arr  = res_filter[0];
                var cur_group_arr = res_filter[1];                

                p_val = cart_anova( cur_data_arr , cur_group_arr );
                for(var j=0;j<group_by_unq.length;j++){
                    var cur_group_by  = group_by_unq[j];
    
                    var cur_data_arr2  = filter_by_array(cur_data_arr   , cur_group_arr , cur_group_by);
                    var cur_group_arr2 = filter_by_array(cur_group_arr  , cur_group_arr , cur_group_by);

                    var scatter_trace = {
                            type        : 'box'           ,
                            name        : cur_group_by    ,
                            boxmean     : 'sd'            ,
                            boxpoints   : 'all'           ,
                            jitter      : 0.5             , 
                            orientation : orientation     ,
                            marker: {
                                color: color_map[ colors[j] ],
                                size : 2.0
                            },
                            showlegend: true
                    };
                    if(orientation=='v'){
                        scatter_trace['y' ] = cur_data_arr2;
                        scatter_trace['x0'] = cur_group_by;
                    }else if(orientation=='h'){
                        scatter_trace['x' ] = cur_data_arr2;
                        scatter_trace['y0'] = cur_group_by;
                    };

                    graphData.push(scatter_trace);
                };

                
                annotation.push({
                    text      : 'p-value='+p_val.toFixed(2)+'' ,
                    showarrow : false                          ,
                    arrowhead : 4                              ,
                    font      : {size : 16 , bold:true}        ,
                    xref      : 'paper' ,
                    yref      : 'paper' ,
                    xanchor   : 'right' ,
                    yanchor   : 'bottom',
                    x         : 0.8     ,
                    y         : 0.95 
                });


                var subID="sub"+i;
                if(split_unqs.length==1){
                    var tmp="<div id='"+subID+"' class='singleplot'></div>"
                }else{
                    var tmp="<div id='"+subID+"' class='subplot'></div>"
                }

                $("#canvasArea").append(tmp)

                var subLayout={
                        yaxis: {
                            showline : true,
                            autorange: true,
                            showgrid : true,
                            zeroline : true,
                            dtick    : 5   ,
                            gridcolor: 'rgb(255, 255, 255)',
                            gridwidth: 1,
                            zerolinecolor: 'rgb(255, 255, 255)',
                            zerolinewidth: 2,
                            showticklabels: true  ,
                            tickmode : 'auto',
                            ntick : 5,
                            tickformat    : '.1'  ,
                            tickangle     : 'auto',
                            tickfont: {
                              family: 'Old Standard TT, serif',
                              size  : 10                      ,
                              color : 'black'
                            },
                            exponentformat: 'e',
                            showexponent  : 'all'                            
                        },

                        xaxis: {
                            showline      : true  ,
                            autorange     : true  ,
                            zeroline      : false ,
                            showticklabels: true  ,
                            tickformat    : '.1'  ,
                            tickangle     : 'auto',
                            tickfont      : {
                              family: 'Old Standard TT, serif',
                              size  : 10                      ,
                              color : 'black'
                            },
                            exponentformat: 'e',
                            showexponent  : 'all'    
                        },

                        title       : cur_split,
                        annotations : annotation
                };

                if(orientation=='v'){
                    subLayout['yaxis' ]['title'] = data_col_name;
                    subLayout['xaxis' ]['title'] = group_col_name;
                }else if(orientation=='h'){
                    subLayout['yaxis' ]['title'] = group_col_name;
                    subLayout['xaxis' ]['title'] = data_col_name;
                };
                
                Plotly.plot('sub'+i, graphData, subLayout, {showLink: false});

            };

            };        

            function draw_timeseries_plot( 
                    cartJson    ,
                    split_by    ,
                    mode        ,
                    num_cols    ,
                    plt_vars_map,
                    x_col_name
            ) {

                studies      =  Object.keys(cartJson)
                study1_pats  =  Object.keys(cartJson[ studies[0] ] )
                all_vars     =  Object.keys(cartJson[ studies[0] ] [ study1_pats[0] ])

                var split_unqs = get_unique_split_values(cartJson , split_by , 'timeSeries');

                plt_vars = Object.keys(plt_vars_map);

                var l_vars = []
                var r_vars = []
                var plt_vars_map2 = {}
                for(var tmp_i=0; tmp_i<plt_vars.length; tmp_i++){
                    plt_vars_map2[plt_vars[tmp_i]]             = {};
                    plt_vars_map2[plt_vars[tmp_i]]['plt_type'] = plt_vars_map[plt_vars[tmp_i]];
                    if(tmp_i%2==0){
                        plt_vars_map2[plt_vars[tmp_i]]['side'] = 'right';
                        l_vars.push(plt_vars[tmp_i]);
                    }else{
                        plt_vars_map2[plt_vars[tmp_i]]['side'] = 'left';
                        r_vars.push(plt_vars[tmp_i])
                    };
                };

                nvars = l_vars.length + r_vars.length



                var timeSubplotData={};

                if(split_unqs.length == 0){split_unqs.push('');}

                var plot_num_if_splitby_patient=-1;

                var colorMap            = {};
                var showLegendReference = {};
                var annotationMap       = {};
                var vlineMap            = {};

                for( var std_i=0; std_i<studies.length; std_i++ ){

                    pats = Object.keys(cartJson[ studies[std_i] ]);
                    for(var pat_i=0; pat_i<pats.length; pat_i++){
                        plot_num_if_splitby_patient = plot_num_if_splitby_patient + 1;

                        for(var var_i=0; var_i<plt_vars.length; var_i++){
                            
                            
                            
                            var newX = [];
                            var newY = [];
                            
                            if (cartJson[ studies[std_i] ][ pats[pat_i]][ plt_vars[var_i] ] !== undefined)
                            {
                            
                                var testArr = cartJson[ studies[std_i] ][ pats[pat_i]][ plt_vars[var_i] ].sort(function(a, b)
                                {
                                    if (a.studyDay < b.studyDay) { return -1; }
                                    else if (a.studyDay > b.studyDay) { return 1; }
                                    else { return 0; }
                                });
                                for (var iTest = 0; iTest < testArr.length; ++iTest)
                                {
                                    if (!Number.isNaN(testArr[iTest].result) || typeof(testArr[iTest].result) === "string")
                                    {
                                        newX.push(testArr[iTest].studyDay);
                                        newY.push(testArr[iTest].result);
                                    }
                                }
                            }

                            var timeseries_x = newX;
                            var timeseries_y = newY;

                            if (plt_vars_map[plt_vars[var_i]] == 'linear') {

                                var scatter_trace = scatterPlot( timeseries_x ,
                                        timeseries_y ,
                                        null         ,
                                        'markers'    ,
                                        null         ,
                                        null
                                );

                                var ts_line_y = timeseries_y;
                                
                                var line_trace = scatterPlot( timeseries_x ,
                                        ts_line_y    ,
                                        null         ,
                                        'lines'      ,
                                        null         ,
                                        null
                                );

                                line_trace['marker']['size']    = 0;
                                if(!colorMap.hasOwnProperty(plt_vars[var_i])){
                                    if(var_i<9){
                                        colorMap[plt_vars[var_i]]=color_map[colors[var_i+1]];
                                    }else{
                                        colorMap[plt_vars[var_i]]=getRandomColor();
                                    }
                                }
                                line_trace['marker']['color']    = colorMap[plt_vars[var_i]];
                                scatter_trace['marker']['size']  = 5;

                                line_trace['legendgroup'] = plt_vars[var_i];
                                line_trace['name'       ] = plt_vars[var_i];

                                if ( std_i > 0 || pat_i > 0){
                                    line_trace['showlegend'] = false;
                                };
                                scatter_trace['showlegend'] = false;


                                var plt_num = 1;
                                line_trace['splitby']    = 'all';
                                scatter_trace['splitby'] = 'all';

                                if(split_by == 'STUDYID'){
                                    plt_num = std_i+1;
                                    line_trace['splitby']    = studies[std_i];
                                    scatter_trace['splitby'] = studies[std_i];
                                }else if(split_by == 'USUBJID'){
                                    plt_num = plot_num_if_splitby_patient+1;
                                    line_trace['splitby']    = pats[pat_i];
                                    scatter_trace['splitby'] = pats[pat_i];

                                };

                                var xax ='';

                                if (plt_num>1){
                                    xax=plt_num
                                }


                                var yax ='';

                                var l_i = l_vars.indexOf(plt_vars[var_i]);
                                var r_i = r_vars.indexOf(plt_vars[var_i]);

                                if(l_i>=0){
                                    yax = (l_i)+1;
                                }else if (r_i>=0){
                                    yax = r_i + l_vars.length+1;
                                };

                                if(yax==1){yax='';}

                                line_trace['xaxis']    = 'x';
                                line_trace['yaxis']    = 'y'+yax;

                                scatter_trace['xaxis'] = 'x';
                                scatter_trace['yaxis'] = 'y'+yax;

                                var splitbyvalue = line_trace['splitby'];
                                if(!timeSubplotData.hasOwnProperty(splitbyvalue)){
                                    timeSubplotData[splitbyvalue]=[];
                                    showLegendReference[splitbyvalue]={};
                                }

                                if(!showLegendReference[splitbyvalue].hasOwnProperty(line_trace['legendgroup'])){
                                    showLegendReference[splitbyvalue][line_trace['legendgroup']]='';
                                    delete line_trace['showlegend'];
                                }

                                timeSubplotData[splitbyvalue].push(line_trace);
                                timeSubplotData[splitbyvalue].push(scatter_trace);


                            }else if(plt_vars_map[plt_vars[var_i]]==='annotation'){

                                cur_y_mod = timeseries_y.map(function(x) { return (x * 1200) - (500) ; } );

                                var scatter_trace = scatterPlot( null     ,
                                        null     ,
                                        null     ,
                                        'markers',
                                        null     ,
                                        null
                                );

                                scatter_trace['showlegend'] = false;
                                scatter_trace['marker']['size'] = 0;

                                var plt_num = 1;
                                scatter_trace['splitby']='all';

                                if(split_by == 'STUDYID'){
                                    plt_num = std_i+1;
                                    scatter_trace['splitby']=studies[std_i];
                                }else if(split_by == 'USUBJID'){
                                    plt_num = plot_num_if_splitby_patient+1;
                                    scatter_trace['splitby']=pats[pat_i];

                                };

                                var xax ='';

                                if (plt_num>1){
                                    xax=plt_num
                                }


                                var yax ='';

                                var l_i = l_vars.indexOf(plt_vars[var_i]);
                                var r_i = r_vars.indexOf(plt_vars[var_i]);

                                if(l_i>=0){
                                    yax = (l_i)+1;
                                }else if (r_i>=0){
                                    yax =  r_i + l_vars.length+1;
                                };

                                if(yax==1){yax='';};


                                scatter_trace['xaxis'] = 'x';
                                scatter_trace['yaxis'] = 'y'+yax;

                                var splitbyvalue=line_trace['splitby'];
                                if(!timeSubplotData.hasOwnProperty(splitbyvalue)){
                                    timeSubplotData[splitbyvalue]=[];
                                    showLegendReference[splitbyvalue]={};
                                }

                                timeSubplotData[splitbyvalue].push(scatter_trace);



                                for(var row_i=0; row_i < timeseries_y.length ; row_i++){
                                    if(timeseries_y[row_i]!="" && timeseries_y[row_i]!=null){

                                        if(!annotationMap.hasOwnProperty(splitbyvalue)){
                                            annotationMap[splitbyvalue]=[];
                                        }
                                        annotationMap[splitbyvalue].push( {
                                            x         : timeseries_x[row_i]               ,
                                            y         : timeseries_y[row_i]  ,
                                            xref      : 'x'                    ,
                                            yref      : 'y'+yax                    ,
                                            text      : plt_vars[var_i],
                                            showarrow : true       ,
                                            arrowhead : 4          ,
                                            ax        : -20        ,
                                            ay        : -20        ,
                                            font      : {
                                                family : 'Courier New, monospace',
                                                size : 10
                                            }
                                        } );

                                    };

                                };
                            }
                            else if(plt_vars_map[plt_vars[var_i]]==='vline'){

                                cur_y_mod = timeseries_y.map(function(x) { return (x * 1200) - (500) ; } );

                                var scatter_trace = scatterPlot( null     ,
                                        null     ,
                                        null     ,
                                        'markers',
                                        null     ,
                                        null
                                );

                                scatter_trace['showlegend'] = false;
                                scatter_trace['marker']['size'] = 0;

                                var plt_num = 1;
                                scatter_trace['splitby']='all';

                                if(split_by == 'STUDYID'){
                                    plt_num = std_i+1;
                                    scatter_trace['splitby']=studies[std_i];
                                }else if(split_by == 'USUBJID'){
                                    plt_num = plot_num_if_splitby_patient+1;
                                    scatter_trace['splitby']=pats[pat_i];
                                };

                                var xax ='';

                                if (plt_num>1){
                                    xax=plt_num
                                }


                                var yax ='';

                                var l_i = l_vars.indexOf(plt_vars[var_i]);
                                var r_i = r_vars.indexOf(plt_vars[var_i]);

                                if(l_i>=0){
                                    yax = (l_i)+1;
                                }else if (r_i>=0){
                                    yax =  r_i + l_vars.length+1;
                                };

                                if(yax==1){yax='';};


                                scatter_trace['xaxis'] = 'x';
                                scatter_trace['yaxis'] = 'y'+yax;

                                var splitbyvalue=line_trace['splitby'];
                                if(!timeSubplotData.hasOwnProperty(splitbyvalue)){
                                    timeSubplotData[splitbyvalue]=[];
                                    showLegendReference[splitbyvalue]={};
                                }

                                timeSubplotData[splitbyvalue].push(scatter_trace);



                                for(var row_i=0; row_i < timeseries_y.length ; row_i++){
                                    if(timeseries_y[row_i]!="" && timeseries_y[row_i]!=null){

                                        if(!vlineMap.hasOwnProperty(splitbyvalue)){
                                            vlineMap[splitbyvalue]=[];
                                        }
                                        vlineMap[splitbyvalue].push( {
                                            type       : 'line'              ,
                                            x0         : timeseries_x[row_i] ,
                                            y0         : 0                   ,
                                            x1         : timeseries_x[row_i] ,
                                            y1         : 6                   ,
                                            line: {
                                                color: 'black'  ,
                                                width: 2        ,
                                                dash : 'dashdot'
                                            }                                ,
                                            xref      : 'x'                  ,
                                            yref      : 'y'+yax              ,
                                        } );
                                        if(!annotationMap.hasOwnProperty(splitbyvalue)){
                                            annotationMap[splitbyvalue]=[];
                                        }
                                        annotationMap[splitbyvalue].push( {
                                            x           : timeseries_x[row_i]                     ,
                                            y           : 5                                       ,
                                            xref        : 'x'                                     ,
                                            yref        : 'y'+yax                                 ,
                                            text        : plt_vars[var_i]+'='+timeseries_y[row_i] ,
                                            showarrow   : true                                    ,
                                            arrowhead   : 4                                       ,
                                            arrowsize   : 1                                       ,
                                            arrowwidth  : 2                                       ,
                                            arrowcolor  : '#636363'                               ,
                                            ax          : 20                                      ,
                                            ay          : -30                                     ,
                                            bordercolor : '#c7c7c7'                               ,
                                            borderwidth : 2                                       ,
                                            borderpad   : 4                                       ,
                                            bgcolor     : '#ff7f0e'                               ,
                                            opacity     : 0.8                                     ,
                                            font      : {
                                                family : 'Courier New, monospace',
                                                size : 12
                                            }
                                        } );

                                    };

                                };
                            };

                        };
                    };
                };

                var subplotId=0;
                for(var splitvalue in timeSubplotData){
                    subplotId=subplotId+1;
                    var subtimeID="timeSubplot"+subplotId;
                    if(Object.keys(timeSubplotData).length==1){
                        var tmp="<div id='"+subtimeID+"' class='singleplot'></div>";
                    }else{
                        if(split_by == 'USUBJID'){
                            var tmp="<div id='"+subtimeID+"' class='patientsubplot'></div>";
                        }else{
                            var tmp="<div id='"+subtimeID+"' class='subplot'></div>";
                        }
                    }
                    $("#canvasArea").append(tmp);
                    var localsplit=[];
                    localsplit.push((splitvalue=='all'?'':splitvalue));
                    var locallayout=gen_split_layout(localsplit , num_cols = num_cols , margin = 0.02 , mode , l_vars , r_vars , x_col_name)
                    locallayout['annotations']=annotationMap[splitvalue];
                    locallayout['shapes'     ]=vlineMap[splitvalue];

                    for (var iFilters = 0; iFilters < queryObj["global_filters"].length; ++iFilters) {
                        if (queryObj["global_filters"][iFilters]["col"] === "TS_DATE_RANGE") {
                            locallayout['xaxis']['range'] = queryObj["global_filters"][iFilters]["value"];
                        }
                    }
                    
                    Plotly.plot(subtimeID, timeSubplotData[splitvalue], locallayout , {showLink: false});
                }


            };


        }
    })
});