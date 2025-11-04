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

vjHO.register('svc-dna-kraken').Constructor=function ()
{
    
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-dna-kraken.js");
        var id = docLocValue("id");
       
       
        vjDS.add("Loading ....", "dsResult", "qpbg_tblqryx4:
        vjDS.add("Loading ....", "dsResultGraph", "static:
        vjDS.add("Loading ....", "dsResultKrona", "http:
        
        var url = "http:
        vjDS['dsResult'].reload(url,true);
        vjDS['dsResult'].parser = function (ds, data) {
            this.dictData = {"k":[],"d":[], "p":[], "c":[], "o":[], "f":[], "g":[], "s":[]};
            var tblArr = new vjTable(data, 0, vjTable_propCSV);
            for (var i=0; i<tblArr.rows.length; ++i) {
              var path = tblArr.rows[i]["path"];
              if (path != undefined) {
                 var sp = path.split("|").pop();
                 if (sp[0] in this.dictData) {
                    var name = sp.split("__").slice(1).join("_");
                    var count = tblArr.rows[i]["count"];
                    this.dictData[sp[0]].push({[name]:count});
                 }
              }
              
            }
            var domain = this.dictData["g"];
            var newData = "name,count\n" + 
                 domain.map(item => {
                   const key=Object.keys(item)[0];
                   const value =item[key];
                   return `"${key}",${value}`;
                 }).join("\n"); 
            this.panelView.onClickMenuNode(this.panelView.objCls,"/rank/genus");
            return data;
        }
        
        var tbl = new vjTableView({
            data: "dsResult"
        });
        
        var panel = new vjBasicPanelView({
            data: ["dsVoid","dsResult"]
            ,formObject:document.forms[formName]
        });

        panel.rows = panel.rows.concat(
            {
                name: "download",
                title: "Download",
                icon: "download",
                align: "left",
                order: "-1",
                description: "Download content",
                path: "/download",
                url: "javascript: vjDS['dsAllDownloads'].reload(vjDS['dsAllDownloads'].url, true, 'download');"
            }
        );
        
    var pieGraph=new vjGoogleGraphView({
        data: 'dsResultGraph'
       ,type:"pie"
       ,series:[ 
                 {col:0, label:true}
                ,{col: 1}
               ]
        ,options:{
          is3D: true
        }
        ,title:""
  });

        var graph_panel = new vjPanelView({
            data:['dsVoid', 'dsResultGraph'],
            iconSize:24,
            formObject: document.forms[formName],
            showTitles:true,
            rows: [
                {name:'rank', align:'left', order:1 ,title: 'Rank', showTitleForInputs: true, path: '/rank', iconSize: 24,icon: 'img/down.gif'},
                {name: 'kingdom', align:'left', order:1 ,title: 'Kingdom',  showTitleForInputs: true, path: '/rank/kingdom', url: callbackForPanel},
                {name: 'phylum', align:'left', order:2 ,title: 'Phylum',  showTitleForInputs: true, path: '/rank/phylum', url: callbackForPanel},
                {name: 'class', align:'left', order:3 ,title: 'Class',  showTitleForInputs: true, path: '/rank/class', url: callbackForPanel},
                {name: 'order', align:'left', order:4 ,title: 'Order',  showTitleForInputs: true, path: '/rank/order', url: callbackForPanel},
                {name: 'family', align:'left', order:5 ,title: 'Family', path: '/rank/family', url: callbackForPanel},
                {name: 'genus', align:'left', order:6 ,title: 'Genus', path: '/rank/genus', url: callbackForPanel},
                {name: 'species', align:'left', order:7 ,title: 'Species', path: '/rank/species', url: callbackForPanel}
                ],
            my_graph: pieGraph
            
        });
        vjDS["dsResult"].panelView = graph_panel;

        var krona_result = new vjIFrameView({
            data: "dsResultKrona"
        });


        var filesStructureToAdd = [
       {
            tabId: 'resultsTable', 
            tabName: "Results Table",
            position: {posId: 'resultsTable', top:'0', bottom:'45%', left:'20%', right:'100%'},
            viewerConstructor: {
                instance: [panel,tbl]
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'pieGraph',
            tabName: "Pie Graph",
            position: {posId: 'graphArea1', top:'45%', bottom:'100%', left:'20%', right:'100%'},
            viewerConstructor: {
                instance: [graph_panel, pieGraph]
            },
              autoOpen: ["computed"]
        },
        {
            tabId: 'kronaReport',
            tabName: "Krona Graph",
            position: {posId: 'graphArea1', top:'45%', bottom:'100%', left:'20%', right:'100%'},
            viewerConstructor: {
                instance: [krona_result]
            },
              autoOpen: ["computed"]
        }
       ];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");

    };
    
    function someCallback (viewer, node, irow)
    {
        console.log("calling from someCallback()")
    };

    function callbackForPanel (viewer, node, irow)
    {
        function generateData (k_dict, rank,graph) {
            var url = "static:
            var lowest_rank = rank[0].toLowerCase();
            graph.title = rank;
            var domain = k_dict[lowest_rank];
            var newData = "name,count\n" + 
                 domain.map(item => {
                   const key=Object.keys(item)[0];
                   const value =item[key];
                   return `"${key}",${value}`;
                 }).join("\n"); 

            vjDS['dsResultGraph'].reload(url + newData, true);
        }
        console.log("calling from callbackForPanel()")
        generateData(vjDS["dsResult"].dictData, node.title, viewer.my_graph);
    };
    
};
