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
    if (this.gapTickLabel===undefined)     this.gapTickLabel    =0.02;
    if (this.gapLabelTitle===undefined) this.gapLabelTitle    =0.02;
    if (this.labelSize===undefined)     this.labelSize        =0.03;
    if (this.showArrow===undefined)     this.showArrow        =true;
    if (this.showTicks===undefined)     this.showTicks        =true;
    if (this.showTickLabes===undefined) this.showTickLabes    =this.showTicks;
    if (this.showMinorTicks===undefined)this.showMinorTicks    =false;
    if (this.showMinorTickLabels===undefined)this.showMinorTickLabels=this.showMinorTicks;
    if (this.showGrid===undefined)        this.showGrid        =true;
    if (this.showMinorGrid===undefined)    this.showMinorGrid    =false;
    if (this.showTitle===undefined)        this.showTitle        =true;


    this.gapTickLabel=compute_tickGap(this,this.gapTickLabel);
    this.gapLabelTitle=compute_titleGap(this,this.gapLabelTitle);
    if(!this.rotation){this.gapTickLabel*=-1;this.gapLabelTitle*=-1;}

    var extend={min:this.min,max:this.max,step:0};

    if(this.showArrow){
        var arrow=new vjSVG_Arrow();
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
        }
    }

    var majorTicks=new vjSVG_MajorTicks({
        origin:{x:0,y:0,z:0},
        numOfTicks:this.numOfTicks,
        gapTickLabel:this.gapTickLabel,
        labels:this.labels,
        extend:extend,
        offset:this.offsetTicks
        });
    this.children.push(majorTicks);

    if(this.showGrid){
        if(this.offsetGrid===undefined && this.offsetTicks!==undefined)this.offsetGrid=this.offsetTicks;
        var majorGrid=new vjSVG_MajorGrid({
            origin:{x:0,y:0,z:0},
            numOfTicks:this.numOfTicks,
            labels:this.labels,
            extend:extend,
            offset:this.offsetGrid
            });
        this.children.push(majorGrid);
    }

    if (!this.titleCrd) this.titleCrd={x:0.5,y:this.gapTickLabel+this.gapLabelTitle+this.labelSize};
    if (!this.title) this.title="Axis";
    var angleText=0;
    if (this.rotation) angleText=this.rotation.angle;
    if (angleText=="90"){this.titleVertical=true; angleText=0;}
    var title=new vjSVG_text({
        crd:this.titleCrd,
        text:this.title,
        anchor:'end',
        isVertical:this.titleVertical,
        angle:0
        });
    this.children.push(title);
}

function vjSVG_Ticks(source)
{

    vjSVG_primitive.call(this,source);

    if (!this.height) this.height=0.01;
    var crdList=new Array();


    var crd1={x:this.crd.x, y:this.crd.y-this.height, z:this.crd.z};
    var crd2={x:this.crd.x, y:this.crd.y+this.height, z:this.crd.z};
    crdList.push(crd1,crd2);
    this.children.push(new vjSVG_trajectory({coordinates:crdList,closed:0}));
    this.children.push(new vjSVG_text({crd:{x:this.crd.x,y:this.crd.y+this.height+this.gapTickLabel},text:this.labels,isVertical:0}));
}


function vjSVG_MajorTicks(source){
    vjSVG_primitive.call(this,source);

    if (!this.numOfTicks) this.numOfTicks=10;
    if(!this.offset)this.offset=0;

    if(!this.labels){
        var labels=new Array();
        for(var i=this.extend.min;i<=this.extend.max;i+=this.extend.step)
            labels.push(parseFloat(i.toPrecision(12)));
        this.labels=labels;
    }
    var stepCord=1/(this.extend.max/this.extend.step);
    for (var s=0;s<labels.length;s++){
        var crd={x:this.origin.x+this.offset,y:this.origin.y,z:this.origin.z};
        this.offset+=stepCord;
        this.children.push(new vjSVG_Ticks({crd:crd,labels:this.labels[s],gapTickLabel:this.gapTickLabel}));
    }
}

function vjSVG_MajorGrid(source){
    vjSVG_primitive.call(this,source);

    if (!this.numOfTicks) this.numOfTicks=10;
    if(!this.offset)this.offset=0;
    if(!this.labels){
        var labels=new Array();
        for(var i=this.extend.min;i<=this.extend.max;i+=this.extend.step)
            labels.push(parseFloat(i.toPrecision(12)));
        this.labels=labels;
    }
    var stepCord=1/(this.extend.max/this.extend.step);
    for (var s=0;s<labels.length;s++){
        var crd={x:this.origin.x+this.offset,y:this.origin.y,z:this.origin.z};
        this.offset+=stepCord;
        var gridLine=new vjSVG_line({crd1:crd,crd2:{x:crd.x,y:crd.y+1,z:crd.z}});
        gridLine.pen={opacity:0.5,width:1,color:'grey'};
        this.children.push(gridLine);
    }
}

function compute_tickGap(obj,gapTick){
    var offset=0;
    if(obj.rotation!==undefined){
        if(!obj.rotation.angle || obj.rotation.angle=="180")offset=obj.labelSize+gapTick;
        offset+=Math.sin(obj.rotation.angle)*gapTick;
    }
    else
        offset=obj.labelSize+gapTick;
    return offset;
}

function compute_titleGap(obj,gapTitle){
    var offset=gapTitle;
    if(obj.rotation!==undefined){
        if(!obj.rotation.angle || obj.rotation.angle=="180")offset+=obj.labelSize+gapTitle;
        offset+=Math.cos(obj.rotation.angle)*obj.labelSize+gapTitle;
    }
    else
        offset+=obj.labelSize+gapTitle;
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
    this.axisY.clockWise=-1;
    var axisX=new vjSVG_Axis (this.axisX);
    var axisY=new vjSVG_Axis(this.axisY);
    axisY.scale={x:1,y:-1,z:1};

    this.children.push(axisX, axisY);
}


function vjSVG_CartesianAxis(source)
{
    vjSVG_primitive.call(this,source);


    var coordinateSystem=new vjSVG_Cartesian2D({
            axisX:{
                numOfTicks:this.x.frequencies,
                labels:this.x.labels,
                labelsSize:'',
                title:this.x.title,
                titleVertical:this.x.titleVertical,
                titlePosition:'',
                titleFontSize:'',
                gapTickLabel:this.gapTickLabel,
                gapLabelTitle:this.gapLabelTitle,
                min:this.x.min,
                max:this.x.max,
                actualSize:this.x.actualSize,
                tickStep:this.x.tickStep
            },

            axisY:{
            numOfTicks:this.y.frequencies,
               labels:this.y.labels,
               labelsSize:'',
               title:this.y.title,
               titleVertical:this.y.titleVertical,
               titlePosition:'',
               titleFontSize:'',
               gapTickLabel:this.gapTickLabel,
               gapLabelTitle:this.gapLabelTitle,
               min:this.y.min,
               max:this.y.max,
               actualSize:this.y.actualSize,
               tickStep:this.y.tickStep
            }
    });
    this.children.push(coordinateSystem);
}

