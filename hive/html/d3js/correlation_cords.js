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


function vjD3JS_CorrelationCord ( viewer )
{
    loadCSS("d3js/css/correlation_cords.css");
    vjD3CategoryView.call(this,viewer);
    var divElement;
    var that = this;
    
    if (this.margin == undefined || !this.margin) {this.margin = 100;}
    if (this.showTicks == undefined) {this.showTicks=0;}
    if (this.showLabels == undefined) {this.showLabels=0;}
    if (this.staticColor){
        this.originalRandom = Math.random()*1000000;
        this.tmpRandom = this.originalRandom;
        this.addSeed = Math.random()*1000000;
    }
    this.categToNumber=0;
    
    this.transformToMatrix = function (data){
        var label = [];
        var leftHeader = [];
        var topHeader = Object.keys(data[0]);
        var topHeaderLength = topHeader.length;
        var intermediateMatrix = matrixCreateEmpty(data.length,topHeaderLength-1);
        for (var i=0; i< data.length; ++i){
            var row = data[i];
            for (var k=0; k < topHeaderLength; ++k){
                var hdr = topHeader[k];
                if (k==0) leftHeader.push(row[hdr]);
                else {
                    intermediateMatrix[i][k-1]= Math.abs(parseFloat(row[hdr]));
                }
            }
        }
        var matrixSquareLength = (topHeader.length-1) + data.length;
        var generalMatrix = matrixCreateEmpty(matrixSquareLength,matrixSquareLength);        
        for (var ir=0; ir< intermediateMatrix.length; ++ir){
            for (var ic=0; ic < intermediateMatrix[ir].length; ++ic){
                var value = intermediateMatrix[ir][ic];
                generalMatrix[topHeaderLength-1+ir][ic] = value;
            }
        }
        for (var ir=0; ir< intermediateMatrix.length; ++ir){
            for (var ic=0; ic < intermediateMatrix[ir].length; ++ic){
                var value = intermediateMatrix[ir][ic];
                generalMatrix[ic][topHeaderLength-1+ir] = value;
            }
        }
        topHeader.splice(0,1)
        label = label.concat(topHeader);
        label = label.concat(leftHeader);
        return {
            matrix: generalMatrix, 
            labels:label
        };
    }
    
    function assignColListToColHdr(hdrArr, specialString) {
        var value = specialString;
        var start = value.match(/\[[0-9]*\:/)[0];
        start = parseInt(start.substring(1,start.length-1)) -1;
        var end = value.match(/\:-?[0-9]*\]/)[0];
        end = parseInt(end.substring(1,end.length-1))-1;
        
        if (end <0){
            end = hdrArr.length + end + 1;
        }
        myArr=new Array();
        for (var ic=0; ic < hdrArr.length; ++ic) {
            if (ic<start) continue;
            if (ic > end) break;
            myArr.push([hdrArr[ic]]);
        }
        return myArr;
    }
    
    function assignColNumToColHdr(arr, hdrArr) {
        var newArr = [];
        var len = arr.length;
        var hdrLen = hdrArr.length;
        for ( var ip=0; ip<len; ++ip){
            var val = arr[ip];
            if (val==undefined) val=ip;
            if (isNaN(val) && val.indexOf("#COL[")!=-1) {
                val = parseInt(val.substring(5,val.length-1))-1;
                if (val<0) val = hdrLen + val +1;
                newArr.push(hdrArr[val]);
            }
            else if (isNaN(val) && val.indexOf("#COLS[")!=-1) {
                var tmpArr = assignColListToColHdr(hdrArr, val);
                newArr = newArr.concat(tmpArr);
            }
            else if (isNaN(val)) {
                newArr.push(val);
            }
            else {
                newArr.push(hdrArr[val]);
            }
        }
        return newArr;
    }

    
    this.pairsTreatment = function(data) {
        var topHeaderArr = Object.keys(data[0]);
        if (typeof(this.pairs.array)!="string") {
            for (var iA=0; iA<this.pairs.array.length; ++iA) {
                this.pairs.array[iA]= assignColNumToColHdr(this.pairs.array[iA], topHeaderArr);
            }
        }
        else {
            this.pairs.array= assignColNumToColHdr(verarr(this.pairs.array), topHeaderArr);
        }
        this.pairValue= assignColNumToColHdr(this.pairValue, topHeaderArr);
        
        var elementToMap = [this.multiColor,this.singleColor];
        for (var ie=0; ie < elementToMap.length; ++ie) {
            var cur_element = elementToMap[ie];
            if (cur_element) {
                var hdrLen = topHeaderArr.length;
                for (var i=0; i< cur_element.columns.length; ++i) {
                    var val = cur_element.columns[i].col;
                    if (val.indexOf("#COL[")!=-1) {
                        val = parseInt(val.substring(5,val.length-1))-1;
                        if (val<0) val = hdrLen + val +1;
                        cur_element.columns[i].col = topHeaderArr[val];
                    }
                }
            }
        }
    }
    
    this.transformToMatrixPairs = function(data)
    {
        var generalMatrix=[];
        var labels={};
        var labels_length=0;
        var labelArr=[];
        var maxRows=0, maxCols=0;
        this.multiColor = this.multiColor || {};
        this.multiColor.columns = this.multiColor.columns || [];
        
        this.pairsTreatment(data);
        
        var lset=[];
        
        for ( var i =0; i<data.length; ++i  ){
            if (this.pairs.eval)
            {
                var label1="", label2="";
                var dt = {};
                for ( var ip=0; ip<this.pairs.array[0].length; ++ip) {dt[this.pairs.array[0][ip]] = data[i][this.pairs.array[0][ip]];}
                label1 = eval (this.pairs.eval[0]);
                for ( var ip=0; ip<this.pairs.array[1].length; ++ip) {dt[this.pairs.array[1][ip]] = data[i][this.pairs.array[1][ip]];}
                label2 = eval (this.pairs.eval[1]);
                if(!labels[label1]){
                    lset.push(label1);
                    labels[label1]=1;
                }
                if(!labels[label2]) { 
                    lset.push(label2);
                    labels[label2]=1;
                }
            }
            else
            {
                for (var iA=0; iA<this.pairs.array.length; ++iA) {
                    var label = "";
                    var multiColorCol = -1;
                    var singleColorCol = -1;
                    for ( var ip=0; ip<this.pairs.array[iA].length; ++ip){ 
                        if(label.length && ip){label+=":";}
                        if (this.categorizeByColumn && ip==0) {
                            label+="category_" + this.pairs.array[iA][ip] + "||";
                        }
                        if (this.multiColor && this.multiColor.columns && ip==0){
                            for (var mti=0; mti < this.multiColor.columns.length; ++mti) {
                                if (this.multiColor.columns[mti].col == this.pairs.array[iA][ip]){
                                    multiColorCol = mti;
                                    if (!this.multiColor.columns[mti].labels || i==0) {
                                        this.multiColor.columns[mti].labels={};
                                        this.multiColor.columns[mti].minIndex = Number.MAX_VALUE;
                                        this.multiColor.columns[mti].maxIndex = Number.MIN_VALUE;
                                    }
                                    break;
                                }    
                            }
                        }
                        if (this.singleColor && this.singleColor.columns && ip==0){
                            for (var mti=0; mti < this.singleColor.columns.length; ++mti) {
                                if (this.singleColor.columns[mti].col == this.pairs.array[iA][ip]){
                                    singleColorCol = mti;
                                    if (!this.singleColor.columns[mti].labels || i==0) {
                                        this.singleColor.columns[mti].labels={};
                                    }
                                    break;
                                }    
                            }
                        }
                        label+=data[i][this.pairs.array[iA][ip]];
                    }
                    if (multiColorCol!=-1 && !this.multiColor.columns[multiColorCol].labels[label]){
                        this.multiColor.columns[multiColorCol].labels[label]=1;
                    }
                    if (singleColorCol!=-1 && !this.singleColor.columns[singleColorCol].labels[label]){
                        this.singleColor.columns[singleColorCol].labels[label]=1;
                    }
                    if(!labels[label]){
                        lset.push(label);
                        labels[label]=1;
                    }
                }
            }
            
            if (this.forVDJ){
                var vLabel = "category_V||" + data[i][this.pairs.array[0][0]];
                var dLabel = "category_D||" + data[i][this.pairs.array[1][0]];
                var jLabel = "category_J||" + data[i][this.pairs.array[2][0]];
                if(!this.dict["V"][vLabel]){
                    this.dict["V"][vLabel]={};
                }
                if (!this.dict["V"][vLabel][dLabel]){
                    this.dict["V"][vLabel][dLabel]={};
                }
                if (!this.dict["V"][vLabel][dLabel][jLabel]){
                    this.dict["V"][vLabel][dLabel][jLabel]=1;
                } 
                
                if(!this.dict["D"][dLabel]){
                    this.dict["D"][dLabel]={};
                }
                if (!this.dict["D"][dLabel][vLabel]){
                    this.dict["D"][dLabel][vLabel] = {};
                }
                if (!this.dict["D"][dLabel][vLabel][jLabel]) {
                    this.dict["D"][dLabel][vLabel][jLabel]=1;
                }
                
                if(!this.dict["J"][jLabel]){
                    this.dict["J"][jLabel]={};
                }
                if (!this.dict["J"][jLabel][dLabel]){
                    this.dict["J"][jLabel][dLabel]={};
                }
                if (!this.dict["J"][jLabel][dLabel][vLabel]) {
                    this.dict["J"][jLabel][dLabel][vLabel]=1;
                }
                
                
            }
            
        }
        var ss=lset.sort();
        labels={};
        
        for(var il=0; il<ss.length; ++il ) { 
            labels[ss[il]]=il;
            labelArr.push(ss[il]);
            for (var ii=0; ii<this.multiColor.columns.length; ++ii){
                var subLabelObj = this.multiColor.columns[ii];
                if (subLabelObj.labels[ss[il]]!=undefined) {
                    subLabelObj.labels[ss[il]] = il;
                    if (il < subLabelObj.minIndex){subLabelObj.minIndex = il;}
                    if (il > subLabelObj.maxIndex){subLabelObj.maxIndex = il;}
                }
            }
            if (this.forVDJ) {
                if (ss[il].indexOf("category_V")!=-1) {
                    if (il < this.colorV.minIndex) this.colorV.minIndex = il;
                    if (il > this.colorV.maxIndex) this.colorV.maxIndex = il;
                }
            }
        }
        
        if (this.forVDJ) {
            this.colorV.color =  d3.scale.linear()
             .domain([this.colorV.minIndex,((this.colorV.minIndex+this.colorV.maxIndex)/2), this.colorV.maxIndex])
             .range(["red","yellow", "violet"]);
        }
        
        
        for ( var i =0; i<data.length; ++i  ){
            var labelList = [];
            if (this.pairs.eval)
            {
                var label1="", label2="";
                var dt = {};
                for ( var ip=0; ip<this.pairs.array[0].length; ++ip) {dt[this.pairs.array[0][ip]] = data[i][this.pairs.array[0][ip]];}
                label1 = eval (this.pairs.eval[0]);
                for ( var ip=0; ip<this.pairs.array[1].length; ++ip) {dt[this.pairs.array[1][ip]] = data[i][this.pairs.array[1][ip]];}
                label2 = eval (this.pairs.eval[1]);
                
                if( labels[label1]===undefined ){
                    labels[label1]=labels_length;
                    labelArr[labels_length]=label1;
                    ++labels_length;
                }
                if( labels[label2]===undefined ) {
                    labels[label2]=labels_length;
                    labelArr[labels_length]=label2;
                    ++labels_length;
                }
                
            }
            else
            {
                for (var iA=0; iA<this.pairs.array.length; ++iA) {
                    var label = "";
                    for ( var ip=0; ip<this.pairs.array[iA].length; ++ip){
                        if(label.length && ip){label+=":";}
                        if (this.categorizeByColumn && ip==0) {
                            label+="category_" + this.pairs.array[iA][ip] + "||";
                        }
                        label+=data[i][this.pairs.array[iA][ip]];
                    }
                    labelList.push(label);
                }
            }
            var value=0;
            for ( var ip=0; ip<this.pairValue.length; ++ip) {
                var cellV = data[i][this.pairValue[ip]];
                if (isNaN(parseFloat(cellV))) value += 1;
                else value+=parseFloat(cellV);
            }
            
            
            
            for (var iL=0; iL < labelList.length; ++iL) {
                var label1 = labelList[iL];
                var ilabel1= labels[label1];
                if (this.forVDJ) {
                    if (label1.indexOf("category_D")!=-1) {
                        this.colorArr[ilabel1] = this.colorD;
                    } else if (label1.indexOf("category_J")!=-1){
                        this.colorArr[ilabel1] = this.colorJ;
                    } else {
                        this.colorArr[ilabel1]=this.colorV.color(ilabel1);
                    }
                }
                for (var j=iL+1; j < labelList.length; ++j) {
                    var label2 = labelList[j]; 
                    var ilabel2= labels[label2];
                    
                    if(!generalMatrix[ilabel1])
                        generalMatrix[ilabel1]=[];
                    if(!generalMatrix[ilabel2])
                        generalMatrix[ilabel2]=[];
                            
                    if(!generalMatrix[ilabel1][ilabel2])
                        generalMatrix[ilabel1][ilabel2]=0;
                    if(!generalMatrix[ilabel2][ilabel1])
                        generalMatrix[ilabel2][ilabel1]=0;
                    
                    generalMatrix[ilabel1][ilabel2]+=value;
                    generalMatrix[ilabel2][ilabel1]+=value;
                    
                    if(ilabel1>maxRows)maxRows=ilabel1+1;
                    if(ilabel2>maxCols)maxCols=ilabel2+1;
                    if (this.forVDJ){
                    }
                }
            }
            
        }
        
        var dim=labelArr.length;
        
        for( var ir=0; ir<dim; ++ir  ) {
            if(!generalMatrix[ir])
                generalMatrix[ir]=[];
            
            for( var ic=0; ic<dim; ++ic  ){
                if( !generalMatrix[ir][ic] ) {
                    generalMatrix[ir][ic]=0;
                }
            }    
        }
        
        return {
            matrix: generalMatrix, 
            labels:labelArr
        };
    }
    
    
    this.d3Compose=function(data){       
        this.tmpRandom = this.originalRandom;
        this.d3Compose_prv(data);
        var thiSS=this;
        var svg=this.d3svg;
        if (this.container) {
            divElement = gObject(this.container); 
            if (this.width == undefined || !this.width){this.width = parseInt(divElement.style.maxWidth);}
            if (this.height == undefined || !this.height){this.height = parseInt(divElement.style.maxHeight);}
            if (!this.height) this.height=400;
            if (!this.width) this.width=800;
        }
        if (!this.forVDJ) this.forVDJ = false;
        if (this.forVDJ) {
            this.dict={V:{},D:{},J:{}};
            this.colorD = "blue";
            this.colorJ = "black";
            this.colorV = {minIndex: Number.MAX_VALUE, maxIndex: Number.MIN_VALUE};
            this.colorArr = [];
            this.colorMat = [];
            
        }
        
        var radius = Math.min(this.width, this.height) / 3 - this.margin;
        
        if (!this.wrapLength)
            this.wrapLength = (Math.min(this.width, this.height) - 2 * (radius + 10))/2;
        
        var innerRadius = radius;
        var outerRadius = innerRadius * 1.08;

        var fill =this.categoryColors;
        
        var arc = d3.svg.arc().innerRadius(innerRadius).outerRadius(outerRadius);
        var matrixObj = (this.pairs ) ? this.transformToMatrixPairs(data) : this.transformToMatrix(data);
        this.matrixObjCpy = matrixObj;

        this.categToNumber = 0;
        
        if (this.oneColorPerCateg && (this.categToNumber == 0 || vjDS[this.data].timeDone > new Date().getTime()-1800))
        {
            this.categToNumber = {};
            for (var i = 0; i < matrixObj.labels.length; i++)
            {
                var curLabel = matrixObj.labels[i];
                var category = curLabel.split(":")[0];
                if (this.categorizeByColumn) {
                    category = curLabel.split("||")[0];
                }
                if (!this.categToNumber[category]){
                    if(this.staticColor){
                        this.categToNumber[category] = Math.round((this.tmpRandom + this.addSeed) % (fill.length-4))+3;
                        this.tmpRandom += this.addSeed;
                    }
                    else
                        this.categToNumber[category] = Math.round((Math.random()*1000000) % (fill.length-4))+3;
                }                    
            }
        }
        
        if (this.multiColor && this.multiColor.columns) {
            for (var im=0; im<this.multiColor.columns.length; ++im) {
                var colorObj = this.multiColor.columns[im];
                var fromColor = (colorObj.from) ? (colorObj.from) : ("red");
                var middleColor = (colorObj.middle) ?  (colorObj.middle) : ("yellow");
                var toColor = (colorObj.to) ?  (colorObj.to) : ("violet");
                colorObj.color = d3.scale.linear()
                                 .domain([colorObj.minIndex,((colorObj.minIndex+colorObj.maxIndex)/2), colorObj.maxIndex])
                                 .range([fromColor,middleColor, toColor]);
                
            }
        }
        const chordWidth = that.fontOptions && that.fontOptions.cordWidth ? parseFloat(that.fontOptions.cordWidth)  : .015;
        var chord = d3.layout.chord()
            .padding(chordWidth)
            .sortSubgroups(d3.descending)
            .matrix(matrixObj.matrix);

        svg.attr("width", this.width)
            .attr("height", this.height)
        
        var gg = svg.append("g")
            .attr("transform", "translate(" + this.width/2  + "," + this.height/2  + ")");
        
        gg.append("g")
            .selectAll("path")
            .data(chord.groups)
            .enter().append("path")
            .style("fill", function(d) {
                if (that.categToNumber != 0){
                    var elementLabel = matrixObj.labels[d.index];
                    if (that.forVDJ) {
                        return that.colorArr[d.index];
                    }
                    if (that.categorizeByColumn){
                        var colorConfigArr = [that.multiColor, that.singleColor];
                        for (var icc=0; icc < colorConfigArr.length; ++icc) {
                            var cur_color_config = colorConfigArr[icc];
                            if (cur_color_config && cur_color_config.columns){
                                for (var im=0; im<cur_color_config.columns.length; ++im) {
                                    var colorObj = cur_color_config.columns[im];
                                    if (colorObj.labels[elementLabel] != undefined){
                                        if (!icc){
                                            return colorObj.color(colorObj.labels[elementLabel]);
                                        } else {
                                            return colorObj.color;
                                        }
                                    }
                                }
                            }
                        }
                        return pastelColor(fill[that.categToNumber[elementLabel.split("||")[0]]],0.6);
                    }
                    return pastelColor(fill[that.categToNumber[matrixObj.labels[d.index].split(":")[0]]],0.6);
                }    
                return pastelColor(fill[d.index],0.6); })
            .style("stroke", function(d) {
                var elementLabel = matrixObj.labels[d.index];
                if (that.forVDJ) {
                    return that.colorArr[d.index];
                }
                if (that.categToNumber != 0){
                    if (that.categorizeByColumn){
                        var colorConfigArr = [that.multiColor, that.singleColor];
                        for (var icc=0; icc < colorConfigArr.length; ++icc) {
                            var cur_color_config = colorConfigArr[icc];
                            if (cur_color_config && cur_color_config.columns){
                                for (var im=0; im<cur_color_config.columns.length; ++im) {
                                    var colorObj = cur_color_config.columns[im];
                                    if (colorObj.labels[elementLabel] != undefined){
                                        if (!icc){
                                            return colorObj.color(colorObj.labels[elementLabel]);
                                        } else {
                                            return colorObj.color;
                                        }
                                    }
                                }
                            }
                        }
                        return pastelColor(fill[that.categToNumber[elementLabel.split("||")[0]]],0.6);
                    }
                    return pastelColor(fill[that.categToNumber[elementLabel.split(":")[0]]],0.6);
                }    
                return pastelColor(fill[d.index],0.6); })
            .attr("d", arc)
            .on("mouseover", fade(.1))
            .on("mouseout", fade(1))
            .on("click", this.onClickCallback)
            .attr("index", function(d) { return d.index})
            .attr("labelFull", function (d){ return matrixObj.labels[d.index];});

          gg.append("g").selectAll(".arc")
            .data(chord.groups)
            .enter().append("svg:text")
            .attr("dy", ".35em")
            .attr("font-weight", "bold")
            .style("font-family", "sans-serif")
            .attr("text-anchor", function(d) { return ((d.startAngle + d.endAngle) / 2) > Math.PI ? "end" : null; })
            .attr("transform", function(d) {
              return "rotate(" + (((d.startAngle + d.endAngle) / 2) * 180 / Math.PI - 90) + ")"
                  + "translate(" + (outerRadius + 50) + ")"
                  + (((d.startAngle + d.endAngle) / 2) > Math.PI ? "rotate(180)" : "");
            })
            .text(function(d) {
                let text  = matrixObj.labels[d.index];
                if (text.match(/category_/)) text = text.split("||")[1];
                if (that.trimTextCallback) text = that.trimTextCallback(text)
                return text;
            })
            .on("click", this.onClickCallback)
            .call(wrap, this.wrapLength, this.totalTextLength,this.fontOptions);
          
        gg.append("g")
            .attr("class", "chord")
            .selectAll("path")
            .data(chord.chords)
            .enter().append("path")
            .attr("d", d3.svg.chord().radius(innerRadius))
            .style("fill", function(d) {
                var elementLabel = matrixObj.labels[d.target.index];
                if (that.forVDJ) {
                    return that.colorArr[d.target.index];
                }
                if (that.categToNumber != 0){
                    if (that.categorizeByColumn) {
                        var colorConfigArr = [that.multiColor, that.singleColor];
                        for (var icc=0; icc < colorConfigArr.length; ++icc) {
                            var cur_color_config = colorConfigArr[icc];
                            if (cur_color_config && cur_color_config.columns){
                                for (var im=0; im<cur_color_config.columns.length; ++im) {
                                    var colorObj = cur_color_config.columns[im];
                                    if (colorObj.labels[elementLabel] != undefined){
                                        if (!icc){
                                            return colorObj.color(colorObj.labels[elementLabel]);
                                        } else {
                                            return colorObj.color;
                                        }
                                    }
                                }
                            }
                        }
                        return pastelColor(fill[that.categToNumber[matrixObj.labels[d.target.index].split("||")[0]]],0.6)
                    }
                    return pastelColor(fill[that.categToNumber[matrixObj.labels[d.target.index].split(":")[0]]],0.6);
                }    
                return pastelColor(fill[d.target.index],0.6); })
            .style("opacity", 1);
        
        var ticks;
        if (this.showTicks){
             ticks= gg.append("g").selectAll("g")
                .data(chord.groups)
              .enter().append("g").selectAll("g")
                .data(groupTicks)
              .enter().append("g")
              .attr("transform", function(d) {
                      return "rotate(" + (d.angle * 180 / Math.PI - 90) + ")"
                              + "translate(" + outerRadius + ",0)";
                });
    
            ticks.append("line")
                .attr("x1", 1)
                .attr("y1", 0)
                .attr("x2", 5)
                .attr("y2", 0)
                .style("stroke", "#000");
            
            ticks.append("text")
                .attr("x", 8)
                .attr("dy", ".35em")
                .attr("font-size", "50px")
                .attr("font-weight", "bold")
                .attr("transform", function(d) { return d.angle > Math.PI ? "rotate(180)translate(-16)" : null; })
                .style("text-anchor", function(d) { return d.angle > Math.PI ? "end" : null; })
                .text(function(d) { 
                    return d.label; 
                });
    
            
            function groupTicks(d) {
              var k = (d.endAngle - d.startAngle) / d.value;
              return d3.range(0, d.value, 1000).map(function(v, i) {
                return {
                   angle: v * k + d.startAngle
                  ,label: i % 5 ? null : v/1000 + "k"
                };
              });
            };
        };
        this.performTheFade = function (opacity, index)
        {
                svg.selectAll("g.chord path")
                .filter(function(d) { 
                    return d.source.index != index && d.target.index != index; 
                 })
                .transition()
                .style("opacity", opacity);
            
        }
        
        function fade(opacity) {
              return function(g, i) {
                    return that.performTheFade(opacity, i);
              };
        };
        
        
    };
}

