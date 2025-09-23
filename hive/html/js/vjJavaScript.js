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
function vjJavaScript(jsobj, code, obj, dataSourceEngine)
{
    vjDataViewViewer.call(this, jsobj);

    this.vjType='vjJavaScript';
    this.container='no_container';
    this.allowPartialLoad =true;
    this.dataSourceEngine=dataSourceEngine;
    if(!this.dataSourceEngine)return;

    this.evalCode=verarr(this.execCode);
    this.data=verarr(this.data);
    this.obj=obj;

    this.addCode=function(code)
    {
        code = verarr(code);
        if (!code.length) return;

        var seen = {};
        for (var is=0; is<this.code.length; is++)
            seen[this.code[is]] = true;

        for (var is=0; is<code.length; ++is ) {
            var c = code[is];

            if (c.indexOf("javascript:")==0) {
                if (seen[c]) continue;
                this.evalCode.push(c);
            } else {
                if (!this.include(c)) continue;
            }

            this.code.push(c);
            seen[c] = true;
        }
    };
    this.code = verarr(this.code);
    this.addCode(code);

    this.nodiv=true;



    this.composerFunction=function( viewer , text )
    {
        var notdone=0;
        for(var i=0; i<this.data.length; i++) {
            var ds=this.getData(i);
            if(ds && ds.state=='done'){
                if(!ds.executedAlready) {
                    javaScriptEngine=this;
                    eval.call(window,ds.data);
                    delete javaScriptEngine;
                    ds.executedAlready=true;
                }
            }
            else ++notdone;
        }


        if(notdone) {
            return ;
        }

        var veval=verarr(this.evalCode);
        if(!veval.length)return ;
        var obj=this.obj;

        for(var i=0; i<veval.length; i++) {
            javaScriptEngine=this;
            eval(veval[i]);
            delete javaScriptEngine;
        }
        obj;
    };
    this.include=function(jsFileName)
    {
        var jsFileVersioned = jsFileName.replace(/^[^:]*:\/\/(hive\.cgi\?f=)?/, "");
        var jsFileBare = jsFileVersioned;
        if (gSysVersion) {
            jsFileBare = jsFileBare.replace("/^\.\d+\//", "");
            if( jsFileBare == jsFileName ) {
                jsFileVersioned = jsFileBare;
            }
        }
        var jsFileUrl ="http:
        var dsname="dsJavaScript_"+jsFileBare;

        if (this.data.indexOf(dsname)!=-1) {
            if( this.dataSourceEngine[dsname].executedAlready)
                return false;
        } else {
            var scripts = document.scripts;
            for (var isc=0; isc<scripts.length; isc++) {
                var src = scripts[isc].getAttribute("src");
                if (src == jsFileBare || src == jsFileVersioned) return false;
            }

            this.dataSourceEngine.add("infrastructure: loading scripts "+jsFileBare, dsname, jsFileUrl);
            this.data.push(dsname);
            this.register_callback(dsname,this,true);
            this.dataSourceEngine[dsname].load(true);
            return true;
        }
        return false;
    };
}


var vjJS = {};



function vjRegisterJavaScriptEngine(type, obj)
{
    var js = vjJS[type];
    if (js) return js;
    js = new vjJavaScript({}, null, obj, vjDS);
    vjJS[type] = js;
    return js;
}
vjRegisterJavaScriptEngine("undefined");

function vjExecute(type, code, obj)
{
    var js = vjRegisterJavaScriptEngine(type, obj);
    js.addCode(code);
    js.load(true);
}

function vjHiveTypeExecute(type, code, obj, dir)
{
    dir = dir || "js-obj";
    var jscode=dir+"/"+type+".js";
    if(gDebug){
        alert("force prevent caching of dynamically loaded javascript files because of debug mode");
        ts = (new Date()).getTime();
        jscode+="?bust="+ ts;
    }
    var vc=verarr(jscode).concat(verarr(code));
    vjExecute(type, vc, obj);
}

