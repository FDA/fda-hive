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
$(function () {
    $.widget("layout.portalview", $.layout.layoutmanager, {

        options: {
            type: 'vertical',
            tabs: [
                /*{
                    name: "events", 
                    title: "Events", 
                    dsName: "dsEvents", 
                    dsURL: "http://?cmd=objList&type=cdrh-advEvent&mode=csv&prop=event_name,created,table_from&cnt=all", 
                    selectCallback: cdrh_home_callbackTableSelect, 
                    keywords:["event"], 
                    typeOfObjectsToBeShown: "cdrh-advEvent",
                   formObject: document.forms['events-form']
                }*/
            ],
            controlOptions: {
                multiSelect: true,
                colorCategorizer: true,
                localSortDisable: true,
                isok: true
            },
            dissableBtns: false
        },

        _onBeforeInit: function() {
            this.options.config = {
                  layout: {
                      layoutType: 'vertical',
                      items: [{
                              id: 'top',
                              size: '70%',
                              layout: {
                                  layoutType: 'horizontal',
                                  items:[{
                                      id: 'objectTabs',
                                      size: '50%',
                                      allowMaximize: false,
                                      tabs: {
                                          items: []
                                      }
                                  },
                                   {
                                      id: 'moreInfoTabs',
                                      size: '50%',
                                        allowMaximize: true,
                                      tabs:{
                                          items: [{
                                              active: true,
                                              title: 'Preview',
                                              name: 'preview',
                                              class: 'preview',
                                              view: {
                                                  name: 'dataview',
                                                  options: {
                                                      dataViewer: 'vjRecordView',
                                                      dataViewerOptions: {
                                                          data: ['dsRecordSpec', 'dsRecordValues'],
                                                          constructionPropagateDown: 10,
                                                          showReadonlyInNonReadonlyMode: true,
                                                          readonlyMode: true,
                                                          RVtag: 'RViewer'
                                                      }
                                                  }
                                              }
                                          },
                                          {
                                              active: false,
                                              title: 'Edit',
                                              name: 'edit',
                                              class: 'edit',
                                              view: {
                                                  name: 'dataview',
                                                  options: {
                                                      dataViewer: 'vjRecordView',
                                                      dataViewerOptions: {
                                                          data: ['dsRecordSpec', 'dsRecordValues'],
                                                          autoStatus: 3,
                                                          constructionPropagateDown: 10,
                                                          showReadonlyInNonReadonlyMode: false,
                                                          readonlyMode: false,
                                                          RVtag: 'RViewer'
                                                      }
                                                  }
                                              }
                                          }]
                                      }
                                  }]
                              }
                      },
                      {
                          id: 'bottom',
                          size: '50%',
                          view: {
                            name: 'dataview',
                            options: {
                                dataViewer: 'vjGoogleGraphView',
                                dataViewerOptions: {
                                }
                             }
                        }                      
                      }],                
                }
            };

            vjDS.add('infrastructure:', "dsVoid", "static:// ");
            vjDS.add("infrastructure: Loading record specification information", "dsRecordSpec", "static://edit:select object on the left to see the details");
            vjDS.add("infrastructure: Loading record data","dsRecordValues", "static://preview:select object on the left to see the details");
            
            var urlToPut = "http://?cmd=objQry&qry=alloftype('plugin_tblqry').map({{visualization:.visualizationArr,argument:.argumentsArr,panelDesc:.panelDesc,keyWords:.keyWordArr,panelIcon:.panelIcon,panelName:.panelName,panelTitle:.panelTitle,panelUrl:.panelUrl,panelObjQry:.panelObjQry,panelPath:.panelPath}})&raw=1";
            vjDS.add("Table query result", "plugins", urlToPut);

            var generalRows = [
                //{name:'load', order:-3 ,title: 'Load New Data' , icon:'upload' , description: 'load new data' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'load');" },
                {name:'source', order:-3 ,title: 'Data Source' , icon:'database' , description: 'select Source', path:"/source", hidden: true},
                {name:'plus', order:-2 ,title: 'New Column' , icon:'plus' , description: 'add new column' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'newcontrol');", path:"/plus" },
                {name:'graphs', order:-1 ,title: 'Graphs' , icon:'pie' , description: 'select graphic to build', path:"/graphs"},
                {name:'diagrams', order:1 ,title: 'Diagrams' , description: 'select graphic to build', menuHorizontal:false, path:"/graphs/diagrams"},
                {name:'column', order:-1 ,title: 'Column' , description: 'select column graph', menuHorizontal:false, path:"/graphs/diagrams/column" ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'colgraph');" },
                {name:'line', order:-1 ,title: 'Line' , description: 'select line to build',menuHorizontal:false, path:"/graphs/diagrams/line" ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'linegraph');" },
                {name:'pie', order:-1 ,title: 'Pie' , description: 'select pie to build', menuHorizontal:false, path:"/graphs/diagrams/pie" ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'piegraph');" },
                {name:'scatter', order:-1 ,title: 'Scatter' , description: 'select scatter to build', menuHorizontal:false, path:"/graphs/diagrams/scatter" ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'scattergraph');" },
                  
                {name:'pager', icon:'page' , align:'right',order:19, title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                {name:'search', align:'right',order:20, isSubmitable: true, prohibit_new: true },
                {name:'analyze', align:'left', order:0, icon:'graph' , title:'Analysis', path: '/analyze' },
                //{name:'stat', align:'left', order:1, icon:'scatter' , title:'Statistics', path: '/stat' },
                {name:'download', align:'left', order:2, icon:'save' , title:'Download the table', path: '/download' },
                {name:'partOfTable', align:'left', order:1, title:'Download visible part of the table', path: '/download/partOfTable', url: "javascript:vjObjEvent(\"onPartOfTable\", \"%TABLECLASS%\");" },
                {name:'entireTable', align:'left', order:2, title:'Download the entire table', path: '/download/entireTable', url: "javascript:vjObjEvent(\"onEntireTable\", \"%TABLECLASS%\");"}
            ];
            var generalRowsToPush;
                
            if (!this.options.dissableBtns)
                 generalRowsToPush = generalRows;
            else
            {
                generalRowsToPush = [];
                    
                for (var i = 0; i < generalRows.length; i++)
                {
                    if (this.dissableBnts.indexOf(generalRows[i].name) >= 0)
                        continue;
                    else
                        generalRowsToPush.push(generalRows[i]);
                }
            }

               var plugginsToPush= {
                 general: { rows: generalRowsToPush },
                 colgraph: {
                         rows:[
                               {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                               {name:'colorx', path:"/colorx", order:0, value:'#FF85C2', type:'color', readonly:true , showTitleForInputs: true , title: "Select x", url: "javascript:vjObjEvent(\"onGraph\",\"%TABLECLASS%\",\"x\", 'colgraph')" },
                               {name:'colory', path:"/colory", order:2, value:'#6699FF', type:'color', readonly:true , showTitleForInputs: true , title: "Select y", url: "javascript:vjObjEvent(\"onGraph\",\"%TABLECLASS%\",\"y\", 'colgraph')" },
                               {name:'namex', path:"/namex", order:1, showTitleForInputs: false , title: "Name for x-axis", type: "text" },
                               {name:'doHistogram', order:1, showTitleForInputs: false , value: false, title: "Histogram mode", type: "checkbox" },
                               {name:'doIntegral', order:2, showTitleForInputs: false , value: false, title: "Integral mode", type: "checkbox" },
                               {name:'clear', icon:'refresh', order:50, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"%TABLECLASS%\",'colgraph');" },
                               {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGraph\", \"%TABLECLASS%\",\"generate\", 'colgraph');javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
                         title: "Column Graph"
                 },
                 linegraph:{
                     rows:[
                          {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                          {name:'colorx', path:"/colorx", order:0, value:'#99FF66', type:'color', readonly:true , showTitleForInputs: true , title: "Select x", url: "javascript:vjObjEvent(\"onGraph\",\"%TABLECLASS%\",\"x\",'linegraph')" },
                          {name:'colory', path:"/colory", order:2, value:'#A3D1FF', type:'color', readonly:true , showTitleForInputs: true , title: "Select y", url: "javascript:vjObjEvent(\"onGraph\",\"%TABLECLASS%\",\"y\",'linegraph')" },
                          {name:'namex', path:"/namex", order:1, showTitleForInputs: false , title: "Name for x-axis", type: "text" },
                          {name:'doHistogram', order:1, showTitleForInputs: false , value: false, title: "Histogram mode", type: "checkbox" },
                          {name:'doIntegral', order:2, showTitleForInputs: false , value: false, title: "Integral mode", type: "checkbox" },
                          {name:'clear', icon:'refresh', order:50, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"%TABLECLASS%\",'linegraph');" },
                          {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGraph\", \"%TABLECLASS%\",\"generate\", 'linegraph');javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
                     title: "Line Graph"
                },
                    piegraph: {
                        rows:[
                             {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                             {name:'colorx', path:"/colorx", order:0, value:'#FFFF99', type:'color', readonly:true , showTitleForInputs: true , title: "Select Category", url: "javascript:vjObjEvent(\"onGraph\",\"%TABLECLASS%\",\"x\",'piegraph')" },
                          {name:'colory', path:"/colory", order:1, value:'#DBB8FF', type:'color', readonly:true , showTitleForInputs: true , title: "Select Measurement", url: "javascript:vjObjEvent(\"onGraph\",\"%TABLECLASS%\",\"y\",'piegraph')" },
                          {name:'clear', icon:'refresh', order:50, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"%TABLECLASS%\",'piegraph');" },
                          {name:'doHistogram', order:1, showTitleForInputs: false , value: false, title: "Histogram mode", type: "checkbox" },
                          {name:'doIntegral', order:2, showTitleForInputs: false , value: false, title: "Integral mode", type: "checkbox" },
                          {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGraph\", \"%TABLECLASS%\",\"generate\", 'piegraph');javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
                     title: "Pie Graph"
                },
                    scattergraph: {
                        rows:[
                          {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                          {name:'colorx', path:"/colorx", order:0, value:'#D685FF', type:'color', readonly:true , showTitleForInputs: true , title: "Select x", url: "javascript:vjObjEvent(\"onGraph\",\"%TABLECLASS%\",\"x\",'scattergraph')" },
                          {name:'colory', path:"/colory", order:2, value:'#B5FFDA', type:'color', readonly:true , showTitleForInputs: true , title: "Select y", url: "javascript:vjObjEvent(\"onGraph\",\"%TABLECLASS%\",\"y\",'scattergraph')" },
                          {name:'namex', path:"/namex", order:1, showTitleForInputs: false , title: "Name for x-axis", type: "text" },
                          {name:'doHistogram', order:1, showTitleForInputs: false , value: false, title: "Histogram mode", type: "checkbox" },
                          {name:'doIntegral', order:2, showTitleForInputs: false , value: false, title: "Integral mode", type: "checkbox" },
                          {name:'clear', icon:'refresh', order:50, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"%TABLECLASS%\",'scattergraph');" },
                          {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onGraph\", \"%TABLECLASS%\",\"generate\", 'scattergraph');javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
                      title: "Scatter Graph"
                    },
                    newcontrol: {
                        rows:[
                          {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                          {name:'newname' , path:"/newname", order: 1, type: "text", url: "javascript:vjObjEvent(\"onAddNewColumn\", \"%TABLECLASS%\");", title:'Column Name' },
                          {name:'newformula', path:"/newformula", order: 2, description: 'add formula for new column' , type:'text', title:'Formula' },
                          //{name:'colType', path:"/colType", order: 3, type:'select', options:[['-1',' '], ['0','String'],['1','Integer'],['2','Real']], title: 'Choose Column Type (Optional)'},
                          {name:'search', align:'right',order:10, isSubmitable: true, prohibit_new: true },
                          {name:'apply', order:8 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onAddNewColumn\", \"%TABLECLASS%\");vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }],
                     title: "New Column",
                     icon: "plus"
                    },
                    editcontrol: {
                        rows:[
                             {name:'back', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"%PANNELCLASS%\",'general');" },
                          {name:'rename', order: 1, title: ' Rename' , showTitleForInputs:true, type: "text", url: "javascript:vjObjEvent(\"onRenameHeader\", \"%TABLECLASS%\");" },
                          {name:'filterOnCol', order:0, title:'Select Column to Filter on', description: 'Select Column to Filter on and Hide Rows'},
                          {name:'colType', order:3, hidden:true, title:'Change Column Type', description: 'Change the type of the column', type:'select', options:[['0','Integer'],['1','String']], showTitleForInputs:true, url: "javascript:vjObjEvent(\"onChgColType\", \"%TABLECLASS%\")"},
                          {name:'stringExpression',  order:1, hidden:true, size:'40', type: "text", path: "/stringExpressionBtn/stringExpression", title: "Expression"},
                          {name:'stringExpressionBtn',  order:4, hidden:true, value:'Enter expression to show rows by', type: "button", url: "javascript:vjObjEvent(\"onExpHide\", \"%TABLECLASS%\");"},
                          {name:'stringExpressionNegationCheck',  order:1, hidden:true, type: "checkbox", title: "Check for Negation", path: "/stringExpressionBtn/stringExpressionNegationCheck"},
                          {name:'stringExpressionRegCheck',  order:2, hidden:true, type: "checkbox", title:"Check for Regular Expressions", path: "/stringExpressionBtn/stringExpressionRegCheck"},
                          {name:'stringExpressionCaseSens',  order:3, hidden:true, type: "checkbox", title:"Check for Case Sensitive", path: "/stringExpressionBtn/stringExpressionCaseSens"},
                          {name:'intExpression',  order:6, hidden:true, value: 'Enter expression to show rows by', type: "button", url: "javascript:vjObjEvent(\"onExpHide\", \"%TABLECLASS%\");"},
                          {name:'intFromExpression',  order:4, hidden:true, title: 'From', showTitleForInputs:true, size:'5', type: "text", path: "/intExpression/intFromExpression"},
                          {name:'intToExpression',  order:5, hidden:true, title: 'To', showTitleForInputs:true, size:'5', type: "text", path: "/intExpression/intToExpression"},
                          {name:'intExclusiveExpression',  order:6, hidden:true, title: 'Exclusive', showTitleForInputs:true, size:'5', type: "checkbox", path: "/intExpression/intExclusiveExpression"},
                          {name:'stringColoration',  order:1, hidden:true, size:'40', type: "text", path:"/stringColorationBtn/stringColoration", title: "Expression"},
                          {name:'stringColorationBtn',  order:5, hidden:true, value:'Enter expression for coloration of rows', type: "button", url: "javascript:vjObjEvent(\"onColorizeRows\", \"%TABLECLASS%\");"},
                          {name:'stringColorationNegationCheck',  order:5, hidden:true, type: "checkbox", title: "Check for Negation", path: "/stringColorationBtn/stringColorationNegationCheck"},
                          {name:'stringColorationRegCheck',  order:6, hidden:true, type: "checkbox", title:"Check for Regular Expressions", path: "/stringColorationBtn/stringColorationRegCheck"},
                          {name:'intFromColoration',  order:7, hidden:true, title: 'From', showTitleForInputs:true, size:'5', type: "text", path: "/intColoration/intFromColoration"},
                          {name:'intToColoration',  order:8, hidden:true, title: 'To', showTitleForInputs:true, size:'5', type: "text", path: "/intColoration/intToColoration"},
                          {name:'intExclusiveColor',  order:9, hidden:true, title: 'Exclusive', showTitleForInputs:true, size:'5', type: "checkbox", path: "/intColoration/intExclusiveColor"},
                          {name:'intColoration',  order:9, hidden:true, value:'Enter expression for coloration of rows', type: "button",  url: "javascript:vjObjEvent(\"onColorizeRows\", \"%TABLECLASS%\");"},
                          {name:'intFiltcolor', order:10, hidden:true, value:"#ff9999", title: 'Choose color' ,  type:'color', showTitleForInputs: true , title: "Select color", path:"/intColoration/intFiltcolor"},
                          {name:'stringFiltcolor', order:10, hidden:true, value:"#ff9999", title: 'Choose color' ,  type:'color', showTitleForInputs: true , title: "Select color", path:"/stringColorationBtn/stringFiltcolor"},
                          {name:'del', order: 15, icon:'delete' , title:'hide', description: 'hide the column', url: "javascript:vjObjEvent(\"onDeleteColumn\", \"%TABLECLASS%\");vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" },
                          {name:'apply', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', isSubmitable: true, prohibit_new: true, url: "javascript:vjObjEvent(\"onUpdate\",\"%PANNELCLASS%\")" }
                          //{name:'intensity', order:9, hidden:true, description: 'Check for Gradient' ,  type:'checkbox', title: "Check for Gradient Coloring", url: "javascript:vjObjEvent(\"onCheckGradient\", \"" + objcls+ "\");"}
                     ]
                    }
               };

             for (var i = 0; i < this.options.tabs.length; i++)
             {
                 var tab = this.options.tabs[i];

                 this._addNewTab(tab);
                 
                 vjDS.add("", tab.dsName, tab.dsURL);
                 var activate = (i == 0 ? true : false);

                 var tableObjCls = "table" + Math.floor(Math.random()*1111111);
             
                 var toAdd = {
                      active: activate,
                      title: tab.title,
                      name: tab.name,
                      layout:{
                          layoutType: 'vertical',
                          allowResize: false,
                          border: 'none',
                          items: [{
                              size: '40',
                              overflow: 'hidden',
                              view: {
                                  name: 'dataview',
                                  options: {
                                      dataViewer: 'vjPluggableToolbar',
                                      dataViewerOptions: {
                                         data: ["plugins", tab.dsName],
                                        iconSize : 24,
                                        showTitles: true,
                                        showTitleFOrInputs: true,
                                        name: "theMainOne",
                                        menuUpdateCallback:"function:vjObjFunc(\"menuUpdateCallback\", \"%TABLECLASS%\");",
                                        formObject: tab.formObject,
                                        currentPlugin: "general",
                                        panelOpenCallback: "function:vjObjFunc(\"onPanelOpen\", \"%TABLECLASS%\");",
                                        clearAllCallback: "function:vjObjFunc(\"onClearAll\", \"%TABLECLASS%\");",
                                        keyWords: this.keyWords,
                                        container: "pluggin"+Math.floor(Math.random()*1111111),
                                        tblClass: tableObjCls,
                                        plugins: plugginsToPush,
                                        //tbl_data: tbl_data,
                                        //onChangeElementValue: myonChangeElementValue,
                                        isok: true
                                      }
                                 }
                             }
                         },
                         {
                             size: '*',
                             view: {
                                 name: 'dataview',
                                 options: {
                                     dataViewer: 'vjTableControlX2',
                                     dataViewerOptions: {
                                         data: tab.dsName,
                                         //container: tableObjCls,
                                         isok: true
                                     }
                                 }
                             }
                         }]
                     }
                 };
              
                 this.options.config.layout.items[0].layout.items[0].tabs.items.push(toAdd);
             }
        },
        
        _addNewTab: function(tab) {
            
        }
    });

}(jQuery));
