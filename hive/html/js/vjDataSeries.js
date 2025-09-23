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
function vjDataSeries(source, wantRegisterDS) {

    vjDataSource.call(this, source);
    if (wantRegisterDS === undefined) wantRegisterDS = true;
    if (wantRegisterDS) this.registerDS(source.name);

    this.callbacks.series_loaded = new Array();
    this.isDataSeries = true;

    if (this.bump === undefined)                this.bump = false;
    if (this.type == 'sankey' || this.type == "anotBox")                    this.bump = true;
    if (this.type == 'column') this.computeColumnFlag = true;
    if (this.columnDefinition) {
        if (this.columnDefinition.x === undefined) this.columnDefinition.x = this.columnDefinition.start;
    }

    this.changeDS=function(source){
        for(var attr in source){
            if(this[attr] && (attr=='window' || attr=='dataLoaded' || attr=='onDataLoaded'
                || attr=='parseRawData' || attr=='scaleRawData'
                    || attr=='bumpRawData' || attr=='computeColumn'
                        || attr=='callbacks' || attr=='has_nontrivial_data'))
                continue;
            this[attr]=source[attr];
        }

        this.registerDS(this.name);
        return this;
    };

    this.window = function(newXStart, newXEnd, resolution) {

        var url = vjDS[this.name].url;
        url = urlExchangeParameters(url, "start", newXStart ? newXStart : '-');
        url = urlExchangeParameters(url, "end", newXEnd ? newXEnd : '-');
        url = urlExchangeParameters(url, "resolution", resolution ? resolution : '-');
        this.reload(url);
    };

    this.dataLoaded = 'onDataLoaded';

    this.has_nontrivial_data = function() {
        return this.state == "done" && this.tblArr && this.tblArr.rows.length;
    };

    this.onDataLoaded = function(callback, content, param) {
        this.level = 0.25 ;
        this.tblArr = new vjTable(content, 0, vjTable_propCSV);
        if (this.coordinates)
            delete this.coordinates;
        if(this.precompute)
            this.tblArr.enumerate(this.precompute);
        if (this.bump)
            this.bumpRawData();
        if (this.type != "raw")
            this.parseRawData();
        if (this.computeColumnFlag)
            this.computeColumn();
        this.call_callbacks("series_loaded");
    };


    this.parseRawData = function() {
        this.min = {
                x : 1e+300,
                y : 1e+300,
                z : 1e+300
            };
        this.max = {
                x : -1e+300,
                y : -1e+300,
                z : -1e+300
            };
        if (this.columnDefinition.y === undefined)    this.columnDefinition.y = this.columnDefinition.weight;
        if (this.tblArr.rows !== undefined && this.tblArr.rows.length){
            if (this.valueMax){
                if (this.valueMax.x) {
                    if(typeof(this.valueMax.x)=="number") this.max.x = this.valueMax.x;
                    if(typeof(this.valueMax.x)=="string") {
                        this.max.x = this.tblArr.rows[0][this.valueMax.x];
                    }
                }
                if (this.valueMax.y) {
                    if(typeof(this.valueMax.y)=="number") this.max.y = this.valueMax.y;
                    if(typeof(this.valueMax.y)=="string") {
                        this.max.y = this.tblArr.rows[0][this.valueMax.y];
                    }
                }
            }
        }
        for ( var i = 0; i < this.tblArr.rows.length; ++i) {

            var node = this.tblArr.rows[i];
            if (this.columnDefinition instanceof Array){
                var serieLength = this.columnDefinition.length;
                for (var is=0;is<serieLength;is++){
                    var nodeX = parseFloat(node[this.columnDefinition[is].x]);
                    var nodeY = parseFloat(node[this.columnDefinition[is].y]);
                    var nodeZ = parseFloat(node[this.columnDefinition[is].z]);
                    if (nodeX < this.min.x) this.min.x = nodeX;
                    if (nodeX > this.max.x) this.max.x = nodeX;

                    if (nodeY < this.min.y) this.min.y = nodeY;
                    if (nodeY > this.max.y) this.max.y = nodeY;

                    if (nodeZ < this.min.z) this.min.z = nodeZ;
                    if (nodeZ > this.max.z) this.max.z = nodeZ;
                }
            }
            else{
                if (this.columnDefinition.x) {
                    node.x = parseFloat(node[this.columnDefinition.x]);
                    if (node.x < this.min.x)
                        this.min.x = node.x;
                    var tmaxX=node.x;
                    if(this.columnDefinition.end)tmaxX=parseFloat(node[this.columnDefinition.end]);
                    if (tmaxX > this.max.x || isNaN(this.max.x)){
                        this.max.x = tmaxX;
                    }

                }

                if (this.columnDefinition.z) {
                    node.z = parseFloat(node[this.columnDefinition.z]);
                    if (node.z < this.min.z)
                        this.min.z = node.z;
                    if (node.z > this.max.z)
                        this.max.z = node.z;

                }

                if (this.columnDefinition.y) {
                    node.y = parseFloat(node[this.columnDefinition.y]);
                    if (node.y < this.min.y)
                        this.min.y = node.y;
                    var tmaxY=node.y;
                    if(this.columnDefinition.weight)tmaxY+=parseFloat(node[this.columnDefinition.weight]);
                    if (tmaxY > this.max.y)
                        this.max.y = tmaxY;

                }
            }
        }

        this.min.x = parseFloat(this.min.x);
        this.max.x = parseFloat(this.max.x);
        this.min.y = parseFloat(this.min.y);
        if (this.type == "anotBox") this.max.y = 1;
        else this.max.y = parseFloat(this.max.y);


        if(!this.isNXminBased) this.min.x = 0;
        if(!this.isNYminBased)  this.min.y = 0;
        if (!this.isNZminBased) this.min.z = 0;

    };

    this.scaleRawData = function(cordinate, type) {

    };


    this.bumpRawData = function() {
        var defStart = 0, defStep = 0, defWeight = 0;
        var nodes = new Array();
        var layers = new Array();
        for ( var ip = 0; ip < this.tblArr.rows.length; ++ip) {
            var node = new Object();
            
            if (this.type != "anotBox") {
                if (!this.columnDefinition.start)
                    this.columnDefinition.start = this.columnDefinition.x ? this.columnDefinition.x
                            : defStep;

                else
                    node.start = parseFloat(this.tblArr.rows[ip][this.columnDefinition.start]);

                if (!this.columnDefinition.end)
                    node.end = node.start + defStep;
                else
                    node.end = parseFloat(this.tblArr.rows[ip][this.columnDefinition.end]);
                if (!this.columnDefinition.weight)
                    node.weight = defWeight;
                else
                    node.weight = parseFloat(this.tblArr.rows[ip][this.columnDefinition.weight]);

                nodes.push(node);
            }
            else if (this.type == "anotBox") {
                var elementToPut = new Object();
                elementToPut.start = parseFloat(this.tblArr.rows[ip][this.columnDefinition.start]);
                elementToPut.end = parseFloat(this.tblArr.rows[ip][this.columnDefinition.end]);
                this.computeLayers(layers,elementToPut, this.level);
                this.tblArr.rows[ip]["y"] = elementToPut.y;
            }
        }
        if(this.type!="anotBox"){

            bumper(nodes);
            if (!this.columnDefinition.y)
                this.columnDefinition.y = 'defaultY';
            for ( var ip = 0; ip < nodes.length; ++ip) {
                this.tblArr.rows[ip][this.columnDefinition.x] = nodes[ip].start;
                if (nodes[ip].place !== undefined)
                    this.tblArr.rows[ip][this.columnDefinition.y] = nodes[ip].place;
            }
        }
    };

    this.computeColumn = function() {

    }
    this.computeLayers = function(layersArray,elementToPut, levelHeight){
        if (levelHeight==undefined || !levelHeight) levelHeight = 0.25;
        if (!layersArray.length){ 
            var layer = new Array();
            elementToPut.y = levelHeight;
            layer.push(elementToPut);
            layersArray.push(layer);
            return elementToPut;
        }
        
        var layersCnt = layersArray.length;
        var createNewLevel = true;
        for (var l=0; l < layersCnt; l++){
            var cur_layer = layersArray[l];
            var elementCnt = cur_layer.length;
            createNewLevel = false;
            for (var ielement=0; ielement<elementCnt; ielement++){
                var cur_element = cur_layer[ielement];
                var next_element = undefined;
                if (ielement+1 < elementCnt) next_element = cur_layer[ielement+1];
                                
                if (next_element==undefined){
                    if ((elementToPut.start < cur_element.start && elementToPut.end < cur_element.start) || (elementToPut.start > cur_element.end && elementToPut.end > cur_element.end)) { 
                        elementToPut.y = levelHeight * (l+1);
                        cur_layer.splice(ielement+1,0,elementToPut);
                        return elementToPut;    
                    } else {
                        if (l+1 == layersCnt) { 
                            var new_level = new Array();
                            elementToPut.y = levelHeight * (l+1+1);
                            new_level.push(elementToPut);
                            layersArray.push(new_level);                            
                            return elementToPut;
                        }
                        else{
                            break;
                        }
                    }
                } else {
                    if (elementToPut.start > cur_element.end && elementToPut.end < next_element.start){
                        elementToPut.y = levelHeight*(l+1);
                        cur_layer.splice(ielement+1,0,elementToPut);
                        return elementToPut;
                    }
                    if (elementToPut.end <= next_element.start){ 
                        break;
                    }
                    else {
                        continue;
                    }
                }
            }
        }
        if (createNewLevel){ 
            var layer = new Array();
            elementToPut.y = levelHeight * (layersCnt+1);
            layer.push(elementToPut);
            layersArray.push(layer);
            return elementToPut;
        }
    }
}

