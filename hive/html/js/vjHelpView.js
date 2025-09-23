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

function vjHelpView ( viewer )
{

    if(!this.icon)this.icon="img/help.png";
    this.history = new Array();
    this.ifFirstComeHere = 1;
    this.historyCurrent = -1;
    this.iconSize=64;
    this.navigationWidth=20;
    this.selfReload=false;
    if(!this.styleBase)this.styleBase="HELP";
    if(!this.tblClass) this.tblClass="TABLEVIEW_table";


    vjDataViewViewer.call(this,viewer);



    this.composerFunction=function( viewer , content)
    {
        if (this.ifFirstComeHere == 1) {
            this.history.push(this.getData(0).url);
            this.historyCurrent++;
        }
        else if(!this.selfReload ){
            this.history.splice(this.historyCurrent + 1, this.history.length - this.historyCurrent+ 1);
            this.history.push(this.getData(0).url);
            this.historyCurrent++;
            this.selfReload=false;
        }
        this.ifFirstComeHere = 0;
        this.content=content;
        this.refresh();
    };


    this.refresh = function ()
    {
        this.div.innerHTML=this.composeText(this.content);
        var divParent = $(this.div).parent();
        divParent.scrollTop(0);
    };



    this.composeText=function(content)
    {
        if (!content) return;
        var element;
        if(this.userInfoView){
            this.elemArr = new vjTable(content, 0, vjTable_hasHeader);
            element = {
                    body:"",
                    title:"",
                    name:"",
                    contact_email:"",
                    contact_phone:"",
                    created:"",
                    display_name:"",
                    education_entry_degree:[],
                    education_entry_institution:[],
                    education_entry_program:[],
                    experience:"",
                    occupation_employer:"",
                    occupation_location:"",
                    occupation_role:""
            }
            var educationArr = ["education_entry_degree","education_entry_institution","education_entry_program"];
            for(var iv = 1;iv<this.elemArr.rows.length;++iv){
                var row = this.elemArr.rows[iv];
                if(educationArr.indexOf(row.name)==-1)
                    element[row.name]=row.value ;
                else element[row.name].push(row.value);
            }
        }
        else{
            element = [
                 { title: "id", content: "" },
                 { title: "title", content: "" },
                 { title: "definition", content: "" },
                 { title: "description", content: "" }
            ];
            for (var i = 0; i < element.length; i++) {
                var idxfrom = content.indexOf('<'+ element[i].title+'>')+2+element[i].title.length;
                var idxto = content.lastIndexOf('</'+element[i].title+'>');
                element[i].content = content.substring(idxfrom, idxto);
            }
        }


        var definition;
        var title;
        var contentP;
        if(!this.userInfoView){
            contentP= element[3].content;
            definition = element[2].content;
            title = element[1].content;
        }
        else{
            contentP = element["body"];
            definition = element["display_name"];
            title = element["title"];
        }



        while (contentP.indexOf("<a href=\"hlp")!=-1) {
            var urlIndexStart = contentP.indexOf("<a href=\"hlp") + 8;
            var urlIndexEnd = contentP.indexOf(".html\">",urlIndexStart) + 6;
            var urlContent = contentP.substring(urlIndexStart, urlIndexEnd);
            contentP = contentP.substr(0, urlIndexStart) + contentP.substr(urlIndexStart).replace(urlContent, "'javascript:vjObjEvent(\"refreshContent\",\"" + this.objCls + "\"," + urlContent + ")'");
        }


        while (contentP.indexOf("<a href=\"select")!=-1) {
            var urlIndexStart = contentP.indexOf("<a href=\"select") + 8;

            var urlIndexEnd = contentP.indexOf("\">",urlIndexStart) + 1;
            var urlContent = contentP.substring(urlIndexStart, urlIndexEnd);

            contentP = contentP.substr(0, urlIndexStart) + contentP.substr(urlIndexStart).replace(urlContent, "'javascript:vjObjEvent(\"refreshContent\",\"" + this.objCls + "\"," + urlContent + ")'");
        }

        var t = "";

        t += "<table border=0 >";

            t+="<tr>";
                t+="<td width='1' valign=top >";
                    t+="<table border=0 width='1'><tr><td colspan=2>";
                    if(this.icon)t+="<img border='0' src='"+this.icon+"' width='"+this.iconSize+"' />";
                    t+="</td></tr><tr><td width='"+this.navigationWidth+"px' >";

                    if (this.historyCurrent > 0) {
                        t += "<button class='linker' type='button' onclick='javascript:vjObjEvent(\"moveBack\",\"" + this.objCls + "\",\" \"); stopDefault(event);'><img border='0' src='img/left.gif' width='"+this.navigationWidth+"'/></button>";
                    } else {
                        t += "<img border='0' src='img/white.gif' width='"+this.navigationWidth+"'/>";
                    }
                    t+="</td><td width='"+this.navigationWidth+"' >";
                    if (this.history.length-this.historyCurrent > 1) {
                        t += "<button class='linker' type='button' onclick='javascript:vjObjEvent(\"moveForward\",\"" + this.objCls + "\",\" \"); stopDefault(event);'><img border='0' src='img/right.gif' width='"+this.navigationWidth+"'/></button>";
                    } else {
                        t += "<img border='0' src='img/white.gif' width='"+this.navigationWidth+"'/>";
                    }
                    t+="</td></tr></table>";
                t+="</td>";
                t+="<td valign=top  colspan=2 class='"+this.styleBase+"_title'  >";
                    t += "<h2>" + title + "</h2>";
                    t += "<p class='"+this.styleBase+"_definition'>"+ definition+"</p>" ;
                t+="</td>";
            t += "</tr>";

            if(this.userInfoView){
                var excludeArray = ["title","body","display_name","created","education_entry_degree","education_entry_institution","education_entry_program"];
                for(var item in element){
                    if(excludeArray.indexOf(item)==-1){
                        t+="<tr >";
                            t += "<td valign=left class='"+this.tblClass+"' colspan=1 >";
                                t += item;
                            t+="</td>";
                            t += "<td valign=left class='"+this.tblClass+"' colspan=2  >";
                            t += element[item];
                            t+="</td>";
                        t+="</tr>";
                    }
                }

                if(element["education_entry_degree"].length){
                    t += "<tr><td colspan = '2'><table border=0  class ='"+this.tblClass+"' >";
                    t += "  <tr>"
                        +"   <th>education_degree</th>"
                        +"     <th>education_institution</th>"
                        +"     <th>education_program</th>"
                        +"  </tr>"
                }
                for(var ir = 0;ir<element["education_entry_degree"].length;ir++){
                    t += "<tr><td>"+element["education_entry_degree"][ir]+"</td>";
                    t +="<td> "+ ((element["education_entry_institution"].length > ir) ? element["education_entry_institution"][ir]: "N/A") +" </td>";
                    t +="<td> "+ ((element["education_entry_program"].length > ir) ? element["education_entry_program"][ir]:"N/A") + " </td>";
                    t +=" </tr>";

                }
                t+= "</table></td></tr>"

            }



            t+="<tr >";
            t += "<td valign=top class='"+this.styleBase+"_body' colspan=3 ";
            t+= ">";

                t+=contentP;
                t+="</td>";
            t+="</tr>";


        t+="</table>";

        return t;
    };

    this.moveBack=function()
    {
        if (this.historyCurrent == 0) {
        } else {
            this.historyCurrent--;
            this.reloadhistoryCurrent();
        }
    };

    this.moveForward = function()
    {
        if (this.history.length-this.historyCurrent <= 1){
        } else {
            ++this.historyCurrent;
            this.reloadhistoryCurrent();
        }
    };

    this.reloadhistoryCurrent = function ()
    {
        this.selfReload=true;
        this.getData(0).reload(this.history[this.historyCurrent], true);
    };

    this.refreshContent = function (objID, urlContent)
    {
        if(urlContent.indexOf("select:")==0){
            this.dataViewEngine.select(urlContent.substring(7),true);
            return ;
        }
        urlContent = "http:
        this.history.splice(this.historyCurrent + 1, this.history.length - this.historyCurrent+ 1);
        this.history.push(urlContent);
        this.historyCurrent++;
        this.selfReload=true;
        this.getData(0).reload(urlContent, true);

    };

}

