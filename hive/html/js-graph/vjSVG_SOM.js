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
function vjSVG_SOM(source)
{
    holdsActualRows=[];
    
    vjSVG_Plot.call(this, source);
    if (!this.name) this.name = this.objID;
    var plot = this;

    if (!this.minCellSize) this.minCellSize = 10;
    if (this.squareCells === undefined) this.squareCells = true;

    if (!this.glyphHeight) this.glyphHeight = 0.03;

    this.geometry = { width: 1, height: 1};

    this.serieDimensions = function() {
        var ret = { rows: 0, columns: 0};
        this.serie = this.collection[0];
        if (!this.serie || !this.serie.has_nontrivial_data())
            return ret;

        for (var i=0; i<this.serie.tblArr.rows.length; i++) {
            var node = this.serie.tblArr.rows[i];
            ret.rows = Math.max(parseInt(node.row) + 1, ret.rows);
            ret.columns = Math.max(parseInt(node.column) + 1, ret.columns);
        }
        return ret;
    };

    this.construct = function(axis, chartarea, scene) {
        var serie_dim = this.serieDimensions();

        if (chartarea.height && !isNaN(chartarea.height) && chartarea.width && !isNaN(chartarea.width))
        {
            var min = Math.min (parseFloat(chartarea.height), parseFloat(chartarea.width));
            this.geometry.width = min/chartarea.width;
            this.geometry.height = min/chartarea.height;
        }

        this.geometry.cellWidth = this.geometry.width / serie_dim.columns;
        this.geometry.cellHeight = this.geometry.height / serie_dim.rows;


        var map_group = new vjSVG_group({
            pen: {"stroke-width": 0}
        });

        for (var i=0; i<this.serie.tblArr.rows.length; i++) {
            var node = this.serie.tblArr.rows[i];
            map_group.children.push(new vjSVG_box({
                crd: {x: parseInt(node.column) * this.geometry.cellWidth, y: 1 - (parseInt(node.row)+1) * this.geometry.cellHeight},
                width: this.geometry.cellWidth,
                height: this.geometry.cellHeight,
                brush: { "fill-opacity": 0.80, fill: vjRGB(node.red, node.green, node.blue).hex() }
            }));
        }
        
        for(i=0; i < this.collection[1].tblArr.rows.length;i++)
        {
            var node = this.collection[1].tblArr.rows[i];
            
            if (node.mapC == -1 || node.mapR == -1)
                continue;
            
            map_group.children.push(new vjSVG_box({
                crd: {x: parseInt(node.mapC) * this.geometry.cellWidth-this.geometry.cellWidth/50, y: 1 - (parseInt(node.mapR)+1)*this.geometry.cellHeight},
                width: this.geometry.cellWidth+this.geometry.cellWidth/25,
                height: this.geometry.cellHeight,
                pen: {stroke: "black",   "stroke-width" : 100*this.geometry.cellWidth}
            }));
        }
        
        for(i=0; i < this.collection[1].tblArr.rows.length;i++)
        {
            var node = this.collection[1].tblArr.rows[i];
            
            if (node.mapC == -1 || node.mapR == -1)
                continue;
            
            var handler =  function(irows, icols, values) { return function() { return plot.callOnclickCallbacks(irows, icols, "map"); } } (node.mapR, node.mapC);
            
            holdsActualRows.push({mapR: node.mapR, mapC: node.mapC, row: node.row});
            
            map_group.children.push(new vjSVG_box({
                crd: {x: (parseInt(node.mapC) * this.geometry.cellWidth)+this.geometry.cellWidth/11, y: 1 - (((parseInt(node.mapR)+1)*this.geometry.cellHeight)-this.geometry.cellWidth/4)},
                width: this.geometry.cellWidth-this.geometry.cellWidth/5,
                height: this.geometry.cellHeight-this.geometry.cellWidth/2,
                pen: {stroke: "white",   "stroke-width" : 100*this.geometry.cellWidth},
                handler: { onclick: handler}
            }));
        }

        this.children.push(map_group);
    };
    
    this.onclickCallbacks = [];
    this.registerOnclickCallback = function(callback, callbackParam, callbackThis, options)
    {
        if (!options)
            options = { right: true, bottom: true, map: true };

        this.onclickCallbacks.push({callback: callback, param: callbackParam, "this": callbackThis, options: options});
    };
    
    this.callOnclickCallbacks = function(irows, icols, type, value)
    {
        if (!this.onclickCallbacks)
            return;

        for (var i=0; i<this.onclickCallbacks.length; i++) {
            var cb = this.onclickCallbacks[i];
            if (!cb.options[type])
                continue;

            var val = -1;
            for (var j = 0 ; j < holdsActualRows.length; j++)
            {
                if (holdsActualRows[j].mapR == irows && holdsActualRows[j].mapC == icols)
                    val = holdsActualRows[j].row;
            }
            
            if (val != -1)
                cb.callback.call(cb["this"] ? cb["this"] : this, irows, icols, type, val, cb.param);
        }
    };

    this.preferredScale = function(proposed) {
        var serie_dim = this.serieDimensions();
        return {x : this.minCellSize * serie_dim.columns, y: this.minCellSize * serie_dim.rows, z: proposed.z};
    };
}
