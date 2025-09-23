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

function vjSVG_Axis(source)
{
    vjSVG_primitive.call(this,source);

    if (this.showGrid===undefined)        this.showGrid        =true;
    if (this.offsetTicks===undefined)    this.offsetTicks    =0;
    if (this.gapTickLabel===undefined)     this.gapTickLabel    =0.08;
    if (this.textTickSize===undefined)    this.textTickSize    ="10px";
    if (this.titleSize===undefined)    this.titleSize    ="10px";
    if (this.gapLabelTitle===undefined) this.gapLabelTitle    =0.05;
    if (this.labelSize===undefined)     this.labelSize        =0.02;
    if (this.showArrow===undefined)     this.showArrow        =true;
    if (this.showArrowHead===undefined)     this.showArrowHead=true;
    if (this.showTicks===undefined)     this.showTicks        =true;
    if (this.showTickLabes===undefined) this.showTickLabes    =this.showTicks;
    if (this.showMinorTicks===undefined)this.showMinorTicks    =false;
    if (this.showMinorTickLabels===undefined)this.showMinorTickLabels=this.showMinorTicks;
    if (this.showGrid===undefined)        this.showGrid        =true;
    if (this.showMinorGrid===undefined)    this.showMinorGrid    =false;
    if (this.showTitle===undefined)        this.showTitle        =true;
    if (this.isY===undefined)            this.isY            =false;


    this.gapTickLabel=-1*compute_tickGap(this,this.gapTickLabel);
    this.gapLabelTitle=-1*compute_titleGap(this,this.gapLabelTitle);
    if(this.rotation && this.rotation.angle)this.anchor='end';
    else this.anchor='middle';

    var extend={min:this.min,max:this.max,step:0};
    
    if(this.showArrow){
        var arrow=new vjSVG_Arrow({showArrowHead:this.showArrowHead});

        this.children.push(arrow);
    }

    if (!this.labels || this.labels.length==0){
        var labels=new Array();
        if(this.min===undefined || this.max===undefined){
            alert("DEVELOPER ALERT: no or empty dataset was loaded");
            for (var i=0;i<1;i+=0.1)
                labels.push(i);
            this.labels=labels;
        }
        else{
            if(!this.numOfTicks){
                if(!this.pixelStep)this.pixelStep=25;
                this.numOfTicks=this.actualSize/this.pixelStep;
            }
            extend=vjSVG_tick_scale_Linear(extend,this.numOfTicks);
            if (!this.isAdjustToGrid) {
                extend.max = this.max;
            }
        }
    }
    var relSize=this.isAdjustToGrid?((extend.max-this.min)/(this.max-this.min)):1;
    var offMin=0;
    if(this.min<extend.min){
        offMin={label:this.min,
                offset:((extend.min-this.min))/(extend.max-this.min)};
    }

    if(this.showTicks){

        var majorTicks=new vjSVG_MajorTicks({
            origin:{x:0,y:0,z:0},
            numOfTicks:this.numOfTicks,
            textTickSize:this.textTickSize,
            gapTickLabel:this.gapTickLabel,
            labels:this.labels,
            extend:extend,
            anchor:this.anchor,
            relativeSize:relSize,
            offsetMin:offMin,
            offsetTicks:this.offsetTicks,
            isY: this.isY
            });
        this.children.push(majorTicks);
    }

    if(this.showGrid){
        if(this.offsetGrid===undefined && this.offsetTicks!==undefined)this.offsetGrid=this.offsetTicks;
        var majorGrid=new vjSVG_MajorGrid({
            origin:{x:0,y:0,z:0},
            numOfTicks:this.numOfTicks,
            labels:this.labels,
            extend:extend,
            offset:this.offsetGrid,
            offsetMin:(offMin?offMin.offset:0),
            relativeSize:relSize
            });
        this.children.push(majorGrid);
    }

    if(this.showTitle){
        if (!this.titleCrd) {
            if (this.isY == true) this.titleCrd={x:0.5,y:-(this.gapTickLabel+this.gapLabelTitle+this.labelSize)-0.2};
            else this.titleCrd={x:0.5,y:this.gapTickLabel+this.gapLabelTitle+this.labelSize};
        }
        if (!this.title) this.title="Axis";
        var angleText=0;
        if (this.rotation) angleText=this.rotation.angle;
        if (angleText=="90"){this.titleVertical=true; angleText=0; }
        var title=new vjSVG_text({
            crd:this.titleCrd,
            text:this.title,
            angle:0
            });

        title.font={
            "font-size": this.titleSize,
            "text-anchor" : this.anchor,
            "writing-mode" : (this.titleVertical ? "tb" : null),
            "glyph-orientation-vertical" : (this.titleVertical ? "0" : null)
        };
        title.brush={"fill-opacity":1};
        this.children.push(title);
    }
    if(this.isAdjustToGrid)
        this.max = extend.max;
}

