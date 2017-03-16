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


if (!javaScriptEngine) var javaScriptEngine = vjJS["undefined"];

function vjD3JS_ParallelCoord ( viewer )
{
    loadCSS("d3js/css/parcoords_plot.css");
    vjD3View.call(this,viewer); // inherit default behaviors of the DataViewer
    
    this.data=viewer.data;
    viewer.marin ? this.margin = viewer.margin : this.margin = {top: 30, right: 30, bottom: 50, left: 30};
    viewer.width ? this.width = viewer.width : this.width = 300;
    viewer.height ? this.height = viewer.height : this.height = 300;
    this.diameter = this.width/2;
    viewer.label ? this.label = viewer.label : this.label="";
    this.fontSize=10;
    viewer.colors ? this.colors = viewer.colors : this.colors = ["#4000ff", "#00ffc9"];
    viewer.labelCol ? this.labelCol = viewer.labelCol : this.labelCol = 0;
    viewer.rangeCol ? this.rangeCol = viewer.rangeCol : this.rangeCol = 1;
    viewer.colSet ? this.colSet = viewer.colSet : this.colSet = undefined;
    viewer.weightCol ? this.weightCol = viewer.weightCol : this.weightCol = {col: 0};
    viewer.accumulateWeight ? this.accumulateWeight = viewer.accumulateWeight : this.accumulateWeight = undefined;
    viewer.idCol ? this.idCol = viewer.idCol : this.idCol = undefined;
    viewer.keyColorPhrase ? this.keyColorPhrase = viewer.keyColorPhrase : this.keyColorPhrase = "";
    
    var that = this;
    this.d3Compose=function(content)
    {
        var margin = this.margin,
            width = this.width - margin.left - margin.right,
            height = this.height - margin.top - margin.bottom;
    
        var x = d3.scale.ordinal().rangePoints([0, width], 1),
            y = {},
            dragging = {};
    
        var line = d3.svg.line()
                    .interpolate("monotone"),
            axis = d3.svg.axis().orient("left"),
            background,
            foreground;
        

        dataSet = this.csvParserFunction (content);
        dataSetNotConverted = this.csvParserFunction (content);
        
        var tblArr = new vjTable(content, 0, vjTable_propCSV, undefined, undefined, undefined, undefined, 1);
        var tblArrHdr = new Array();
        
        for (var i = 0; i < tblArr.hdr.length; i++)
            tblArrHdr.push(tblArr.hdr[i].name);
        

        if (this.keyColorColName && this.keyColorColName != "")
            this.rangeCol = tblArrIndexOf(tblArr, this.keyColorColName);
        
        var labelColName = tblArr.hdr[this.labelCol];
        var rangeColName = tblArr.hdr[this.rangeCol];
            
        var min = Number.MAX_VALUE, max = Number.MIN_VALUE;
        colDictionary = new Array(tblArr.hdr.length);
        for (var i = 0; i < colDictionary.length; i++)
            colDictionary[i]=[];
        
        var weightColDictionary = {};
        
        if (typeof this.weightCol.col == "number")//if a number was passed in
            this.weightCol.name = tblArr.hdr[this.weightCol.col].name;
        
        for (var i = 0; i < tblArr.rows.length; i++)
        {
            if (parseFloat(tblArr.rows[i].cols[this.rangeCol]) > max) max = parseFloat(tblArr.rows[i].cols[this.rangeCol]);
            if (parseFloat(tblArr.rows[i].cols[this.rangeCol]) < min) min = parseFloat(tblArr.rows[i].cols[this.rangeCol]);
            
            var weightKey = "";
            for (var j = 0; this.colSet && j < this.colSet.length; j++)
            {
                if (this.colSet[j].colName && tblArrIndexOf(tblArr, this.colSet[j].colName) >=0)
                {
                    var jj = tblArrIndexOf(tblArr, this.colSet[j].colName);
                    if (colDictionary[jj].indexOf(tblArr.rows[i][this.colSet[j].colName]) < 0)
                        colDictionary[jj].push(tblArr.rows[i][this.colSet[j].colName]);
                        
                    weightKey += tblArr.rows[i][this.colSet[j].colName] + ":";
                }
                else if (typeof this.colSet[j].colIndex == "number" && this.colSet[j].colIndex >= 0)
                {
                    if (colDictionary[this.colSet[j].colIndex].indexOf(tblArr.rows[i].cols[this.colSet[j].colIndex]) < 0)
                        colDictionary[this.colSet[j].colIndex].push(tblArr.rows[i].cols[this.colSet[j].colIndex]);
                    
                    this.colSet[j].colName = tblArr.hdr[this.colSet[j].colIndex].name;
                    weightKey += tblArr.rows[i].cols[this.colSet[j].colIndex] + ":";
                }
            }
            
            if (!weightColDictionary[weightKey]) weightColDictionary[weightKey] = 0;
            
            if (this.weightCol instanceof Object)
            {
                var cellValue = tblArr.rows[i][this.weightCol.name];
                var value = parseFloat(cellValue.split(this.weightCol.splitBy)[this.weightCol.indexBy]);
                weightColDictionary[weightKey] += value;
            }
            else
                weightColDictionary[weightKey]++;
            
            if (this.colSet.indexOf(this.rangeCol) < 0)
            {
                var cellValue = tblArr.rows[i][rangeColName.name];
                
                if (this.keyColorPhrase && this.keyColorPhrase != "")
                {
                    var allSplits = cellValue.split(" ");
                    
                    for (var kk = 0; kk < allSplits.length; kk++)
                    {
                        if (allSplits[kk].indexOf(this.keyColorPhrase) >= 0)
                        {
                            cellValue = allSplits[kk];
                            break;
                        }
                    }
                }
                
                if (colDictionary[this.rangeCol].indexOf(cellValue) < 0)
                    colDictionary[this.rangeCol].push(cellValue);
            }
        }
        
        for (var i = 0; i < colDictionary.length; i++)
            colDictionary[i].sort();
        
        if (colDictionary.length < this.rangeCol) return;
        
        if (min > max || min == max)
        {
            min = 0; 
            max = colDictionary[this.rangeCol].length;
        }
    
        var svg = this.d3svg
            .attr("width", width + margin.left + margin.right)
            .attr("height", height + margin.top + margin.bottom)
            .append("g")
                .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
        
        var tooltip = d3.select("body")
            .append("div")
            .style("border", "1.5px solid black")
            .style("background-color", "white")            
            .style("position", "absolute")
            .style("z-index", "150")
            .style("visibility", "hidden");
        
        var blue_to_brown = d3.scale.linear()
            .domain([min, max])
            .range(this.colors)
            .interpolate(d3.interpolateLab);
        
        
        //here we need to get rid of all of the columns which will not be used to generate the data
        for (var i = 0; i < dataSet.length; i++)
        {
            var curRow= dataSet[i];
            
            var k=-1;
            for (var key in curRow)
            {
                k++;
                //if (key == labelColName.name || key == this.weightCol) continue;
                if (this.colSet == undefined || findInColSet (key, k))
                {
                    var actualIndex = tblArrHdr.indexOf(key);
                    if (colDictionary[actualIndex].length > 0)
                        curRow[key] = colDictionary[actualIndex].indexOf(curRow[key]);
                }
                /*else
                    curRow[key]="";*/
            }
        }
        
        var finalLabelArray = [];
        for (var i = 0; i < colDictionary.length; i++)
        {
            if (colDictionary[i].length == 0) continue;
            
            for (var j = 0; j < colDictionary[i].length; j++)
                finalLabelArray.push(colDictionary[i][j]);
        }
    
          // Extract the list of dimensions and create a scale for each.
        x.domain(dimensions = d3.keys(dataSet[this.labelCol]).filter(function(d,i) {
            return /*(d != labelColName.name) && */(that.colSet == undefined || findInColSet (d,i)) && (y[d] = d3.scale.linear()
                .domain(d3.extent(dataSet, function(p) { return +p[d]; }))
                .range([height, 0]));
        }));
        
        function findInColSet (name, index)
        {
            if (that.colSet == undefined) return true;
            var actualIndex = tblArrHdr.indexOf(name);
            
            for (var i = 0; i < that.colSet.length; i++)
            {
                if (actualIndex == that.colSet[i].colIndex) return true;
                if (name == that.colSet[i].colName) return true;
            }
        }

      // Add grey background lines for context.
      background = svg.append("g")
          .attr("class", "background")
        .selectAll("path")
              .data(dataSet)
        .enter().append("path")
            .attr('stroke-width', 0.8)
              .attr('stroke', "#eaeaea")
            .attr("d", path);
      
      var lineFunction = d3.svg.line()
         .interpolate("cardinal");

      // Add blue foreground lines for focus.
      foreground = svg.append("g")
          .attr("class", "foreground")
          .selectAll("path")
              .data(dataSet)
          .enter().append("path")
              .attr('stroke-width', function (d, i) {
                  if (that.accumulateWeight)
                  {
                      var curD = dataSetNotConverted[i];
                      
                      var weightKey = "";
                    for (var j = 0; that.colSet && j < that.colSet.length; j++)
                    {
                        if (that.colSet[j].colName && tblArrIndexOf(tblArr, that.colSet[j].colName) >=0)
                            weightKey += curD[that.colSet[j].colName] + ":";
                    }
                    
                    return Math.log2( weightColDictionary[weightKey])==-Infinity ? 0.5 : 0.5+Math.log2( weightColDictionary[weightKey]);
                  }
                  
                  return Math.log2(d[that.weightCol])==-Infinity ? 0.5 : 0.5+Math.log2(d[that.weightCol]); 
              })
              .attr('stroke', function(d) {
                  if (typeof d[rangeColName.name] == "string")
                  {
                      var cellValue = d[rangeColName.name];
                      
                      if (that.keyColorPhrase && that.keyColorPhrase != "")
                    {
                        var allSplits = cellValue.split(" ");
                        
                        for (var kk = 0; kk < allSplits.length; kk++)
                        {
                            if (allSplits[kk].indexOf(that.keyColorPhrase) >= 0)
                            {
                                cellValue = allSplits[kk];
                                break;
                            }
                        }
                        
                    }
                      
                      return blue_to_brown(colDictionary[that.rangeCol].indexOf(cellValue));
                  }
                  
                  return blue_to_brown(d[rangeColName.name]); })
            .attr("d", path);
      
      svg.selectAll("path")
          .on("mouseover", function(node, x, y){
              tooltip.text("Name: " + node[labelColName.name]);
              return tooltip.style("visibility", "visible");})
          .on("mousemove", function(){return tooltip.style("top",
                  (d3.event.pageY-10)+"px").style("left",(d3.event.pageX+10)+"px");})
        .on("mouseout", function(){return tooltip.style("visibility", "hidden");});

      // Add a group element for each dimension.
      var g = svg.selectAll(".dimension")
          .data(dimensions)
          .enter().append("g")
              .attr("class", "dimension")
              .attr("transform", function(d) { 
                  return "translate(" + x(d) + ")"; })
              .call(d3.behavior.drag()
                .origin(function(d) { 
                    return {x: x(d)}; })
                .on("dragstart", function(d) {
                  dragging[d] = x(d);
                  background.attr("visibility", "hidden");
                })
                .on("drag", function(d) {
                  dragging[d] = Math.min(width, Math.max(0, d3.event.x));
                  foreground.attr("d", path);
                  dimensions.sort(function(a, b) { return position(a) - position(b); });
                  x.domain(dimensions);
                  g.attr("transform", function(d) { return "translate(" + position(d) + ")"; })
                })
                .on("dragend", function(d) {
                  delete dragging[d];
                  transition(d3.select(this)).attr("transform", "translate(" + x(d) + ")");
                  transition(foreground).attr("d", path);
                  background
                      .attr("d", path)
                    .transition()
                      .delay(500)
                      .duration(0)
                      .attr("visibility", null);
                }));

      // Add an axis and title.
      g.append("g")
          .attr("class", "axis")
          .each(function(d) { 
              d3.select(this).call(axis.scale(y[d]).ticks(colDictionary[tblArrIndexOf(tblArr, d)].length)); })
        .append("text")
          .style("text-anchor", "middle")
          .attr("y", -9)
          .text(function(d) { 
              return d; })
          .on("click",function(d) {
              that.keyColorColName = "";
              that.rangeCol=tblArrIndexOf(tblArr, d);
              that.refresh();}
          );
              //return d.substring (0, d.indexOf("-label")); }); //this just places the labels on the axis. This is not the tick values

      
      d3.selectAll(".axis")
          .each(function(curCol){
              d3.select(this).selectAll(".tick").selectAll("text")
              .text(function (d,i,j){
                  var x;
                  for (x=0; x < tblArr.hdr.length; x++)
                      if (tblArr.hdr[x].name == curCol) break;
                  return colDictionary[x][d];
              });
          })
      
      // Add and store a brush for each axis.
      g.append("g")
          .attr("class", "brush")
          .each(function(d) {
            d3.select(this).call(y[d].brush = d3.svg.brush().y(y[d]).on("brushstart", brushstart).on("brush", brush));
          })
        .selectAll("rect")
          .attr("x", -8)
          .attr("width", 16);
    
        function position(d) {
          var v = dragging[d];
          return v == null ? x(d) : v;
        }
    
        function transition(g) {
          return g.transition().duration(500);
        }
    
        // Returns the path for a given data point.
        //gotta figure out how to make the line's interpolations = "basis"
        //wasn't able to do it
        function path(d) {
          var l = line(dimensions.map(function(p) { return [position(p), y[p](d[p])]; }));
          return l;
        }
    
        function brushstart() {
          d3.event.sourceEvent.stopPropagation();
        }
    
        // Handles a brush event, toggling the display of foreground lines.
        function brush() {
          var actives = dimensions.filter(function(p) { return !y[p].brush.empty(); }),
              extents = actives.map(function(p) { return y[p].brush.extent(); });
          
          var labelsUsed=[];
          foreground.style("display", function(d) {
            var tmp = actives.every(function(p, i) 
            {
                   if (labelsUsed.length == 0 && !(extents[i][0] <= d[p] && d[p] <= extents[i][1]))
                       return false;
                   
                var tmp2={};
                for (var key in d)
                {
                    if (key == undefined) continue;
                    
                    var col = tblArrHdr.indexOf(key);
                    if (col == -1) continue;
                    
                    var ii;
                    for (ii = 0; ii < that.colSet.length; ii++)
                        if ((that.colSet[ii].colIndex == col || that.colSet[ii].colName == key) && that.colSet[ii].strings) break;
                    
                    if (ii != that.colSet.length) 
                        tmp2[key] = colDictionary[col][d[key]];
                    else
                        tmp2[key]=d[key];
                }
                
                
                if (extents[i][0] <= d[p] && d[p] <= extents[i][1])
                {
                    labelsUsed.push(tmp2);
                    return true;
                }
                var pos = jsonArrIndexOf(labelsUsed, tmp2)
                if ( pos >= 0 )
                {
                    labelsUsed.splice(pos,1);
                }
                 return false;
            });
            return tmp ? null : "none";
          });
          
          if (that.selectedCallback)
              that.selectedCallback(that, labelsUsed);
        }
    };
    
    function jsonArrIndexOf (array, jsonObj)
    {
        for (var i = 0; i < array.length; i++)
        {
            var possible = true;
            for (var key in jsonObj)
            {
                if (!array[i][key]){possible = false; break;}
                if (array[i][key] != jsonObj[key]) {possible = false; break;}
            }
            
            if (possible) return i;
        }
        
        return -1;
    }
    
    function tblArrIndexOf(tblArr, hdrName)
    {
        for (var ii = 0; ii < tblArr.hdr.length; ii++)
            if (tblArr.hdr[ii].name == hdrName) return ii;
        
        return -1;
    }
    
    return this;
}

