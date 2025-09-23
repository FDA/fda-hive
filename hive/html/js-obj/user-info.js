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
javaScriptEngine.include("js/vjHelpView.js");

vjHO.register('user-info').Constructor=function ()
{

    if(this.objCls)return;
    this.resolution=200;

    this.fullview=function(node,dv)
    {
        this.node = node;
        this.mode='fullview';
        this.create(dv,node.id);

    };

    this.preview = function(node,dv)
    {

        this.node = node;

        this.mode='preview';
        this.create(dv,node.id);

    };

    this.typeName="user-info";


    this.objCls="obj-"+this.typeName+Math.random();
    vjObj.register(this.objCls,this);

    this.active_url=new Object();
    this.inactive_url=new Object();
    this.viewersToAdd=new Array();
    this.viewers=new Object();

    this.idNulldsName="default";
    this.viewers.length=0;
    this.urlSet ={
            'dsHelp':{
                active_url:"http:
                objs:"ids",
                header:"id,name,path,value",
                title:"retrieve obj information"
            }

    };

    this.create=function(dvORtab, id, geometry, formName, stickytabs){

        this.load(dvORtab,id, geometry, formName);

    };

    this.load = function(dvORtab, id, geometry,formName)
    {
            return this.realload ( {dvORtab: dvORtab, id: id , formName : formName } , this.node.whichTab) ;


    };







    this.realload = function(parameters, content )
    {

        dvORtab=parameters.dvORtab;
        id=parameters.id;
        formName=parameters.formName;


        this.loadedID=id;
        this.dvs=(typeof(dvORtab)=="string") ? vjDV[dvORtab] : verarr(dvORtab.obj) ;
        if(!this.loadedID|| !this.dvs)
            return;

        if(formName)this.formName=formName;
        this.constructed=false;

        this.viewers.length=0;
        this.viewersToAdd=[];
        this.loaded=true;


        this.formNames='';
        var formNode=gAncestorByTag(gObject(this.dvs[0].name),"form");
        if(formNode)
            this.formName=formNode.attributes['name'].value;


        this.formNames='';
        var formNode=gAncestorByTag(gObject(this.dvs[0].name),"form");
        if(formNode)
            this.formName=formNode.attributes['name'].value;

        this.length=0;


        this.addviewer('viewHelp', new vjHelpView ({
            data:'dsHelp',
            width:this.dvs[0].width,
            icon:'img/user.gif',
            name:'Help',
            userInfoView:true,
            formName:this.formName
        }));


        if(this.onLoad){
            funcLink(this.onLoad, this, this.viewers);
        }

        this.constructed=true;
        if(this.dvs[0].tabs){

            this.dvs[0].addTab("User-Info","user",[this.viewers['viewHelp']]);

        }
        else{
            this.dvs[0].remove();
            this.viewersToAdd=this.dvs[0].add(this.viewersToAdd).viewers;
        }

        this.dvs[0].render();
        if( this.onFullviewRenderCallback && this.mode != 'preview' ) {
            funcLink(this.onFullviewRenderCallback, this);
        }
        if( this.onPreviewRenderCallback && this.mode == 'preview' ) {
            funcLink(this.onPreviewRenderCallback, this);
        }

        this.dvs[0].load('rerender');
        if( this.onFullviewLoadCallback && this.mode != 'preview' ) {
            funcLink(this.onFullviewLoadCallback, this);
        }
        if( this.onPreviewLoadCallback && this.mode == 'preview' ) {
            funcLink(this.onPreviewLoadCallback, this);
        }

        if(this.onRender){
            funcLink(this.onRender, this, this.viewers);
        }
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};
