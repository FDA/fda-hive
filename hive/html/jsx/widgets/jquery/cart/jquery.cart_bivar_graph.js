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
    $.widget("cart.cart_bivar_graph", {

        options: {
            value: 'STUDYID',
            data:  "tes"
        },

        _create: function () {
            
            $("#biVarGraph").append("<div id='canvas'></div>")
            
            var graphData;
            var filterColumn = this.options.value ;

            var res = this.options.data ;            
            
            Plotly.setPlotConfig({ modeBarButtonsToRemove: ['sendDataToCloud'] });
            $('#middle').remove();
                            
                  graphData = JSON.parse(res);
                var grph_typ = ["Bar", "Box", "Scatter"];

                graphDataKeys = Object.keys(graphData);
                $("#x_bivar").appendOptions(graphDataKeys);
                $("#y_bivar").appendOptions(graphDataKeys);
                $("#bivar_graph_type").appendOptions(grph_typ);

                $("#bivar_clr_by").appendOptions(graphDataKeys);
                $("#bivar_clr_by").prepend('<option  selected>NONE</option>');
                
                var grpByvals = get_unique_groupBy_values(graphData, filterColumn);

                $("#bivar_group").appendOptions(grpByvals);
                $("#bivar_group").prepend('<option  selected>NONE</option>');

            $("#bivarGrphBtn").click(function () {
                $('#center').remove();


                var graphType = $("#bivar_graph_type").val();
                var xvalueField = $("#x_bivar").val();
                var yvalueField = $("#y_bivar").val();

                var colorByField = $("#bivar_clr_by").val();
                var filterValue  = $("#bivar_group").val();

                var canvas = $('<div id="center"><div id="canvasArea" style="height:98% width:99%"></div>');

                
                var cur_mode     = null;
                var x_array      = [];
                var y_array      = [];
                var color_array  = [];
                var filter_array = [];

                if (filterValue === "NONE") {

                    x_array = graphData[xvalueField];
                    y_array = graphData[yvalueField];
                    if (colorByField != 'NONE') {
                        color_array = graphData[colorByField];
                    }
                }
                else {
                    filter_array = graphData[filterColumn];
                    xArr         = graphData[xvalueField ];
                    yArr         = graphData[yvalueField ];
                    if (colorByField != 'NONE') {
                        cArr = graphData[colorByField];
                    }
                    for (var i = 0; i < filter_array.length; i++) {
                        if (filter_array[i] == filterValue) {
                            x_array.push(xArr[i]);
                            y_array.push(yArr[i]);
                            if (colorByField != 'NONE') {
                                color_array.push(cArr[i]);
                            }
                        }
                    }
                };
                
                if (graphType === "Scatter") {    

                    traces = draw_scatter_plot(cartJson  = graphData     , 
                                              x_col_name = xvalueField   , 
                                              y_col_name = yvalueField   ,
                                              labelByField = null        ,
                                              colorByField = colorByField
                    );

                    TESTER = document.getElementById('canvas');
                    Plotly.newPlot(TESTER, traces, {margin: { t: 0 }});
                }
                else if (graphType === "Bar") {
                    
                    traces_layout = draw_bar_plot(
                                  cartJson        = graphData     , 
                                  x_col_name      = xvalueField   , 
                                  y_col_name      = yvalueField   , 
                                  filter_col_name = filterColumn  , 
                                  filter_value    = filterValue
                    );

                    traces = traces_layout[0];
                    layout = traces_layout[1];  
                    TESTER = document.getElementById('canvas');
                    Plotly.newPlot(TESTER, traces, layout, {margin: { t: 0 }});

                }
                else if (graphType === "Box") {
                    $('#canvasArea').empty();


                    traces_layout = draw_box_plot(    
                                        cartJson       = graphData    , 
                                        data_col_name  = yvalueField  , 
                                        group_col_name = xvalueField  ,
                                        orientation    = 'v'          ,
                                        filter_col_name = filterColumn,
                                        filter_value    = filterValue

                    )

                    traces = traces_layout[0];
                    layout = traces_layout[1];  
                    TESTER = document.getElementById('canvas');
                    Plotly.newPlot(TESTER, traces, layout, {margin: { t: 0 }});
                }
            });


            function get_unique_groupBy_values(cartJson, split_by) {

                var uniqueValues = [];
                for (var i = 0; i < cartJson[split_by].length; i++) {
                    if (uniqueValues.indexOf(cartJson[split_by][i]) < 0) {
                        uniqueValues.push(cartJson[split_by][i]);
                    };
                };
                return uniqueValues;
            };


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

            function draw_scatter_plot(cartJson    , 
                                       x_col_name  , 
                                       y_col_name  ,
                                       labelByField,
                                       colorByField
            ){

                var label_array = cartJson[labelByField];
                var color_array = cartJson[colorByField];
                var x_array     = cartJson[x_col_name  ].map(parseFloat);
                var y_array     = cartJson[y_col_name  ].map(parseFloat);
    
                var col_by_unqs = []
                if(colorByField!='' && colorByField!=null && colorByField!='NONE' ){ 
                    col_by_unqs = new Set(color_array)
                    col_by_unqs = Array.from(col_by_unqs).sort().filter(Boolean)
                }else{
                    col_by_unqs = ['All']
                }
    
                var graphData  = [];
    
                var cur_x_arr       = x_array 
                var cur_y_arr       = y_array 
                var cur_col_array   = color_array 
                var cur_label_array = label_array
    
                for(var j=0;j<col_by_unqs.length;j++){
                    var cur_col_by  = col_by_unqs[j];
    
                    if (cur_col_by == 'All'){
                        var cur_x_arr2       = cur_x_arr;
                        var cur_y_arr2       = cur_y_arr;
                        var cur_col_array2   = cur_col_array;
                        var cur_label_array2 = cur_label_array;
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
    
                return graphData;

            };

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


            function draw_bar_plot(cartJson, x_col_name, y_col_name, filter_col_name, filter_value ) {
                $('#canvasArea').empty();

                var x_array     = cartJson[ x_col_name ];
                var y_array     = cartJson[ y_col_name ];
                
                cross_tab_json = {};

                if (filter_col_name != "NONE" && filter_value!="NONE" ){
                    var fltr_array  = cartJson[filter_col_name  ];

                    x_array  = filter_by_array(x_array , fltr_array , filter_value);
                    y_array  = filter_by_array(y_array , fltr_array , filter_value);
                }

                var cur_y_unqs = new Set(y_array)
                var cur_y_unqs = Array.from(cur_y_unqs).sort()
                cur_y_unqs.unshift('All')

                var cur_x_unqs = new Set(x_array)
                var cur_x_unqs = Array.from(cur_x_unqs).sort()
                cur_x_unqs.unshift('All')
                for(var j = 0; j < cur_y_unqs.length; j++){
                    var cur_y_grp = cur_y_unqs[j];
                    cross_tab_json[ cur_y_grp ] = {};

                    for(var k = 0; k < cur_x_unqs.length; k++){
                        var cur_x_grp = cur_x_unqs[k];
                        var res_cnt = 0;
                        if(cur_y_grp == 'All'){
                            if(cur_x_grp == 'All'){
                                res_cnt = x_array.length;
                            }else{
                                res_cnt =  x_array.filter(x => x===cur_x_grp).length; 
                            };
                        }else{
                            if(cur_x_grp == 'All'){
                                res_cnt =  y_array.filter(x => x===cur_y_grp).length;
                            }else{
                                res_cnt =  get_cross_tab_cnt( x_array , 
                                                              y_array ,
                                                              cur_x_grp,
                                                              cur_y_grp ); 
                            };
                        };
                        cross_tab_json[ cur_y_grp ][cur_x_grp] = res_cnt;
                    };

                };



                
                var sum = 0;

                for(var j = 0; j < cur_y_unqs.length; j++){
                    var cur_y_grp = cur_y_unqs[j];
                    for(var k = 0; k < cur_x_unqs.length; k++){
                        var cur_x_grp = cur_x_unqs[k];

                        var xtotal = cross_tab_json['All'][cur_x_grp];
                        var ytotal = cross_tab_json[cur_y_grp]['All'];
                        var N      = cross_tab_json['All']['All']
                        var actual = cross_tab_json[cur_y_grp][cur_x_grp];

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
                pval=cur_pval;



                var graphData    = [];
                var annotation   = [];

                for (var j = 0; j < cur_y_unqs.length; j++) {
                    yfield = cur_y_unqs[j];

                    yps = []
                    xps = []
                    ycnt = []
                    for (var k = 0; k < cur_x_unqs.length; k++){
                        xfield = cur_x_unqs[k];
                        if( ( yfield in cross_tab_json) && ( xfield in cross_tab_json[yfield]) ){
                            xps.push( xfield );
                            yps.push( cross_tab_json[yfield][xfield] / cross_tab_json['All'][xfield] );
                            ycnt.push( cross_tab_json[yfield][xfield] );
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
                    text      : 'p-value= '+pval.toFixed(2)+'' ,
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
                        
                        title       : "", 
                        annotations : annotation
                };

                return [graphData,subLayout]
            

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

            function draw_box_plot(    cartJson       , 
                                       data_col_name  , 
                                       group_col_name ,
                                       orientation    ,
                                       filter_col_name,
                                       filter_value

            ){


            var data_array  = cartJson[data_col_name ].map(parseFloat);
            var group_array = cartJson[group_col_name];

            if (filter_col_name != "NONE" && filter_value!="NONE" ){
                var fltr_array  = cartJson[filter_col_name  ];

                data_array  = filter_by_array(data_array   , fltr_array , filter_value);
                group_array  = filter_by_array(group_array , fltr_array , filter_value);
            }

            group_by_unq = new Set(group_array)
            group_by_unq = Array.from(group_by_unq).sort().filter(Boolean)


            var graphData  = [];
            var annotation = [];

            var res_filter = filter_by_null( data_array, group_array )

            var data_array  = res_filter[0];
            var group_array = res_filter[1];                

            p_val = cart_anova( data_array , group_array );
            for(var j=0;j<group_by_unq.length;j++){
                var cur_group_by  = group_by_unq[j];
    
                var data_array2  = filter_by_array(data_array   , group_array , cur_group_by);
                var group_array2 = filter_by_array(group_array  , group_array , cur_group_by);

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
                    scatter_trace['y' ] = data_array2;
                    scatter_trace['x0'] = cur_group_by;
                }else if(orientation=='h'){
                    scatter_trace['x' ] = data_array2;
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

                    title       : "",
                    annotations : annotation
            };

            if(orientation=='v'){
                subLayout['yaxis' ]['title'] = data_col_name;
                subLayout['xaxis' ]['title'] = group_col_name;
            }else if(orientation=='h'){
                subLayout['yaxis' ]['title'] = group_col_name;
                subLayout['xaxis' ]['title'] = data_col_name;
            };
            
            return [graphData, subLayout]
            

            };

        }
    });
});