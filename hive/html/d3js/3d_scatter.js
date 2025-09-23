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
loadJS('threejs/dat.gui.js')
function vjD3JS_3dScatter ( viewer )
{
    vjD3View.call(this,viewer);
    let that = this
    this.current_tab = viewer.id ?  document.querySelector(`#${viewer.id}`) : document.body
    this.gui = null

    this.options =  this.options || {}

    let categoryObject = {}
    let categoryArr = []

    var defaultColor = '#C80000';
    var textDefaultSize = this.options.textSize ? this.options.textSize : 16;
    var textDefaultColor = this.options.textColor ? this.options.textColor : "black";
    var textDefaultFont = this.options.font ? this.options.font : "Arial" ;
    let dataShape = {
        wireframe: false,
        opacity: 0.7
    }

    let dataCamera = {
        zoom: 1,
        position: {
            z: 200,
            x: -100,
            y: 100
        }
    }

    let dataScatter = {
        rotation: {y: 0}
    }

    let camera,
        height,
        width,
        renderer,
        scene,
        parent,
        infoDiv,
        graphDiv,
        legendRectDomId,
        legendDomId,
        coordinates,
        dataTable,
        scatterPlot
    ;

    this.createRenderer = function(){
        renderer = new THREE.WebGLRenderer({
            antialias: true
        });

        renderer.setSize(width, height);
        renderer.setClearColor(0xffffff, 1.0);
    }

    function createCamera() {
        const fov = 45;
        const aspect = width / height;
        const near = 1;
        const far = 10000;

        camera = new THREE.PerspectiveCamera(fov, aspect, near, far);
        camera.position.z = dataCamera.position.z;
        camera.position.x = dataCamera.position.x;
        camera.position.y = dataCamera.position.y;
        camera.zoom = dataCamera.zoom

    } 

    this.setWidthHeight = function() {
        var defaultWidth = 500;
        var defaultHeight = 500;
        
        var chartAreaPercentage = '90%';
        if (this.options !=undefined && this.options.chartArea != undefined && this.options.chartArea.length){
            var chA = parseFloat(this.options.chartArea);
            if (chA > 1) chartAreaPercentage = chA/100;
        }
        else chartAreaPercentage = parseFloat(chartAreaPercentage)/100;
        
        var parentDiv = gObject(this.container);
        var maxHeight = parseInt(parentDiv.style.maxHeight);
        var maxWidth = parseInt(parentDiv.style.maxWidth);
        
        width = this.width ? this.width : (this.options!=undefined ? (this.options.width ? this.options.width : 0) : 0);
        height = this.height ? this.height : (this.options!=undefined ? (this.options.height ? this.options.height : 0) : 0);
        if (!width) width = maxWidth ? maxWidth : defaultWidth;
        if (!height) height = maxHeight ? maxHeight : defaultHeight;
        
        width = width * chartAreaPercentage;
        height = height * chartAreaPercentage;

        this.options['width'] = width
        this.options['height'] = height
    }

    this.dataTreatment = function (data){
        var shapeList=["octahedron","box","sphere","cylinder","donut"];
        coordinates = [];
                
        var xCol = this.coordinates.x ? this.coordinates.x : 'x';  
        var yCol = this.coordinates.y ? this.coordinates.y : 'y';  
        var zCol = this.coordinates.z ? this.coordinates.z : 'z';
        var labelCols = this.options.labels || "1"; 


        var tbl = new vjTable(data,0,vjTable_propCSV);
        for( var id=1; id<this.data.length;  id++){
            var n=new vjTable(this.getData(id).data,0,vjTable_propCSV);
            tbl.appendTbl(n);;
        }

        xCol = ( !isNaN(xCol) && tbl.hdr[parseInt(xCol) - 1] ) 
                    ? (tbl.hdr[parseInt(xCol) - 1].name) 
                    : (xCol);
        yCol = ( !isNaN(yCol) && tbl.hdr[parseInt(yCol) - 1] ) 
                    ? (tbl.hdr[parseInt(yCol) - 1].name) 
                    : (yCol);
        zCol = ( !isNaN(zCol) && tbl.hdr[parseInt(zCol) - 1] ) 
                    ? (tbl.hdr[parseInt(zCol) - 1].name) 
                    : (zCol);


        labelCols = verarr(labelCols);
        
        var tmpArr = [];
        for (var i=0; i<labelCols.length; ++i) {
            var lb = labelCols[i]; 
            lb = ( !isNaN(lb) && tbl.hdr[parseInt(lb) - 1] ) ? (tbl.hdr[parseInt(lb) - 1].name) : (lb);
            tmpArr.push(lb);
        }
        labelCols = tmpArr;
        
        let colorShift = this.colorShift || 3;
        for (var ir=0; ir< tbl.rows.length; ++ir){
            var row = tbl.rows[ir];
            var lb="";
            for (var il=0; il < labelCols.length; ++il) {
                if (il) lb += "; ";
                lb += "" + labelCols[il] + ": " + row[labelCols[il]];
            }
            coordinates[ir] = {
                               x: + row[xCol] ,
                               y: + row[yCol],
                               z: + row[zCol],
                               label: "" + lb
                           };
            if (this.options.category){
                var columnForCategory = ( !isNaN(this.options.category) ) ? ( tbl.hdr[parseInt(this.options.category) - 1].name ) : (this.options.category);
                
                var catValue = row[columnForCategory];
                
                if (categoryObject.categories[catValue] == undefined) {
                    categoryObject.categories[catValue] = {};
                    categoryObject.categories[catValue].index = categoryObject.length;
                   
                    var color = gClrTable[( colorShift + (categoryObject.categories[catValue].index) ) % gClrTable.length];
                    ++categoryObject.length;
                    if (this.options.colors && this.options.colors[categoryObject.categories[catValue].index] != undefined){
                        color = this.options.colors[categoryObject.categories[catValue].index];
                    }
                    categoryObject.categories[catValue].color = color;
                    categoryObject.categories[catValue].shape = shapeList[(categoryObject.categories[catValue].index >= shapeList.length) ? (categoryObject.categories[catValue].index/shapeList.length) : categoryObject.categories[catValue].index];
                }
                categoryArr.push(catValue);
            }
        }
        return tbl;
    }

    function v(x, y, z){
        return new THREE.Vector3(x, y, z);
    }

    this.createScatterPlot = function () {
        var format = d3.format("+.3f");
        scatterPlot = new THREE.Object3D();
        scene.add(scatterPlot);

        scatterPlot.rotation.y = dataScatter.rotation.y;
        
        
        var xMinMaxArr = d3.extent(coordinates, function (d) {return d.x; }),
        yMinMaxArr = d3.extent(coordinates, function (d) {return d.y; }),
        zMinMaxArr = d3.extent(coordinates, function (d) {return d.z; });

        var xMinMaxObj = {min: xMinMaxArr[0], max: xMinMaxArr[1]};
        var yMinMaxObj = {min: yMinMaxArr[0], max: yMinMaxArr[1]};
        var zMinMaxObj = {min: zMinMaxArr[0], max: zMinMaxArr[1]};
        
        
        var vpts = {
            xMax: xMinMaxObj.max,
            xCen: (xMinMaxObj.max + xMinMaxObj.min) / 2,
            xMin: xMinMaxObj.min,
            yMax: yMinMaxObj.max,
            yCen: (yMinMaxObj.max + yMinMaxObj.min) / 2,
            yMin: yMinMaxObj.min,
            zMax: zMinMaxObj.max,
            zCen: (zMinMaxObj.max + zMinMaxObj.min) / 2,
            zMin: zMinMaxObj.min
        }
    
        xScale = d3.scale.linear()
                      .domain([xMinMaxObj.min, xMinMaxObj.max])
                      .range([-50,50]);
        
        yScale = d3.scale.linear()
                      .domain([yMinMaxObj.min, yMinMaxObj.max])
                      .range([-50,50]);
        
        zScale = d3.scale.linear()
                      .domain([zMinMaxObj.min, zMinMaxObj.max])
                      .range([-50,50]);
     
        var xAxisCoords = new THREE.Geometry();
            xAxisCoords.vertices.push(
                    v(xScale(vpts.xMin), yScale(vpts.yCen), zScale(vpts.zCen)), v(xScale(vpts.xMax), yScale(vpts.yCen), zScale(vpts.zCen))
            );
        
        var yAxisCoords = new THREE.Geometry();
            yAxisCoords.vertices.push(  
                    v(xScale(vpts.xCen), yScale(vpts.yMin), zScale(vpts.zCen)), v(xScale(vpts.xCen), yScale(vpts.yMax), zScale(vpts.zCen))
            ); 
                
        var zAxisCoords = new THREE.Geometry();
            zAxisCoords.vertices.push(
                    v(xScale(vpts.xCen), yScale(vpts.yCen), zScale(vpts.zMax)), v(xScale(vpts.xCen), yScale(vpts.yCen), zScale(vpts.zMin))
            );
            
        var axisConfig = new THREE.LineBasicMaterial({
            color: 0x000000
        });
        
        var xAxisLine = new THREE.Line(xAxisCoords, axisConfig); 
        var yAxisLine = new THREE.Line(yAxisCoords, axisConfig);
        var zAxisLine = new THREE.Line(zAxisCoords, axisConfig);
        
        scatterPlot.add(xAxisLine);
        scatterPlot.add(yAxisLine);
        scatterPlot.add(zAxisLine);
        
        
        var boxCoords = new THREE.Geometry();
        boxCoords.vertices.push( 
                                v(xScale(vpts.xMin), yScale(vpts.yMax), zScale(vpts.zMin)), v(xScale(vpts.xMax), yScale(vpts.yMax), zScale(vpts.zMin)),
                v(xScale(vpts.xMax), yScale(vpts.yMin), zScale(vpts.zMin)),v(xScale(vpts.xMin), yScale(vpts.yMin), zScale(vpts.zMin)),
                v(xScale(vpts.xMin), yScale(vpts.yMax), zScale(vpts.zMin)),
                v(xScale(vpts.xMin), yScale(vpts.yMax), zScale(vpts.zMax)), v(xScale(vpts.xMax), yScale(vpts.yMax), zScale(vpts.zMax)),
                v(xScale(vpts.xMax), yScale(vpts.yMin), zScale(vpts.zMax)), v(xScale(vpts.xMin), yScale(vpts.yMin), zScale(vpts.zMax)),
                v(xScale(vpts.xMin), yScale(vpts.yMax), zScale(vpts.zMax)), v(xScale(vpts.xMax), yScale(vpts.yMax), zScale(vpts.zMax)),
                v(xScale(vpts.xMax), yScale(vpts.yMax), zScale(vpts.zMin)), v(xScale(vpts.xMax), yScale(vpts.yMin), zScale(vpts.zMin)),
                v(xScale(vpts.xMax), yScale(vpts.yMin), zScale(vpts.zMax)), v(xScale(vpts.xMin), yScale(vpts.yMin), zScale(vpts.zMax)),
                v(xScale(vpts.xMin), yScale(vpts.yMin), zScale(vpts.zMin))
        ); 

        var boxConfig = new THREE.LineBasicMaterial({
            color: 0x333333 
        });
        
        var box = new THREE.Line(boxCoords, boxConfig);

        scatterPlot.add(box);
        
    
        
        var titleX = createText2D('-X',textDefaultColor,textDefaultFont,textDefaultSize);
        titleX.position.x = xScale(vpts.xMin) - 12,
        titleX.position.y = 5;
        scatterPlot.add(titleX);
    
        var valueX = createText2D(format(xMinMaxObj.min),textDefaultColor,textDefaultFont,textDefaultSize);
        valueX.position.x = xScale(vpts.xMin) - 12,
        valueX.position.y = -5;
        scatterPlot.add(valueX);

        var titleX = createText2D('X',textDefaultColor,textDefaultFont,textDefaultSize);
        titleX.position.x = xScale(vpts.xMax) + 12;
        titleX.position.y = 5;
        scatterPlot.add(titleX);
    
        var valueX = createText2D(format(xMinMaxObj.max),textDefaultColor,textDefaultFont,textDefaultSize);
        valueX.position.x = xScale(vpts.xMax) + 12,
        valueX.position.y = -5;
        scatterPlot.add(valueX);
    
        var titleY = createText2D('-Y',textDefaultColor,textDefaultFont,textDefaultSize);
        titleY.position.y = yScale(vpts.yMin) - 5;
        scatterPlot.add(titleY);
    
        var valueY = createText2D(format(yMinMaxObj.min),textDefaultColor,textDefaultFont,textDefaultSize);
        valueY.position.y = yScale(vpts.yMin) - 15,
        scatterPlot.add(valueY);
    
        var titleY = createText2D('Y',textDefaultColor,textDefaultFont,textDefaultSize);
        titleY.position.y = yScale(vpts.yMax) + 15;
        scatterPlot.add(titleY);
    
        var valueY = createText2D(format(yMinMaxObj.max),textDefaultColor,textDefaultFont,textDefaultSize);
        valueY.position.y = yScale(vpts.yMax) + 5,
        scatterPlot.add(valueY);
    
        var titleZ = createText2D('-Z ' + format(zMinMaxObj.min),textDefaultColor,textDefaultFont,textDefaultSize);
        titleZ.position.z = zScale(vpts.zMin) + 2;
        scatterPlot.add(titleZ);
    
        var titleZ = createText2D('Z ' + format(zMinMaxObj.max),textDefaultColor,textDefaultFont,textDefaultSize);
        titleZ.position.z = zScale(vpts.zMax) + 2;
        scatterPlot.add(titleZ);
    
        var mat = new THREE.PointsMaterial({
            vertexColors: true,
            size: 10
        });
        
        this.createDataPoints()
       
    }

    this.createDataPoints = function () {
         var domEvent = new THREEx.DomEvent(camera,renderer.domElement);
         var pointCount = coordinates.length;
         
         for (var i = 0; i < pointCount; i ++) {
             var radius = 2 ;
             var color = defaultColor;
             
             var shapeMaterial = new THREE.MeshBasicMaterial({
                 wireframe: dataShape.wireframe,
                 transparent:true,
                 opacity: dataShape.opacity,
                 depthTest:true,
                 depthWrite:true,
                 color:color,
             });
             
             
             var shapeGeometry=new THREE.SphereGeometry( radius, 32, 16 );
 
             if (this.options.category){
                 this.options
                 color = categoryObject.categories[categoryArr[i]].color;
                 shapeGeometry=composeGeometryShape(categoryObject.categories[categoryArr[i]].shape);
             }
 
             var clr = new THREE.Color(color);
             shapeMaterial.color.r = clr.r;
             shapeMaterial.color.g = clr.g;
             shapeMaterial.color.b = clr.b;
 
             var shape = new THREE.Mesh(shapeGeometry, shapeMaterial);
 
             shape.position.x = xScale(coordinates[i].x);
             shape.position.y = yScale(coordinates[i].y);
             shape.position.z = zScale(coordinates[i].z);
             shape.name = "" + coordinates[i].label;
             shape.myColor = color; 
             
        
             domEvent.bind(shape, "mouseover", function (curObj) {
                curObj.target.scale.x *= 1.25
                curObj.target.scale.z *= 1.25
                curObj.target.scale.y *= 1.25
                infoDiv.innerHTML = `<p style="margin:0px">${curObj.target.name}</p>`
                infoDiv.style.width = curObj.target.name.length * 10
                infoDiv.style.backgroundColor = (curObj.target.myColor ? ( curObj.target.myColor + '45') : "#009999") 
                infoDiv.style.padding = "5px 30px"
                renderer.render(scene, camera);
             });

             domEvent.bind(shape, "mouseout", function (curObj) {
                curObj.target.scale.x /= 1.25
                curObj.target.scale.z /= 1.25
                curObj.target.scale.y /= 1.25
                renderer.render(scene, camera);
             });
             
             scatterPlot.add(shape);
             
             if (this.options.label) {
                 var labelCol = (this.options.label[0]=="#") ? (dataTable.hdr[parseInt(this.options.label.substring(1)) - 1].name) : (this.options.label) ;
                 var labelName = dataTable.rows[i][labelCol];
                 
                 var label = createText2D(labelName,textDefaultColor,textDefaultFont,textDefaultSize-2);
                 label.position.x = xScale(coordinates[i].x),
                 label.position.y = yScale(coordinates[i].y);
                 label.position.z = zScale(coordinates[i].z);
                 scatterPlot.add(label);
             }
         }
    }

    function createTextCanvas(text, color, font, size) {
        size = size || 16;
        var canvas = document.createElement('canvas');
        var ctx = canvas.getContext('2d');
        var fontStr = (size + 'px ') + (font || 'Arial');
        ctx.font = fontStr;
        var w = ctx.measureText(text).width;
        var h = Math.ceil(size);
        canvas.width = w;
        canvas.height = h;
        ctx.font = fontStr;
        ctx.fillStyle = color || 'black';
        ctx.fillText(text, 0, Math.ceil(size * 0.8));
        return canvas;
    }
    
    function createText2D(text, color, font, size, segW, segH) {
        var canvas = createTextCanvas(text, color, font, size);
        var plane = new THREE.PlaneBufferGeometry(canvas.width, canvas.height, segW, segH);
        var tex = new THREE.Texture(canvas);
        var planeMat = new THREE.MeshBasicMaterial({
            map: tex,
            color: 0xffffff,
            transparent: true
        });
        
        var mesh = new THREE.Mesh(plane, planeMat);
        mesh.scale.set(0.25, 0.25, 0.25);
        mesh.doubleSided = true;
        return mesh;
    }
    
    function composeGeometryShape(shapeName) {
        var radius=2;
        
        switch (shapeName.toUpperCase()) {
            case "CYLINDER":
                return new THREE.CylinderGeometry(radius, radius, 2*radius, 40, false);;
            case "OCTAHEDRON": 
                return new THREE.OctahedronGeometry(radius*1.5);
            case "DONUT":
                return new THREE.TorusGeometry( radius, 0.4, 2*radius, 8 );
            case "BOX":
                return new THREE.BoxGeometry( radius*1.5, radius*1.5, radius*1.5 );
            default:
                return new THREE.SphereGeometry( radius, 32, 16 );
        }
    }

    let animator ;
    function animate(an) {
        if (an) {
            animator = requestAnimationFrame(animate, renderer.domElement);
        }
    
        camera.lookAt(scene.position);
        renderer.render(scene, camera);
    };

    let init = false;

    this.d3Compose=function(data){ 
        this.div.innerHTML = `
                            <div data-scatter="info"> </div>
                            <div data-scatter="graph"></div>
                        `
        infoDiv = this.div.querySelector(`[data-scatter="info"]`)
        graphDiv = this.div.querySelector(`[data-scatter="graph"]`)

        if(!init){
            this.d3Compose_prv(data);

            categoryObject = {length:0, categories:{}, legendDomId: legendDomId, legendRectDomId: legendRectDomId};
            dataTable = this.dataTreatment(data);


            init = true
        }


        this.renderPlot()
    }

    this.scatterRefresh = function() {
        alert('refresh')
    }

    this.renderPlot = function(){
        scene = new THREE.Scene();
        this.setWidthHeight()
        this.createRenderer()
        createCamera()

        graphDiv.appendChild(renderer.domElement);

        this.createScatterPlot()

        
        var down = false;
        var sx = 0,
            sy = 0;
        
        graphDiv.onmousedown = function(ev) {
            animate(true)
            down = true;
            sx = ev.clientX;
            sy = ev.clientY;
        };
        
        graphDiv.onmouseup = function() {
            cancelAnimationFrame( animator )
            down = false;
            dataCamera.position = {...camera.position}
            dataScatter.rotation.y = scatterPlot.rotation.y
        };
        
        graphDiv.onmousemove = function(ev) {
            if (down) {
                var dx = ev.clientX - sx;
                var dy = ev.clientY - sy;
                scatterPlot.rotation.y += dx * 0.01;
                camera.position.y += dy;
                sx += dx;
                sy += dy;
            }
        }
        
        animate();

        createGUI()
    }

    this.update = function(){
        that.d3Compose()
    }
    
    function createGUI() {
        that.div.style.position = 'relative'

        that.gui = new dat.GUI({ autoPlace: true, scrollable:false});
        const gui_id = `gui-${viewer.id}`
        that.gui.domElement.setAttribute('id' , gui_id)

        let current_gui = that.div.querySelector(`#${gui_id}`)
        if(current_gui) current_gui.remove()

        that.gui.domElement.style.position = 'absolute'
        that.gui.domElement.style.top = '40px'
        that.gui.domElement.style.right = '0'

        that.div.append(that.gui.domElement)
        var camGUI = that.gui.addFolder('Graph');

        camGUI.add(camera, 'zoom', 0, 5).name('Zoom').listen().onChange(function(z){  
            dataCamera.zoom = z
            camera.updateProjectionMatrix()
            animate()
        });
        if(that.options.category){
            Object.keys(categoryObject.categories).forEach( function(cat){
                camGUI.addColor(categoryObject.categories[cat],'color').name(cat).onFinishChange(function(arg){ 
                    categoryObject.categories[cat].color = arg
                    that.update()
                })
            })
        }
        camGUI.add(dataShape, 'wireframe',dataShape.wireframe).name('Wireframe').listen().onFinishChange(function(bool){    
             dataShape.wireframe = bool  
             that.update()
         });
        camGUI.add(dataShape, 'opacity',0.1 , 1).name('Opacity').listen().onFinishChange(function(op){   
              dataShape.opacity = op  
              that.update()
         });
        camGUI.open();
    }
}

