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

function vjPluggableToolbar(viewer) {
    if (viewer.container)
        this.container = viewer.container;
    if (viewer.notToOpenArgs) this.notToOpen = viewer.notToOpenArgs;
    else this.notToOpen = {};
    
    vjPanelView.call(this, viewer);
    
    function findInArray(array, elem, value, substring)
    {
        for (var i = 0; i < array.length; i++)
        {
            if (array[i][elem] && array[i][elem] == value)
                return i;
            if (array[i][elem] && substring && array[i][elem].indexOf(value) >= 0)
                return i;
        }
        
        return -1;
    }

    if (!this.plugins) {
        this.plugins = {
            general: {
                rows: this.rows,
                tree: null,
                icon: null,
                title: ""
            }
        };
    }
    else
    {
        for (var plugin in this.plugins)
        {
            var pluginArr = this.plugins[plugin].rows;
            
            if (this.notToOpen[plugin] && this.notToOpen[plugin].length == 0)
            {
                delete this.plugins[plugin];
                var res = findInArray(this.plugins.general.rows, "name", plugin);
                if (res >= 0 )
                    this.plugins.general.rows.splice(res, 1);
                res = findInArray(this.plugins.general.rows, "path", plugin, true);
                while (res >= 0)
                {
                    var pluginName = this.plugins.general.rows[res].name;
                    delete this.plugins[pluginName];
                    this.plugins.general.rows.splice(res, 1);
                    res = findInArray(this.plugins.general.rows, "path", plugin, true);
                }
                continue;
            }
            
            for (var i = 0; i < pluginArr.length; i++)
            {
                var aaa = pluginArr[i].url;
                
                if (aaa && typeof aaa === "string")
                {
                    var index = aaa.indexOf ("%PANNELCLASS%");
                    if (index > 0)
                        pluginArr[i].url = aaa.substring (0, index)+this.objCls + aaa.substring(index + "%PANNELCLASS%".length);
                    index = aaa.indexOf ("%TABLECLASS%");
                    if (index > 0 && this.tblClass)
                        pluginArr[i].url = aaa.substring (0, index)+this.tblClass + aaa.substring(index + "%TABLECLASS%".length);
                }

                if (this.notToOpen[plugin] && this.notToOpen[plugin].indexOf(pluginArr[i].name)>= 0)
                {
                    pluginArr.splice(i,1);
                    i--;
                }
                    
            }
        }
        
        for (var plugin in this.notToOpen)
        {
            if (this.notToOpen[plugin].length > 0)
                continue;
            
            var res = findInArray(this.plugins.general.rows, "name", plugin);
            if (res >= 0)
                this.plugins.general.rows.splice(res, 1);
            res = findInArray(this.plugins.general.rows, "path", plugin, true);
            while (res >= 0)
            {
                this.plugins.general.rows.splice(res, 1);
                res = findInArray(this.plugins.general.rows, "path", plugin, true);
            }
        }
            
    }
    if (this.tbl_data && this.tbl_dataCallback)
    {
        if (this.tbl_data.indexOf("%PANNELCLASS%") > 0)
        {
            var aaa = this.tbl_data;
            if(aaa && typeof aaa === "string"){
                var index = aaa.indexOf("%PANNELCLASS%");
                if (index > 0)
                    this.tbl_data = aaa.substring (0, index)+this.objCls + aaa.substring(index + "%PANNELCLASS%".length);
            }            
        }

        vjDS.add("List of loadable files", this.tbl_data, "static:
                [{obj: this, func: this.tbl_dataCallback}]);
    }
        
    if (!this.currentPlugin) {
        this.currentPlugin = "basic";
    }
    
    var _super_rebuildTree = this.rebuildTree;
    this.rebuildTree = function(viewer, content, tbl, toRedraw, other) 
    {
        var redrawing = this.currentPlugin;
        if (other && this.plugins[other])
            redrawing = other;
        this.rebuildPlugins(this.getData(0).data);
        this.rows = this.plugins[redrawing].rows;
        this.tree = this.plugins[redrawing].tree;
        var ret = _super_rebuildTree.call(this, viewer, content, new vjTable(null, 0, vjTable_propCSV));
        this.plugins[redrawing].rows = this.rows;
        this.plugins[redrawing].tree = this.tree;
        return ret;
    };

    var _super_redrawMenuView = this.redrawMenuView;
    this.redrawMenuView = function (node, toRedraw) 
    {
        var redrawing = this.currentPlugin;
        if (toRedraw && this.plugins[toRedraw])
            redrawing = toRedraw;
        
        this.rows = this.plugins[redrawing].rows;
        this.tree = this.plugins[redrawing].tree;
        var ret = _super_redrawMenuView.call(this, node);

        this.plugins[redrawing].rows = this.rows;
        this.plugins[redrawing].tree = this.tree;

        return ret;
    };
    
    this.rebuildPlugins = function(data) {
        var panelsObj;
        if (data.indexOf("error: ") != 0) {
            try {
                panelsObj = JSON.parse(data);
            } catch(E) {
                return;
            };
        }
        
        this.additionalPanels = [];
        this.predefinedCategories = [];
        
        for (var i = 0; i < panelsObj.length; i++)
        {
            var currentPanel = panelsObj[i];
            var tqsToUse = currentPanel.tqsToUseOnApply;
            var keyWordArray = currentPanel.keyWords;
            var toPrint = currentPanel.toPrint;
            var notUpdatePage = currentPanel.notUpdatePage;
            
            if (currentPanel.panelName == 0 || currentPanel.panelIcon == 0 || currentPanel.panelTitle == 0 || (this.notToOpen[currentPanel.panelName] && this.notToOpen[currentPanel.panelName].length == 0))
                continue;
            
            if (this.keyWords)
            {
                var encountered = 0;
                for (var ik = 0; ik < keyWordArray.length; ik++)
                {
                    if (this.keyWords.indexOf(keyWordArray[ik].keyWord) >= 0)
                        encountered ++;
                }
                
                if (encountered == 0 && this.keyWords.indexOf("*") < 0)
                    continue;
            }
            
            this.plugins[currentPanel.panelName] = {
                rows: [
                    {name:'apply', align:'left', order:100 ,title: 'Apply' , icon:'recSet' , description: 'load data', url: (notUpdatePage ? "javascript:vjObjEvent(\"notUpdate\",\"" + this.objCls + "\",\"" + sanitizeStringJS(currentPanel.panelName) + "\");" : "javascript:vjObjEvent(\"onUpdate\",\"" + this.objCls + "\",\"" + sanitizeStringJS(currentPanel.panelName) + "\");") },
                    {name:'back', align:'left', order:-1 ,title: 'Back' , icon:'recRevert' , description: 'return to main toolbar' ,  url: "javascript:vjObjEvent(\"onPanelOpen\", \"" + this.objCls + "\",'general');" },
                    {name:'clear', align:'left', icon:'refresh', order:99, title:'Clear', url: "javascript:vjObjEvent(\"onClearAll\", \"" + this.objCls+ "\",\""+sanitizeStringJS(currentPanel.panelName)+"\");" }
                ],
                tree: null,
                title: currentPanel.panelTitle,
                order: i
            };
            
            if (toPrint)
                this.plugins[currentPanel.panelName].toPrint = toPrint;
            if (tqsToUse){
                this.plugins[currentPanel.panelName].tqsToUse = tqsToUse;
            }
            
            if (currentPanel.argument == 0 || currentPanel.argument.length == 0)
            {
                if (! this.noPanelOpen)
                    this.noPanelOpen = new Array ();
                
                this.noPanelOpen.push(currentPanel.panelName);
                
                if (currentPanel.panelPath && this.plugins["general"])
                {
                    var original = currentPanel.panelPath.substring (1,currentPanel.panelPath.indexOf("/",1));
                    
                    for (var x = 0; x < this.plugins["general"].rows.length; x++)
                    {
                        if (this.plugins["general"].rows[x].name == original)
                        {
                            this.plugins["general"].rows[x].hidden = false;
                            break;
                        }
                    }
                    
                    if (currentPanel.panelObjQry)
                        this.plugins.general.rows.push({name:currentPanel.panelName, align:'left', path:currentPanel.panelPath, value: currentPanel.panelObjQry,title: (currentPanel.panelTitle ? currentPanel.panelTitle : currentPanel.panelName) , url: "javascript:vjObjEvent(\"onChangeObjQry\", \"" + this.objCls + "\",\""+sanitizeStringJS(currentPanel.panelObjQry)+"\");"});    
                    else
                        this.plugins.general.rows.push({name:currentPanel.panelName, path:currentPanel.panelPath, align:'left', order:-1 ,title: (currentPanel.panelTitle ? currentPanel.panelTitle : currentPanel.panelName) ,url: "javascript:vjObjEvent(\"onPanelOpen\",\"" + this.objCls + "\", \""+sanitizeStringJS(currentPanel.panelName)+"\");"});
                }
                else
                {
                    if (currentPanel.panelObjQry)
                        this.plugins.general.rows.push({name:currentPanel.panelName, path:"/analyze/"+currentPanel.panelName, align:'left',  value: currentPanel.panelObjQry, title: (currentPanel.panelTitle ? currentPanel.panelTitle : currentPanel.panelName), url: "javascript:vjObjEvent(\"onChangeObjQry\", \"" + this.objCls + "\",\""+sanitizeStringJS(currentPanel.panelObjQry)+"\");"});
                    else
                        this.plugins.general.rows.push({name:currentPanel.panelName, path:"/analyze/"+currentPanel.panelName, align:'left', order:-1 ,title: (currentPanel.panelTitle ? currentPanel.panelTitle : currentPanel.panelName) ,url: "javascript:vjObjEvent(\"onPanelOpen\",\"" + this.objCls + "\", \""+sanitizeStringJS(currentPanel.panelName)+"\");"});
                }

                continue;
            }
            
            var currentPlugin = this.plugins[currentPanel.panelName];
            var advanced = false;

            for (var ia = 0 ; currentPanel.argument && ia < currentPanel.argument.length; ia++)
            {
                var button = currentPanel.argument[ia].argumentButton;
                var name = currentPanel.argument[ia].argumentName;
                var type = currentPanel.argument[ia].argumentType;
                var value = currentPanel.argument[ia].argumentValue;
                var size = currentPanel.argument[ia].argumentSize;
                var description = currentPanel.argument[ia].argumentDesc;
                var path = currentPanel.argument[ia].argumentPath;
                
                if (button == 0 || name == 0 || type == 0 || (this.notToOpen[currentPanel.panelName] && this.notToOpen[currentPanel.panelName].indexOf(name) >= 0))
                    continue;

                var descToPut = "";
                if (description)
                    descToPut = description;

                let argToRow = {
                    name: name, 
                    order:ia*2, 
                    align:'left', 
                    description: descToPut, 
                    type:'select', 
                    readonly:false , 
                    showTitleForInputs: true , 
                    title: button, 
                    isSubmitable:true
                }
                
                if (type=="select")
                {
                    var selectArray = [];
                    while(value.indexOf("|") >= 0)
                    {
                        var a=value.substring(0,value.indexOf("|"));
                        var slashes = a.indexOf("
                        
                        var pos = a.substring(0,slashes);
                        var str = a.substring(slashes+3);
                        
                        value = value.substring (value.indexOf("|")+1);
                        selectArray.push([pos,str]);
                    }
                    var slashes = value.indexOf("
                    
                    var pos = value.substring(0,slashes);
                    var str = value.substring(slashes+3);
                    if (pos.length) selectArray.push([pos,str]);

                    argToRow.options = selectArray
                    argToRow.value = 0
                    
                    if (path && path == "/"){
                        currentPlugin.rows.push(argToRow);
                        continue;
                    }else if(path && path.indexOf ("/") == 0) {
                        argToRow.path = path

                        currentPlugin.rows.push({ argToRow });
                        continue;
                    }
                    
                    if (advanced == false)
                    {
                        currentPlugin.rows.push({
                            name:"advanced", 
                            align:'left', 
                            order:ia*2, 
                            description: descToPut, 
                            value:0, 
                            type:'button', 
                            readonly:false , 
                            showTitleForInputs: false , 
                            title: "Advanced", 
                            isSubmitable:true , 
                            path:"/advanced"
                        });
                        advanced = true;
                    }
                    argToRow.path = "/advanced/" + name 
                    currentPlugin.rows.push(argToRow);
                }
                else if (type == "string")
                {
                    argToRow.type = 'text'
                    if (size) argToRow.size = size
                    currentPlugin.rows.push(argToRow);
                }
                else if (type == "checkbox")
                {
                    var val = 0;
                    if (value)
                        val = value;
                    
                    argToRow.value = val
                    argToRow.type = 'checkbox'

                    if (path && path.indexOf ("/") == 0){
                            argToRow.path = path
                    }    
                    currentPlugin.rows.push(argToRow);
                }
                else {
                    argToRow.group = type
                    argToRow.value = value
                    argToRow.path = "/" + name
                    argToRow.readonly = true
                    argToRow.type = 'color'
                    argToRow.url = "javascript:vjObjEvent(\"startColorCategorizer\",\"" + this.objCls+ "\",\""+sanitizeStringJS(currentPanel.panelName)+"\", \""+sanitizeStringJS(name)+"\",\""+sanitizeStringJS(value)+"\",\""+sanitizeStringJS(type)+"\");" 
                    currentPlugin.rows.push(argToRow);
                }
            }

            let generalrow = {
                name: currentPanel.panelName, 
                order: i, 
                align:'left', 
                path:"/analyze/"+ currentPanel.panelName, 
                align:'left', 
                title: (currentPanel.panelTitle ? currentPanel.panelTitle : currentPanel.panelName) ,
                url: "javascript:vjObjEvent(\"onPanelOpen\", \"" + this.objCls+ "\",\""+sanitizeStringJS(currentPanel.panelName)+"\");" 
            }

            generalrow.path = !currentPanel.panelPath ? `/analyze/${currentPanel.panelName}`: currentPanel.panelPath
            let foundrow = false
            this.plugins.general.rows.forEach(function(row){
                if(row.path === generalrow.path) foundrow =true
            })
            if(!foundrow) this.plugins.general.rows.push(generalrow);
        }
        this.rebuildPluginTrees();
    };
    
    this.applyChangeRow = function (panelName, name, field, value)
    {
        if (!this.plugins[panelName])
            return;
        
        for (var i = 0; i < this.plugins[panelName].rows.length; i++)
        {
            if (this.plugins[panelName].rows[i].name == name)
            {
                this.plugins[panelName].rows[i][field] = value;
                return;
            }
        }
        
    };
    
    this.rebuildPluginTrees = function() {
        for (var pluginName in this.plugins) {
            this.rows = this.plugins[pluginName].rows;
            this.tree = this.plugins[pluginName].tree;
            _super_rebuildTree.call(this, this, "", new vjTable(null, 0, vjTable_propCSV));
            this.plugins[pluginName].rows = this.rows;
            this.plugins[pluginName].tree = this.tree;
        }
        if(this.plugins[this.currentPlugin])
        {
            this.tree = this.plugins[this.currentPlugin].tree;
            this.rows = this.plugins[this.currentPlugin].rows;
        }
    };
    
    this.onClickColor = function(viewer, pluginName, colorPath) {
        var node = this.plugins[pluginName].tree.findByPath(colorPath);
        if (this.colorCallback) {
            funcLink(this.colorCallback, this, node.value, pluginName);
        }
    };
    
    this.onClearAll = function(viewer, pluginName) {
        if (this.clearAllCallback) {
            funcLink(this.clearAllCallback, this, pluginName);
        }
    };
    
    var _super_onChangeElementValue = this.onChangeElementValue;
    this.onChangeElementValue = function (container, nodepath, elname) {
        _super_onChangeElementValue.call (this, container, nodepath, elname);
        if (this.onChangeElementValueCallback) {
            funcLink(this.onChangeElementValueCallback, this, nodepath, elname);
        }
    };
    
    this.onPanelOpen = function(viewer, pluginName) {
        this.currentPlugin = pluginName;
        if (this.panelOpenPreCallback) {
            funcLink(this.panelOpenPreCallback, this, pluginName);
        }
        if (this.panelOpenCallback) {
            funcLink(this.panelOpenCallback, this, pluginName);
        }
        this.rebuildTree();
        this.redrawMenuView();
    };
    
    this.startColorCategorizer = function (viewer, panel, selector, color, type)
    {
        if (this.oldSelector)
            $("#" + this.oldSelector+"_menuButton").removeClass("PANEL_table_select");
        
        this.oldSelector = selector;
        $("#" + this.oldSelector+"_menuButton").addClass("PANEL_table_select");
        
        this.currentSelector = selector;
        this.currentPanel = panel;
        this.currentColor = color;
        this.currentType = type;
        if (this.startColorCategorizerCallback) {
            funcLink(this.startColorCategorizerCallback, this, panel, selector, color, type);
        }
    };
    
    this.notUpdate = function (viewer, panel){
        if (this.notUpdate) {
            funcLink(this.notUpdateCallback, this, panel);
        }
    };
    
    this.onChangeObjQry = function (viewer, query)
    {
        if (this.plugins["load"])
        {
            this.rebuildTree(this, "", "", "", "load");
            
            for (var i = 0; i < this.plugins["load"].rows.length; i++)
            {
                if (this.plugins["load"].rows[i].name == "objQry")
                {
                    this.plugins["load"].rows[i].value = query;
                    this.plugins["load"].tree.findByName("objQry").value = query;
                    this.onUpdate();
                }
            }
        }
    };
    
    this.rebuildPluginTrees();
   
}

