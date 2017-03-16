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

function vjVisualElement(flList,objCls) {
    // this.zIndex=100;
    this.objCls = objCls?objCls:"vjVisual";
    this.singleOpen = true;
    this.underMouseDiv = null;
    this.relativeToDoc = 1000;
    this.readonlyMode = false;
    this.RVtag="RV-";

    if (!this.flList)
        this.flList = new Array();
    // if(isok( flList) ) this.flList=this.flList.concat(flList);

    vjObj.register(this.objCls, this); // needed to find this object in events
                                        // by object container

    this.mouseOverDiv = function(cont, nme, ison) {
        if (ison)
            this.underMouseDiv = nme;
        else
            this.underMouseDiv = null;
        window.status = this.underMouseDiv;
    };

    this.closeall = function(cont, except, byclick) {
        for ( var iv = 0; iv < this.flList.length; ++iv) {
            var flV = this.flList[iv];
            if (flV.open)
                continue;
            if (flV.name == except)
                continue;
            if (byclick && (flV.name == this.underMouseDiv || (flV.adoptedFloatingLayers && flV.adoptedFloatingLayers.indexOf(this.underMouseDiv)>-1) ) )
                continue;
            this.doWinOP(flV.name, 'close');
        }

    };

    this.getfl = function (name) {
        for ( var iv = 0; iv < this.flList.length; ++iv) {
            var flV = this.flList[iv];
            if(flV.name && flV.name==name){
                return flV;
            }
        }
        return ;
    };

    this.winop = function(cont, nme, whattodo, countForScroll,popUpPosition, callback) {
        // if(this.singleOpen){
        // this.closeall(cont, nme);
        // }
        if (this.underMouseDiv) {
            // if( this.underMouseDiv==nme && whattodo!='toggle')return ;
            // else
            if (this.underMouseDiv != nme)
                this.doWinOP(this.underMouseDiv, 'close');
        }

        this.underMouseDiv = nme;
        this.doWinOP(nme, whattodo, countForScroll);
        // alert("showing "+nme);

        // ++this.zIndex;
        ++gObjZindex;
//        gObject(nme).style.zIndex = gObjZindex;// this.zIndex; !! only the container should increase in its z-index
        var nmeContObj = gObject(nme + "-container");
        nmeContObj.style.zIndex = gObjZindex;// this.zIndex;
        if (popUpPosition && typeof(popUpPosition)=="object"){
            nmeContObj.style.left = popUpPosition.x +"px";
            nmeContObj.style.top = popUpPosition.y + "px";
        }
        
        if(callback) {
            var flv = this.getfl(nme) || this.flVs[nme];
            if(flv && flv.helpcallback) {
                funcLink(flv.helpcallback, this, flv);
            }
        }
    };

    this.doWinOP = function(nme, whattodo, countForScroll) {

        if (whattodo == 'toggle') {
            gObjectSet(nme + "-win-close", '-', '-', '-', 'toggle', '-', '-');
            gObjectSet(nme + "-win-open", '-', '-', '-', 'toggle', '-', '-');
            gObjectSet(nme + "-container", '-', '-', '-', 'toggle', '-', '-');
            if(gObject(nme+ "-container").style.visibility!="hidden"){
                var dv = vjDV[nme + "Viewer"];
                if (dv)
                    dv.refresh();
            }
        } else if (whattodo == 'close') {
            gObjectSet(nme + "-win-close", '-', '-', '-', 'hide', '-', '-');
            gObjectSet(nme + "-win-open", '-', '-', '-', 'show', '-', '-');
            gObjectSet(nme + "-container", '-', '-', '-', 'hide', '-', '-');
        } else if (whattodo == 'open') {
            gObjectSet(nme + "-win-close", '-', '-', '-', 'show', '-', '-');
            gObjectSet(nme + "-win-open", '-', '-', '-', 'hide', '-', '-');
            gObjectSet(nme + "-container",
                    countForScroll ? document.body.scrollLeft + 10 : '-',
                    countForScroll ? document.body.scrollTop + 10 : '-', '-',
                    'show', '-', '-');

            if (!this.flVs[nme]._opened) {
                var dv = vjDV[nme + "Viewer"];
                if (dv) {
                    dv.refresh();
// dv.render(true);
// dv.load();
                    this.flVs[nme]._opened = true;
                }
            }
        }
    };

    this.generate = function(flList, notimmediate, ignorepassive) {
        if (isok(flList))
            this.flList = this.flList.concat(flList);

        if (notimmediate)
            return;

        var icoSize = 16;
        var tAll = "";
        // alert(gPgW + ' '+ gPgH);
        if(!this.flVs)this.flVs=new Object();
        for ( var iv = 0; iv < this.flList.length; ++iv) {
            var flV = this.flList[iv];
            this.flVs[flV.name] = flV;
            if (flV.blocked)
                continue;

            if (this.relativeToDoc) {
                flV.x = parseInt(gPgW * flV.x / this.relativeToDoc);
                flV.y = parseInt(gPgH * flV.y / this.relativeToDoc);
                flV.cx = parseInt(gPgW * flV.cx / this.relativeToDoc);
                flV.cx = parseInt(gPgH * flV.cy / this.relativeToDoc);
            }
            flV.div = gObject(flV.name + "-visual");

            var mover = " onMouseOver='vjObjEvent(\"mouseOverDiv\", \""
                    + this.objCls + "\",\"" + flV.name
                    + "\",1);' onMouseOut='vjObjEvent(\"mouseOverDiv\", \""
                    + this.objCls + "\",\"" + flV.name + "\",0);' ";
            var t = "";

            if (!flV.div)
                t += "<div style='position:"
                        + (flV.position ? flV.position : "absolute") + ";left:"
                        + flV.x + "px;top:" + flV.y + "px;width:" + flV.cx
                        + "px;height:" + flV.cy + "px;visibility:"
                        + (flV.hidden ? "hidden" : "visible") + "' id='"
                        + flV.name + "'  >";
            else
                t += "<div style='position:"
                        + (flV.position ? flV.position : "relative")
                        + ";visibility:" + (flV.hidden ? "hidden" : "visible")
                        + "' id='" + flV.name + "'  >";
            if (flV.onlyPopup != true) {
                t += "<table  "+ (flV.classTag ? flV.classTag : "class='VISUAL_table'" )+" width='"+(flV.width ? flV.width : "100%" )+"' >";
                if (flV.title) {
                    t += "<tr ";
                    if(!flV.noBGImage) {
                        t+="style='display:block;background:url(\""+(flV.bgImage ? flV.bgImage : "img/bg-hexagon.jpg" )+"\");'";
                    }else {
                        t+="style='display:block;background-color : transparent;'";
                    }
                    t+=">";
                    if (!flV.passive || ignorepassive) {
                        if (!flV.noicon) {
                            t += "<th style='border:0;' align=left width=1 valign=top  >";

                            t += "<span id='"
                                    + flV.name
                                    + "-win-close' "
                                    + (flV.open == true ? ""
                                            : "style='visibility:hidden;display:none;'")
                                    + " >";
                            t += "<a href='javascript:vjObjEvent(\"winop\", \""
                                    + this.objCls + "\",\"" + flV.name
                                    + "\",\"close\");'>";
                            t += "<img border=0 src=\"img/winup.gif\" height="
                                    + icoSize + " />";
                            t += "</a>";
                            t += "</span>";

                            t += "<span id='"
                                    + flV.name
                                    + "-win-open' "
                                    + (flV.open == true ? "style='visibility:hidden;display:none;'"
                                            : "") + " >";
                            t += "<a href='javascript:vjObjEvent(\"winop\", \""
                                    + this.objCls + "\",\"" + flV.name
                                    + "\",\"open\");'>";
                            t += "<img border=0 src=\"img/windown.gif\" height="
                                    + icoSize + " />";
                            t += "</a>";

                            t += "</span>";
                            t += "</th>";
                        }
                    }
                    t += "<th style='border:0;' align=left valign=middle bgcolor='transparent' width='99%' >";
                    if (!flV.passive || ignorepassive)
                        t += "<a href='javascript:vjObjEvent(\"winop\", \""
                                + this.objCls + "\",\"" + flV.name
                                + "\",\"toggle\");'>";
                        if(flV.icon) {
                            t+="<table><tr><td width=1 valign=middle width='90%' >";
                            t += "<img border=0 src=\""+flV.icon+"\" height="
                                + (flV.icoSize ? flV.icoSize : icoSize ) + " />";
                            t += "</td><td >";
                        }
                        /*
                        if(flV.rightTitle) {
                            t+="<table border=0><tr><td >";
                        }*/
                        t += flV.title;

                        /*if(flV.rightTitle) {
                            t+="</td><td>";
                             t+=flV.rightTitle;
                            t+="</td></tr></table>";
                        }*/
                        if(flV.icon) {
                            t += "</td></tr></table>";
                        }
                    if (!flV.passive || ignorepassive)
                        t += "</a>";

                    t += "</th>";
                    
                    if (flV.help) {
                        t += "<th  style='border:0;' align=left width=1 valign=top  >" 
                                +"<span id='" + flV.name + "-win-help-close' >"
                                    +"<a href='javascript:vjObjEvent(\"winop\", \"" + this.objCls + "\",\"" + flV.name + "\",\"open\", null,null,true);'>"
                                        +"<img border=0 src=\"img/help.png\" height=" + (icoSize < 30 ? 30 : icoSize) + " />"
                                    +"</a>"
                                +"</span>"
                            +"</th>"
                    }

                    t += "</tr>";
                }

                t += "<tr>";
                t += "<td bgcolor='white' colspan=3 align='"
                        + (flV.align ? flV.align : "left") + "' ";
                if ((!flV.passive || ignorepassive) && flV.clickBriefExpand)
                    t += "onclick='vjObjEvent(\"winop\", \"" + this.objCls
                            + "\",\"" + flV.name + "\",\"toggle\");'";
                t += " >";
                if (isok(flV.brief)) {
                    t += "<span id='" + flV.name + "-brief' >";
                    t += flV.brief;
                    //t += "<small>";
                }
                else {
                    t += "<span id='" + flV.name + "BriefViewer' >";
                    t += "</span>";
                }
                if (isok(flV.briefSpans)) {
                    t += "<span id='" + flV.name + "-briefspans' >";
                    var rvs=verarr(flV.briefSpans);
                    for( var irv=0; irv<rvs.length; ++irv )
                        t+="<span id='"+(flV.briefSpanTag ? flV.briefSpanTag : "" ) + rvs[irv]+ "'></span>";
                    t += "</span>";
                }

                t += "</td>";
                t += "</tr>";

                /*
                 * t+="<tr>"; t+="<td bgcolor='white' colspan=3 >"; t+="<span
                 * id='"+flV.name+"-container' "+ ( flV.open==true ? "" :
                 * "style='visibility:hidden;display:none;'" ) + " >"; t+="<span
                 * id='"+flV.name+"Viewer' >"; t+="</span>"; t+="</span>";
                 * t+="</td>";
                 *
                 * t+="</tr>";
                 */
                t += "</table>";
                if(flV.afterBriefHTML)
                    t+=flV.afterBriefHTML;
            }


            var to="";
            var oo=gObject(flV.name+"-container");
            if(!oo)t += "<div class='VISUAL_popup' style='position:"
                    + (flV.open ? (flV.position ? flV.position : "relative")
                            : (flV.position ? flV.position : "absolute"))
                    + ";visibility:" + (flV.open ? "visible" : "hidden")
                    + "' id='" + flV.name + "-container' " + mover + " >";
            to += "<table bgcolor='white' width='100%'>";
            if (flV.popupCloser) {
                to += "<tr><td align=right valign=middle>";
                to += "<a href='javascript:vjObjEvent(\"winop\", \""
                        + this.objCls + "\",\"" + flV.name + "\",\"close\");'>";
                to += "<small>close</small><img src='img/delete.gif' height=16 border=0 />";
                to += "</a>";
                to += "</td></tr>";
            }

            to += "<tr><td>";
            if (isok(flV.body)) {
                to += flV.body;
            } else {
                to += "<span id='" + flV.name + "Viewer' >";
                to += "</span>";
            }
            to += "</td></tr></table>";

            if(oo)
                oo.innerHTML=to;
            else t+=to;
            if(!oo)t += "</div>";
            t += "</div>";

            // alert(flV.name);
            if (flV.div)
                flV.div.innerHTML = t;
            else
                tAll += t;

        }

        var rr = "vjObjEvent(\"closeall\", \"" + this.objCls + "\",\"-\",1);";
        if (gOnDocumentClickCallbacks.indexOf(rr) == -1)
            gOnDocumentClickCallbacks += rr;

        // if(dowrite) {
        // document.write(t);
        // gFloatingElements+=t;
        // gCreateFloatingElements(t);
        // }
        // else
        this.closeall();
        if(flList)
            this.flList=new Array();
        return tAll;
    };

    if (isok(flList))
        this.generate(flList, false);// =this.flList.concat(flList);

}

var vjVIS = new vjVisualElement();

//# sourceURL = getBaseUrl() + "/js/vjVisualElements.js"