function vjD3JS_ParallelCoordAC_Control ( viewer )
{
    var origianlViewer = viewer;
    ttThis = this;
    
    
    this.menu = new vjPanelView({
        data: ['dsVoid'],
        formObject: document.forms[formName],
        iconSize: 24,
        rows:[
              { name : 'keyCol', type : 'text', size : '20', isSubmitable : true, prefix : 'Column Name for Coloration', align : 'left', order : '4', title: "colName"},
              { name : 'keyVal', type : 'text', size : '20', isSubmitable : true, prefix : 'Key for Coloration', align : 'left', order : '4', title: "key"},
              { name : 'switchTables', type : 'select', title:" ", isSubmitable : true, value:"dsRefMap", options:[['dsRefMap','Reference Mapping Table'], ['dsCombMap','Combination Mapping Table']], align : 'left', order : '5'},
              { name : 'apply', order:100 , title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, align: "right", url: updateColors}
              ]
    }); 
    
    this.graph = new vjD3JS_ParallelCoord(viewer);
    
    function updateColors(viewer, node)
    {
        ttThis.graph.keyColorColName = viewer.tree.root.children[0].value;
        ttThis.graph.keyColorPhrase = viewer.tree.root.children[1].value;
        var val = viewer.tree.root.children[2].value;
        if (val == "dsRefMap"){
            ttThis.graph.dataCol = 2;
            ttThis.graph.rangeCol = 2;
            ttThis.graph.colSet = [{colIndex:1, strings:true},{colIndex:2, strings:true},{colIndex:3, strings:true}];
            ttThis.graph.weightCol = {name:"sequence", splitBy:"rpt=", indexBy:1};
        }
        else{
            ttThis.graph.dataCol = 2;
            ttThis.graph.rangeCol = 2;
            //ttThis.graph.labelCol = idCol;
            ttThis.graph.colSet = [{colIndex:0, strings:true},{colIndex:1, strings:true},{colIndex:2, strings:true}];
            ttThis.graph.weightCol = {col:0};
            ttThis.graph.keyColorColName = "count";
        }
        vjDS["dsGraphDS"].reload (vjDS[val].url, true);
        vjDS.dsGraphDS.register_callback (callRefresh);
        //ttThis.graph.refresh(vjDS[val].data);
        
        function callRefresh(a,b,c,d){
            ttThis.graph.refresh();
        }
    };
    
    
    return [this.menu, this.graph];
}

