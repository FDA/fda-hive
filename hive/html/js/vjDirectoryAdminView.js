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

function vjDirectoryAdminView(viewer) {
    vjDirectoryView.call(this,viewer);
    this.systemFolderList = [
    {
        id : "all",
        name : "All",
        title : "My objects",
        icon : "img/folder-apps.gif",
        order : 2,
        block : {addIn : true,removeFrom : true,delObj : true,moveObj: true},
        isVirtual : true
    }];

    this.onRender = function (){
        if(!currentFolder.length && this.userId){
            this.currentFolder = this.tree.accumulate("node.id=="+this.userId, "node.path")[0];
        }
        var setFolder=this.currentFolder.length ? this.currentFolder: this.systemFolder.All.path;
        this.onClickNode(this.container,setFolder);
    };

    this.findChildrenID=function(node){
        var children =  new Array();
        for(var ci=0;ci<node.children.length;ci++){
            children = children.concat(this.findChildrenID(node.children[ci]));
        }
        if(!children.length ){
            if(this.idtype && (parseInt(node[this.idtype])!=isNaN))
                children.push(node[this.idtype]);
        }
        return children;
    };

    this.onSelectCaptureCallback = function(view, node) {
        if (!node.selected) {
            this.currentFolder = "";
            return false;
        }
        this.currentFolder = node.path ? node.path : "";

        var idList = this.findChildrenID(node);
        var idString="";
        for(var ii=0;ii<idList.length;ii++){
            idString += ","+ idList[ii];
        }
        var url = "";
        if (node[this.idtype] == "all")
            url = "?cmd=allStat&raw=1&mode=csv&type=base_system_type%2B";
        else{
            if(idList.length>0){
                url = "?cmd=allStat&userPerspective=" + idString.substring(1)
                + "&raw=1&mode=csv&type=base_system_type%2B";
            }else{
                alert("this one do not have obj id");
            }

        }
        ajaxDynaRequestPage(url, {
            objCls : this.objCls,
            node : node,
            callback : 'onSelectedFolder'
        }, vjObjAjaxCallback);
    };
}

function vjDirectoryAdminControl(viewer) {


    var arr = new Array("dsActions");
    arr = arr.concat(viewer.data);
    if(viewer.selectInsteadOfCheck===undefined)viewer.selectInsteadOfCheck=true;

    var viewerTree = new vjDirectoryAdminView(viewer);

    var viewerPanel = new vjPanelView(
            {
                data : arr,
                formObject : viewer.formObject,
                iconSize : 24,
                hideViewerToggle : true,
                showTitles : false,
                rows : [{
                            name : new RegExp(/.*/),
                            hidden : true
                        },
                        {
                            name : 'delete',
                            align : 'left',
                            order : 3,
                            hidden : false,
                            icon : 'delete',
                            url : "function:vjObjFunc('deleteObject','"
                                    + viewerTree.objCls + "')",
                            is_obj_action:true,
                            prohibit_new : true,
                            description : 'remove the folder'
                        } ],
                isok : true
            });
    if(viewerTree.selectInsteadOfCheck){
        if(viewerTree.objectsDependOnMe===undefined)
            viewerTree.objectsDependOnMe=[viewerPanel];
        if(viewerTree.actionsOnLeaves===undefined)viewerTree.actionsOnLeaves=true;
    }
    gClip.objectsDependOnMe.push(viewerPanel);
    return [ viewerPanel, viewerTree ];

}

