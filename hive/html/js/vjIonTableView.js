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

function vjIonTableView ( viewer )
{    
    


    
    var shortListViewers=[];
    var longListViewers=[];
    var joinTableViewers=[];
    
    var ionObj = viewer.ionObject;
    var tbl_Cfg = viewer.tbl_Cfg;
    var joinTbl_Cfg = viewer.joinTbl_Cfg;
    
    
    var mainV_config = {
            ionfile: "sdtmIon.ion",
            ionObject: viewer.ionObject,
            dsname: {
                        shortList:{},
                        longList: "dsIonDetails",
                        joinList: "dsIonJoin"    
                    }
    };
    
    mainV_config.registerShortListDs = function(name,dsname) {
        this.dsname.shortList[name] = dsname;
    }
    
    mainV_config.constructIonUrl = function(ionObj,ionQry, ionfile, noHeader) {
        if (!ionfile)
            ionfile = this.ionfile;
        if (noHeader==undefined)
            noHeader = true;
        
        var cmd = "qpbg_tblqryx4:
        
        var url = cmd + "tqs=[{\"op\":\"ionwander\",\"arg\":{\"ionId\":\""+ ionObj + "\",\"ionFile\":\"sdtmIon.ion\",\"qry\":\"" + encodeURIComponent(ionQry) + "\"}}]";
        if (noHeader){
            url =  urlExchangeParameter(url, "qryPrintsHdr", 0);
        }    
        url =  urlExchangeParameter(url, "cnt", "20");
        return url;
    }
    

    
    mainV_config.constructJoinTblUrl = function(joinCfg) {
        var ionQry = "";
        var primKey = "";
        
        for (var it=0; it<joinCfg.length; ++it) {
            var cur_tbl = joinCfg[it];
            var ionVarName = "" + cur_tbl.tbl + "" + it;
            var ionForEachName = ionVarName + "_f";
            if (it) {
                ionQry += ionVarName + "=find.row(tbl=" + cur_tbl.tbl + ",name=" + cur_tbl.columnName + ",value=" + primKey + ")[0:30];";
            }
            else {
                primKey="" + ionVarName + ".value";
                ionQry  = "" + ionVarName + "=find.row(tbl=" + cur_tbl.tbl + ",name=" + cur_tbl.columnName + ");";
                header += cur_tbl.columnName ;
            }
            
            ionQry += "" + ionForEachName + "=foreach(" + cur_tbl.whatToPrint.join(",") + ");";
            ionQry += "" + ionVarName + "" + it + "=find.row(tbl=" + cur_tbl.tbl + ",#R="+ionVarName + ".#R,name=" + ionForEachName + ".1);";
            ionQry += "ddict.2(" + primKey + "," +  ionVarName + "" + it + ".name," + ionVarName + "" + it + ".value);";
        }
        
        
        var url = this.constructIonUrl(this.ionObject, ionQry,0,false);
        url = urlExchangeParameter(url, "maxCnt", 100000);
        return {url: url};
    }
    
    
    
    
    
    for (var is=0; is< tbl_Cfg.length; ++is) {
        var cur_element = tbl_Cfg[is];
        var dsname = "ds_ion_" + cur_element.tbl;
        
        mainV_config.registerShortListDs(cur_element.tbl,dsname);
        
        var viewer_obj = {name: cur_element.tbl, title:cur_element.title ,viewers:[]}
        
        var startEnd = "[0:20]";
        
        startEnd="";
        
        var ionQry = "a=find.row(tbl=" + cur_element.tbl+ ",name=" + cur_element.key + ")"+ startEnd +";m=foreach("+ cur_element.shortListHeaders.join(",") +");c=find.row(tbl=" + cur_element.tbl + ",#R=a.#R,name=m.1);ddict.2(a.value,c.name,c.value)";
        
        
        var url = mainV_config.constructIonUrl(ionObj,ionQry);
        
        var tblHeader = "" + cur_element.key + "," + cur_element.shortListHeaders.join(",") + "\n";
        
        vjDS.add("Loading " + cur_element.title + " :) ",dsname,url,0,tblHeader);
        
        var panel = new vjBasicPanelView({
            data:["dsVoid",dsname],
            rows:[
                    {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
                    {name: 'search', align: 'right', type: 'search', prefix:"Search : ", title: 'Search a value', description: 'search field', order:'1', isSubmitable:true},
                    {name:'pager', icon:'page' ,align:'right', title:'per page', order:1, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']}
            ],
            configToRetrieve: cur_element,
            mainV_config: mainV_config,
            ionObj: ionObj,
            formObject: viewer.formObject
        }); 
        
        var tbl = new vjTableView({
            data: dsname,
            bgColors : [ '#f2f2f2', '#ffffff' ],
            maxTxtLen: 32,
            isStickyHeader:true,
            selectCallback: populateLongListView,
            configToRetrieve: cur_element,
            ionObj: ionObj,
            mainV_config: mainV_config
        });
        
        viewer_obj.viewers.push(panel);
        viewer_obj.viewers.push(tbl);
        
        shortListViewers.push(viewer_obj);
    }
    
        

    var longLDS = mainV_config.dsname.longList;
    vjDS.add("Loading details table",longLDS,"static:
    
    var detail_panel = new vjBasicPanelView({
        data:["dsVoid",longLDS],
        rows:[
                {name:'refresh', title: 'Refresh' ,order:-1, icon:'refresh' , description: 'refresh the content of the control to retrieve up to date information' ,  url: "javascript:vjDS['$(dataname)'].reload(null,true);"},
                {name: 'search', align: 'right', type: 'search', title: 'Search a value', description: 'search field', order:'1', isSubmitable:true},
                {name:'pager', icon:'page' ,align:'right', title:'per page', order:1, description: 'page up/down or show selected number of objects in the control' , type:'pager', counters: [10,20,50,100,1000,'all']}
        ],
        configToRetrieve: cur_element,
        mainV_config: mainV_config,
        ionObj: ionObj,
        formObject: viewer.formObject
    }); 
    
    var detailTable = new vjTableView({
        data: longLDS,
        bgColors : [ '#f2f2f2', '#ffffff'],
        mainV_config: mainV_config
    });
    
    longListViewers.push(detail_panel);
    longListViewers.push(detailTable);    
    

    var joinUrl = mainV_config.constructJoinTblUrl(joinTbl_Cfg);
    
    var joinLDS = mainV_config.dsname.joinList;
    
    vjDS.add("Loading join table",joinLDS,joinUrl.url,0,"A,B,C,D,E,F,G,H\n");
    
    var joinTbl = new vjTableView({
        data: joinLDS,
        maxTxtLen: 32,
        bgColors : [ '#f2f2f2', '#ffffff' ],
        mainV_config: mainV_config
    });
    joinTableViewers.push(joinTbl);


    
    function populateLongListView (viewer,rowNode,ir,ic) {
        var mainVCfg = viewer.mainV_config;
        var longLDsName = mainVCfg.dsname.longList;
        var myCurConfig = viewer.configToRetrieve;
        var keyToQry = myCurConfig.key;
        var value = rowNode[keyToQry];
        var hdrToDisplay= myCurConfig.longListHeaders;
        var tblToSearch = myCurConfig.tbl;
    
        var ionQry = "";
        if (!hdrToDisplay) {
            ionQry= "a=find.row(tbl=" + tblToSearch+ ",name=" + keyToQry + ",value=\'" + value + "\');b=find.row(tbl=" + tblToSearch + ",#R=a.#R);";
        }
        else ionQry = "a=find.row(tbl=" + tblToSearch+ ",name=" + keyToQry + ",value=\"" + value+ "\");headers=foreach("+ hdrToDisplay.join(",") +");b=find.row(tbl=" + tblToSearch + ",#R=a.#R,name=headers.1);";
        
        ionQry += "printCSV(b.name,b.value);";

        var url = mainVCfg.constructIonUrl(viewer.ionObj,ionQry);
        
        vjDS[longLDsName].reload(url,true);
    }
    
    function performSearch (viewer,rowNode,ir,ic) {
        console.log("searching ... ");
        alert("searching...")
    }
    
    
    return {shortList: shortListViewers, longList: longListViewers, joinTable: joinTableViewers};
}


function dictionarizeTable(data,keyCol,colToCollapse,valueOfColToCollapse,keyColumnRename) {
    if (!data) {
        return data;
    }
    var myData = data;
    
    var splitData = myData.split("\n");
    var totalLines = splitData.length;
    
    var myDictionary = {};
    var headerDictionary={};
    for (var i = 0; i < totalLines; i++) {
        if (i==0)
            continue;
        if (!splitData[i].length) {
            continue;
        }
        var myCellArr = splitData[i].split(",");
        var totCols = myCellArr.length;
        var myKey ="";
        for (var ic=0; ic<totCols; ic++) {
            var myCell = quoteForCSV(myCellArr[ic]);
            if (ic==keyCol) {
                if (!(myCell in myDictionary)) {
                    myDictionary[myCell]={};
                }
                myKey=myCell;
            }
            if (ic==colToCollapse) {
                if (!(myCell in myDictionary[myKey])) {
                    myDictionary[myKey][myCell]=myCellArr[valueOfColToCollapse];
                }
                if (!(myCell in headerDictionary)) {
                    headerDictionary[myCell]=1;                    
                }
            }
        }
    }
    
    var arrayOfHeaderKeys = Object.keys(headerDictionary);
    var lengthArrayOfHeaderKeys = arrayOfHeaderKeys.length;
    
    var output = "";
    
    if (!keyColumnRename) {
        var keyRow = splitData[0];
        var splitKeyRow = keyRow.split(",");
        keyColumnRename = splitKeyRow[keyCol];
    }
    output += keyColumnRename + ",";
    
    
    for (var i=0; i<lengthArrayOfHeaderKeys; i++) {
        var headerKey = arrayOfHeaderKeys[i];
        output += headerKey;
        if (i == lengthArrayOfHeaderKeys-1) {
            output += "\n";
            continue;
        }
        output += ",";
    }
    var arrayOfUSUBJIDKeys = Object.keys(myDictionary);
    var length = arrayOfUSUBJIDKeys.length;
    
    for (var i=0; i<length; i++) {
        var a = arrayOfUSUBJIDKeys[i];
        var b = myDictionary[a];
        output += a + ",";
        
        for (var j=0; j < lengthArrayOfHeaderKeys; j++) {
            var c = arrayOfHeaderKeys[j];
            var value = b[c];
            if (!value) {
                output += "\"\"";
            } else {
                output += value;
            }
            if (j == lengthArrayOfHeaderKeys-1) {
                output += "\n";
                continue;
            }    
            output += ",";
        }
    }
    return output;
}

