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
function drawAreaPlot(seriesData, colNames, divName, plotTitle, xTitle, yTitle, tickFreq) {

        var data = new google.visualization.DataTable();
        data.addColumn('number', colNames[0]);
        data.addColumn('number', colNames[1]);
    data.addColumn('number', colNames[2]);
        data.addColumn({type: 'string', role: 'style'});

        var rows = [];
        for (var i in seriesData){
                rows.push([seriesData[i]["x"],  seriesData[i]["y1"], seriesData[i]["y2"], seriesData[i]["style"]]);
        }
        data.addRows(rows);

        var options = {
          title: plotTitle,
          hAxis: {title: xTitle, minValue: 0, maxValue: 15},
          vAxis: {title: yTitle, minValue: 0, maxValue: 15},
      legend: {position: 'top', maxLines: 3}
        };

    var chart = new google.visualization.AreaChart(document.getElementById(divName));
        chart.draw(data, options);
}


function drawScatterPlot(seriesData, divName, plotTitle, xTitle, yTitle, tickFreq) {

        var data = new google.visualization.DataTable();

        data.addColumn('number', 'x-values');
        data.addColumn('number', 'y-values');
        data.addColumn({type: 'string', role: 'style'});

        var rows = [];
        for (var i in seriesData){
                rows.push([seriesData[i]["x"],  seriesData[i]["y1"], seriesData[i]["style"]]);
        }
        data.addRows(rows);

    
    var options = {
          title: plotTitle,
          hAxis: {title: xTitle, minValue: 0, maxValue: 15},
          vAxis: {title: yTitle, minValue: 0, maxValue: 15},
          legend: 'none'
        };
     var chart = new google.visualization.ScatterChart(document.getElementById(divName));
        chart.draw(data, options);


}


function drawSingleSeries(seriesData, divName, xTitle, yTitle, tickFreq) {


        var data = new google.visualization.DataTable();
    data.addColumn('number', 'x-values');
        data.addColumn('number', 'y-values');
        data.addColumn({type: 'string', role: 'annotation'});
    data.addColumn({type: 'string', role: 'style'});
    var rows = [];
        for (var i in seriesData){
        seriesData[i]["annotation"] = (seriesData[i]["annotation"] == undefined ? null : seriesData[i]["annotation"]);
        rows.push([
                seriesData[i]["x"]  
                ,seriesData[i]["y1"]
                ,seriesData[i]["annotation"]
                ,seriesData[i]["style"]
            ]);
        }
        data.addRows(rows);


    var xTextStyle = { color: '#000', fontSize: '10'}; 
    var yTextStyle = {color: '#000',fontSize: '10'};
    var titleTextStyle = { color:'#777777', fontName:'', fontSize:11, bold:false, italic:false};
    var toolTip = {isHtml: true, textStyle:  {fontName: 'TimesNewRoman', fontSize: 10,bold: false}};

    var options = {'title':''
        ,titleTextStyle: titleTextStyle
        ,tooltip: toolTip
        ,annotations: {
            textStyle:  {fontName: 'TimesNewRoman', fontSize: 10,bold: false}
        }
        ,explorer: {actions:['dragToZoom','rightClickToReset']}
        ,vAxis:{title:yTitle, textStyle:yTextStyle, minValue:0}
        ,hAxis: {title: xTitle,textStyle:xTextStyle,slantedText:true,slantedTextAngle:45,showTextEvery:2}
        ,chartArea: { width: "100%", height: "50%", left:50, top:50}
        ,bar:{groupWidth:2}
        ,legend: 'none'
        ,width: "100%"
            ,height: "100%"
    };
        
    var my_div = document.getElementById(divName);
    var my_chart = new google.visualization.ColumnChart(my_div);
        my_chart.draw(data, options);



}


function drawToyChart(plotDivId) {

        var data = new google.visualization.DataTable();
        data.addColumn('string', 'Topping');
        data.addColumn('number', 'Slices');
        data.addRows([
          ['Mushrooms', 3],
          ['Onions', 1],
          ['Olives', 1],
          ['Zucchini', 1],
          ['Pepperoni', 2]
        ]);
        var options = {'title':'How Much Pizza I Ate Last Night',
                       'width':400,
                       'height':300};

        var chart = new google.visualization.PieChart(document.getElementById(plotDivId));
        chart.draw(data, options);

}