function vjSVG_Ticks(source) {
    vjSVG_primitive.call(this, source);

    if (!this.height)
        this.height = 0.01;
    var crdList = new Array();

    var crd1 = {
        x : this.crd.x,
        y : this.crd.y - this.height,
        z : this.crd.z
    };
    var crd2 = {
        x : this.crd.x,
        y : this.crd.y + this.height,
        z : this.crd.z
    };
    crdList.push(crd1, crd2);

    this.children.push(new vjSVG_trajectory({
        coordinates : crdList,
        closed : 0
    }));
    if (this.isY) this.gapTickLabel = - 0.03;
    var tickText = new vjSVG_text({
        crd : {
            x : this.crd.x,
            y : this.crd.y + this.height + this.gapTickLabel
        },
        text : this.labels
    });
    tickText.font={"text-anchor":this.anchor, "font-size":this.textTickSize};
    tickText.brush={"fill":"black","fill-opacity":1};
    this.children.push(tickText);
}


function vjSVG_MajorTicks(source) {
    vjSVG_primitive.call(this, source);
    var distance = this.extend.max - this.extend.min;
    if (distance>1000 && this.numOfTick >20) this.textTickSize = "9px";

    if (this.relativeSize === undefined)
        this.relativeSize = 1;
    if (!this.numOfTicks)
        this.numOfTicks = 10;
    if (!this.offset)
        this.offset = 0;

    if (!this.labels) {
        var labels = new Array();
        for ( var i = this.extend.min; i <= this.extend.max; i += this.extend.step)
            labels.push(parseFloat(i.toPrecision(12)));
        this.labels = labels;
    }
    var curr_tick = this.offset;
    if (this.offsetMin) {
        var crd = {
            x : this.origin.x + curr_tick,
            y : this.origin.y,
            z : this.origin.z
        };
        this.children.push(new vjSVG_Ticks({
            crd : crd,
            labels : this.offsetMin.label,
            gapTickLabel : this.gapTickLabel,
            anchor : this.anchor,
            textTickSize: this.textTickSize,
            isY: this.isY
        }));
        curr_tick += this.offsetMin.offset;
    }
    var stepCord = this.extend.step*(this.relativeSize-(this.offsetMin ? this.offsetMin.offset : 0))/(this.extend.max - this.extend.min);
    

    var labelsObj = new Object();

    for ( var s = 0; s < this.labels.length ; s++) {

        labelsObj[this.labels[s]] = {
            x : this.origin.x + curr_tick,
            y : this.origin.y,
            z : this.origin.z
        };
        curr_tick += stepCord;
    }
    for ( var s = 0; s < this.labels.length; s++) {
        this.children.push(new vjSVG_Ticks({
            crd : labelsObj[this.labels[s]],
            labels : this.labels[s],
            gapTickLabel : this.gapTickLabel,
            anchor : this.anchor,
            textTickSize: this.textTickSize,
            isY: this.isY
        }));
    }
}


