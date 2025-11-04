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
javaScriptEngine.include("jsim/json_basics.js");
    
    
function vjRecView (viewer) 
{ 

    this.objCls=vjObjRegisterUnique(this);
    
    jsonSetDefaults( this,{
        readonly:docLocValue("readonly")
        ,types:viewer.types ? viewer.types : (docLocValue("types") ? docLocValue("types")  : "base_user_type")
        ,ids:docLocValue("ids")
        ,layoutLocation: viewer.layoutLocation ? viewer.layoutLocation  : "recViewLayout"
        ,layoutSetup: viewer.layoutSetup
        ,separatePictures:viewer.separatePictures
        ,autoLayout: viewer.autoLayout ? viewer.autoLayout  : false 
    }) ;
    
    this.setNewID=function(id,type)
    {
        this.recordViewer.hiveId=id;
        vjDS["dsValues"+this.objCls].reload("http:
        if(type){
            vjDS["dsSpec"+this.objCls].reload("http:
            this.type=type;
        }
        

    }
    
    this.recview_init=function()
    {
        var id=(this.ids.indexOf("-")==0) ? this.ids.substring(1) : this.ids;
            
        vjDS.add("infrastructure: ", "dsSpec"+this.objCls, "http:
        vjDS.add("infrastructure: ", "dsValues"+this.objCls, (id) ? "http:

        
        
        this.recviewControls = new vjHTMLView ({
            data: "dsControls"+this.objCls
            ,formObject: document.forms['formGeneric']
        });
        
        if(this.separatePictures) { 
            this.recviewPictures = new vjHTMLView ({
                data: "dsPictures"+this.objCls
                ,formObject: document.forms['formGeneric']
            });
        }
        
        var hideSections=docLocValue("hs");
        if(!hideSections)
            gObject("dv-"+this.layoutLocation).className='sectHid';
        
        var v={
            data : ["dsSpec"+this.objCls,"dsValues"+this.objCls]
            ,RVtag : 'RView'
            ,autoStatus: 3
            ,hiveId: this.ids
            ,recordEditingCommand: "recview"
            ,callbackBeforeConstruction: hideSections ? null : "function:vjObjFunc('layout_init','"+this.objCls+"')" 
            ,objType:this.types
            ,implementSaveButton: true 
            ,implementCopyButton: true 
            ,reloadObjIDFromData: true
            ,showRoot:false
            ,autoexpand: 4
            ,readonlyMode: this.readonly
            ,formObject: document.forms['form-Generic']
            ,constructAllNonOptionField: true
            ,constructionPropagateDown:10
            ,implementSetStatusButton:true
            ,autoDescription:true
        }
        
        jsonSetDefaults( v,viewer);
        this.recordViewer = new vjRecordView (v);

        vjDS.add("infrastructure: ", "dsControls"+this.objCls, "static:
                +"<span id='"+this.recordViewer.RVtag+"_SPECIAL_BUTTONS'></span>"
                +"</center>");
        
        vjDV.add("dv-"+this.layoutLocation,"1000","1000");
        vjDV["dv-"+this.layoutLocation].add( "infrastructure: ", "table", "tab", this.recordViewer);
        vjDV["dv-"+this.layoutLocation].render();
        vjDV["dv-"+this.layoutLocation].load();
        
        
        

    }
    
    this.layout_init=function()
    {
        if(this.layoutConstructed) 
            return ;
        
        this.layoutConstructed=true;
        
        if(!this.recordViewer.listSections){
            this.recordViewer.listSections= new Array({tag:"root",arr: new Array ("root")  });
        }
        
        var htmlViewers= new Array () ;
        this.recviewSections= new Array ();
                
        this.picSections="";
        for( var ih=0; ih<this.recordViewer.listSections.length; ++ih) {
            var title=ih==0 ? (this.TypeTitle ? this.TypeTitle : this.types)  : this.recordViewer.listSections[ih].tag;
            
            var t="";
            for ( var i=0; i<this.recordViewer.listSections[ih].arr.length; ++i) 
                t+="<span id='"+this.recordViewer.RVtag+"-"+this.recordViewer.listSections[ih].arr[i]+"' ></span><br/>";  
            
            if(this.separatePictures && title=="Picture") {
                this.picSections+=t;
                continue;
            }
            
            vjDS.add("infrastructure:","ds"+ih+this.objCls,"static:
            
            
            htmlViewers [ih] = new vjHTMLView ({
                data : "ds"+ih+this.objCls
                ,formObject: document.forms['formGeneric']
            });
            
            
            
            var con={
                active : ih==0 ? true: false 
                ,title : title  
                ,name : 'list-'+ih
                ,view : {
                    name : "dataview"
                    ,options : {
                        preload: true
                        ,instance: htmlViewers[ih]
                    }
                }
            };


            this.recviewSections.push(con);
            
            
            
        }
        if(this.separatePictures)
            vjDS.add("infrastructure: ", "dsPictures"+this.objCls, "static:
            
        
        if(this.layoutSetup) 
            this.layoutSetup(this);
        

    }
    
    
    
    ajaxDynaRequestPage("?cmdr=objList&prop=title&search=[type:type]&prop_name=name&prop_val="+this.types, {thiSS:this}, setTypeTitle);
    
}    
function setTypeTitle (param, v, a ) 
{
            
        var p= v.indexOf("=");
        if(p!=-1)v=v.substring(p+1);
        
        param.thiSS.TypeTitle=v;
        param.thiSS.recview_init();

}
var recPopupArr = [] ;
var lastZIndex=2000;
function popupOn ( dataId)
{
    if(recPopupArr.length && recPopupArr[recPopupArr.length-1]==dataId ) return ;
    var o=$("[data-id='"+dataId+"']");
    o.find("[title='Maximize']").click();
    if(lastZIndex>=2000)o[0].style["z-index"]=lastZIndex+1;
    lastZIndex=parseInt(o[0].style["z-index"]);
    recPopupArr.push(dataId);
}
function popupOff ( )
{
    if(!recPopupArr.length) return ;
    var dataId=recPopupArr[recPopupArr.length-1];
    $("[data-id='"+dataId+"']").find("[title='Minimize']").click();
    recPopupArr.pop();
    if(recPopupArr.length ){ 
        var o = $("[data-id='"+recPopupArr[recPopupArr.length-1]+"']");
        o.find("[title='Maximize']").click();
    }        
}
document.onkeyup = function(event) {
    if (event.key === "Escape"){popupOff();}
    
    return 1;
    
    
}



