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
// _/_/_/_/_/_/_/_/_/_/_/
// _/
// _/ Global Viewer locator
// _/
// _/_/_/_/_/_/_/_/_/_/_/

function vjObjGlobalMapper()
{
    this.find=function(container)
    {
        return this[container];
    };
    this.register=function(container, obj)
    {
        this[container]=obj;
    };
    this.unregister=function(container){
        delete this[container];
    };
}
var vjObj=new vjObjGlobalMapper();

function vjObjDelayedEvent (milliseconds){
    var args=Array.prototype.splice.call(arguments, 1);
    setTimeout(function(){vjObjEvent.apply(null,args);},milliseconds);
}

function vjObjDetailedEvent(event,eventValidationObj,funcname,container){
      var obj=eventValidationObj.container?vjObj[eventValidationObj.container]:undefined;
      var args = new Array();
      var e=event?event:window.event;
      args.push(e);
      if(eventValidationObj.args){
          for(var i = 0; i < eventValidationObj.args.length; i++) {
                args.push(arguments[i]);
          }
      }


      var res=0;
      if(obj && obj[eventValidationObj.func])
        res=obj[eventValidationObj.func].apply(obj, args);
      else
        res=eval(eventValidationObj.func).apply(obj, args);
      if(!res)
          return res;

      var arglist = new Array();
      for(var i = 2; i < arguments.length; i++) {
        arglist.push(arguments[i]);
      }
      return vjObjEvent.apply(null,arglist);
}

function vjObjEvent(funcname, container)
{
  //window.status = 'f:'+funcname +"->" + container;
  var obj=vjObj[container];
  //alert(funcname + " - " + container + "---"+obj);
  if(!obj)return;

  var arglist = new Array();
  for(var i = 1; i < arguments.length; i++) {
    arglist.push(arguments[i]);
  }

  var res=0;
  if(obj[funcname])
    res=obj[funcname].apply(obj, arglist);
  else
    res=eval(funcname).apply(obj, arglist);
  
  if(res) {
      stopEventPropagation();
      //if(event.stopPropagation){event.stopPropagation();}
      //event.cancelBubble=true;
  }
  return res;
}


function vjObjFuncDummy()
{
    return 0;
}

function vjObjFunc( funcname, thisscontainer )
{
    var object=vjObj[thisscontainer];
    return {obj:object, func: (!object)  ? vjObjFuncDummy : object[funcname] };
}

function vjObjAjaxCallback(params, body)
{

  var obj=vjObj[params.objCls];
  if(!obj)return;

  var res=0;
  if(obj[params.callback])
    res=obj[params.callback].apply(obj, new Array (params, body ) );
  else
    res=eval(params.callback).apply(obj, new Array (params, body ) );
  return res;
}

function vjObjEventGlobalFuncName( funcname, thisscontainer )
{
    return funcNameComposed="__"+funcname+"__"+thisscontainer+"__";
}
function vjObjEventGlobalFuncBody( funcname, thisscontainer , timeout )
{

    var t="function "+vjObjEventGlobalFuncName( funcname, thisscontainer ) +" () {var a=arguments.callee.toString().split('__');";
    if(timeout)t+="setTimeout('vjObjEvent(\"'+a[1]+'\",\"'+a[2]+'\")',"+timeout+")";
    else t+="vjObjEvent(a[1],a[2]);";
    t+=";return}";

    return t;
}

//# sourceURL = getBaseUrl() + "/js/vjObj.js"