function vjSVG_MajorGrid(source) {
    vjSVG_primitive.call(this, source);

    if (this.relativeSize === undefined)
        this.relativeSize = 1;
    if (!this.numOfTicks)
        this.numOfTicks = 10;
    if (!this.offset)
        this.offset = 0;
    if (!this.labels) {
        var labels = new Array();
        for ( var i = this.extend.min; i <= this.extend.max; i += this.extend.step)
            labels.push(parseFloat(i.toPrecision(12)));
        this.labels = labels;
    }
    var curr_line = this.offset;
    if (this.offsetMin) {
        var crd = {
            x : this.origin.x + curr_line,
            y : this.origin.y,
            z : this.origin.z
        };
        var gridLine = new vjSVG_line({
            crd1 : crd,
            crd2 : {
                x : crd.x,
                y : crd.y + 1,
                z : crd.z
            }
        });

        gridLine.pen = {
            "stroke-opacity" : 0.3,
            "stroke-width" : 0.5,
            "stroke" : 'grey'
        };
        this.children.push(gridLine);
        curr_line += this.offsetMin;
    }
    var stepCord = this.extend.step*(this.relativeSize-(this.offsetMin ? this.offsetMin : 0))/(this.extend.max - this.extend.min);
    
    for ( var s = 0; s < this.labels.length; s++) {
        var crd = {
            x : this.origin.x + curr_line,
            y : this.origin.y,
            z : this.origin.z
        };
        curr_line += stepCord;
        var gridLine = new vjSVG_line({
            crd1 : crd,
            crd2 : {
                x : crd.x,
                y : crd.y + 1,
                z : crd.z
            }
        });
        gridLine.pen = {
            "stroke-opacity" : 0.3,
            "stroke-width" : 0.2,
            "stroke" : 'grey'
        };
        this.children.push(gridLine);
    }
}

function compute_tickGap(obj,gapTick){
    var offset=gapTick;
    if(obj.rotation!==undefined){
    }
    else
        offset+=obj.labelSize;
    return offset;
}

function compute_titleGap(obj,gapTitle){
    var offset=gapTitle;
    if(obj.rotation!==undefined){
    }
    else
        offset+=obj.labelSize;
    return offset;
}


function vjSVG_tick_scale_Linear(extend,numT){
    var span = extend.max - extend.min,
    step = Math.pow(10, Math.floor(Math.log(span / numT) / Math.LN10)),
    err = numT / span * step;
    if (err <= .15) step *= 10;
    else if (err <= .35) step *= 5;
    else if (err <= .75) step *= 2;
    extend.min = Math.ceil(extend.min / step) * step;
    extend.max = (Math.floor(extend.max / step)+0.5)*step;
    extend.step = step;
    return extend;
}

function vjSVG_Cartesian2D (source)
{
    vjSVG_primitive.call(this,source);
    if(!this.axisX)  this.axisX=new vjSVG_Axis ();
    this.axisX.clockWise=1;
    if(!this.axisY){
        this.axisY=new vjSVG_Axis ({
            rotation:{
                crd:{x:0,y:0,z:0},
                vec:{x:0,y:0,z:1},
                angle:90
                }
        }) ;
    }
    if (!this.axisY.rotation) this.axisY.rotation={crd:{x:0,y:0,z:0},vec:{x:0,y:0,z:1},angle:90};

    if(!this.axisX.isHidden) {
        var a = new vjSVG_Axis (this.axisX);
        this.axisX.max = a.max;
        this.children.push(a);
    }
    if(!this.axisY.isHidden){
        this.axisY.scale={x:1,y:-1,z:1};
        this.axisY.isY=true;
        var b = new vjSVG_Axis (this.axisY);
        this.axisY.max = b.max;
        this.children.push(b);
    }
}


function vjSVG_CartesianAxis(source)
{
    vjSVG_primitive.call(this,source);
    if (!this.x.numOfTicks)this.x.numOfTicks = 5;
    if (!this.y.numOfTicks)this.y.numOfTicks = 5;
    return new vjSVG_Cartesian2D({axisX:this.x,axisY:this.y});

}

