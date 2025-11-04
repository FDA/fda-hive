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

if (!javaScriptEngine) var javaScriptEngine = vjJS["undefined"];
javaScriptEngine.include("js/vjTreeSeries.js");
javaScriptEngine.include("js-graph/vjSVG_Phylogram.js");
javaScriptEngine.include("js-graph/vjSVG_plot.js");
javaScriptEngine.include("js-graph/vjSVG_base.js");
javaScriptEngine.include("js-graph/vjSVG_Axis.js");
javaScriptEngine.include("js-graph/vjSVG_primitive.js");
javaScriptEngine.include("js/vjSVGView.js");
javaScriptEngine.include("js-graph/vjSVG_Defaults.js");
javaScriptEngine.include("js-graph/vjSVG_General.js");

javaScriptEngine.include("d3js/zoomable_lineGraph.js");


function vjSpectraList(viewer,mode)
{
    if (!this.name)
        this.name = "Spectra-List";
    if (mode=="fullview") {
        this.checkable=false;
    }
    if (viewer.parentObjCls) this.parentObjCls = viewer.parentObjCls;
    this.iconSize = 0;
    this.bgColors = [ '#efefff', '#ffffff' ];
    this.defaultEmptyText = '';
    this.startAutoNumber = 1;
    this.maxTxtLen = 32;
    this.cols=[
               {    name : new RegExp(/.*/),
                       hidden : true}
                 ,{    name : "id",
                       hidden : false,}
                 ,{    name : "^name",
                    hidden : false,}

               ];

    if (viewer.checkCallback) this.checkCallback = viewer.checkCallback; 
    vjTableView.call(this, viewer);
}


function vjSpectraPanelView(viewer)
{
    this.iconSize=30;
    this.showTitles=true;
    if (viewer.parentObjCls) this.parentObjCls = viewer.parentObjCls;
    if (viewer.relatedViewers) this.relatedViewers = viewer.relatedViewers;
    this.rows=verarr(this.rows).concat([
         { name: 'results',
           icon: 'img/graph2.gif',
           title: 'Mode',
           path:'/results',
           align:'left',
           description: "Graph type" }
        ,{  name: 'original',
            type:"checkbox",
            title: 'Spectrum after binning' ,
            path:'/results/original',
            align:'left',
            description: " Spectrum after binning",
            label:"bin",
            value:true,
            color:"darkred",
        },{ name: 'savgol',
           type:"checkbox",
           title: 'Smoothing by SavGol',
           path:'/results/savgol',
           label:"savgol",
           align:'left',
           color:"blue",
           description: "SavGol Smooth"
        },{ name: 'wavlet',
           type:"checkbox",
           title: 'Smoothing by Wavlet',
           path:'/results/wavlet',
           label:"wavlet",
           align:'left',
           color:"green",
           description: "Wavlet Smooth" 
        },{ name: 'denoising',
           type:"checkbox",
           title: 'Denoising',
           path:'/results/denoising',
           align:'left',
           label:"nobaseline",
           color:"purple",
           description: "Denoising" 
         },{ name: 'baselining',
           type:"checkbox",
           title: 'Baselining',
           label:"nobaseline",
           path:'/results/baselining',
           align:'left',
           color:"purple",
           description: "Baselining" ,
        },{ name: 'reconstructed',
            type:"checkbox",
            title: 'Reconstructed Spectrum',
            label:"reconstructed spectrum",
            path:'/results/reconstructed',
            align:'left',
            color:"purple",
            description: "reconstructed spectrum" ,
         },{ name: 'peak',
           type:"checkbox",
           title: 'Peak Detection',
           path:'/results/peak',
           align:'left',
           color:"darkred",
           description: "Peak Detection" ,
         },{ name: 'submit',
            type:"button",
            title: 'submit',
            path:'/results/submit',
            align:'left',
            color:"darkred",
            description: "submit",     
            url: "javascript:changeURL(thisObj,args[1])"
        },{ name: 'peakInfo',
            hide:true,
            type:"button",
            title: 'Show peak information',
            align:'right',
            color:"darkred",
            description: "show/hide peak information",
            url: "javascript:showHidePeakInfo(thisObj,args[1])"
          },{ name: 'resolution',
            type: "text",
            showTitle: true,
            title: 'Resolution',
            prefix: "Resolution: ",
            align:'right',
            isSumittable: true,
            description: "Resolution" ,
            value: "1000",
            url: "javascript:changeURL(thisObj,args[1])"}
        ]);
    vjPanelView.call(this, viewer);

}

