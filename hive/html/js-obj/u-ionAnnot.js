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

vjHO.register('u-ionAnnot').Constructor=function ()
{

    // two public functions which must be supported
    this.fullview=function(node,dv)
    {
        this.mode = 'fullview';
        this.preview(node,dv);
    };

    this.preview = function(node,dv)
    {
        this.mode = 'preview';
        this.create(dv,node);
    }
    this.create = function(dv,node){
        this.load(dv,node);
        this.construct();
    }
    
    this.load = function(dvORtab,node){
        this.loaded = true;
        this.loadedID = node.id;
        //var dv_obj = dv.obj.length ? dv.obj[0] : dv.obj;
        this.current_dvORtab=dvORtab;
        this.dvname = this.current_dvORtab.obj.name;
        var formObject = gAncestorByTag(gObject(this.dvname), "form");
        if (formObject)
            this.formName = formObject.name;
        this.addviewer("Table Preview", new vjTextView({ 
                                                        data: "annot_preview"
                                                        ,defaultEmptyText: "file's corrupted"
                                                       }));
        
        this.addviewer("Panel Preview", new vjPanelView({ 
            data: ["dsVoid","annot_preview"]
            ,defaultEmptyText: "file's corrupted"
            ,formObject:document.forms[this.formName]
            , rows: [
                     {name:'recordTypes',isSubmitable: true, isSubmitter: true,title:'Record Type',type:'select',order:1,options:[["seqID","Sequence Identifier","Sequence Identifier"],['type','Type','Type'],['relation','Relation','Relation']]}
                     ]
           }));
        
    }
    
    this.construct = function(){
        if (!this.loaded) return;
        this.constructed = true;
        
        if (this.mode=='preview') {
            var dv=this.current_dvORtab.obj;
            var origNumofTabs=dv.tabs.length;
            this.dvname=dv.name;
            this.dvinfo=dv.name;
            dv.addTab("Annotation Preview","table",[this.viewers['Panel Preview'],this.viewers['Table Preview']]);
            dv.render(true);
            dv.load('rerender');
        }
    }
    this.typeName="u-ionAnnot";
    this.objCls = "obj-"+this.typeName+Math.random();
    vjObj.register(this.objCls, this);
    
    this.viewersToAdd =[];
    
    this.addUrlSet({
        "annot_preview": {
            title: "Retrieving file preview",
            active_url:"http://?cmd=ionAnnotTypes&fromComputation=0&recordTypes=seqID" ,
            objs: "ionObjs"
        }
    });

    
    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
    function blabla (viewer,node,ir,ic) {
         viewer.onUpdate();
    }
};
