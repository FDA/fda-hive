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

vjHO.register('svc').Constructor=function ()
{
    if(this.objCls)return;
    vjHiveObjectSvcBase.call(this);
    // two public functions which must be supported
    this.fullview=function(node,dv)
    {
        this.mode='fullview';
        this.node=node;
        this.create(dv,node.id);
    };

    this.preview = function(node,dv)
    {
        this.mode='preview';
        this.node=node;
        this.create(dv,node.id);
    };

    this.typeName="svc";

    this.autoClickedReference=1;

    this.objCls="obj-svc"+Math.random();
    vjObj.register(this.objCls,this);

    //this.tabs={icons:["process"],names:["progress"]};

    this.active_url=new Object();
    this.inactive_url=new Object();
    this.idNulldsName="default";
    this.viewersToAdd=[];
    this.viewers=new Object();
    this.viewers.length=0;
    this.active_url={'progress' : "http://?cmd=-qpRawCheck"};
    this.inactive_url={'progress':"static://"};

    this.addUrlSet({
        'progress': {
            active_url:"http://?cmd=-qpRawCheck&showreqs=0",
            objs:"reqObjID",
            title: "infrastructure: Showing progress"
        },
        'download': {
            active_url:"http://?cmd=-qpRawCheck&showreqs=0",
            objs:"reqObjID",
            title: "infrastructure: Preparing progress downloadable report"
        },
        'info': {
            active_url:"static://",   //for both hitpie and hitlist
            objs:"req",
            parser: this.onGetInfo,
            title: "infrastructure: Showing progress info"
        },
        'inputs': {
            make_ds: function(title, name, active, id) {
                var ds = new vjProgressControl_InputsDataSource({name: name, url: "static://", title: title});
                ds.registerDS();
                if (active)
                    ds.makeURL({id:id, count:20}, true);
                return ds;
            },
            make_active_url: function(url, id) {
                return this.getDS["inputs"].makeURL({id:id, count:20});
            },
            title: "Retrieving list of inputs"
        },
        'outputs': {
            make_ds: function(title, name, active, id) {
                var ds = new vjProgressControl_OutputsDataSource({name: name, url: "static://", title: title});
                ds.registerDS();
                if (active)
                    ds.makeURL({id:id, count:20}, true);
                return ds;
            },
            make_active_url: function(url, id) {
                return this.getDS["outputs"].makeURL({id:id, count:20});
            },
            title: "Retrieving list of outputs"
        }
    });


    this.create=function(dvORtab,id)
    {
        if(this.mode=='preview'){
            this.loadedID=id;
            this.formName='';
            var formNode=gAncestorByTag(gObject(dvORtab.obj.name),"form");
            if(formNode)
                this.formName=formNode.attributes['name'].value;

            this.loaded=true;
            this.constructed=false;
            this.current_dvORtab=dvORtab;

            this.dvinfo=this.current_dvORtab.obj.name;

            this.viewers.length=0;


            var whereToJump=this.node ? (this.node.submitter ? this.node.submitter : this.node.svc ) : null;
            var whatText= node.svcTitle ? (node.svcTitle+" for <br/>"+node.name) : (node._brief ? node._brief : "");

            if(!this.hideProgress){ 
                this.addviewer('progress,inputs,outputs,info,progress_download', new vjProgressControl({
                    data: { progress: 'progress',progress_download: 'download', inputs: 'inputs', outputs: 'outputs', progress_info:'info' },
                    prefixHTML: whereToJump  ? ("<a href='?cmd="+whereToJump+"&id="+id+"'><table width='100%' border=0 class='HIVE_section_title'><tr><td width='1' ><img border=0 src='img/eye.gif' width=64 /></td><td width='99%'>"+whatText+"</td></tr></table></a>") : "",
                    formName: this.formName
                }));
            }
            this.addDownloadViewer();

            ++this.viewers.length;

        }
        this.constructed=true;

        var dv=this.current_dvORtab.obj;
        if(!this.hideProgress){
            dv.addTab("progress","process",[this.viewers.progress, this.viewers.inputs, this.viewers.outputs,this.viewers.info,this.viewers.progress_download]).viewtoggles = -1;
        }
        dv.addTab("available downloads","download",[this.viewers.download]);

        dv.render();
        if( this.onPreviewRenderCallback && this.mode == 'preview' ) {
            funcLink(this.onPreviewRenderCallback, this);
        }
        if( this.onFullviewRenderCallback && this.mode != 'preview' ) {
            funcLink(this.onFullviewRenderCallback, this);
        }

        dv.load('rerender');
        if( this.onPreviewLoadCallback && this.mode == 'preview' ) {
            funcLink(this.onPreviewLoadCallback, this);
        }
        if( this.onFullviewLoadCallback && this.mode != 'preview' ) {
            funcLink(this.onFullviewLoadCallback, this);
        }
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};