function changeURLold(thisObj,node){

    var nodeMode = "selectedNodes";
    if (thisObj.relatedObj.obj.mode=="fullview") nodeMode ="checkedNodes";
    for (var ii=0;ii<thisObj.relatedObj.obj.viewers[thisObj.relatedObj.directViewer[0]][nodeMode].length;++ii){
        var selectedSpectraList_ID = thisObj.relatedObj.obj.viewers[thisObj.relatedObj.directViewer[0]][nodeMode][ii].id;
        var selectedSpectraList_Name = thisObj.relatedObj.obj.viewers[thisObj.relatedObj.directViewer[0]][nodeMode][ii].name;
        var fnPeak = selectedSpectraList_ID + "-ms-" + "peak2major" + ".csv";
        var objsDir = thisObj.relatedObj.obj.loadedID;
        if (thisObj.relatedObj.obj.selfDir && thisObj.relatedObj.obj.selfDir==1) objsDir = selectedSpectraList_ID;

        var urlPeak = "qpbg_tblqryx4:
        var spectraView = thisObj.relatedObj.obj.viewers[thisObj.relatedObj.indirectViewers[ii]];

        if (node.name === "peakDetection"){
            spectraView.seriePeak.hidden = false;

            var fnLine = selectedSpectraList_ID + "-ms-" + "nobaseline" + ".csv";
            var urlLine = "qpbg_tblqryx4:

            spectraView.serieLine.color = node.color;

            var axisTitle = selectedSpectraList_Name + "_" + node.description ;
            spectraView.Axis.x.title = axisTitle;

            thisObj.relatedObj.obj.getDS(spectraView.dataPeak).reload(urlPeak,true);
            thisObj.relatedObj.obj.getDS(spectraView.dataSpectra).reload(urlLine,true);


        }
        else{

            var fn = selectedSpectraList_ID + "-ms-" + node.label + ".csv";
            var url = "qpbg_tblqryx4:

            spectraView.serieLine.color = node.color;

            spectraView.seriePeak.hidden = true;

            var selectedSpectraList_Name = thisObj.relatedObj.obj.viewers[thisObj.relatedObj.directViewer[0]][nodeMode][ii].name;
            var axisTitle = selectedSpectraList_Name + "_" + node.description ;
            spectraView.Axis.x.title = axisTitle;

            thisObj.relatedObj.obj.getDS(spectraView.dataPeak).reload(urlPeak,true);
            thisObj.relatedObj.obj.getDS(spectraView.dataSpectra).reload(url,true);

        }
    }
}
function changeURL(viewer,node){
    var parentV = vjObj[viewer.parentObjCls];
    var inp_list = parentV.viewerArr[viewer.relatedViewers[0]];
    var spectra_v = parentV.viewerArr[viewer.relatedViewers[1]];
        if (node.name.indexOf("resolution") != -1) {
            var url = vjDS[viewer.relatedDatasource[0]].url;
            url = urlExchangeParameter(url,    "resolution", node.value);
            vjDS[viewer.relatedDatasource[0]].reload(url,true);
            return;
        }
        var resol = viewer.tree.findByName("resolution").value || 1000;
        var node_list = viewer.tree.accumulate("node.type=='checkbox'","node");
        for (var iN=0; iN < node_list.length; ++iN) {
            var cur_node = node_list[iN];
            var cur_dsname ="";
            for (var ids=0; ids < viewer.relatedDatasource.length; ++ids) {
                var dsname = viewer.relatedDatasource[ids];
                if (dsname.indexOf(cur_node.name)!=-1) {
                    cur_dsname = dsname;
                    break;
                }
            }
            if (cur_dsname.length && spectra_v.className_info) {
                var class_info = spectra_v.className_info[cur_dsname];
                var type_list = Object.keys(class_info["type"]);
                for (var it=0; it<type_list.length; ++it) {
                    var cur_type = type_list[it];
                    var cur_info = class_info["type"][cur_type];
                    var className = cur_info.className.split(" ");
                    var attribute = cur_info["attribute"];
                    var value = cur_node.value ? cur_info[attribute] : cur_info["default"]; 
                    if (attribute.indexOf("stroke")!=-1) {
                        d3.selectAll("." + className[0] + "." + className[1]).attr("style", "" + attribute + ":" + value)
                    }
                    else d3.selectAll("." + className[0] + "." + className[1]).attr(attribute, value);
                }
            }
            if (cur_dsname.length && spectra_v.line_set && spectra_v.line_set[dsname]) {
                spectra_v.line_set[dsname].hidden = cur_node.value ? false : true;
            }
            else if (cur_dsname.length && spectra_v.column_set && spectra_v.column_set[dsname]) {
                spectra_v.column_set[dsname].hidden = cur_node.value ? false : true;
            }
        }
}

