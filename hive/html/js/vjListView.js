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

function vjListView ( viewer )
{
    vjDataViewViewer.call(this,viewer);



    this.composerFunction=function( viewer, content  )
    {
        var jsonObj=eval ('('+content+')');

        if(this.info) jsonObj.info=this.info;

        var elemID=this.container;
        var checkbox=jsonObj.info ? jsonObj.info.checkbox : 0;
        var t="";

        if( jsonObj.info ) {
            t+="<table width='100%' border='0'>"
                + "<tr>"
                + " <td width='40%'>" + this.generateObjectFilter(elemID) + "</td>"
                + " <td width='20%' class='RowRegular'>"+this.generateObjectActions(jsonObj.objectActions, elemID)+"</td>"
                + " <td width='20%'>"+this.generateGroupActions(jsonObj.objectActions, elemID, jsonObj.messages)+"</td>"
                + "</tr>"
                + "</table>";

        }
        t+="<table width='100%'>";

            t+=this.generateHeaderText( checkbox,jsonObj.fields,elemID);


            for( var ir=0; ir<jsonObj.records.length ; ++ir) {
                t+=this.generateRowText(checkbox,jsonObj.records[ir], jsonObj.fields,elemID,jsonObj.objectActions,ir,jsonObj.info);
            }

        t+="</table>";
        var myProcessedContent=t;

        this.div.innerHTML=myProcessedContent;

    };






    this.generateHeaderText=function( checkbox,fields,objectID)
    {
        var t="";
        t+="<tr id='"+objectID+"_header' >";

        if (checkbox==1){
            t+="<td "
                + " class='"+fields[0].fclass+"'"
                + " align='center' "
                + ">"
                + "<input type=" + '"' + "checkbox" + '"'
                + " id=" + '"' + "checkAll_" + objectID + '"'
                + " name=" + '"' + "checkAll_" + objectID + '"'
                + " onclick=" + '"' + "vjObjEvent('onSelectObjectCheckbox','"+this.objCls+"','" + objectID +"')" + '"'
                + ">"
                + "</td>";
            }
          t+="<td "
            + " class='"+fields[0].fclass+"'"
              + " align='center' "
              + "> ID </td>";

        for ( var ic=0; ic<fields.length; ++ ic){
            var fld=fields[ic];
            var cellWD=this.getVarValue(fld,fld.tag,"width");

            t+="<td ";
            t+=" id='"+fld.id+"'";
            t+=" class="+fld.fclass+" ";
            if(cellWD)t+=" width="+cellWD + (this.widthInPixels ? "" : "%");
            t+=" align="+fld.align;
            t+=" title="+fld.tip;
            t+=">"+fld.name+"</td>";

        }
        t+="</tr>";

        return t;

    };
    this.generateRowText=function(checkbox,record,fields,objectID,objectActions,rowSequence,info)
    {
        var t="";
        var cellBG = "";
        var startid=info ? eval(info.startid) : 0;
        if (startid){
            startid = startid + rowSequence;
        }
        else{
            startid=rowSequence;
        }

        if (this.colorbg && this.colorbg.length){
              cellBG = " bgcolor='" + this.colorbg[rowSequence%this.colorbg.length] + "'";
          }

        t+="<tr id='"+objectID+"_"+record.id+"' >";

        if (checkbox==1){
            var allowedActions= "";

            for ( var f=0; f< fields.length; ++f )  {
                if (fields[f].type=='actions'){
                   var tag=fields[f].tag;
                   allowedActions =eval("record."+tag);
                   break;
                }
            }

            t+="<td align='center'><input "
                + " type=" + '"' + "checkbox" + '"'
                + " id=" + '"' + objectID + "_c_" + record.id + '"'
                + " name=" + '"' + objectID + "_c_" + record.id + '"'
                + " onclick=" + '"' + "this.doSelectRecord('" + objectID + "','" + record.id + "',this.checked)" + '"'
                + " att_allowedActions=" + '"' + allowedActions + '"';
                + "></td>";
        }

            t+="<td align='center' class='RowRegular' " + cellBG  +">" + startid + "</td>";


        for ( var f=0; f< fields.length; ++f )  {

           var tag=fields[f].tag;
           var fType = fields[f].type;
           var val=eval("record."+tag);

           t+= this.getCellContent(fields[f],f,val,record,objectID,fType,objectActions,rowSequence);


        }

        t+="</tr>";
        return t;
    };

    this.generateObjectActions=function(objectActions, objectID)
    {
        var t="";
        var val_2show = "";
        if (objectActions.length==0){
            t=val_2show;
        }
        else {
        for (var oa=0; oa<objectActions.length; ++oa){
                       val_2show = val_2show + "&nbsp;&nbsp;&nbsp;" + "<a href="
                           + '"'
                           + "hive.cgi"
                           + objectActions[oa].url
                           + '"'
                           + " target='_blank' "
                           + ">"
                           + objectActions[oa].name
                           +"</a>";
               }

           t+=val_2show;
        }
           return t;
    };
    this.generateGroupActions=function(actions, objectID, messages){

        var t;

        if (actions.length==0){
            t="&nbsp;";
        }
        else {
            t="<select name=" + '"' + "ga" + "_" + objectID + '"'
                + " id=" + '"' + "ga" + "_" + objectID + '"'
                + " class='TextField'"
                + " onchange=" + '"' + "vjObjEvent('onSelectGroupAction','"+this.objCls+"',this)" + '"'
                + ">"
                + "<option value='' >Select</option>";

           for (var ga=0; ga<actions.length; ++ga){
               t+="<option value=" + '"' + actions[ga].id + "|" + actions[ga].url + '"'
                   + ">"
                   + actions[ga].name
                   + "</option>";
           }

           t+="</select>";

           t+="&nbsp;<input type=" + '"' + "button" + '"'
                   + " name=" + '"' + "btn_Submit_" + objectID + '"'
                   + " value='Submit' "
                   + " class=" + '"' + "TextField" + '"'
                   + " onclick=" + '"' + "vjObjEvent('onSubmitGroupAction','"+this.objCls+"','" + objectID +"')" +'"'
                   + ">";
        }
        return t;
    };


    this.generateObjectFilter=function(objectID){
            var t="";
            t+="<input type=" + '"' + "text" + '"'
            + " name=" + '"'+ "search_" + objectID+ '"'
            + " id=" + '"'+ "search_" + objectID+ '"'
            + ">"
            + "&nbsp;"
            + "&nbsp;<input type=" + '"' + "button" + '"'
            + " name=" + '"' + "btn_Search_" + objectID + '"'
            + " class=" + '"' + "TextField" + '"'
            + " onclick=" + '"' + "vjObjEvent('onSearch','"+this.objCls+"','" + objectID +"')" +'"'
            + ">";
            return t;
    };



    this.doSelectRecord=function(objectID,recordID,checkedFG){
        var rowID = objectID + "_" + recordID;
        if (checkedFG==true){
            document.getElementById(rowID).bgColor='#CCCCCC';
        }
        else{
            document.getElementById(rowID).bgColor='#FFFFFF';
        }
    };

    this.doCheckSelection=function(objectID){
        var checkboxName = objectID + "_c_";
        var all = document.getElementsByTagName("*");
        var selectionFound = 0;

        for (var i = 0; i<all.length; i++){
            var elementName = all[i].name;
            if (elementName){
                    if (elementName.indexOf(checkboxName)!=-1){
                        if (all[i].checked==true){
                            selectionFound = 1;
                            break;
                        }
                    }
            }
        }
        return selectionFound;
    };



    this.onSelectGroupAction=function(container,groupActionField)
    {
        var objectID = groupActionField.name.replace("ga_","");
        var groupActionFull = groupActionField.value;
        var groupAction = groupActionFull.substring(0,groupActionFull.indexOf("|"));

        var checkboxName = objectID + "_c_";
        var all = document.getElementsByTagName("*");

        for (var i = 0; i<all.length; i++){
            var elementName = all[i].name;
            if (elementName){
                    if (elementName.indexOf(checkboxName)!=-1){
                        var checkboxAllowedActions = all[i].getAttribute("att_allowedActions");
                        var recordID = elementName.replace(checkboxName,"");
                        if (checkboxAllowedActions.indexOf(groupAction)==-1 && groupAction!=""){
                            this.doSelectRecord(objectID,recordID,false);
                            all[i].checked=false;
                            all[i].disabled=true;
                        }
                        else if (groupAction==""){
                            all[i].disabled=false;
                        }
                    }
            }
        }

    };
    this.onSubmitGroupAction=function(container,objectID){
        var groupActionField = "ga_" + objectID;
        var selectedGroupActionFull = document.getElementById(groupActionField).value;
        var selectedGroupActionURL = selectedGroupActionFull.substring(selectedGroupActionFull.indexOf("|")+1);

        if (selectedGroupActionFull==""){
            alert("Please select a group action you want to perform.");
            document.getElementById(groupActionField).focus();
        }
        else{
            var selectionFound = this.doCheckSelection(objectID);
            if (selectionFound==0){
                alert("Please select at least one object to perform a group action.");
                document.getElementById(groupActionField).focus();
            }
            else{
                var url_2use = selectedGroupActionURL + this.doSelectRecords(objectID);
                alert(url_2use);
            }
        }
    };

    this.onSearch=function(container,objectID){
        var searchFieldName = "search_" + objectID;
        var searchString = document.getElementById(searchFieldName).value;
        if (searchString==""){
            alert("Please enter a search criteria.");
            document.getElementById(searchFieldName).focus();
        }
        else {
            alert(searchString);
        }
    };

    this.onFilterByCheckbox=function(container,objectID,Filter_ID,Value_ID,Selection_FG){
        alert(objectID + ' ' + Filter_ID + ' ' + Value_ID + ' ' + Selection_FG);
    };

    this.onSelectObjectCheckbox=function(container,objectID){
        var checkboxName = objectID + "_c_";
        var all = document.getElementsByTagName("*");
        var checkAll_checkbox = "checkAll_"+objectID;
        var Selection_FG = document.getElementById(checkAll_checkbox).checked;
        for (var i = 0; i<all.length; i++){
            var elementName = all[i].name;
            if (elementName){
                    if (elementName.indexOf(checkboxName)!=-1){
                        var recordID = elementName.replace(checkboxName,"");
                        if (all[i].disabled==false){
                            all[i].checked = Selection_FG;
                            this.doSelectRecord(objectID,recordID,Selection_FG);
                        }
                    }
            }
        }
    };

    this.getVarValue=function(field, tname, nameparam)
    {
        var valu;

        var formTag=eval ("this."+tname );
        if(formTag)
            valu = eval ("formTag."+nameparam );
        if(valu) return valu;

        var fieldTag=eval( "field."+tname );
        if(fieldTag)
            valu=eval ("fieldTag."+nameparam );
        return valu;
    };




    this.getCellContent=function(field, fieldSequence,fieldValue,record,objectID,fieldType,objectActions,rowSequence){
        var val2return = "";
        var val_2show = "";
        var cellBG = "";

      if (fieldValue){
        if (fieldType=='actions'){
               var action_array = fieldValue.split(",");
               for (var a=0; a<action_array.length; ++a){
                   for (var r=0; r<objectActions.length; ++r){
                       if (objectActions[r].id == action_array[a]){
                           val_2show = val_2show + "&nbsp;&nbsp;&nbsp;" + "<a href="
                                               + '"'
                                               + "hive.cgi"
                                               + objectActions[r].url
                                               + record.id
                                               + '"'
                                               + " target='_blank' "
                                               + ">"
                                               + objectActions[r].name
                                               +"</a>";
                       }
                   }
               }
          }
        else{
            val_2show = fieldValue;
        }
        var lnkl=this.getVarValue(field, field.tag, "link");
        if( lnkl && fieldType!='actions'){
            if(lnkl!=''){
                val_2show = "<a href="
                   + '"'
                   + "javascript:"
                   + lnkl
                   + "('" + objectID + "','" + record.id + "')"
                   + '"'
                   + ">"
                   + val_2show
                   +"</a>";
            }
        }
      }
      else{
          val_2show = "&nbsp;";
      }

      if (this.colorbg && this.colorbg.length){
          cellBG = " bgcolor='" + this.colorbg[rowSequence%this.colorbg.length] + "'";
      }


        val2return = "<td id='"+objectID+"_"+record.id+"_"+ fieldSequence +"' "
                     + " class='" + field.dataClass + "'"
                     + " align='" + field.align + "'"
                     + cellBG
                     + ">"
                         + val_2show
                         + "</td>";

        return val2return;
    };

    this.onSubmitFilter=function(container,filterButton){
        alert(filterButton.name);
    };

    this.onRemoveFilter=function(container,filterButton){
        alert(filterButton.name);
    };

}












