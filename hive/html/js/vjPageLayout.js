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

/**
 * This function laysout a VJPAGE
 */
function vjPageLayout()
{
    this.objCls="vjPageLayout";//+Math.random();
    vjObj.register(this.objCls,this);


    this.ok=true;
    this.maxTxtLen=32;
    this.DownloaderReqID=0;
    this.register

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Menu and Toolbars
    // _/

    this.initMenu=function( datasourceURL, formname,wheretogo)
    {
        vjDS.add("infrastructure: Constructing Menu", "dsMenu" , "http://?cmd=objList&type=menuitem&mode=csv" );

        var topMenu=new vjMenuView({ data:'dsMenu', formObject:document.forms[formname] }) ;
        topMenu.iconSize=20;
        // create data store for Menu
        if(!wheretogo)wheretogo="dvMenu";
        vjDV.add(wheretogo).frame="none";
        vjDV.dvMenu.add( "menu", "commands", "menu", topMenu );

        vjDV.dvMenu.render();
        vjDV.dvMenu.load();
        //alert(formname+" while contstructing" );

        // create data store for Popup Menu
        /*
        vjDS.add("Loading...", "dsMenuPopup" , "static://" );
        vjDV.add("dvMenuPopup").frame="none";
        vjDV.dvMenuPopup.add( "menu", "commands", "menu", new vjMenuView({data:'dsMenuPopup',rootMenuStyle:"vertical",maxMenuDepthOpen:2 })  );

        vjDV.dvMenuPopup.render();
        vjDV.dvMenuPopup.load();
        */
    };

    this.initToolbar=function(pagename, datasourceURL, formname )
    {

        vjDS.add("infrastructure: Constructing Toolbar", "dsToolbar"+pagename, datasourceURL );

        // create data store for Menu
        vjDV.add("dvToolbar"+pagename,"100%","100%").frame="none";
        vjDV["dvToolbar"+pagename].add( "toolbar", "", "menu",
            new vjPanelView({
                data:"dsToolbar"+pagename,
                formObject:document.forms[formname],
                iconSize:48,
                showTitles:true
                })
        );

        vjDV["dvToolbar"+pagename].render();
        vjDV["dvToolbar"+pagename].load();
    };


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Record Viewers
    // _/

    this.initStandardRecordViewer=function(dataview,tagWord,formname, readonly, editonly , doShowReadonlyInNonReadonlyMode)
    {

        vjDS.add("infrastructure: Loading Objects Metadata Information", "ds"+tagWord+"Record" , "static://preview:Select objects to see detail metadata" );
        vjDS.add("infrastructure: Obect Specifications","ds"+tagWord+"Spec" , "static:// " );

        vjDV[dataview].add( "details", "rec", "tab",
            [
                new vjRecordView({
                    data:["ds"+tagWord+"Spec","ds"+tagWord+"Record"],
                    showRoot: 0,
                    autoStatus:3,
                    autoDescription:false,
                    objType:"",
                    readonlyMode: readonly ? true : false,
                    editExistingOnly: editonly ? true : false,
                   // constructionPropagateDown:2,
                    showReadonlyInNonReadonlyMode: doShowReadonlyInNonReadonlyMode,
                    RVtag: "RV"+tagWord,
                    formObject:document.forms[formname],
                    isok: true,
                    implementSetStatusButton: readonly ? false : true
                } )
            ]);
    };

    this.initStandardProgressViewer=function (datasource, formname , doneCallbackFun, reportWhenNotDone, totalOnly )
    {

        var viewerProgress=new vjProgressView ({
            data: datasource,
            icon:'tree',
            showRoot: true,
            showLeaf : true ,
            checkLeafs: true,
            checkBranches: true,
            icons: { leaf: 'img/process.gif' },
            showChildrenCount: true,
            showTotalOnly:totalOnly,
            formObject:document.forms[formname],
            autoexpand: "all",
            //icons:{plus:'img/process.gif'},
            //iconSize:24,
            //showRoot: totalOnly ? 0 : 1,
            doneCallback: doneCallbackFun,
            reportEvenWhenNotDone:reportWhenNotDone,

//            maxTxtLen:48,
            isok:true });

        return viewerProgress;
    };


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Panels
    // _/

    this.initStandardObjListPanel=function (tagWord,formname, rowlist)
    {
        //rowlist=verarr(rowlist);

        var cmdlist= [
                {name:new RegExp(/.*/), align:'left'},
                {name:'refresh', align:'left', order:-1 ,title: 'Refresh' , icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS.ds"+tagWord+".reload(null,true);"},
                {name:'download', align:'left', title:'Download', description: 'download the content of this object to local computer', url:"?cmd=objFile&ids=$(ids)", icon: 'download', single_obj_only:true},
                {name:'detail', icon: 'eye', description: 'examine, review, edit details of the object' ,hidden:true, prohibit_new: true  },
                //{name:'create', align:'left', showTitle : true , order:0, title:'Add', description: 'add new objects to the list' , url:"javascript:vjDV.select('dv"+tagWord+"Viewer.add.0',true)" , icon:'plus' },
                {name:'delete', target: 'ajax' , icon: 'delete', description: 'remove the object from the repository', refreshDelay: 1000, prohibit_new: true },
                {name:'admin', hidden:true, description: 'administer this object', prohibit_new: true },
                {name:'edit', target: 'edit' , icon: 'rec', description: 'edit the content of the object', prohibit_new: true  },
                {name:'share', target:'share', icon: 'share' , description: 'share the object with other users of the system; esablish permissions.', prohibit_new: true  },
                {name:'pager', icon:'page' , align:'right',order:2, title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                {name:'search', align:'right',order:10, isSubmitable: true, prohibit_new: true }
                ];
        //alert(tagWord+ " " + jjj(rowlist));
        rowlist=cmdlist.concat(verarr(rowlist));

        var viewerPanel = new vjPanelView({
            data:["dsActions", "ds"+tagWord ],
            formObject: document.forms[formname],
            iconSize:24,
            hideViewerToggle:true,
            showTitles: false,
            rows:rowlist,
            //precompute:"if(node.target) node.url='javascript:linkURL(\"'+node.url+'\",\"'+ node.target+'\")'; node.target=null;",
            precompute:"if(node.target) node.url='javascript:linkURL(\"'+node.url+'\",\"'+ node.target+'\")'; node.target=null;",
            isok:true });


        return viewerPanel;
    };

    this.initBasicObjListPanel=function (tagWord,formname, rowlist,tagWord2,empty)
    {
        rowlist=verarr(rowlist);
        if (empty===undefined) empty= true;
        if (empty==true){
            var rowlist=rowlist.concat( [
                    {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS.ds"+tagWord+".state=\"\";vjDS.ds"+tagWord+".load();"},
                    {name:'pager', icon:'page' , title:'per page',order:2, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                    { name: 'search', align: 'right', type: ' search',order:10, isSubmitable: true, title: 'Search', description: 'search sequences by ID',order:'1', url: "?cmd=objFile&ids=$(ids)" }
                    ]);
        }
        var datalist=new Array();
        datalist[0]="dsVoid";
        if(tagWord!==undefined)datalist[1]="ds"+tagWord;
        if(tagWord2!==undefined)datalist[2]="ds"+tagWord2;
        var viewerPanel = new vjPanelView({
            data:datalist,
            formObject: document.forms[formname],
            iconSize:24,
            hideViewerToggle:true,
            rows: rowlist,
            isok:true });

        return viewerPanel;
    };


    this.initAlignNavigatorPanel = function (tagWord, formname, rowlist, alPosistion) {
        rowlist = verarr(rowlist);
        var rowlist = rowlist.concat([
                { name: 'download', title: 'download', icon: 'download', align:'left', description: 'Download full content', url: "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(url,'cnt','-'),'down','1'))" },
                { name : 'separ', type : 'separator',align:'left' },
                { name: 'refresh', title: 'Refresh', icon: 'refresh', align:'left', description: 'refresh the content of the control to retrieve up to date information', url: "javascript:vjDS.ds" + tagWord + ".state=\"\";vjDS.ds" + tagWord + ".load();" },
                { name: 'pager', icon: 'page', title: 'per page', align: 'left', description: 'page up/down or show selected number of objects in the control', type: 'pager', counters: [10, 20, 50, 100, 1000, 'all'] },
                { name: 'search', isRgxpControl: true, rgxpOff:true,align: 'right', order: Number.MAX_VALUE, type: ' search', isSubmitable: true, title: 'Search', description: 'Search sequences. Regular expressions are currently off', url: "?cmd=objFile&ids=$(ids)" },
        //{ name: 'isRxp', icon:'off_icon', isSubmitable: true, value: false,align: 'right', order: '100', description: 'Regular expressions are currently off' },
                {name: 'alHigh', type: 'text', forceUpdate: true, size: '10', isSubmitable: true, prefix: 'Position', align: 'right', description: 'Jump to specific genomic position', order: '1', title: (alPosistion ? alPosistion : ' '), value: (alPosistion ? alPosistion : ' ') },
                { name: 'alTouch', title: 'Touching', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '2', description: 'Show only reads that touch the position' },
                { name: 'randomize', title: 'Shuffle', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '4', description: 'Show results in non sorted order' },
                { name: 'alPosSearch', title: 'Search pattern in the specified position',path:'/Options/alPosSearch', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '3', description: 'Search pattern occurs only in selected position' }
                ]);

        var viewerPanel = new vjPanelView({
            data: ["dsVoid", "ds" + tagWord],
            formObject: document.forms[formname],
            iconSize: 24,
            icons:{expand:null},
            LastIndexCol: 0,
            hideViewerToggle: true,
            rows: rowlist,
            isok: true
        });

        return viewerPanel;
    };



    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Uploader
    // _/

    this.uploaderSubmit = function(form, tagWord)
    {
        var qty = 0;
        var tgs = [tagWord, "auto_" + tagWord];
        for(var k = 0; k < tgs.length; ++k) {
            var els = form.elements[tgs[k]];
            if( els ) {
                if( els.length ) {
                    for(var i = 0; i < els.length; ++i) {
                        if(els[i].value) {
                            ++qty;
                        } else {
                            // drop empty inputs
                            els[i].parentNode.parentNode.removeChild(els[i].parentNode);
                            --i;
                        }
                    }
                } else if(els.value) {
                    ++qty;
                } else {
                    els.parentNode.parentNode.removeChild(els.parentNode);
                }
            }
        }
        if( qty ) {
            form.submit();
            gObject(tagWord+"_upload_msg").className="sectVis";
        } else {
            this.uploaderAddUpload(tagWord);
        }
        
        if (this.uploaderSubmitCallback)
            this.uploaderSubmitCallback(form, tagWord, qty);
        
        return true;
    }

    this.uploaderAddUpload = function(tagWord)
    {
        tagWord = sanitizeElementId(tagWord);
        var q = gObject(tagWord + "_upl_qty");
        var ur = gObject(tagWord + "_uploader_rows");
        if(q && ur) {
            q.value = parseInt(q.value) + 1;
            var n = document.createElement("div");
            n.id = tagWord + q.value + "_div";
            n.innerHTML = "<img title='delete file from upload' src='img/minus.png' width='16' height='16' onclick='var dd=gObject(\"" + tagWord + q.value + "_div\"); if(dd){dd.parentNode.removeChild(dd)}' />"
                            + "<input type='file' name='" + tagWord + "' id='" + tagWord + q.value + "' multiple/>";
            ur.appendChild(n);
            n = gObject(tagWord + q.value);
            if( parseInt(q.value) > 1 && n ) {
                n.click(); // open file chooser window, but only from second upload
            }
        }
    }
    this.onSelectPopupList = function (viewer, node)
    {
        var list = viewer.accumulate("node.selected", "node");
        
        
        var txt = "<table class='TABLEVIEW_table'>";
        var nn = 0;
        for( var ind in list ) { //Maybe we want to make it multifield in the future
            var obj = list[ind];
            txt += "<tr><td class='TABLEVIEW_table_cell'>";
            txt += "<input type='text' style='display:none' name='onbehalf_"+(++nn)+"' value='"+obj.id+"'></span>";
            txt += "<span>"+(obj._brief?obj._brief:obj.name)+"<span>";
            txt += "</td></tr>";
        }
        txt += "</table>";

        var container = gObject(viewer.tagWord+"_uploader_onbehalfofuser");
        container.innerHTML = txt;

        container = gObject(viewer.tagWord+"_uploader_onbehalfofuser_clean");
        container.style.display='block';
        gModalClose(viewer.myFloaterName+"Div");
    }
    this.onGetPopupList = function(myFloaterName,nodelist) {
        if( !this.popObjectExplorerViewer ) {
            console.log("explorerview object is missing");
            return;
        }
        var tagWord = this.popObjectExplorerViewer.tagWord;
        var container = gObject(tagWord+"_uploader_objlinks");
        if( !container ) {
            console.log("element \""+tagWord+"_uploader_objlinks\" is missing");
            gModalClose(myFloaterName+"Div");
            return;
        }
        var txt = "<table class='TABLEVIEW_table'>";
        var nn = 0;
        for( var objid in nodelist ) {
            var obj = nodelist[objid];
            txt += "<tr><td class='TABLEVIEW_table_cell'>";
            txt += "<input type='text' style='display:none' name='subj_"+(++nn)+"' value='"+obj.id+"'></span>";
            txt += "<span>"+(obj._brief?obj._brief:obj.id)+"<span>";
            txt += "</td></tr>";
        }
        txt += "</table>";
        
        container.innerHTML = txt;

        container = gObject(tagWord+"_uploader_objlinks_clean");
        container.style.display='block';
        
        gModalClose(this.popObjectExplorerViewer.myFloaterName+"Div");
    }
    
    this.uploaderOnBehalfOfUser = function (tagWord, myFloaterName)
    {
        var container = myFloaterName+"Viewer";
        var ds = vjDS["ds"+container];
        if(!ds) {
            ds = vjDS.add("Retrieving Objects Metadata Information","ds"+container,"http://?cmd=usrList&active=1&grp=0&primaryGrpOnly=1&cnt=20&raw=1");
        }
        if(!this.popObjectPanel) {
            this.popObjectPanel = new vjPanelView({
                data:[ "dsVoid", "ds"+container],
                formObject: document.forms["form-floatingDiv"],
                iconSize:24,
                rows: [
                    {name:'pager', icon:'page' , title:'per page', isSubmitable: true, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                    {name: 'search', align: 'right', type: ' search', isSubmitable: true, title: 'Search', description: 'search sequences by ID' }
                ],
                isok:true });
        }
        var defaultOutlineShow = [
               {name:new RegExp(/./), hidden:true },
               {name: '_brief', hidden: false, title: 'Summary' },
               {name: 'description', hidden: false, title: 'Description' },
               {name: 'created', hidden: false, title: 'Created', type: 'datetime'},
               { name: '^id$', hidden: false, title: 'Identifier' },
               {name: 'name', hidden: false, title: 'Name' },
               {name: 'icon', hidden: false, type:'icon', title: 'Icon' },
               {name: new RegExp(/Title.*/i), hidden: false, title: 'Title' }];
        if(!this.popObjectViewer) {
            this.popObjectViewer=new vjTableView( {
                    data: "ds"+container,
                    formObject: document.forms["form-floatingDiv"],
                    bgColors:['#F0F3F9','#ffffff'] ,
                    cols: defaultOutlineShow,
                    checkable:false,
                    maxTxtLen:128,
                    tagWord:tagWord,
                    myFloaterName:myFloaterName,
                    selectCallback: "function:vjObjFunc('onSelectPopupList','" + this.objCls + "','" + tagWord + "')",
                    defaultIcon:'rec',
                    geometry:{ width:'90%',height:'100%'},
                    iconSize:0,
                    isok:true });
        }
        if(vjDV[container])
            delete vjDV[container];
        vjDV.add(container, (this.popUpViewerWidth && !isNaN(this.popUpViewerWidth))? parseInt(this.popUpViewerWidth):600,300);
        vjDV[container].add("select","table","tab",[ this.popObjectPanel, this.popObjectViewer ]);
        vjDV[container].frame = "notab",
        vjDV[container].render();
        vjDV[container].load();
        gModalCallback="true";
        var clickCount=0;
        var xx=(gMoX+1000>gPgW) ? gPgW-1000 : gMoX;
        gModalOpen(myFloaterName+"Div", gModalCallback , xx, gMoY, clickCount);
    }
    
    this.onCleanOnBehalfOfUserList = function (tagWord,myFloaterName) {
        var container = gObject(tagWord+"_uploader_onbehalfofuser");
        if( !container ) {
            console.log("element \""+tagWord+"_uploader_onbehalfofuser\" is missing");
            return;
        }
        container.innerHTML = "";
        container = gObject(tagWord+"_uploader_onbehalfofuser_clean");
        container.style.display='none';
    }
    
    this.onCleanPopupList = function(tagWord,myFloaterName) {
        var container = gObject(tagWord+"_uploader_objlinks");
        if( !container ) {
            console.log("element \""+tagWord+"_uploader_objlinks\" is missing");
            return;
        }
        container.innerHTML = "";
        container = gObject(tagWord+"_uploader_objlinks_clean");
        container.style.display='none';
    }
    
    this.uploaderObjectLink = function(tagWord,myFloaterName)
    {
        if(this.popObjectExplorerViewer){
            this.popObjectExplorerViewer.destroyDS();
            delete this.popObjectExplorerViewer; 
        }
         this.popObjectExplorerViewer=new vjExplorerBaseView({
             container:myFloaterName+"Viewer",
             formFolders:"form-floatingDiv",
             formTables:"form-floatingDiv",
             formSubmit:"form-floatingDiv",
             preselectedFolder:"/All",
             formPreviews:"form-floatingDiv",
             isNShowactions:true,
             subTablesAttrs : [{    tabname : "All",
                                   tabico : "folder-apps",
                                   url : { type : "genome+" , prop:"id,_brief,created" }
                               }],
             folders_DV_attributes : {
                 width : 200,
                 height : 400,
                 hideDV:true
             },
             tables_DV_attributes : {
                 width : 700,
                 height : 400,
                 maxtabs : 8,
                 isok:true
             },
             submit_DV_attributes : {
                 width : "100%",
                 height : 60,
                 frame : "notab",
                 isok:true
             },
             preselectedFolder:"/all",
             myFloaterName: myFloaterName,
             isSubmitMode:true,
             autoexpand:0,
             isNdisplay_Previews:true,
             isNoUpload:true,
             tagWord:tagWord,
             onSubmitObjsCallback: "function:vjObjFunc('onGetPopupList','" + this.objCls + "')",
             drag:false,
             isok:true
         });
         var explorer = this.popObjectExplorerViewer;
         
         explorer.init();
         explorer.render();
         explorer.load();
         gModalCallback="true";
         var clickCount=0;
         var xx=(gMoX+1000>gPgW) ? gPgW-1000 : gMoX;
         gModalOpen(myFloaterName+"Div", gModalCallback , xx, gMoY, clickCount);
    }

    this.uploaderBody = function (tagWord, formname )
    {
        var myFloaterName="floater-"+this.objCls;
        var tMyFloater=gObject("dvFloatingDiv").outerHTML;
        var tMyFloater=tMyFloater.replace(/dvFloating/g,myFloaterName);
        tMyFloater=tMyFloater.replace(/href=\"#\" onclick=\".*\return false;\">/g, "href=\"#\" onclick=\"vjObjEvent('onClosepop','"+this.objCls+"'\)return false;\">");
        gCreateFloatingDiv(tMyFloater);
        
        tagWord = sanitizeElementId(tagWord);
        var t = "" +
            "<span style='display:none;'>" +
                "<input type='hidden' name='cmd' value='submit' />" +
                "<input type='hidden' name='raw' value='1' />" +
                "<input type='hidden' name='req' value='0' />" +
                "<input type='hidden' name='tagWord' value='" + tagWord + "' />" +
                "<input type='hidden' id='" + tagWord + "_upl_qty' value='0' />" +
            "</span>" +
            "<br/>" +
            "<table border='0' width='100%'>" +
                "<tr><td align='center'><div id='" + tagWord + "_uploader_rows'></div></td></tr>" +
                "<tr>" +
                    "<td align='center'>" +
                        "<input type='button' value='Add Another File' onclick='vjPAGE.uploaderAddUpload(\"" + tagWord + "\")' />" +
                        "<input onclick='vjPAGE.uploaderSubmit(this.form,\"" + tagWord + "\")' type='button' name='" + tagWord + "-submit' id='" + tagWord + "-submit' value='Start Upload'>" +
                    " </td>" +
                "</tr><tr>" +
                    "<td align='center'>" +
                        "<table style='display:inline'>"+
                            "<tr>" +
                                "<td>Reference genome(s)</td>" +
                                "<td>" +
                                    "<table class='REC_input' >" +
                                        "<tr>"+
                                            "<td><div style='max-height:600px;max-width:300px;overflow:auto' id='" + tagWord + "_uploader_objlinks'></div></td>"+
                                            "<td style='vertical-align:top'><input class='linker' type='button' onclick='vjPAGE.uploaderObjectLink(\""+tagWord+"\",\""+myFloaterName+"\")' value='...'></td>" +
                                        "</tr>" +
                                    "</table>" +
                                "</td>" +
                                "<td><small>(used when uploading alignments)</small></td>" +
                                "<td><img src='img/delete.gif' style='display:none' height='23' border='0' id='"+tagWord+"_uploader_objlinks_clean' onClick='vjPAGE.onCleanPopupList(\""+tagWord+"\",\""+myFloaterName+"\");'></td> " +
                            "</tr>" +
                        "</table>"+
                    "</td>" +
                "</tr><tr>" +
                    "<td align='center'>" +
                        "<hr/>" +
                        "<label for='chkauto_" + tagWord + "'><input checked type='checkbox' id='chkauto_" + tagWord + "' name='chkauto_" + tagWord + "' value='-1' " +
                        "onClick=\"gObject('idx_" + tagWord + "').disabled=gObject('qc_" + tagWord + "').disabled=gObject('screen_" + tagWord + "').disabled=!this.checked;\"/>" +
                        "automatically process file(s)</label> " +
                        "<label for='idx_" + tagWord + "'><input checked type='checkbox' id='idx_" + tagWord + "' name='idx_" + tagWord + "' value='1'/>Index</label> " +
                        "<label for='qc_" + tagWord + "'><input checked type='checkbox' id='qc_" + tagWord + "' name='qc_" + tagWord + "' value='1'/>Quality Control</label> " +
                        "<label for='screen_" + tagWord + "'><input checked type='checkbox' id='screen_" + tagWord + "' name='screen_" + tagWord + "' value='1'/>Screening</label> " +
                        "<br /><br />" +
                        "<span style='font-style: italic;'>If file type is recognized it will be decompressed, indexed, etc. File extension is used to determine file types</span>" +
                    "</td>" +
                "</tr><tr>" +
                    "<td align='center'>" +
                        "<table style='display:inline'>"+
                            "<tr>" +
                                "<td>On behalf of the user</td>" +
                                "<td>" +
                                    "<table class='REC_input' >" +
                                        "<tr>"+
                                            "<td><div style='max-height:600px;max-width:300px;overflow:auto' id='" + tagWord + "_uploader_onbehalfofuser'></div></td>"+
                                            "<td style='vertical-align:top'><input class='linker' type='button' onclick='vjPAGE.uploaderOnBehalfOfUser(\""+tagWord+"\",\""+myFloaterName+"\")' value='...'></td>" +
                                        "</tr>" +
                                    "</table>" +
                                "</td>" +
                                "<td><img src='img/delete.gif' style='display:none' height='23' border='0' id='"+tagWord+"_uploader_onbehalfofuser_clean' onClick='vjPAGE.onCleanOnBehalfOfUserList(\""+tagWord+"\",\""+myFloaterName+"\");'></td> " +
                            "</tr>" +
                        "</table>"+
                    "</td>" +
                "</tr><tr>" +
                    "<td align='center' class='sectHid' id='" + tagWord + "_upload_msg'>" +
                        "<b>Please, DO NOT LEAVE THIS PAGE until your browser has finished the upload</b>" +
                    "</td>" +
                "</tr>" +
            "</table>";
        t += "<br/><iframe name='" + formname + "IFrame' id='" + formname + "IFrame' frameBorder='0' scrolling='auto' width='90%'></iframe>";
        return t;
    };

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Downloader
    // _/
    this.initStandardDownloader=function (divname, tagWord, formname, cgiCmd)
    {
        document.forms[formname].onsubmit = function(e) {
            var tagWord=this.elements['tagWord'].value;
            this.elements[tagWord+"-submit"].className="sectHid";
            gObject(tagWord+"_upload_msg").className="sectVis";
            return true;
        };

        document.forms[formname].target=formname+"IFrame";
        if(!document.forms[formname].action)document.forms[formname].action="dmUploader.cgi";
        document.forms[formname].method="POST";
        document.forms[formname].enctype="multipart/form-data";

        vjDS.add("infrastructure: Object Specifications", "ds" + tagWord + "Spec", "http://?cmd=propspec&type=svc-download");
        vjDS.add("infrastructure: Constructing Uploader", "ds" + tagWord + "Uploader", "static://" + vjPAGE.uploaderBody(tagWord, formname));
        vjDS.add("infrastructure: Dropbox", "ds" + tagWord + "Dropbox", "http://?cmd=dropboxlist");
        var viewers=[
            new vjHTMLView({
                name:'local file uploader',
                hidden:false,
                icon: 'upload',
                data: "ds"+tagWord+"Uploader",
                callbackRendered: "javascript:vjPAGE.uploaderAddUpload('" + tagWord + "')",
                isok:true}),
            new vjRecordView({
                name:'external downloader',
                icon: 'download',
                data:["ds"+tagWord+"Spec"],
                formObject: document.forms[formname],
                showRoot: false,
                autoStatus:3,
                hidden:true,
                objType:'svc-download',
                doNotDrawDelete:true,
                //debug:true,
                rows: [
                 /*{name:new RegExp(/.* /), type:'hidden'},
                 {name:'name', is_hidden_fg:false },
                 {name:'baseURL', is_hidden_fg:false },
                 {name:'datasource', is_hidden_fg:false },
                 {name:'uri', is_hidden_fg:false },
                 {name:'url_to_uris', is_hidden_fg:true },
                 {name:'download_concurrency', is_hidden_fg:false }*/
                 {name:'external_password', is_hidden_fg:true },
                 {name:'login', is_hidden_fg:true}
                 ],
                cmdPropSet:cgiCmd,
                noFileErrorText:true,
                accumulateWithNonModified:true,
                accumulateWithoutHidden:true,
                constructionPropagateDown: 2,
                //Initially, it's true, which will prohibit the popUp of explorer viewer, I am not sure the original purpose to set it true
                popNExplorer:false,
                forceLoad:true,
                callbackRendered:"function:vjObjFunc(\"checkDropboxElements\",\"" + this.objCls + "\" )",
                RVtag: "RV"+tagWord,
                prefixHTML:"<table class='HIVE_table' width='100%' ><tr><td colspan=3 align=center><input type=button name=c value='START DOWNLOAD' onclick='vjDV.locate(\""+divname+".add._active\").saveValues(0,true,vjPAGE.downloadSubmitted);' /></td></tr></table>",
                appendHTML:"<table class='HIVE_table' width='100%' ><tr><td colspan=3 align=center><input type=button name=c value='START DOWNLOAD' onclick='vjDV.locate(\""+divname+".add._active\").saveValues(0,true,vjPAGE.downloadSubmitted);' /></td></tr></table>",
                isok:true
                }),
            new vjTreeView({
                name:'dropbox',
                hidden:true,
                formObject: document.forms[formname],
                icon: 'upload',
                iconSize:24,
                data: "ds"+tagWord+"Dropbox",
                showRoot:1,
                showLeaf : true ,
                checkLeafs: true,
                checkBranches: true,
                checkPropagate: true,
                showChildrenCount: true,
                autoexpand: 'all',
                icons:{plus:'img/folder-hive.gif', minus:'img/folder-hive.gif', empty:'img/folder-hive.gif' , leaf: null},

                checkLeafCallback:"function:vjObjFunc(\"checkDropboxElements\",\""+this.objCls+"\" )",
                checkBranchCallback:"function:vjObjFunc(\"checkDropboxElements\",\""+this.objCls+"\" )",
                appendHTML:"<table class='HIVE_table' width='100%' ><tr><td colspan=3 align=left><input type=button name=c value='START DOWNLOAD' onclick='vjDV.locate(\""+divname+".add._active\").saveValues(0,true,vjPAGE.downloadSubmitted);' /></td></tr></table>",

                isok:true})

            ];
        if(divname){
            var tb=vjDV[divname].add( "add", "upload", "tab", viewers);
            tb.viewtoggles=-1;
        }
        return viewers;
    };

    this.checkDropboxElements=function(viewer,node)
    {
        if( !viewer._rendered ) {
            return;
        }
        var dropboxviewer = viewer.tab.viewers[2];
        var checklist = verarr(dropboxviewer.accumulate("node.checked==1 && node.leafnode","'dropbox:/'+node.path"));
        var downloader = viewer.tab.viewers[1];
        var cont = downloader.getElementValue("uri");
        if( cont ) {
            var carr = new vjTable(cont, 0, 0, 0, 0, ",;\\s", "keep_empty!!!");
            cont = "";
            for( var ir = 0; ir < carr.rows.length; ++ir ) {
                for( var ic = 0; ic < carr.rows[ir].cols.length; ++ic ) {
                    var c = carr.rows[ir].cols[ic];
                    if( c ) {
                        c = c.toString();
                        if( c.match(/["']?dropbox:\/\//) ) { // dropbox item ?
                            for( var il = 0; il < checklist.length; ++il ) {
                                if(c == checklist[il]) {
                                    break;
                                }
                            }
                            if( il >= checklist.length ) { // not in the checked list ... do not append
                                continue;
                            }
                            cont += "\"" + checklist[il].replace(/"/g, "\"\"") + "\"\n";
                            checklist[il] = ""; // empty the ones added already
                        } else {
                            cont += "\"" + c.replace(/"/g, "\"\"") + "\"\n";
                        }
                    }
                }
            }
        } else {
            cont = "";
        }
        for( var il = 0; il < checklist.length; ++il ) {
            if( checklist[il].match(/["']?dropbox:\/\//) ) { // dropbox item ?
                cont += "\"" + checklist[il].replace(/"/g, "\"\"") + "\"\n";
            }
        }
        downloader.changeElementValue("uri", cont, 0, true);
    };

    this.downloadSubmitted = function(obj, text)
    {
        if (text.indexOf("error") == -1 && text.indexOf("err.") == -1) {
            alert("Download request has been submitted. The progress can be tracked in current folder's Data-loading tab.\n\nAll downloaded items will appear in current folder and/or it's subfolders.");
        } else {
            alert("The information you provided insufficient or has errors, please review and resubmit your request.");
        }
    };

    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ Taxonomy
    // _/

    this.initTaxonomyBrowserTree=function(dvname, tabname,formname, srch)
    {
        var baseURL="http://vio.cgi?vioDBPath=taxtree&bioprojectPath=taxToBioproject&whatToPrint=taxid|rank|matchname|path|parentid|name|allname|bioprojectID|childrenCnt&whatToSearch=name";
        vjDS.add("Retrieving Taxonomy tree","dsNCBITaxonomyTree", baseURL + "&taxid=1&depth=2&cmd=ncbiTaxBrowseDown"+(srch ? "&prop_val="+srch : "" ));//.parser=dataset_check;
        vjDS.add("infrastructure: Constructing taxonomy control","dsTaxonomyActions" , "static:// " );

        var viewerTaxTreePanel = new vjPanelView({
            data:['dsTaxonomyActions', 'dsNCBITaxonomyTree' ],
            formObject: document.forms[formname],
            iconSize:24,
            cmdUpdateURL: baseURL + "&cmd=ncbiTaxBrowseAll&cnt=20",

            rows: [
                //{name:new RegExp(/.*/) },
                {name: 'search', align: 'right', type: 'search', isSubmitable: true, url: ''}]
            });

        var viewerTaxTree = new vjTreeView({

            data: 'dsNCBITaxonomyTree',
            formObject: document.forms[formname],
            showRoot:2,
            showLeaf : true ,
            checkLeafs: true,
            checkBranches: true,
            doNotPropagateUp: true,
            icons: { leaf: 'img/dna.gif' },
            showChildrenCount: true,
            autoexpand: 'all',
            //precompute:"node.path=node.idpath;node.title=node.name;",
            cmdMoreNodes: baseURL + "&taxid=$(taxid)&depth=2&cmd=ncbiTaxBrowseDown",
            postcompute: "if(!parseInt(node.bioprojectID)) node.uncheckable=true;",//alerJ('aaaaaa',node);",s
            objectsDependOnMe: [dvname + '.' + tabname + '.0'],
            isok:true
            });
        vjDV[dvname].add( tabname, "tree", "tab", [ viewerTaxTreePanel, viewerTaxTree ]);
        return viewerTaxTree;
    };


    // _/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // _/
    // _/ List and Hierarchy Viewer Generic
    // _/

    this.initListAndTreeViewerTab=function(dsurl, dvname, tabname, tabico, recordviewer, formname, fullpanel, active,  addCmd, selectCallback, bgColorMap,isdrag)
    {
        var cntViewersIn=(vjDV[dvname].tabs && vjDV[dvname].tabs[tabname]) ? vjDV[dvname].tabs[tabname].viewers.length : 0;

        var dsname='ds-'+dvname+'-'+tabname;
        vjDS.add("infrastructure: Retrieving List of Objects", dsname , dsurl );
        if(!active)active="list";

        if(!isok(bgColorMap))bgColorMap=['#f2f2f2','#ffffff'] ;

        var hideCmd=[{name:new RegExp(/.*/), align:'left', hidden: false }];

        var    editCmd = [
                {name:'delete', align:'left', order:3, hidden:false, target: 'ajax' , icon: 'delete', description: 'remove the object from the repository' , refreshDelay:1000, prohibit_new: true, is_forced_action: true},
                {name:'admin', align:'left', order:13, hidden:true, description: 'administer this object', prohibit_new: true},
                {name:'edit', align:'left', order:14, hidden: false, target: 'edit' , icon: 'rec', description: 'edit the content of the object', prohibit_new: true },
                {name:'share', align:'left', order:15, hidden: false, target:'share', icon: 'share' , description: 'share the object with other users of the system; esablish permissions.', prohibit_new: true }
                ];

        var viewCmd= [
                //{name:new RegExp(/.*/), align:'left', hidden: true },
                {name:'add', align:'left', order:1, hidden:false, showTitle : true , title:'Add', description: 'add new objects to the list' , url:"javascript:vjDV.select('"+dvname+".add.0',true);" , icon:'plus', prohibit_new: true },
                {name:'refresh', align:'left', order:0, hidden: false, align:'left', title: 'Refresh' , icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['"+dsname+"'].reload(null,true);"},
                {name:'detail', align:'left', order:1, hidden: false, icon: 'eye', description: 'examine, review, edit details of the object' ,hidden:true },
                {name:'download', align:'left', order:2, hidden: false, align:'left', title:'Download', description: 'download the content of this object to local computer', url:"?cmd=objFile&ids=$(ids)", icon: 'download', single_obj_only:true},
                {name:'pager', align:'right', order:99, hidden: false, icon:'page' , title:'per page', description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']},
                {name:'search', align:'right', order:100, hidden: false, isSubmitable: true }
                ];

        var rowlist=(fullpanel ? hideCmd.concat(editCmd,viewCmd) : hideCmd.concat(viewCmd) );
        if(addCmd)rowlist=rowlist.concat(addCmd);

        var viewerPanel = new vjPanelView({
            data:["dsActions", dsname ],
            formObject: document.forms[formname],
            iconSize:24,
            hideViewerToggle:true,
            showTitles: false,
            rows:rowlist,
            isok:true });


        var viewerList=new vjTableView( {
            name:'list',
            icon:'list',
            data: dsname,
            hidden: active!='list' ? true: false,
            formObject: document.forms[formname],
            bgColors:bgColorMap,
            cols: [
                {name:new RegExp(/.*/), hidden:true },// , url: (( typeof(recordviewer) == "function" ) ? recordviewer : vjHC.recordViewNode) },
                {name:'id', hidden: false, title:'ID', order: 1 } ,
                {name:'name', hidden: false, title:'Name', wrap: true, order: 2 } ,
                {name:'size', hidden: false, title:"Size" , align: "right", type: "bytes", order: 3 } ,
                {name:'hierarchy', hidden: false, title:"Hierarchy" , align: "left"} ,
                {name:'created', hidden: false, title:"Created" , align: "right", type: "datetime", order: 4 } ,
                {name:'source', hidden: false, title:'Source', wrap: true }
                ],
            vjHCAssociatedRecordViewer: recordviewer+".details.0" ,
            selectCallback: selectCallback ? selectCallback : vjHC.recordViewNode ,
            checkable:true,
            objectsDependOnMe:[ dvname+'.'+tabname+'.'+cntViewersIn ],
            defaultIcon:'rec',
            iconSize:16,
            maxTxtLen:this.maxTxtLen,
            maxTextLenCutExtensionAfterSymbols: "." , // to cut extensions when maxTextLen is Reached
            defaultEmptyText:'no accessible information to show',
            geometry:{ width:'100%'},
            drag:true,
            dragRow:true,
            isok:true });


        var viewerTree=new vjTreeView( {
            name:'hierarchy',
            icon:'tree',
            hidden: active!='tree' ? true: false,
            data: dsname,
            hierarchyColumn: 'path',
            showRoot: true,
            showLeaf : true ,
            checkLeafs: true,
            checkBranches: true,
            hideEmpty: false,
            icons: { leaf: 'img/dna.gif' },
            showChildrenCount: true,
            checkPropagate:true,
            formObject:document.forms[formname],
            linkLeafCallback: selectCallback ? selectCallback : vjHC.recordViewNode  ,
            autoexpand:'all',
            maxTxtLen:this.maxTxtLen,
            vjHCAssociatedRecordViewer:recordviewer+".details.0",
            precompute: "row.path=(row.hierarchy ? row.hierarchy : '/') +row['name'];",
            //postcompute:"if(node.name=='inpos.fa')node.uncheckable=true;alert(node.name + ' - ' + node.uncheckable);",
            objectsDependOnMe:[ dvname+'.'+tabname+'.'+cntViewersIn ],
            drag:isdrag,
            isok:true });

        vjDV[dvname].add( tabname, tabico, "tab", [ viewerPanel,viewerList,viewerTree ]).viewtoggles=-1;

    };




    this.initStandardProcessViewer=function (dvname, datasource, formname , callbackSelect, callbackCheck,  depends , addCols)
    {
        var colList=[
            {name:new RegExp(/.*/), hidden:true },
            {name:'name', hidden: false, title:'Name' , order: 2.5} ,
            {name:'^id$', hidden: false, title:'ID' , order: 1 } ,
            //{name:'reqID', hidden: false, title:'Request' } ,
            {name:'svcTitle', hidden: false, title:'Task', order: 4} ,
            {name:'created', hidden: false, title:'Created', order: 4.5, type: 'datetime'} ,
            {name:'uri', title:'Source', order: 5} ,
            {name:'progress100', hidden: false, title:"Progress%" , align: "right", type: 'percent', order: 2} ,
            {name:'status', order: 2.1233, hidden: false, title:"Status" , align: "left", type: 'choice', choice: ['unknown','waiting','processing','running','suspended','done','killed','failure','error','unknown'] }
            ];
        if(addCols)colList=colList.concat(verarr(addCols));

        var viewerProcessList=new vjTableView( {
            data: datasource,
            icon:"processSvc",
            bgColors:['#f2f2f2','#ffffff'] ,
            formObject: document.forms[formname],
            cols:colList,
            checkable: true ,
            checkCallback: callbackCheck ,
            selectCallback: callbackSelect,
            geometry:{ width:'90%'},
            objectsDependOnMe:depends,
            isNrefreshOnLoad:true,
            maxTxtLen:32,
            iconSize:0,
            isok:true}  );
        //alerJ('d',viewerProcessList)
        return viewerProcessList;
    };





    this.initRecordViewerSingle=function( rv, formname, type , id, rvTag, doSaveButton, isreadonly, isfake)
    {
        var rec=type;//(pos!=-1) type.substring(pos+1) : type
        vjDV[rv].add( rec+": "+(id ? id : type), "table", "tab",
            new vjRecordView( {
                data: id ? ["dsRecordSpec"+rv+type, "dsRecordVals"+rv + id ] : [ "dsRecordSpec"+rv+type],

                formObject:document.forms[formname],
                hiveId: (id && !isfake) ? id : type ,
                showRoot: true,
                autoStatus:3,
                //debug:1,
                autoDescription:false,
                // constructionPropagateDown:true,
                RVtag:rvTag ? rvTag : null,
                objType: type,
                constructionPropagateDown:10,
                implementSaveButton: !isreadonly,
                implementCopyButton: true,
                implementTreeButton:true,
                showReadonlyInNonReadonlyMode: id ? true : false,
                constructAllNonOptionField: id ? true : true,
                implementSetStatusButton:id ? true : false,
                readonlyMode:isreadonly,
                accumulateWithNonModified: true,
                accumulateWithoutHidden: (id && !isfake) ? true : false,
                isok:true
            })
        );

        vjDV[rv].render();
        vjDV[rv].load();
    };

    this.initRecordViewers=function (rv, formname,  types,ids, rvTag,isreadonly, isfake)
    {
        types=verarr(types);
        for( var it=0; it< types.length; ++it ) {
            vjDS.add("infrastructure: Object Specifications", "dsRecordSpec"+rv+types[it] ,"http://?cmd=propspec&type="+types[it]+"&types=1" );
        }
        for( ; it< ids.length; ++it ) {
            types.push("autoType"+ids[it]);
            vjDS.add("infrastructure: Object Specifications", "dsRecordSpec"+rv+types[it] ,"http://?cmd=propspec&ids="+ids[it]+"&types=1" );
        }

        if( isok(ids) || (ids instanceof Array && ids.length )) {
            for( var ii=0; ii< ids.length; ++ii ){
                  vjDS.add("infrastructure: Retrieving Objects Metadata Information", "dsRecordVals"+rv+ids[ii] ,"http://?cmd=propget&mode=csv&ids="+ids[ii] );
                  vjPAGE.initRecordViewerSingle( rv , formname, types[ (ii<types.length) ? ii : types.length-1 ] , ids[ii] , rvTag,null,isreadonly, isfake);
            }
        }

        else {
            for( var it=0; it< types.length; ++it )
                vjPAGE.initRecordViewerSingle( rv, formname, types[it], null, rvTag,null,isreadonly, isfake);
        }

    };

    /**
     * sitemap function fully generates a sitemap.  The sitemap is a tree view that enables the user to navigate quickly through the system.
     * 
     */
    this.sitemap=function(dvName,formName,visibleFields,header)
    /*
     * visibleFields is an array of visible fields 
     * header is true or false
     * 
     */
    
    {
        var dsName='ds'+dvName;
        vjDS.add('infrastructure:', "dsVoid", "static:// ");
        vjDS.add("infrastructure: ",dsName,"http://?cmd=objList&type=webpage&mode=csv");

        // Add the viewer.  The sizes sent here don't seem to change the size of the viewer in the browser
        vjDV.add(dvName,gPgW,gPgH).frame='none';

        // Create the toolbar for the sitemap object with the search attribute
        var viewerToolbar=new vjPanelView({
            data:['dsVoid', dsName]
            ,formObject:document.forms[formName]
            ,iconSize:24
            ,showTitles:true
            ,rows: [
                {name:'search', align:'left',order:10, isSubmitable: true, prohibit_new: true }
                ]
            });

        // Construct the visible fields from visibleFields array
        var _visibleFields = new Array();
        
        // Hide all columns by default
        _visibleFields.push({
            name:new RegExp(/.*/), 
            hidden:true
        })
        
        // Construct the object telling the vjTableView which columns should be visible
        // based on the visibleFields variable passed in
        for (var i = 0; i < visibleFields.length; i++) {
            _visibleFields.push({
                name:visibleFields[i],
                hidden:false
                    });
        }
        
        this.autoexpand = this.autoexpand || 100;  // By default, show all nodes
        this.offSetColumn = this.offSetColumn || 150;

        // Create the vjTableView object to show the sitemap.
        var viewerTree=new vjTableView({
            data:dsName
            ,formObject: document.forms[formName]
            ,autoexpand: this.autoexpand
            ,showRoot:0
            ,offSetColumn: this.offSetColumn
            ,isNheader: !header
            ,showLeaf:true
            ,cols:_visibleFields
            ,treeColumn:'path'
            ,appendCols :  [{header:{name:"path",title:'Tool', type:"treenode",order:1},cell:""}]
            ,showIcons:false
            ,precompute: "if(row.sm_descriptive_text_html && row.sm_descriptive_text_html.length > this.viewer.maxTxtLen){ this.viewer.maxTxtLen = row.sm_descriptive_text_html.length + this.offSetColumn};row.name=row.id;row.url=row.link_url;row.title=(row.sm_icon ? '<img src=?cmd=objFile&raw=1&ids='+row.sm_icon+'&filename=_.png width=15 height=15 />' : '' )+' '+row.title+ ' ' +(row.reference_url ? '<img src=\"img/help.gif\" width=16 />' :'' ) + ' ' "
            });

        vjDV[dvName].add('tabname',"dna","tab",[viewerToolbar,viewerTree]);
        vjDV[dvName].render();
        vjDV[dvName].load();

    }

    this.projectmap=function(dvName,formName)
    {
        var dsName='ds'+dvName;
        vjDS.add('infrastructure:', "dsVoid", "static:// ");
        vjDS.add("infrastructure: ",dsName,"http://?cmd=objList&type=dev-project&mode=csv");

        vjDV.add(dvName,1024*1024,1024*1024).frame='none';


        var viewerToolbar=new vjPanelView({
            data:['dsVoid', dsName]
            ,formObject:document.forms[formName]
            ,iconSize:24
            ,showTitles:true
            ,rows: [
                {name:'search', align:'left',order:10, isSubmitable: true, prohibit_new: true }
                ]
            })

        var viewerTree=new vjTreeView({
            data:dsName
            ,formObject: document.forms[formName]
            ,autoexpand:100
            ,showRoot:0
            ,showLeaf:true
            ,showIcons:false
            ,precompute: "row.name=row.id;row.title=row.dev_title"

            });

        vjDV[dvName].add('tabname',"dna","tab",[viewerToolbar,viewerTree]);
        vjDV[dvName].render();
        vjDV[dvName].load();

    }


}


var vjPAGE=new vjPageLayout();
vjDS.add("infrastructure: Loading Actions", "dsActions" , "http://?cmd=objList&type=action&mode=csv" );

//# sourceURL = getBaseUrl() + "/js/vjPageLayout.js"