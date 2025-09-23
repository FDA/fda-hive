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


function vjD3JS_DynamicPie ( viewer )
{
    loadCSS("d3js/css/dynamic_pie.css");
    vjD3View.call(this,viewer);
    
    this.data=viewer.data;
    viewer.marin ? this.margin = viewer.margin : this.margin = {top:0, right:0, bottom:50, left:0};
    viewer.width ? this.width = viewer.width : this.width = 300;
    viewer.height ? this.height = viewer.height : this.height = 300;
    this.diameter = this.width/2;
    viewer.label ? this.label = viewer.label : this.label="";
    this.fontSize=10;
    viewer.colors ? this.colors = viewer.colors : this.colors = ["purple", "cyan"];
    viewer.labelCol ? this.labelCol = viewer.labelCol : this.labelCol = 0;
    
    this.logValue = false;
    
    this.d3Compose=function(content)
    {
        if (content.indexOf("error") == 0)
            return;
        
        var radius = Math.min(this.width, this.height) / 3.5;
        this.first = true;
        var that = this;
        
        if (!this.wrapLength)
            this.wrapLength = (this.width - 2 * (radius + 10))/2;
        
        this.tblArr=new vjTable(content, 0, vjTable_propCSV, undefined, undefined, undefined, undefined, 1);
        
        if (this.tblParsedCallback){
            this.tblArr = this.tblParsedCallback(this);
        }
        
        var toPut = [], labels = [];
        var colToUseData = "", colToUseLabel = "";
        if (this.dataCol < this.tblArr.hdr.length && this.dataCol >= 0)
            colToUseData = this.tblArr.hdr[this.dataCol].name;
        if (this.labelCol < this.tblArr.hdr.length && this.labelCol >= 0)
            colToUseLabel = this.tblArr.hdr[this.labelCol].name;
        
        var max=Number.MIN_VALUE, min=Number.MAX_VALUE;
        var domainLabels = [];
        var total = 0;
        for (var i = 0; i < this.tblArr.rows.length; i++)
        {
            var num = parseFloat(this.tblArr.rows[i][colToUseData]);
            toPut.push (num);
            labels.push({label:this.tblArr.rows[i][colToUseLabel], value:num, valueLog: Math.log(num)+1});
            
            if (this.logValue)
                num = Math.log(num)+1;
            
            total += num;
            
            if (num > max) max = num;
            if (num < min) min = num;
            
            if (this.colorDomain && domainLabels.indexOf(this.tblArr.rows[i][colToUseLabel]) < 0)
                domainLabels.push(this.tblArr.rows[i][colToUseLabel]);
        }
        
        var color;
        
        if (domainLabels.length > 0)
            this.colors.splice (domainLabels.length+1);
    
        if (this.colorDomain)
            color = d3.scale.ordinal()
                .domain (domainLabels)
                .range (this.colors);
        else
            color = d3.scale.linear()
                .domain([min, max])
                .range(this.colors);
        
        for (var i=0; i < labels.length; i++)
            labels[i].color = color (labels[i].label);

        var arc = d3.svg.arc()
            .outerRadius(radius);
        
        var anotherOuterArc = d3.svg.arc()
            .innerRadius(radius * 0.7)
            .outerRadius(radius * 0.9);
        
        var outerArc = d3.svg.arc()
            .innerRadius(radius*1.2)
            .outerRadius(radius);
            
        var pie = d3.layout.pie();
    
        stuff = labels;
        var pie = d3.layout.pie()
            .sort(null)
            .value(function(d) {
                if (!that.logValue)
                    return d.value;
                else
                    return d.valueLog;
            });
        
        
        var svg = this.d3svg
            .datum(labels)
            .attr("width", this.width)
            .attr("height", this.height - this.margin.bottom)
          .append("g")
            .attr("transform", "translate(" + this.width / 2 + "," + this.height / 2 + ")");
        
        svg.append("g")
            .attr("class", "slices");
    
        var arcs = svg.selectAll("g.arc")
            .data(pie)
          .enter().append("g")
            .attr("class", "arc");
    
        arcs.append("path")
            .attr("fill", function(d, i) { 
                if(that.colorDomain)
                    return color(d.data.label);
                return color(d.value); })
            .on("click", function (node, b, c){
                if (that.onClickCallback)
                    return that.onClickCallback(that, node, b, c);
            })
          .transition()
            .ease("linear")
            .duration(750)
            .attrTween("d", tweenPie)
          .transition()
            .ease("elastic")
            .delay(function(d, i) { return 2000 + i * 50; })
            .duration(750)
            .attrTween("d", tweenDonut);
    
        function tweenPie(b) {
            b.innerRadius = 0;
            var i = d3.interpolate({startAngle: 0, endAngle: 0}, b);
            return function(t) { return arc(i(t)); };
        }
    
        function tweenDonut(b) {
          b.innerRadius = radius * .6;
          var i = d3.interpolate({innerRadius: 0}, b);
          return function(t) { return arc(i(t)); };
        }

        var otherPie = d3.layout.pie()
            .sort(null)
            .value(function(d) {
                if (!that.logValue)
                    return d.value;
                else
                    return d.valueLog;
            });
        
        

        svg.append("g")
            .attr("class", "labels")
        svg.select(".labels").append("text");
        svg.append("g")
            .attr("class", "lines");
        
        
        var text = svg.select(".labels").selectAll("text")
            .data(otherPie(labels), function (d,i){
                if (Array.isArray(d))
                    return ;
                return d.data.label});
    
        text.enter()
            .append("text")
            .attr("dy", ".35em")
            .text(function(d) {
                if (that.logValue)
                    return d.data.label + " : " + d.data.value + "(" + Math.round(d.data.valueLog/total*100) + "%)";
                return d.data.label + " : " + d.data.value + "(" + Math.round(d.data.value/total*100) + "%)";
            })
            .call(wrap, this.wrapLength)
            .on("click", function (node, b, c){
                if (that.onClickCallback)
                    return that.onClickCallback(that, node, b, c);
            });
        
        function midAngle(d){
            return d.startAngle + (d.endAngle - d.startAngle)/2;
        }
    
        text.transition().duration(1000)
            .attrTween("transform", function(d) {
                this._current = this._current || d;
                var interpolate = d3.interpolate(this._current, d);
                this._current = interpolate(0);
                return function(t) {
                    var d2 = interpolate(t);
                    var pos = outerArc.centroid(d2);
                    pos[0] = radius * (midAngle(d2) < Math.PI ? 1.12 : -1.12);
                    return "translate("+ pos +")";
                };
            })
            .styleTween("text-anchor", function(d){
                this._current = this._current || d;
                var interpolate = d3.interpolate(this._current, d);
                this._current = interpolate(0);
                return function(t) {
                    var d2 = interpolate(t);
                    return midAngle(d2) < Math.PI ? "start":"end"; 
                };
            });
    
        text.exit()
            .remove();

        var polyline = svg.select(".lines").selectAll("polyline")
            .data(otherPie(labels), function (d,i){
                if (Array.isArray(d))
                    return ;
                return d.data.label});
        
        polyline.enter()
            .append("polyline")
            .on("click", function (node, b, c){
                if (that.onClickCallback)
                    return that.onClickCallback(that, node, b, c);
            });
    
        polyline.transition().delay(0)
            .attrTween("points", function(d){
                this._current = this._current || d;
                var interpolate = d3.interpolate(this._current, d);
                this._current = interpolate(0);
                return function(t) {
                    var d2 = interpolate(t);
                    var pos = outerArc.centroid(d2);
                    pos[0] = radius * 0.95 * (midAngle(d2) < Math.PI ? 1.12 : -1.12);
                    return [anotherOuterArc.centroid(d2), outerArc.centroid(d2), pos];
                };            
            });
        
        polyline.exit()
            .remove();
        
        
        this.d3area.append("label")
            .text("Regular View")
            .append("input")
                .attr("type", "radio")
                .attr("name","mode" + this.container)
                .attr("value","grouped" + this.container);
        
        this.d3area.append("label")
            .text("Hyper View")
            .append("input")
                .attr("type", "radio")
                .attr("name","mode" + this.container)
                .attr("value","stacked" + this.container)
                .attr("checked");
        
        d3.selectAll("input").on("change", change);
        
        if (!this.logValue)
                d3.select("input[value=\"grouped" + this.container + "\"]").property("checked", true);
        else
            d3.select("input[value=\"stacked" + this.container + "\"]").property("checked", true);
        
            function change() 
        {
                var container = this.parentElement.parentElement.parentElement.id
                var viewer = vjDV[container];
                
                if (this.value === ("grouped"+container))
            {
                    viewer.logValue = false;
                    viewer.refresh();
                }
                else
                {
                    viewer.logValue = true;
                    viewer.refresh();
                }
          }
    };
    
    return this;
}
