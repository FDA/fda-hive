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
function vjSVGView ( viewer )
{

    vjDataViewViewer.call(this,viewer);

    if(!this.geometry)this.geometry=new Object();
    if(!this.showAxis)this.showAxis=true;
    if(!this.downloadGenerateText) this.downloadGenerateText = "Generate downloadable graph";
    if(!this.downloadGeneratingText) this.downloadGeneratingText = "[Generating downloadable graph]";
    if(!this.downloadLinkText)this.downloadLinkText="Download graph as SVG file";
    if (!this.downloadDataSource) this.downloadDataSource = false;

    this.composerFunction=function( viewer , content)
    {
        this.content=content;

        this.refresh();
    };

    this.associateData=function(viewer,content)
    {
        this.data=verarr(this.data);
        this.plots=verarr(this.plots);
        for(var i=0;i<this.plots.length;++i){
            var plot=this.plots[i];
            plot.collection=verarr(plot.collection);
            for(var ic=0;ic<plot.collection.length;++ic){
                var dsname=verarr(plot.collection[ic].name);
                var serObj=plot.collection[ic];
                for (var is=0 ,i ; is<dsname.length; ++is ) {
                    for(i=0; i<this.data.length; i++){
                        if (this.data[i] == dsname[is])
                            break;
                    }
                    if(i==this.data.length)
                        this.data.push(dsname[is]);
                }
            }
        }
        for ( var is=0; is<this.data.length ; ++is){
            if(!vjDS[this.data[is]])
                alert("DEVELOPER ALERT: datasource "+this.data[is]+ " is not found");


            vjDS[this.data[is]].register_callback( {'obj': vjDS[this.data[is]]});
        }
    };


    this.refresh=function()
    {
        if (this.downloadLink === undefined || this.downloadLinkManage) {
            this.downloadLinkManage = true;
        }
        var max_width;
        if (this.width)
            max_width = this.width;
        else if (this.tab && this.tab.parent)
            max_width = this.tab.parent.width;
        else
            max_width = $("#"+this.container).width();
        if (this.resize_info && !isNaN(parseInt(this.resize_info.newW)))
            max_width = this.resize_info.newW;

        var max_height;
        if (this.height)
            max_height = this.height;
        else if (this.tab && this.tab.parent)
            max_height = this.tab.parent.height;
        else
            max_height = $("#"+this.container).height();
        if (this.resize_info && !isNaN(parseInt(this.resize_info.newH)))
            max_height = this.resize_info.newH;

        if( !this.hasPlotwithData() && this.defaultEmptyText ) {
            this.div.innerHTML = "<div style='text-align:center;position:relative;width:"+max_width+"px;height:"+max_height+"px;'>" +
                    this.defaultEmptyText+"</div>";
            this.svgdivSizer = this.div.firstChild;
            return;
        }

        if(!this.geometry.width)
            this.geometry.width = max_width;

        if(!this.geometry.height)
            this.geometry.height =  max_height - (this.downloadLinkManage?30:0) < 0 ? 0 : max_height - (this.downloadLinkManage?30:0);

        if(!this.geometry0) {
            if (!max_width || !max_height) {
                return;
            }
            this.geometry0 = { width: this.geometry.width, height: this.geometry.height, max_width: max_width, max_height: max_height };
        }

        if(this.resize_info) {
            this.geometry.width = parseInt(this.geometry0.width * max_width / this.geometry0.max_width);
            this.geometry.height = parseInt(this.geometry0.height * max_height / this.geometry0.max_height);
        }

        if(!this.chartArea) {
            this.chartScale0 = this.getChartScale();
            this.chartArea = { width: this.chartScale0.x, height: this.chartScale0.y };

            this.rescaleChartArea(this.chartScale0, 0, true);
        } else if (this.resize_info) {
            if (!this.chartScale0)
                this.chartScale0 = this.getChartScale();

            var newScale = { x: this.chartScale0.x * max_width / this.geometry0.max_width, y: this.chartScale0.y * max_height / this.geometry0.max_height, z: 1 };
            var minimalScale = this.getPreferredScale({x: 1, y: 1, z: 1});

            if (this.chartArea.width < this.geometry.width || this.chartArea.height < this.geometry.height || this.chartArea.width > minimalScale.x || this.chartArea.height > minimalScale.y) {
                this.rescaleChartArea(newScale, 0, true);
            } else {
                this.rescaleChartArea(this.chartScale0, 0, true);
            }
            this.updateGeometry();
        }
        
        if (isNaN(parseFloat(this.chartArea.width))) this.chartArea.width = "95%";
        if (isNaN(parseFloat(this.chartArea.height))) this.chartArea.height = "95%";

        var width = this.geometry.width;
        var height = this.geometry.height;
        
      
        this.scene=new vjSVGBase();
        this.scene.TransformTest = function(coord) {
            return coord; 
        };
        
        this.scene.transformtest2 = function(coord) {
            var x=coord.x
            var y=coord.y
            var z=coord.z
            var rx=screen.width/2;
            var ry=screen.height/2;
            
            
            if (x>rx){
                var c=x-rx;
                var theta=Math.acos(c/rx)*(180/Math.PI);
                var alpha= adjust(theta,rx);
                var _x= (Math.cos(alpha)*rx-c);
                x = x +_x;
         
            }        
            else if(x<rx){
                var c=rx-x;
                var theta=Math.acos(c/rx)*(180/Math.PI);
                var alpha= adjust(theta,rx);
                var _x= Math.cos(alpha)*rx-c;
                x = x - _x;
            }

            if (y>ry){
                var c=y-ry;
                var theta=Math.acos(c/ry)*(180/Math.PI);
                var alpha= adjust(theta,ry);
                var _y= (Math.cos(alpha)*ry-c);
                y = y + _y;
            }       
            else if(y<ry){
                var c=ry-y;
                var theta=Math.acos(c/ry)*(180/Math.PI);
                var alpha= adjust(theta,ry);
                var _y= (Math.cos(alpha)*ry-c);  
                y = y - _y;
            }
            coord.x=x;
            coord.y=y;
            
            return coord; 
        };
        this.scene.transformtwo = function(coord) {
            var myobj= [];
            var F=3;
            var angle=120;
            var x=coord.x
            var y=coord.y
            var z=coord.z
            var cx=screen.width/2;
            var cy=screen.height/2;
            
               if(cx>x)
                   {
                       x= F*Math.tan(x+cx)*angle*(Math.PI / 180);

                   }  
               else if(cx<x)
                   {
                       x= F*Math.tan(cx-x)*angle*(Math.PI / 180);
                   }
               if(cy>y)
                   {
                   y= F*Math.tan(y+cy)*angle*(Math.PI / 180);
                   }
            else if(cy<y)
                {
                       y= F*Math.tan(cy-y)*angle*(Math.PI / 180);
                }
               coord.x=x+coord.x;
                coord.y=y+coord.y;
           
            return coord;
        };
        this.scene.sizeXYZ={x:width,y:height, z:0};
        this.scene.shiftXYZ={x:width,y:height, z:0};

        this.div.innerHTML = "<div style='position:relative;width:"+max_width+"px;height:"+max_height+"px;'><div style='position:absolute;'></div></div>";

        if (this.downloadLinkManage) {
            var ttable="<table id='" + this.container + "-svg-download-positioner' style='position:absolute;top:"+height+"px;' width="+width+">";
            ttable +="<tr> <td>" + "<a id='"+this.container+"-svg-download' href='#'>" + "<img width=16 height=16 src=./img/download.gif style='vertical-align:middle;'/>&nbsp;<small id='" + this.container + "-downloadTextSVG'>" + this.downloadGenerateText + "</small></a>";
            ttable +="</td>";

            if (this.downloadDataSource){
                var download_id = this.container +"-dataSource-svg-download";
                var dsHTML = "<select id='"+ download_id +"' >";
                for (var ids=0; ids < this.data.length; ++ids) {
                    var iids = ids+1;
                    dsHTML += "<option value=" + this.data[ids]+ ">serie " + iids + "</option>";
                }
                dsHTML += "</select>";

                ttable += "<td style='text-align:right'>";
                ttable +=  dsHTML + "&nbsp; <input type=button value='download' onclick=\"function tt(aa){var a=gObject(aa).value;var url=vjDS[a].url;vjDS[a].reload(url,true,'download')};tt(\'"+download_id+"\')\"/>";
                   ttable += "</td>";
            }
            ttable+="</tr> </table>";
            this.div.innerHTML += ttable;

            this.downloadLink = gObject(this.container+"-svg-download");
        }

        this.svgdivSizer = this.div.firstChild;
        this.svgdiv = this.svgdivSizer.firstChild;
        this.svgdiv.innerHTML="<svg id='"+this.container+"-svg' xmlns='"+this.scene.elementSource+"' version='1.1' xmlns:xlink='http:
            "</svg>";
        this.scene.svg=gObject(this.container+"-svg");
        this.compose();
    };

    this.hasPlotwithData = function () {

        this.plots=verarr(this.plots);
        for(var i=0;i<this.plots.length;++i){
            var plot=this.plots[i];
            var collections=verarr(plot.collection);
            for(var ic=0;ic<collections.length;++ic){
                if( collections[ic].has_nontrivial_data() ) {
                    return true;
                }
            }
        }
        return false;
    };

    this.absoluteSize = function(size, reference, defaultCallback) {
        if (!size)
            return defaultCallback ? defaultCallback.call(this, reference) : 0;

        if (typeof size=="string" && size.substr(size.length-1)=="%")
            return reference * parseInt(size.substr(0, size.length-1))/100;

        return parseInt(size);
    };
    this.relativeSize = function(size, reference, defaultCallback) {
        if (!size)
            return defaultCallback ? defaultCallback.call(this, reference) : 0;

        if (typeof size=="string" && size.substr(size.length-1)=="%")
            return parseInt(size.substr(0, size.length-1))/100;

        return parseInt(size)/reference;
    };
    this.getChartScale = function() {
        var ret = {
            x: this.absoluteSize(this.chartArea ? this.chartArea.width : 0, this.geometry.width, function(){return this.geometry.width;}),
            y: this.absoluteSize(this.chartArea ? this.chartArea.height : 0, this.geometry.height, function(){return this.geometry.height;}),
            z: 1
            
        };
        if(__getEnvironment()=="mobile")
            ret=this.MobilegetChartScale();
        return ret;
    };
    
    this.MobilegetChartScale = function() {
    
        var ret = {
               x:screen.width,
               y: screen.height*.70,
               z: 1
               
           };
        return ret;
    };
    this.getPreferredScale = function(scale) {
        if (!scale)
            scale = this.getChartScale();

        for (var iplot=0; iplot<this.plots.length; ++iplot) {
            if (!this.plots[iplot].preferredScale)
                continue;
            var pref = this.plots[iplot].preferredScale(scale);
            for (var c in scale)
                scale[c] = Math.max(scale[c], pref[c]);
        }
        return scale;
    };
    this.updateGeometry = function() {
        function sumOnlyAbsolutes() {
            var sum = 0;
            for (var i=0; i<arguments.length; i++) {
                var a = arguments[i];
                if ((typeof a !== "string" || a.substr(a.length-1) !== "%") && !isNaN(a))
                    sum += parseInt(a);
            }
            return sum;
        };

        this.geometry.width = Math.max(this.geometry.width, sumOnlyAbsolutes(this.chartArea.width, this.chartArea.left, this.chartArea.right));
        this.geometry.height = Math.max(this.geometry.height, sumOnlyAbsolutes(this.chartArea.height, this.chartArea.top, this.chartArea.bot));
        if (this.scene && this.scene.svg) {
            this.svgdivSizer.style.width = parseInt(this.geometry.width);
            this.scene.svg.setAttribute("width", parseInt(this.geometry.width));
            this.svgdivSizer.style.height = parseInt(this.geometry.height);
            this.scene.svg.setAttribute("height", parseInt(this.geometry.height));
            if (this.downloadLink && this.downloadLinkManage) {
                var positioner = gObject(this.container + "-svg-download-positioner");
                if (positioner) {
                    positioner.style.top = this.geometry.height + "px";
                }
            }
        }
    };

    this.defaultRescaleChartArea = function(plotScale, threshold, force) {
        if (!force && !threshold) threshold = 1.5;
        var origScale = this.getChartScale();
        if (!force && plotScale.x / origScale.x < threshold && plotScale.y / origScale.y < threshold)
            return false;

        if (this.chartArea.left == undefined)
            this.chartArea.left = this.chartArea.right = (parseInt(this.geometry.width) - origScale.x)/2;
        if (this.chartArea.top == undefined)
            this.chartArea.top = this.chartArea.bot = (parseInt(this.geometry.height) - origScale.y)/2;

        this.chartArea.width = plotScale.x;
        this.chartArea.height = plotScale.y;
     
        if(__getEnvironment()=="mobile"){
            this.chartArea.height = screen.height*.70;
            this.chartArea.width = screen.width;
        }

        return true;
    };
    if (!this.rescaleChartArea) this.rescaleChartArea = this.defaultRescaleChartArea;

    this.compose=function()
    {




        if (this.rescaleChartArea(this.getPreferredScale()))
            this.updateGeometry();

        var chartScale = this.getChartScale();
        this.scene.scaleMatrix(chartScale);
        this.scene.translateMatrix({
            x: this.relativeSize(this.chartArea.left, parseInt(this.geometry.width), function(){return (1-chartScale.x/parseInt(this.geometry.width))/2;}),
            y: 1 + this.relativeSize(this.chartArea.top, parseInt(this.geometry.height), function(){return (1-chartScale.y/parseInt(this.geometry.height))/2;}),
            z: 0
        });
        this.scene.scaleMatrix({x:1,y:-1,z:1});

        for(var iplot=0; iplot<this.plots.length; ++iplot)
        {
            this.plots[iplot].ownerView = this;
            this.plots[iplot].children=[];
            if (this.selectCallback) {
                this.plots[iplot].selectCallback = this.selectCallback;
            }
            this.plots[iplot].construct(this.Axis,this.chartArea,this.scene);
            this.plots[iplot].render(this.scene);
        }

        if (this.downloadLink) {
            if (this.objectUrl) {
                URL.revokeObjectURL(this.objectUrl);
                this.objectUrl = 0;
                this.objectBlob = null;
            }
            var download_link = (typeof(this.downloadLink) === "string") ? gObject(this.downloadLink) : this.downloadLink;
            if (!download_link.onclick) {
                download_link.onclick = (function(viewer) {
                    return function() {
                        viewer.onDownloadGenerateClick();
                    }
               } (this));
            }
            delete download_link.download;
            var download_text = gObject(this.container + "-downloadTextSVG");
            if (download_text) {
                download_text.innerHTML = this.downloadGenerateText;
            }
        }
        this.scene.svg.handler = {
                'onmouseover' : "function:vjObjFunc('Test','" + this.scene.svg.id + "')",
                'onmouseout': "function:vjObjFunc('Test','" + this.scene.svg.id + "')",
                "onclick":"function:vjObjFunc('Test','" + this.scene.svg.id + "')",
                "onmousemove" : "function:vjObjFunc('Test','"+this.scene.svg.id+"')"
        };
     
        return ;
    };
    
    this.Test =  function(ir,svgObj,evt)
    {
        svgObj.setAttribute("stroke-width",5);
        console.log("Hello I am handler");
    };
    this.inherited_resize = this.resize;
    this.resize = function(diffW, diffH) {
        this.resize_info = this.inherited_resize(diffW, diffH);
        return this.resize_info;
    };

    this.inherited_unlink = this.unlink;
    this.unlink=function() {
        for (var iplot=0; iplot<this.plots.length; iplot++) {
            this.plots[iplot].unlink();
        }
        this.plots = [];
        this.inherited_unlink();
    };

    this.init = function () {
        this.associateData();
    };

    this.init();

    this.objectUrl = null;
    this.objectBlob = null;

    this.onDownloadGenerateClick = function() {
        if (this.downloadLink) {
            var async = false;

            if (!this.objectUrl) {
                var images = this.svgdiv.getElementsByTagNameNS(vjSVGElementSource, "image");
                var cnt_image_fixup_needed = 0;
                for (var i = 0; i < images.length; i++) {
                    var href = images[i].getAttributeNS("http:
                    if (href && !href.startsWith("data:")) {
                        cnt_image_fixup_needed++;
                    }
                }

                if (cnt_image_fixup_needed) {
                    async = true;
                    var download_text = gObject(this.container + "-downloadTextSVG");
                    if (download_text) {
                        download_text.innerHTML = this.downloadGeneratingText;
                    }

                    var cnt_image_fixup_done = 0;
                    var svgdiv = document.createElement("div");
                    svgdiv.appendChild(this.svgdiv.firstChild.cloneNode(true));
                    var images = svgdiv.getElementsByTagNameNS(vjSVGElementSource, "image");

                    for (var i = 0; i < images.length; i++) {
                        var href = images[i].getAttributeNS("http:
                        if (href && !href.startsWith("data:")) {
                            (function (viewer, image, href) {
                                var xhr = new XMLHttpRequest();
                                xhr.open("GET", href, true);
                                xhr.responseType = "arraybuffer";
                                xhr.onload = function() {
                                    var content_type = xhr.getResponseHeader("Content-Type") || "image/png";
                                    var data = "data:" + content_type + ";base64," + btoa((function(u8ary) {
                                        var accum = "";
                                        for(var j = 0; j < u8ary.length; j++) {
                                            accum += String.fromCharCode(u8ary[j]);
                                        }
                                        return accum;
                                    } (new Uint8Array(xhr.response))));
                                    image.setAttributeNS("http:
                                    cnt_image_fixup_done++;

                                    if (cnt_image_fixup_done >= cnt_image_fixup_needed) {
                                        viewer.objectBlob = new Blob([svgdiv.innerHTML], {type: "image/svg+xml"});
                                        viewer.objectUrl = URL.createObjectURL(viewer.objectBlob);
                                        viewer.setDownloadLink(svgdiv);
                                    }
                                }
                                xhr.send();
                            } (this, images[i], href));
                        }
                    }
                }
            }

            if (!async) {
                this.setDownloadLink();
            }
        }
    };

    this.setDownloadLink = function(svgdiv) {
        if (!svgdiv) {
            svgdiv = this.svgdiv;
        }
        if (!this.objectUrl) {
            this.objectBlob = new Blob([svgdiv.innerHTML], {type: "image/svg+xml"});
            this.objectUrl = URL.createObjectURL(this.objectBlob);
        }

        var download_link = (typeof(this.downloadLink) === "string") ? gObject(this.downloadLink) : this.downloadLink;
        delete download_link.onclick;
        download_link.download = this.getTitle() + ".svg";
        download_link.href = this.objectUrl;
        if (__isIE && navigator.msSaveBlob) {
            var that = this;
            download_link.onclick = function() {
                window.navigator.msSaveBlob(that.objectBlob, download_link.download);
            }
        }
        var download_text = gObject(this.container + "-downloadTextSVG");
        if (download_text) {
            download_text.innerHTML = this.downloadLinkText;
        }
    };
}
function adjust(angle, radius){
    var Inc=radius/90,diff,alpha = 0;
    if (angle<90){
        diff=angle-90;
        alpha=angle-(angle/Inc);
    }
    else if (angle>90){
        diff=angle-90;
        diff=90-diff;
        alpha=angle+(diff/Inc);
      
    }
    alpha=alpha * (Math.PI / 180);
    return alpha;
}

