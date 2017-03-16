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

function vjIonExplorerView(viewer)
{
    var explorer="Explorer";
    this.proportionsTree=60;
    
    
    var dsTree=vjDS.add("hierarchy of available folders", "ds"+explorer+"Tree" ,"http://"+ 
            //"dna.cgi?cmd=objList&type=folder&prop=name,child&mode=csv");
            "i.cgi?cmd=brgetjson&sub=%24root&brCnt=20&brStart=0&brSearchTotals=1"
            );
    
    var dsList=vjDS.add("list of available objects", "ds"+explorer+"List" , "http://dna.cgi?cmd=objList&type=folder&prop=name,childs&mode=csv");
                    
    var viewerHC = new vjIonHCView ({
        formObject : document.forms[this.formFolders],
        name : 'hierarchy',
        icon : 'tree',
        data : "ds"+explorer+"Tree",
        dataFormat : "json",
        doNotShowRefreshIcon: true,
        
        hierarchyColumn : 'path',
        highlightRow : true,
        expandSize : 12,
        folderSize : 24,
        showRoot : 0,
        showLeaf : true,
        isok : true
    });


    var viewerList = new vjTableView({
        formObject : document.forms[viewer.formname],
        name : 'list',
        icon : 'list',
        data : "ds"+explorer+"List",
        
        bgColors : [ '#DFDFFF', '#ffffff' ],
        multiSelect:true,
        iconSize : 24,
        iconWidth : 24,
        defaultEmptyText : 'hcion: no accessible information to show',
        isok : true
    });


            
    vjDV.add("dv"+explorer+"Tree", viewer.geometry.width*this.proportionsTree/100 ,viewer.geometry.height );
    vjDV["dv"+explorer+"Tree"].add("Ion Honeycomb","tree","tab",[viewerHC]);
    vjDV["dv"+explorer+"Tree"].load();
    vjDV["dv"+explorer+"Tree"].render();
    
    
    vjDV.add("dv"+explorer+"List", viewer.geometry.width*(100-this.proportionsTree)/100 ,viewer.geometry.height );
    vjDV["dv"+explorer+"List"].add("Details","list","tab",[viewerList]);
    vjDV["dv"+explorer+"List"].load();
    vjDV["dv"+explorer+"List"].render();
    
    
    return {tree: viewerHC, list: viewerList};
};


//# sourceURL = getBaseUrl() + "/js/vjIonExplorerView.js"