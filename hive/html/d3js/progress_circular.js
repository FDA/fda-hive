
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


function vjD3JS_ProgressCircular ( viewer )
{
    loadCSS("d3js/css/progress_circular.css");
    vjD3View.call(this,viewer); // inherit default behaviors of the DataViewer
    
    this.data=viewer.data;
    viewer.marin ? this.margin = viewer.margin : this.margin = {top:0, right:0, bottom:30, left:0};
    viewer.width ? this.width = viewer.width : this.width = 300;
    viewer.height ? this.height = viewer.height : this.height = 300;
    viewer.label ? this.label = viewer.label : this.label="";
    this.fontSize=10;


    var _mouseClick;

    var _value= 0,
        _minValue = 0,
        _maxValue = 100;

    var _arc = d3.svg.arc()
        .startAngle(0); //just radians

    this.d3Compose=function(data)
    {
        if (data.indexOf("unknown") == 0) {
            data = "name,parent,cnt,reqID,grpID,svcID,stat,progress,progress100,actTime,takenTime,doneTime,waitTime,execTime,reportType,orderExecute,runningBefore,act\n"
                    +"Total Progress,0,0,0,0,0,5,0,100,0,0,0,0,0,0,0,0,0";
        }
        
        var progressPercentage = -1;
        var status = 0;
        this.tblArr=new vjTable(data, 0, vjTable_propCSV, undefined, undefined, undefined, undefined, 1);
        

        for (var i = 0; i < this.tblArr.hdr.length; i++)
        {
            var tmpHdrInfo = this.tblArr.hdr[i];
            
            if (tmpHdrInfo.name == "progress100")
                progressPercentage = (this.tblArr.rows[0][tmpHdrInfo.name]);
            if (tmpHdrInfo.name == "stat")
            {
                status = parseInt(this.tblArr.rows[0][tmpHdrInfo.name]);
                if (status == 5)
                {
                    progressPercentage = "100";
                    break;
                }
            }
        }
        
        if (progressPercentage < 0)
            progressPercentage = "0";
        this.progressPercentage = progressPercentage;
                
        if (this.callbackParsed)
            this.callbackParsed(this);
        
        
        _arc = d3.svg.arc()
            .startAngle(0)    
            .endAngle(parseInt(progressPercentage)*36/10 * (Math.PI/180));
        
        // Select the svg element, if it exists.
        this.d3Compose_prv(progressPercentage);
        var svg = this.d3svg;
    
        _value = progressPercentage;
            
        this.measure();
    
        svg.attr("width", this.width)
            .attr("height", this.height);
    
    
        var background = svg.append("g").attr("class","component")
            .attr("cursor","pointer");

    
        background.append("rect")
            .attr("class","background")
            .attr("width", this.width)
            .attr("height", this.height);
    
        background.append("path")
            .attr("transform", "translate(" + this.width/2 + "," + this.width/2 + ")")
            .attr("d", _arc);
    
        background.append("text")
            .attr("class", "label")
            .attr("transform", "translate(" + this.width/2 + "," + (this.width + this.fontSize) + ")")
            .text(this.label);
       var g = svg.select("g")
            .attr("transform", "translate(" + this.margin.left + "," + this.margin.top + ")");
    
    
        //check if the current data has error in the progress data (if errors, then color red)
       //if 100, color green
       var classOfArc = "arc";
       if (status == 5)
           classOfArc = "arcComplete";
       if (status > 5)
           classOfArc = "arcError";
       
        svg.append("g").attr("class", "arcs");
        var path = svg.select(".arcs").selectAll(".arc").data(progressPercentage);
        path.enter().append("path")
            .attr("class",classOfArc)
            .attr("transform", "translate(" + this.width/2 + "," + this.width/2 + ")")
            .attr("d", _arc);
    
        svg.append("g").attr("class", "labels");
        var label = svg.select(".labels").selectAll(".label").data(progressPercentage);
        label.enter().append("text")
            .attr("class","label")
            .attr("y",this.width/2+this.fontSize/3)
            .attr("x",this.width/2)
            .attr("cursor","pointer")
            .attr("width",this.width)
            .text(function (d) { 
                return Math.round((_value-_minValue)/(_maxValue-_minValue)*100) + "%" })
            .style("font-size",this.fontSize+"px");
    
        path.exit().transition().duration(500).attr("x",1000).remove();
    }

    this.measure = function() {
        var smallest = this.width;
        if (this.height < this.width)
            smallest = this.height;
        
        this.width=smallest - this.margin.right - this.margin.left;
        this.height=smallest;
        this.fontSize=this.width*.2;
        _arc.outerRadius(this.width/2);
        _arc.innerRadius(this.width/2 * .85);
    }


    

    return this;

}
