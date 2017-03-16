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
function default_object_ui(inputList,proctype,qpservice,links,linkedProcList,visiProcParameters,defaultProcParameters,procObject,qpsvcObject,qpServiceDefaultParams,visiProcParametersRaw,visualElements,algorithmicParameterIDPrefix){

    if(inputList)this.inputList                    = inputList;
    else this.inputList= new Array();

    if(proctype)this.proctype                    = proctype;
    else this.proctype = '';

    if(qpservice)this.qpservice                    = qpservice;
    else this.qpservice = '';

    if(links)this.links                            = links;
    else this.links = new Array();

    if(visualElements)this.visualElements        = visualElements;
    else this.visualElements = new Array();

    if(linkedProcList)this.linkedProcList        = linkedProcList;
    else this.linkedProcList = new Array();

    if(visiProcParameters)this.visibleProcParameters    = visiProcParameters;
    else this.visibleProcParameters= new Array();

    if(visiProcParametersRaw)this.visiProcParametersRaw    = visiProcParametersRaw;
    else this.visiProcParametersRaw= '';

    if(defaultProcParameters)this.defaultProcParameters    = defaultProcParameters;
    else this.defaultProcParameters = new Array();

    if(algorithmicParameterIDPrefix)this.algorithmicParameterIDPrefix = algorithmicParameterIDPrefix;
    else this.algorithmicParameterIDPrefix = "RV-"; /* Default prefix for RecordViewer's ids */

    if(qpServiceDefaultParams)this.qpServiceDefaultParams    = qpServiceDefaultParams;
    else this.qpServiceDefaultParams = new Array();

    if(procObject)this.procObject    = procObject;
//    else this.procObject ;

    if(qpsvcObject)this.qpsvcObject    = qpsvcObject;
//    else this.qpsvcObject =null;


    this.setInput            = function(input){

        if(input)this.inputList=input;
        return this.inputList;
    };


    this.setProctype        = function(proc){
        if(proc)this.proctype=proc;
        return this.proctype;
    };


    this.setQpservice        = function(qpsrv){
        if(qpsrv)this.qpservice=qpsrv;
        return this.setQpservice;
    };


    this.setLinks            = function(Links)
    {
        if(Links)this.links=Links;
        return this.links;
    };

    this.setVisualElements    = function(visualElements)
    {
        if(visualElements)this.visualElements=visualElements;
        return this.visualElements;
    };

    this.setLinkedProcs    = function(linkedProc){
        if(linkedProc)this.linkedProcList    =linkedProc;
        return this.linkedProcList;
    };


    this.setProcObject            = function(procObject){

        if(procObject)this.procObject=procObject;
        return this.procObject;
    };

    this.qpsvcObject            = function(qpsvcObject){

        if(qpsvcObject)this.qpsvcObject=qpsvcObject;
        return this.qpsvcObject;
    };


    this.setDefaultProcParameters=function(arr,prefix,extend)
    {
        if(!isok(arr)){
            if(!isok(this.defaultProcParameters)){
//                alert("default proc parameters array is empty;")
                return 0;
            }
            else
                arr=this.defaultProcParameters;
        }

        if(this.procObject===undefined){
//            alert("undefined proc Object");
            return 0;
        }
        if(prefix)
            this.algorithmicParameterIDPrefix=prefix;

        for (var i=0; i<arr.length-1; i+=2) {
            this.procObject.setValue(arr[i],arr[i+1]);
        }
    };

    this.setVisibleProcParameters=function(arr,prefix,extend)
    {
        if(!isok(arr)){
            if(!isok(this.visibleProcParameters)){
//                alert("visible proc parameters array is empty;")
                return 0;
            }
            else
                arr=this.visibleProcParameters;
        }

        if(!extend)
            this.visiProcParametersRaw = "";


        for (var i=0; i<arr.length; i++) {
            this.visiProcParametersRaw += "<span id='"+(isok(this.algorithmicParameterIDPrefix)?this.algorithmicParameterIDPrefix:'') + arr[i]+ "'></span>";
        }
        return this.visiProcParametersRaw;

    };


    this.setQpServiceDefaultParams=function(arr,prefix,extend)
    {
        if(!isok(arr)){
            if(!isok(this.qpServiceDefaultParams)){
//                alert("default qpService Parameters array is empty;")
                return 0;
            }
            else
                arr=this.qpServiceDefaultParams;
        }

        if(this.qpsvcObject===undefined){
            alert("undefined qpsrv Object");
            return 0;
        }
        for (var i=0; i<arr.length-1; i+=2) {
            this.qpsvcObject.setValue(arr[i],arr[i+1]);
        }
        return arr;
    };



    this.customize_before_submission=function(){

    };

    this.customize_after_computing=function(){

    };


    this.initBefore= function(){
        this.init(false);
    };

    this.initDone= function(){
        this.init(true);
    };

    this.init= function(done){
        this.setVisibleProcParameters();
        if(!done){
            this.setDefaultProcParameters();
            this.setQpServiceDefaultParams();
        }
        this.setLinkedProcs();
        this.setVisualElements();
        this.setLinks();
        this.setQpservice();
        this.setProctype();
        this.setInput();
        if(this.initCallBack!==undefined)this.initCallBack();
    };

    this.setSubjectHelpSize = function(w, h) {
        if (!w) w = 500;
        if (!h) h = 500;
        this.subject_help_size = [w, h];
    };

    this.addSubjectHelp = function(divname) {
        if (!this.subject_help_size) this.setSubjectHelpSize();
        if (this.subject_help && this.subject_help.length > 0) {
            vjDV.add(divname, this.subject_help_size[0], this.subject_help_size[1]);
            for (var i=0; i<this.subject_help.length; i++) {
                var h = this.subject_help[i];
                var name = h.name ? h.name : "help";
                var icon = h.icon ? h.icon : "help";
                var description = h.description ? h.description : "infrastructure: Help document for this particular page";
                var dsname = "dsHexagonSubjectHelp_tab_" + i;
                var url = h.url;
                vjDS.add(description, dsname, url);
                vjDV[divname].add(name, icon, "tab", [ new vjHelpView ( { data: dsname} ) ] );
            }
            vjDV[divname].render();
            vjDV[divname].load();
        }
    };

    this.styleBackground = function(domname, varname) {
        if (!this[varname])
            return;
        var d = gObject(domname);
        if (!d)
            return;

        if (this[varname].background) {
            d.style["background-image"] = "url('" + this[varname].background + "')";
            d.style["background-repeat"] = "no-repeat";
        }
        if (this[varname].backgroundColor) {
            d.style["background-color"] = this[varname].backgroundColor;
        }
        if(this[varname].backgroundSize) {
            d.style["background-size"]=this[varname].backgroundSize;
        }
    };
}
