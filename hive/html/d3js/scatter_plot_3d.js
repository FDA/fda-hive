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

// Create a 3d scatter plot within d3 selection parent.
function scatterPlot3d( parent )
{
  var x3d = parent  
    .append("x3d")
      .style( "width", parseInt(parent.style("width"))+"px" )
      .style( "height", parseInt(parent.style("height"))+"px" )
      .style( "border", "none" )
      
  var scene = x3d.append("scene")

  scene.append("orthoviewpoint")
     .attr( "centerOfRotation", [5, 5, 5])
     .attr( "fieldOfView", [-5, -5, 15, 15])
     .attr( "orientation", [-0.5, 1, 0.2, 1.12*Math.PI/4])
     .attr( "position", [8, 4, 15])

  var rows = initializeDataGrid();
  var axisRange = [0, 10];
  var scales = [];
  var initialDuration = 0;
  var defaultDuration = 800;
  var ease = 'linear';
  var time = 0;
  var axisKeys = ["x", "y", "z"]

  // Helper functions for initializeAxis() and drawAxis()
  function axisName( name, axisIndex ) {
    return ['x','y','z'][axisIndex] + name;
  }

  function constVecWithAxisValue( otherValue, axisValue, axisIndex ) {
    var result = [otherValue, otherValue, otherValue];
    result[axisIndex] = axisValue;
    return result;
  }

  // Used to make 2d elements visible
  function makeSolid(selection, color) {
    selection.append("appearance")
      .append("material")
         .attr("diffuseColor", color||"black")
    return selection;
  }

  // Initialize the axes lines and labels.
  function initializePlot() {
    initializeAxis(0);
    initializeAxis(1);
    initializeAxis(2);
  }

  function initializeAxis( axisIndex )
  {
    var key = axisKeys[axisIndex];
    drawAxis( axisIndex, key, initialDuration );

    var scaleMin = axisRange[0];
    var scaleMax = axisRange[1];

    // the axis line
    var newAxisLine = scene.append("transform")
         .attr("class", axisName("Axis", axisIndex))
         .attr("rotation", ([[0,0,0,0],[0,0,1,Math.PI/2],[0,1,0,-Math.PI/2]][axisIndex]))
      .append("shape")
    newAxisLine
      .append("appearance")
      .append("material")
        .attr("emissiveColor", "lightgray")
    newAxisLine
      .append("polyline2d")
         // Line drawn along y axis does not render in Firefox, so draw one
         // along the x axis instead and rotate it (above).
        .attr("lineSegments", "0 0," + scaleMax + " 0")

   // axis labels
   var newAxisLabel = scene.append("transform")
       .attr("class", axisName("AxisLabel", axisIndex))
       .attr("translation", constVecWithAxisValue( 0, scaleMin + 1.1 * (scaleMax-scaleMin), axisIndex ))

   var newAxisLabelShape = newAxisLabel
     .append("billboard")
       .attr("axisOfRotation", "0 0 0") // face viewer
     .append("shape")
     .call(makeSolid)

   var labelFontSize = 0.6;

   newAxisLabelShape
     .append("text")
       .attr("class", axisName("AxisLabelText", axisIndex))
       .attr("solid", "true")
       .attr("string", key)
    .append("fontstyle")
       .attr("size", labelFontSize)
       .attr("family", "SANS")
       .attr("justify", "END MIDDLE" )
  }

  // Assign key to axis, creating or updating its ticks, grid lines, and labels.
  function drawAxis( axisIndex, key, duration ) {

    var scale = d3.scale.linear()
      .domain( [-5,5] ) // demo data range
      .range( axisRange )
    
    scales[axisIndex] = scale;

    var numTicks = 8;
    var tickSize = 0.1;
    var tickFontSize = 0.5;

    // ticks along each axis
    var ticks = scene.selectAll( "."+axisName("Tick", axisIndex) )
       .data( scale.ticks( numTicks ));
    var newTicks = ticks.enter()
      .append("transform")
        .attr("class", axisName("Tick", axisIndex));
    newTicks.append("shape").call(makeSolid)
      .append("box")
        .attr("size", tickSize + " " + tickSize + " " + tickSize);
    // enter + update
    ticks.transition().duration(duration)
      .attr("translation", function(tick) { 
         return constVecWithAxisValue( 0, scale(tick), axisIndex ); })
    ticks.exit().remove();

    // tick labels
    var tickLabels = ticks.selectAll("billboard shape text")
      .data(function(d) { return [d]; });
    var newTickLabels = tickLabels.enter()
      .append("billboard")
         .attr("axisOfRotation", "0 0 0")     
      .append("shape")
      .call(makeSolid)
    newTickLabels.append("text")
      .attr("string", scale.tickFormat(10))
      .attr("solid", "true")
      .append("fontstyle")
        .attr("size", tickFontSize)
        .attr("family", "SANS")
        .attr("justify", "END MIDDLE" );
    tickLabels // enter + update
      .attr("string", scale.tickFormat(10))
    tickLabels.exit().remove();

    // base grid lines
    if (axisIndex==0 || axisIndex==2) {

      var gridLines = scene.selectAll( "."+axisName("GridLine", axisIndex))
         .data(scale.ticks( numTicks ));
      gridLines.exit().remove();
      
      var newGridLines = gridLines.enter()
        .append("transform")
          .attr("class", axisName("GridLine", axisIndex))
          .attr("rotation", axisIndex==0 ? [0,1,0, -Math.PI/2] : [0,0,0,0])
        .append("shape")

      newGridLines.append("appearance")
        .append("material")
          .attr("emissiveColor", "gray")
      newGridLines.append("polyline2d");

      gridLines.selectAll("shape polyline2d").transition().duration(duration)
        .attr("lineSegments", "0 0, " + axisRange[1] + " 0")

      gridLines.transition().duration(duration)
         .attr("translation", axisIndex==0
            ? function(d) { return scale(d) + " 0 0"; }
            : function(d) { return "0 0 " + scale(d); }
          )
    }  
  }

  // Update the data points (spheres) and stems.
  function plotData( duration ) {
    
    if (!rows) {
     console.log("no rows to plot.")
     return;
    }

    var x = scales[0], y = scales[1], z = scales[2];
    var sphereRadius = 0.2;

    // Draw a sphere at each x,y,z coordinate.
    var datapoints = scene.selectAll(".datapoint").data( rows );
    datapoints.exit().remove()

    var newDatapoints = datapoints.enter()
      .append("transform")
        .attr("class", "datapoint")
        .attr("scale", [sphereRadius, sphereRadius, sphereRadius])
      .append("shape");
    newDatapoints
      .append("appearance")
      .append("material");
    newDatapoints
      .append("sphere")
       // Does not work on Chrome; use transform instead
       //.attr("radius", sphereRadius)

    datapoints.selectAll("shape appearance material")
        .attr("diffuseColor", 'steelblue' )

    datapoints.transition().ease(ease).duration(duration)
        .attr("translation", function(row) { 
          return x(row[axisKeys[0]]) + " " + y(row[axisKeys[1]]) + " " + z(row[axisKeys[2]])})

    // Draw a stem from the x-z plane to each sphere at elevation y.
    // This convention was chosen to be consistent with x3d primitive ElevationGrid. 
    var stems = scene.selectAll(".stem").data( rows );
    stems.exit().remove();

    var newStems = stems.enter()
      .append("transform")
        .attr("class", "stem")
      .append("shape");
    newStems
      .append("appearance")
      .append("material")
        .attr("emissiveColor", "gray")
    newStems
      .append("polyline2d")
        .attr("lineSegments", function(row) { return "0 1, 0 0"; })

    stems.transition().ease(ease).duration(duration)
        .attr("translation", 
           function(row) { return x(row[axisKeys[0]]) + " 0 " + z(row[axisKeys[2]]); })
        .attr("scale",
           function(row) { return [1, y(row[axisKeys[1]])]; })
  }

  function initializeDataGrid() {
    var rows = [];
    // Follow the convention where y(x,z) is elevation.
    for (var x=-5; x<=5; x+=1) {
      for (var z=-5; z<=5; z+=1) {
        rows.push({x: x, y: 0, z: z});
     }
    }
    return rows;
  }

  function updateData() {
    time += Math.PI/8;
    if ( x3d.node() && x3d.node().runtime ) {
      for (var r=0; r<rows.length; ++r) {
        var x = rows[r].x;
        var z = rows[r].z;
        rows[r].y = 5*( Math.sin(0.5*x + time) * Math.cos(0.25*z + time));
      }
      plotData( defaultDuration );
    } else {
      console.log('x3d not ready.');
    }
  }

  initializeDataGrid();
  initializePlot();
  setInterval( updateData, defaultDuration );
}


