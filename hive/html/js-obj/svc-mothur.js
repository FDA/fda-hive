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
javaScriptEngine.include("js/vjAlignmentView.js");

vjHO.register('svc-mothur').Constructor=function ()
{

    if(this.objCls)return;

    if (!this.defaultDownloadWildcard)
        this.defaultDownloadWildcard = "*.{gff,out,txt}";

    vjHiveObjectSvcBase.call(this);

    this.resolution=200;

    // two public functions which must be supported
    this.fullview=function(node,dv)
    {
        this.mode='fullview';
        this.query=node.query;
        this.loadedID=node.id;
        this.create(dv,node.id);
    };

    this.customizeSvcDownloads = function(obj) {
        this.parent["svc"].customizeDownloads(this.downloadViewerDescriptions, this.defaultDownloadWildcard);
    };

    this.preview = function(node,dv)
    {
//        this.parent.preview('svc',node,dv);
        this.parent.preview("svc", node, dv, "vjObj['"+this.objCls+"'].customizeSvcDownloads();");
        if(!node.status || parseInt(node.status)<5) return;
        this.mode='preview';
        this.create(dv,node.id);
    };

    this.typeName="svc-mothur";


    this.objCls="obj-"+this.typeName+Math.random();
    vjObj.register(this.objCls,this);


    this.active_url=new Object();
    this.inactive_url=new Object();
    this.viewersToAdd=new Array();
    this.viewers=new Object();

    this.idNulldsName="default";
    this.viewers.length=0;
    this.urlSet({
            'tasks' :{
                title:"retrieving task history",
                active_url:"http://?cmd=objFile&filename=task_array.csv",
                objs:"ids"
            },
            'result-text' : {
                title: "Retrieving text from file",
                active_url: "static://",
                objs:"ids"
//                active_url: "http://?cmd=objFile&filename=file_o_3019493.summary&maxSize=2000&offset=5&raw=1"
            }
    });


    this.create = function(dv, id) {
        this.load(dv, id);
        this.construct(dv, id);
    };

    this.load = function(dvORtab, id, geometry,formName){
        this.loadedID=id;
        if(typeof(dvORtab)=="string")dvORtab=vjDV[dvORtab];
        this.formName='';
        if(formName)this.formName=formName;
        this.viewers.length=0;
        this.viewersToAdd=[];
        this.loaded=true;
        this.constructed=false;

        this.dvresults=dvORtab.obj.length ? dvORtab.obj[0] : dvORtab.obj;
        this.dvresulsPopUp = dvORtab.obj.length ? dvORtab.obj[1] : null;


        var formNode = gAncestorByTag(gObject(this.dvresults.name),"form");
        if(formNode){
            this.formName=formNode.attributes['name'].value;
            formName = this.formName;
        }

        this.viewers.length=0;

        this.taskDS = this.makeDS("tasks");
        this.taskDS.register_callback( {func:this.onTasksLoaded, obj:this} );

        if (this.mode == "preview")
            return;


        // fullview viewers

        this.addviewer ("result-text", new vjTextView({
            data : "result-text",
            formObject : formNode,
            defaultEmptyText : 'select a row to show content',
            geometry : {
                width : '100%'
            },
            isok : true
        }));
        this.addviewer ("result-text-panel", new vjBasicPanelView({
            data : "result-text",
            formObject : formNode,
            defaultEmptyText : 'select a row to show content',
            geometry : {
                width : '100%'
            },
            rows:[
              { name: 'download', align: 'left', showTitle: true, hidden: false, order: 3, title: 'Download', description: 'Download table', icon: 'download',url: downloadResult}
                  ],
            isok : true
        }));

        this.addDownloadViewer(this.downloadViewerDescriptions);


    };


    this.construct=function(dv, id){

        if(!this.loaded || !this.dvresults) return;

        this.constructed=true;


        if(this.mode=='preview'){
//            this.dvresults=this.current_dvORtab[0].name;
            //this.dvresults.addTab(this.tabs.names[1], this.tabs.icons[1], [this.viewers["download"]]);
            //this.dvresults.render();
            //this.dvresults.load('rerender');

        }
        else {
            this.dvresults.addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers["download"]]);
            this.dvresults.render();

            this.dvresulsPopUp.addTab(this.tabs.names[1], this.tabs.icons[1], [this.viewers["result-text-panel"],this.viewers["result-text"]]);
            this.dvresulsPopUp.render();

            this.dvresults.load('rerender');
            this.dvresulsPopUp.load('rerender');
            this.taskDS.load();
        }

    };
    function downloadResult(panel, node){
        var dataName = panel.data[0];
        var myUrl = panel.dataSourceEngine[dataName].dataurl;
        myUrl = myUrl.replace(/\&maxSize=2000/g,"");
        //myUrl = urlExchangeParameter(myUrl, "maxSize", "-1");
        vjDS[dataName].reload(myUrl,true,"download");
    };


    this.onTasksLoaded = function (ext_params,content) {
        this.tasksTbl = new vjTable(content,0,vjTable_propCSV);

        var irow = 0 ,txt = 0 , ds_name =0, fileExtensions;
        for( var ir = 0 ; ir < this.tasksTbl.rows.length ; ++ir ) {
            fileExtensions="{" ;
            irow = this.tasksTbl.rows[ir];

            if (irow.Inputs == "" && irow.Outputs == "") continue; // prevent showing the tabs without any content
            
            var array1=irow.Inputs.split("-");
            var array2=irow.Outputs.split("-");
            
            for(var ix=0;ix<array1.length;ix++){
                if (ix >0) fileExtensions += ",";
                fileExtensions += array1[ix];
            }
            for(var ix=0;ix<array2.length;ix++){
                fileExtensions += "," + array2[ix];
            }

            //txt+=irow.Inputs+","+irow.Outputs;
            fileExtensions += "}";
            var cur_url = "http://?cmd=propget&mode=csv&prop=none&ids="+ thisProcessID + "&files=" + fileExtensions;
            ds_name = "id_"+this.loadedID+"_function_"+ir;

            vjDS.add(undefined,ds_name,cur_url,0,"ids,file,number,outputs\n");
            //vjDS.add(undefined,ds_name,cur_url,0,"ids,file,number,outputs\n");
            var dd = vjDS[ds_name];
            dd.inputsList = array1;
            dd.outputsList = array2;
            dd.parser = function (ds,text){
                var tbl = new vjTable(text,0,vjTable_propCSV);
                var inputsArray = new Array();
                var outputsArray = new Array();
                for (var ir=0; ir < tbl.rows.length; ++ir){
                    var ff = tbl.rows[ir].outputs; // 
                    for (var el =0; el < ds.inputsList.length; ++el){
                        if (ds.inputsList[el].indexOf(ff)!=-1){
                            inputsArray.push(ff);
                            break;
                        }
                    }
                    for (var el =0; el < ds.outputsList.length; ++el){
                        if (ds.outputsList[el].indexOf(ff) !=-1){
                            outputsArray.push(ff);
                            break;
                        }
                    }
                }

                ds.data = "Inputs,Outputs\n";
                var length = inputsArray.length > outputsArray.length ? inputsArray.length : outputsArray.length;
                if ((inputsArray.length && outputsArray.length) || outputsArray.length){
                    for (var ii=0; ii < length; ++ii){
                        if (ii>0) ds.data += "\n";
                        if (inputsArray[ii] != undefined){
                            ds.data += inputsArray[ii] + ",";
                        }
                        else ds.data += ",";
                        if (outputsArray[ii] != undefined){
                            ds.data += outputsArray[ii] ;
                        }
                    }
                } else {
                    //ds.data += ",ERROR\n";
                    for (var ii=0; ii < inputsArray.length; ++ii){
                        if (ii>0) ds.data += "\n";
                        if (inputsArray[ii] != undefined){
                            ds.data += inputsArray[ii] + ",";
                        }
                        else ds.data +=",";
                        ds.data+= "ERROR";
                    }    
                }
                return ds.data;
            }

            var viewer = new vjTableView({
                data:ds_name,
                bgColors : [ '#efefff', '#ffffff' ],
                cols:[
                      {name:"Inputs", url: popUpInputs},
                      {name:"Outputs", url: popUpOutputs}
                      ]
                });
            function popUpInputs (vv, table, irow, icol){
                if (table["Inputs"]) {
                    var url = "http://?cmd=objFile&ids="+ thisProcessID+"&filename="  +table["Inputs"].replace(/\s+/g, '')+"&maxSize=2000";
                    vjDS[vjDV["dvGenemarkResultsPopUpViewer"].tabs[0].viewers[0].data[0]].reload(url,true);
                    vjVIS.winop("vjVisual", "dvGenemarkResultsPopUp", "open", true);
                }
            };
            function popUpOutputs (vv, table, irow, icol){
                if (table["Outputs"] && table["Outputs"].indexOf("ERROR") == -1){
                    var url = "http://?cmd=objFile&ids="+ thisProcessID+"&filename="  +table["Outputs"].replace(/\s+/g, '')+"&maxSize=2000";
                    vjDS[vjDV["dvGenemarkResultsPopUpViewer"].tabs[0].viewers[0].data[0]].reload(url,true);
                    vjVIS.winop("vjVisual", "dvGenemarkResultsPopUp", "open", true);
                }
            };

            this.dvresults.addTab( irow.Functions, "commands", [viewer]);
        }

        this.dvresults.render();
        this.dvresults.load('rerender');
    }

    this.tabs = {
            names: ["PROGRESS", "Result Display" ,"help"],
            icons: ["table", "table", "help"]
        };

    this.downloadViewerDescriptions = {
        "qp-blastp.out" : "BlastP output file",
        "genmark_sequence.gff" : "Genemark outuput file"
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};
