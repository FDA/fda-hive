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
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    //_/
    //_/ Visualize-able objects
    //_/
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/




        //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
        //_/
        //_/ base object which is the base class of all other visualizeable objects
        //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//TODO: Create default property objects for required properties of primitives (eg box: vjPropSVG_box = new Object({x:0,y:0,z:0,height:1,width:1});)and ensure properties.
function vjSVG_primitive(source)
{
    //if (!this.objID || !this.objID.length )
    this.objID="svg"+Math.random();
    vjObj.register(this.objID,this);

    this.children=new Array();

    this.clone=function (source)
    {
         var i=0;  // copy all the features from base object
         for (i in source) {
            this[i] = source[i];
         }
    };
    this.clone(source);
    this.renderChildren=function(svg)
    {
        if(!this.children )return ;
        for(var i=0; i<this.children.length;++i ) {
            if(this.children[i].render)
                this.children[i].render(svg);
        }
    };

    this.render=function(svg)
    {
        this.pushMyAttributes(svg);
        if(this.draw){
            var obj=this.draw(svg);
            if (this.svgID) {
                obj.id = this.svgID;
            }
            if(this.graphCallbacks){
                for(i in this.graphCallbacks){
                    obj.setAttribute(i,this.graphCallbacks[i]);
                }
            }
            if(this.title)
                svg.setTitle(this.title);
            if(this.desc)
                svg.setDesc(this.desc);
            if(this.attribute) // object Attribute ==> attribute={"tagName":value}
                //alerJ("new attribute ", this.attribute)
                svg.setNewAttribute(this.attribute);
        }
        this.renderChildren(svg);
        this.popMyAttributes(svg);
    };

    this.pushMyAttributes=function(svg)
    {
        if (this.pen)  svg.setPen(this.pen);
        if (this.brush) svg.setBrush(this.brush);
//        if (this.svgID) svg.setSVGID(this.svgID);
        if (this.font) svg.setFont(this.font);
        if (this.link) svg.setLink(this.font);
        if (this.handler) {
//            for (ii in this.handler){
//                this.handler[ii]='vjSVG_GIhandler(\"'+this.handler[ii]+'\",\"'+this.objID+'\",\"'+this.coordinates[0].irow+'\");'; //call whatever function from the wrapper
//            }
            svg.setHandler(this.handler);
        }

        if (this.matrix) {
            var currentMatrix = svg.getCurrentFromArray(svg.matrixArray);
            svg.setMatrix( matrixMultiplication(currentMatrix, this.matrix) );
        }

        if (this.translation) {
            svg.translateMatrix( this.translation );
        }

        if (this.rotation) {
            svg.rotateMatrix(this.rotation.crd,this.rotation.vec,this.rotation.angle);
        }

        if (this.scale) {
            svg.scaleMatrix(this.scale);
        }

    };
    this.popMyAttributes=function(svg){

        if (this.pen)   svg.popPen();
        if (this.brush) svg.popBrush();
//        if (this.svgID) svg.popSVGID();
        if (this.font)  svg.popFont();
        if (this.link)  svg.popLink();
        if (this.handler) svg.popHandler();

        if (this.matrix) svg.popMatrix();
        if (this.rotation)svg.popMatrix();
        if (this.scale)svg.popMatrix();
        if (this.translation)svg.popMatrix();

    };

    this.unlink=function() {
        for (var i=0; i<this.children.length; i++)
            this.children[i].unlink();
        this.children = [];
        vjObj.unregister(this.objID);
    };
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/
//_/ other simple objects
//_/

function vjSVG_trajectory(source)
{
    vjSVG_primitive.call(this, source);
    this.draw=function(svg)
    {
        return svg.trajectory(this.coordinates,this.closed,this.lineCmd,this.objID);
    };

}
function vjSVG_regularpolygon(source)
{
    vjSVG_primitive.call(this, source);
    this.draw=function(svg)
    {
        return svg.regularpolygon(this.crd,this.radius,this.npoint,this.objID);
    };
}

function vjSVG_box(source) // width, height
{
    vjSVG_primitive.call(this, source,this.objID);
    var crdList=new Array();
    var point1=this.crd;
    var point2={x:this.crd.x,y:this.crd.y+this.height,z:this.z};
    var point3={x:this.crd.x+this.width,y:this.crd.y+this.height,z:this.z};
    var point4={x:this.crd.x+this.width,y:this.crd.y,z:this.z};
    crdList.push(point1,point2,point3,point4);
    //if (this.crd.y=="NaN") alert("thi y " + this.crd.y)
    //alerJ("PrimitivePoint3",point3)
    this.draw=function(svg)
    {
        return svg.trajectory(crdList,1,"",this.objID);
    };

}

function vjSVG_symbol(source)
{
    vjSVG_primitive.call(this, source);
    this.draw=function(svg)
    {
        return svg.symbol(this.definition,this.crd,this.size,this.objID,this.isUnscaled);
    };

}

function vjSVG_circle(source)
{
    vjSVG_primitive.call(this, source);
    this.draw=function(svg)
    {
        return svg.circle(this.crd, this.r, this.objID);
    };
}

function vjSVG_arc(source)
{
    vjSVG_primitive.call(this, source);
    this.draw=function(svg)
    {
        return svg.arc(this.crd, this.startAngle, this.endAngle  , this.rx ,this.ry, this.rotation, this.howToClose,this.objID );
    };
}

function vjSVG_regularstar(source)
{
    vjSVG_primitive.call(this, source);
    this.draw=function(svg)
    {
        return svg.regularstar(this.crd,this.radius,this.npoint,this.objID);
    };

}

function vjSVG_text(source)
{
    vjSVG_primitive.call(this, source);

    // Use sensible pen/brush settings by default
    if (!this.pen) this.pen = DefaultTextPen;
    if (!this.brush) this.brush = DefaultTextBrush;

    this.draw=function(svg)
    {
        //svg.text(this.x,this.y,this.text,this.isVertical,this.angle);
        var objAttr = {text:this.text,isVertical:this.isVertical,angle:this.angle,"text-anchor":this.anchor,dx:this.dx,dy:this.dy};
        if (this.ellipsizeWidth)
            objAttr.ellipsizeWidth = this.ellipsizeWidth;
        return svg.text(this.crd, objAttr, this.objID);
    };
}

function vjSVG_line(source)
{
    vjSVG_primitive.call(this, source);

    this.draw=function(svg)
    {
        return svg.line(this.crd1,this.crd2,this.lineCmd,this.objID);
    };
}

function vjSVG_group(source)
{
    vjSVG_primitive.call(this,source);
    this.primitiveRender = this.render;
    this.render = function(svg) {
        this.pushMyAttributes(svg);
        var g = svg.group(this.objID + "_group");
        this.popMyAttributes(svg);
        var old_svg_grpobj = svg.curGroupNode;
        //g.svgID = svg.curGroupNode.id;
        g.id = this.objID + "_group";
        svg.curGroupNode = g;
        this.primitiveRender(svg);
        old_svg_grpobj.appendChild(g);
        svg.curGroupNode = old_svg_grpobj;
    };
}

    // _/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Composite primitives
    // _/
    // _/_/_/_/_/_/_/_/_/_/_/_/

function vjSVG_Arrow(source){
    vjSVG_primitive.call(this,source);

    if (!this.width) this.width=0.03;
    var origin={x:0,y:0,z:0};
    var end={x:1.05,y:0,z:0};

    var line=new vjSVG_line({crd1:origin,crd2:end});
    var arrowHead=new vjSVG_symbol({
        definition:"arrowhead",
        crd:end,
        size:this.width,
        rotation:{
            crd:end,
            vec:{x:0,y:0,z:1},
            angle:270
        }
    });
    arrowHead.brush={color:'black',rule:"nonzero"};

    if (this.showArrowHead === true) this.children.push(arrowHead,line);
    if (this.showArrowHead === false) this.children.push(line);
}

function vjSVG_image(source) {
    vjSVG_primitive.call(this, source);

    this.draw = function(svg) {
        return svg.image(this.crd, this.url, this.width, this.height, this.preserveAspectRatio);
    };
}
