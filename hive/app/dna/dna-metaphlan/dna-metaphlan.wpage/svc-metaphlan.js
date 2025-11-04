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

vjHO.register('svc-metaphlan').Constructor=function ()
{
    
    this.fullview=function(node, whereToAdd)
    {
        console.log("Constructing Full view from svc-metaphlan.js");
        var id = docLocValue("id");
       
        vjDS.add("Retrieving list of downloadable files", "dsAllDownloads", "http:
       
        vjDS.add("Loading ....", "dsResult", "qpbg_tblqryx4:
        vjDS.add("Loading ....", "dsResultGraph", "static:
        
        var k_dict = {"k":{},"p":{},"c":{},"o":{},"f":{},"g":{},"s":{},"t": {}};

        vjDS['dsResult'].parser = function (ds, text) {
            var rows = text.split('\n');
            var new_text = "";
            for (var ir=0; ir < rows.length;++ir) {
                if (rows[ir].startsWith("#")) {
                    continue;
                }
                if (rows[ir].length <1) {
                    continue;
                }
                var row_sp = rows[ir].replaceAll(/,/g,";");
                row_sp = row_sp.split("\t");
                new_text += row_sp.join(",") + "\n";
                var rank = row_sp[0].split('|');
                var last_element = rank[rank.length-1].split("__");
               
                k_dict[last_element[0]][last_element[1]] = row_sp[2];
                
                
            }
            console.log("k dict: ", k_dict);
            var url = "static:
            var lowest_rank = 'g';
            var species = Object.keys(k_dict[lowest_rank]);
            for (var is=0; is< species.length; ++is) {
                url = url + "Genus: " + species[is] + "," +k_dict[lowest_rank][species[is]] + "\n";
            }
            console.log(url);
            vjDS['dsResultGraph'].reload(url, true);
            return "clade_name,NCBI_tax_id,relative_abundance,additional_species\n" + new_text;
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
        

        var graph_panel = new vjPanelView({
            data:['dsVoid', 'dsResultGraph'],
            iconSize:24,
            formObject: document.forms[formName],
            showTitles:true,
            rows: [
                {name:'rank', align:'left', order:1 ,title: 'Rank', showTitleForInputs: true, path: '/rank', iconSize: 24},
                {name: 'kingdom', align:'left', order:1 ,title: 'Kingdom',  showTitleForInputs: true, path: '/rank/kingdom', url: callbackForPanel},
                {name: 'phylum', align:'left', order:2 ,title: 'Phylum',  showTitleForInputs: true, path: '/rank/phylum', url: callbackForPanel},
                {name: 'class', align:'left', order:3 ,title: 'Class',  showTitleForInputs: true, path: '/rank/class', url: callbackForPanel},
                {name: 'order', align:'left', order:4 ,title: 'Order',  showTitleForInputs: true, path: '/rank/order', url: callbackForPanel},
                {name: 'family', align:'left', order:5 ,title: 'Family', path: '/rank/family', url: callbackForPanel},
                {name: 'genus', align:'left', order:6 ,title: 'Genus', path: '/rank/genus', url: callbackForPanel},
                {name: 'species', align:'left', order:7 ,title: 'Species', path: '/rank/species', url: callbackForPanel}
                ],
            my_data: k_dict
            
        });
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
        }
       ];
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        
        
        algoWidgetObj.openTab("downloadAllFiles");
        
        
    };
    
    function someCallback (viewer, node, irow)
    {
        console.log("calling from someCallback()")
    };

    function callbackForPanel (viewer, node, irow)
    {
        function generateData (k_dict, rank) {
            var url = "static:
            var lowest_rank = rank[0].toLowerCase();
            var species = Object.keys(k_dict[lowest_rank]);
            for (var is=0; is< species.length; ++is) {
                url = url + rank + ": " + species[is] + "," +k_dict[lowest_rank][species[is]] + "\n";
            }
            vjDS['dsResultGraph'].reload(url, true);
        }
        console.log("calling from callbackForPanel()")
        generateData(viewer.my_data, node.title);
    };
    
};