function showHidePeakInfo(viewer,node) {
    var parentV = vjObj[viewer.parentObjCls];
    var inp_list = parentV.viewerArr[viewer.relatedViewers[0]];
    var spectra_v = parentV.viewerArr[viewer.relatedViewers[1]];
    var peak_className = ".peakInfo_text";
    if (spectra_v.graphId) {
        peak_className+="."+spectra_v.graphId;
    }
    
    if (node.hide==true){
        node.hide = false;
        node.title = "Hide peak information";
        viewer.refresh();
        spectra_v.graphOptions.hidePeakInfo = false;
        d3.selectAll(peak_className).attr("font-size","12px");
        
    }
    else {
        node.hide = true;
        node.title="Show peak information";
        viewer.refresh();
        spectra_v.graphOptions.hidePeakInfo = true;
        d3.selectAll(peak_className).attr("font-size","0px");
    }
    
}
function vjSpectraViewOld(viewer) {

      this.color = "red";
      this.serieLine = new vjDataSeries({
             name: viewer.dataSpectra,
             title: "Spectrum loading",
             url: viewer.urlSpectra,
             columnDefinition:{x:"Dalton",y:"Intensity"},
             type:"line",
             isNXminBased: true,
             color:this.color,
             width: 1.2,
             isok: true
       });

        this.seriePeak = new vjDataSeries({
            name: viewer.dataPeak,
            title: "Peak Loading",
            url: viewer.urlPeak,
            columnDefinition:[{x:"daltons",y:"intensity"}],
            type:"column",
            byX:true,
            isNXminBased: true,
            labels:["name","daltons","intensity"],
             makeGroup: true,
             hidden:true,
             addText:{
                 "name":"name"
                 ,"angle":90
                 ,"size":"8"
                 ,"fill-opacity":1
                 },
             isok:true
        });

      this.plotGraph = new vjSVG_Plot();

      this.plotGraph.add(this.serieLine);
      this.plotGraph.add(this.seriePeak);

      this.plots = [this.plotGraph];
      this.Axis={
              x:{
                  title:"",
                  showGrid: true,
                  textTickSize: "10px",
                  showArrowHead:false
              },
              y:{
                  title:"",
                  showGrid:true,
                  isHidden: false,
                  showArrowHead: false,
                  showTitle:false
              }
          };

      this.hideOnEmptyData = true;
      this.downloadLink = false;
      vjSVGView.call(this, viewer);
}

function vjJustSpectraView(viewer) {
    this.color = "red";
    this.serieLine = new vjDataSeries({
           name: viewer.data,
           title: "Spectrum loading",
           url: viewer.url,
           columnDefinition:{x:"Dalton",y:"Intensity"},
           type:"line",
           isNXminBased: true,
           color:this.color,
           width: 1.2,
           isok: true
     });



    this.plotGraph = new vjSVG_Plot();

    this.plotGraph.add(this.serieLine);


    this.plots = [this.plotGraph];
    this.Axis={
            x:{
                title:"",
                showGrid: true,
                textTickSize: "10px",
                showArrowHead:false
            },
            y:{
                title:"",
                showGrid:true,
                isHidden: false,
                showArrowHead: false,
                showTitle:false
            }
        };

    this.hideOnEmptyData = true;
    this.downloadLink = false;
    vjSVGView.call(this, viewer);
}

function fusionObj(viewbase)
{
    if(!viewbase)viewbase=this;
    var i=0;
    for ( i in viewbase ) {
        this[i] = viewbase[i];
    }

}

function vjSpectraView (viewer) {
    fusionObj.call(this,viewer);
    
    
    vjD3JS_lineGraph.call(this,viewer);
}

