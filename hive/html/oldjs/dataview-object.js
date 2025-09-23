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

vDV.generateObjView=function( form, content  ) 
{

    var jsonObj=objView_listToJSon( content );
    
    if(form.info) jsonObj.info=form.info;

    var elemID=form.container;
    var checkbox=jsonObj.info ? jsonObj.info.checkbox : 0;
    var t="";

    if( jsonObj.info ) {
        t+="<table width='100%' border='0'>"
            + "<tr>"
            + " <td width='40%'>" + objView_generateObjectFilter(elemID, jsonObj.messages) + "</td>"
            + " <td width='20%' class='RowRegular'>"+objView_generateObjectActions(jsonObj.objectActions, elemID)+"</td>"
            + " <td width='20%'>"+objView_generateGroupActions(jsonObj.groupActions, elemID, jsonObj.messages)+"</td>" 
            + "</tr>"
            + "</table>"
            + objView_generateGroupFilters(jsonObj.objectFilters, elemID);  
    }        
    t+="<table width='100%'>";
         
        t+=objView_generateHeaderText(form, checkbox,jsonObj.fields,elemID);
            
            
        for( var ir=0; ir<jsonObj.records.length ; ++ir) { 
            t+=objView_generateRowText(checkbox,jsonObj.records[ir], jsonObj.fields,elemID,jsonObj.recordActions,form,ir,jsonObj.info);
        }

    t+="</table>";
    var myProcessedContent=t;

    document.getElementById(form.container).innerHTML=myProcessedContent;
             
}




function objView_listToJSon( content )
{
    if(content.substr(0,10).indexOf("{")!=-1 ) 
        return eval ('('+content+')');
    
    var rows=content.split("\n");
    
    var jsonObj=new Object( {
        fields: new Array () , 
        records : new Array() 
        });
    
    for ( var ir=0, ireal=0; ir<rows.length; ++ir) { 
        if(rows[ir].length==0) continue;
        var cols=rows[ir].split(",");
        if(cols.length==0)continue;
        
        if(ireal==0 ){ 
            for( var ic=0; ic<cols.length; ++ic) {
                if(cols[ic].length==0)continue;

                jsonObj.fields[jsonObj.fields.length]= {
                    id: (ic+1),
                    tag: ("F"+(ic+1)),
                    type:'data',
                    name: cols[ic],
                    align: 'left',
                    fclass:'ColumnHeading',
                    dataClass:'RowRegular',
                    typ: 'Column '+cols[ic]
                    };
            } 
        }else { 
            var obj= {id: ir };
            for( var ic=0; ic<cols.length; ++ic) {
                if(cols[ic].length==0)continue;
                obj["F"+(ic+1)]=cols[ic];

            }            
            jsonObj.records[jsonObj.records.length]=obj;
        }
        ++ireal;
    }
    
    return jsonObj;
}


function objView_jafar(form, field, tname, nameparam) 
{
    var valu;
    
    var formTag=eval ("form."+tname );
    if(formTag)
        valu = eval ("formTag."+nameparam );
    if(valu) return valu;
    
    var fieldTag=eval( "field."+tname );
    if(fieldTag)
        valu=eval ("fieldTag."+nameparam );
    return valu;
}





function objView_generateHeaderText(form, checkbox,fields,objectID)
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
            + " onclick=" + '"' + "f_selectObjectCheckbox('" + objectID +"')" + '"'
            + ">"
            + "</td>";
        }
      t+="<td " 
        + " class='"+fields[0].fclass+"'"
          + " align='center' "
          + "> ID </td>";
    
    for ( var ic=0; ic<fields.length; ++ ic){
        var fld=fields[ic];
        var cellWD=objView_jafar(form,fld,fld.tag,"width");

        t+="<td ";
        t+=" id='"+fld.id+"'";
        t+=" class="+fld.fclass+" ";
        if(cellWD)t+=" width="+cellWD + (form.widthInPixels ? "" : "%");
        t+=" align="+fld.align;
        t+=" title="+fld.tip;
        t+=">"+fld.name+"</td>";
        
    }
    t+="</tr>";
    
    return t;
    
}
function objView_generateRowText(checkbox,record,fields,objectID,recordActions,form,rowSequence,info)
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

    if (form.colorbg && form.colorbg.length){
          cellBG = " bgcolor='" + form.colorbg[rowSequence%form.colorbg.length] + "'";
      }
    
    t+="<tr id='"+objectID+"_"+record.id+"' >";
       
    if (checkbox==1){ 
        var allowedActions;
        allowedActions = "";

        for ( var f=0; f< fields.length; ++f )  {
            if (fields[f].type=='actions'){
               var tag=fields[f].tag;
               var allowedActions =eval("record."+tag);
               break;
            }
        }
        
        t+="<td align='center'><input "
            + " type=" + '"' + "checkbox" + '"' 
            + " id=" + '"' + objectID + "_c_" + record.id + '"' 
            + " name=" + '"' + objectID + "_c_" + record.id + '"'
            + " onclick=" + '"' + "f_selectRecord('" + objectID + "','" + record.id + "',this.checked)" + '"'
            + " att_allowedActions=" + '"' + allowedActions + '"'
            + "></td>";
    } 
    
        t+="<td align='center' class='RowRegular' " + cellBG  +">" + startid + "</td>";
            
        
    for ( var f=0; f< fields.length; ++f )  { 
       
       var tag=fields[f].tag;
       var fType = fields[f].type;
       var val=eval("record."+tag);

       t+= f_cellContent(form,fields[f],f,val,record,objectID,fType,recordActions,rowSequence);
       

    }
   
    t+="</tr>";
    return t;
}
function f_selectRecord(objectID,recordID,checkedFG){
    var rowID = objectID + "_" + recordID;
    if (checkedFG==true){
        document.getElementById(rowID).bgColor='#CCCCCC';
    }
    else{
        document.getElementById(rowID).bgColor='#FFFFFF';        
        
    }
}
function objView_generateObjectActions(objectActions, objectID)
{
    var t="";
    var val_2show = "";   
    if (objectActions.actions.length==0){
        t=val_2show;
    }
    else {
    for (var oa=0; oa<objectActions.actions.length; ++oa){                                       
                   val_2show = val_2show + "&nbsp;&nbsp;&nbsp;" + "<a href=" 
                       + '"' 
                       + "hive.cgi"
                       + objectActions.actions[oa].url
                       + '"'
                       + " target='_blank' "
                       + ">"
                       + objectActions.actions[oa].name
                       +"</a>";
           }
    
       t+=val_2show;
    }
       return t;
}
function objView_generateGroupActions(groupActions, objectID, messages){

    var t;
    
    if (groupActions.actions.length==0){
        t="&nbsp;";
    }
    else {
        t="<select name=" + '"' + "ga" + "_" + objectID + '"' 
            + " id=" + '"' + "ga" + "_" + objectID + '"'
            + " class='TextField'"
            + " onchange=" + '"' + "selectGroupAction(this)" + '"'
            + ">"
            + "<option value=" + '""' + ">" + messages.lbl_GA_Dropdown_Select + "</option>";

       for (var ga=0; ga<groupActions.actions.length; ++ga){        
           t+="<option value=" + '"' + groupActions.actions[ga].id + "|" + groupActions.actions[ga].url + '"'  
               + ">"
               + groupActions.actions[ga].name
               + "</option>";
       }
    
       t+="</select>";

       t+="&nbsp;<input type=" + '"' + "button" + '"'
               + " name=" + '"' + "btn_Submit_" + objectID + '"'
               + " value=" + '"' + messages.lbl_GA_btn_Submit + '"'
               + " class=" + '"' + "TextField" + '"'
               + " onclick=" + '"' + "f_submitGroupAction('" + objectID +"','"+messages.msg_GA_Dropdown_Validate +"','"+ messages.msg_GA_Checkbox_Validate + "')" +'"'
               + ">";
    }
    return t;
}

function    selectGroupAction(groupActionField){
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
                        f_selectRecord(objectID,recordID,false);
                        all[i].checked=false;
                        all[i].disabled=true;
                    }
                    else if (groupAction==""){
                        all[i].disabled=false;
                    }
                }
        }
    }

}
function f_submitGroupAction(objectID, msg_GA_Dropdown_Validate , msg_GA_Checkbox_Validate){
    var groupActionField = "ga_" + objectID;
    var selectedGroupActionFull = document.getElementById(groupActionField).value;
    var selectedGroupActionURL = selectedGroupActionFull.substring(selectedGroupActionFull.indexOf("|")+1);

    if (selectedGroupActionFull==""){
        alert(msg_GA_Dropdown_Validate);
        document.getElementById(groupActionField).focus();
    }
    else{
        var selectionFound = f_checkSelection(objectID);
        if (selectionFound==0){
            alert(msg_GA_Checkbox_Validate);    
            document.getElementById(groupActionField).focus();    
        }
        else{
            var url_2use = selectedGroupActionURL + f_selectRecords(objectID);
            alert(url_2use);
        }
    }
}

function f_checkSelection(objectID){
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
}
function f_selectRecords(objectID){
    var checkboxName = objectID + "_c_"; 
    var all = document.getElementsByTagName("*");
    var selectedRecords = "";
    
    for (var i = 0; i<all.length; i++){
        var elementName = all[i].name;
        if (elementName){
                if (elementName.indexOf(checkboxName)!=-1){
                    if (all[i].checked==true){
                        var recordID = elementName.replace(checkboxName,"");
                        selectedRecords = selectedRecords + "," + recordID;
                    }
                }
        }
    }
    selectedRecords = selectedRecords.substring(1);
    
    return selectedRecords;
}
function objView_generateObjectFilter(objectID, messages){
        var t="";
        t+="<input type=" + '"' + "text" + '"' 
        + " name=" + '"'+ "search_" + objectID+ '"' 
        + " id=" + '"'+ "search_" + objectID+ '"'
        + ">"
        + "&nbsp;"
        + "&nbsp;<input type=" + '"' + "button" + '"'
        + " name=" + '"' + "btn_Search_" + objectID + '"'
        + " value=" + '"' + messages.lbl_GA_btn_Search + '"'
        + " class=" + '"' + "TextField" + '"'
        + " onclick=" + '"' + "f_search('" + objectID +"','"+messages.msg_GA_Search_Validate+"')" +'"'
        + ">";
        return t;
}
function f_search(objectID, msg_GA_Search_Validate){
    var searchFieldName = "search_" + objectID;
    var searchString = document.getElementById(searchFieldName).value;
    if (searchString==""){
        alert (msg_GA_Search_Validate);
        document.getElementById(searchFieldName).focus();
    }
    else {
        alert(searchString);
    }
}
function objView_generateGroupFilters(objectFilters, objectID){

    var t = "";
    var count_dropdown = 0;
    var count_checkbox = 0;
    var count_radio = 0;
    
    if (objectFilters.filters.length==0){
        t="";         
    }
    else  {

        t+="<table width='100%' border='0'>"
            + "<tr>";

            for (var gf=0; gf<objectFilters.filters.length; ++gf){        
                t+="<td class='RowRegular'>"; 
                if (objectFilters.filters[gf].type=="checkbox"){
                    count_checkbox = count_checkbox + 1;
                    t+="<input type=" + '"' + "checkbox" + '"'
                        + " class='TextField' "
                        + " name=" + '"' + objectID + "_f_" + objectFilters.filters[gf].id + '"'
                        + " id=" + '"' + objectID + "_f_" + objectFilters.filters[gf].id + '"'
                        + " value=" + '"' + objectID + "_f_" + objectFilters.filters[gf].values.id + '"'
                        + " onclick=" + '"' + "f_filterByCheckbox('" + objectID + "','" + objectFilters.filters[gf].id + "','" + objectFilters.filters[gf].values[0].id + "',this.checked)" + '"'
                        + ">"
                        + "&nbsp;" + objectFilters.filters[gf].name;
                } 
                else if (objectFilters.filters[gf].type=="dropdown"){
                    count_dropdown = count_dropdown + 1;
                    t+="<select name=" + '"' + "f" + "_" + objectID + '"' 
                        + " id=" + '"' + "f" + "_" + objectID + '"'
                        + " class='TextField'"
                        + ">"
                        + "<option value=" + '""' + ">" + objectFilters.filters[gf].name + "</option>";

                    for (var f=0; f<objectFilters.filters[gf].values.length; ++f){        
                        t+="<option value=" + '"' + objectFilters.filters[gf].values[f].id + '"'  
                        + ">"
                        + objectFilters.filters[gf].values[f].value
                        + "</option>";
                    }
                    t+="</select>";
                }
                t+="</td>";
            }
            
            if (count_dropdown>0){
                t+="<td>"
                    + "<input type=" + '"' + "button" + '"'
                    + " name=" + '"' + "btn_Filter_" + objectID + '"'
                    + " value=" + '"' + "Submit" + '"'
                    + " class=" + '"' + "TextField" + '"'
                    + " onclick=" + '"' + "f_submitFilter(this)" +'"'
                    + ">"
                    + "<input type=" + '"' + "button" + '"'
                    + " name=" + '"' + "btn_FilterRemove_" + objectID + '"'
                    + " value=" + '"' + "Remove" + '"'
                    + " class=" + '"' + "TextField" + '"'
                    + " onclick=" + '"' + "f_removeFilter(this)" +'"'
                    + ">"                    
                    +"</td>";
            }
        

            t+="</tr>"
                +"</table>";
    }
    return t;
}
function f_filterByCheckbox(objectID,Filter_ID,Value_ID,Selection_FG){
    alert(objectID + ' ' + Filter_ID + ' ' + Value_ID + ' ' + Selection_FG);
}
function selectObjectFilter(objectFilterField, objectID){
    var objectID = objectFilterField.name.replace("gf_","");
    var objectFilterFull = objectFilterField.value;
    var objectFilter = objectFilterFull.substring(0,objectFilterFull.indexOf("|"));

    var checkboxName = objectID + "_c_"; 
    var all = document.getElementsByTagName("*");
    
    var t="";
    t+="<tr id='"+objectID+"_filter' >";
    
     for ( var ic=0; ic<objectFilterField.length; ++ ic){
            var fld=objectFilterField[ic];

            t+="<td ";
            t+=" id='"+fld.id+"'";
            t+=" align="+fld.align;
            t+=">"+fld.name+"</td>";
            
        }
    
    for (var i = 0; i<all.length; i++){
        var elementName = all[i].name;
        if (elementName){
                if (elementName.indexOf(checkboxName)!=-1){
                    var checkboxAllowedFilters = all[i].getAttribute("att_allowedFilters");
                    var recordID = elementName.replace(checkboxName,"");
                    if (checkboxAllowedFilters.indexOf(groupFilter)==-1 && groupFilter!=""){
                        f_selectRecord(objectID,recordID,false);
                        all[i].checked=false;
                        all[i].disabled=true;
                    }
                    else if (objectFilter==""){
                        all[i].disabled=false;
                    }
                }
        }
    }

}
function f_CheckUncheckButtons(objectID, messages, checkbox){ 
    var t="";
    if (checkbox==1){
    t+= "<input type=" + '"' + "button" + '"'
        + " name=" + '"' + "btn_CheckAll_" + objectID + '"'
        + " value=" + '"' + messages.lbl_GA_btn_CheckAll + '"'
        + " class=" + '"' + "TextField" + '"'
        + " onclick=" + '"' + "f_selectObjectCheckbox('" + objectID +"',true)" +'"'
        + ">";
    
    t+= "<input type=" + '"' + "button" + '"'
        + " name=" + '"' + "btn_UncheckAll_" + objectID + '"'
        + " value=" + '"' + messages.lbl_GA_btn_UncheckAll + '"'
        + " class=" + '"' + "TextField" + '"'
        + " onclick=" + '"' + "f_selectObjectCheckbox('" + objectID +"',false)" +'"'
        + ">";
    } else {};
    return t;
}
function f_selectObjectCheckbox(objectID){
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
                        f_selectRecord(objectID,recordID,Selection_FG);                    
                    }
                }
        }
    }
}
    function f_cellContent(form,field, fieldSequence,fieldValue,record,objectID,fieldType,recordActions,rowSequence){
        var val2return = "";
        var val_2show = "";
        var cellBG = "";
        var cellWD="";
        
      if (fieldValue){
        if (fieldType=='actions'){
               var action_array = fieldValue.split(",");
               for (var a=0; a<action_array.length; ++a){
                   for (var r=0; r<recordActions.actions.length; ++r){
                       if (recordActions.actions[r].id == action_array[a]){
                           val_2show = val_2show + "&nbsp;&nbsp;&nbsp;" + "<a href=" 
                                               + '"' 
                                               + "hive.cgi"
                                               + recordActions.actions[r].url
                                               + record.id
                                               + '"'
                                               + " target='_blank' "
                                               + ">"
                                               + recordActions.actions[r].name
                                               +"</a>";
                       } 
                   }
               }
          }
        else{
            val_2show = fieldValue;
        }
        var lnkl=objView_jafar(form, field, field.tag, "link");
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
      
      if (form.colorbg && form.colorbg.length){
          cellBG = " bgcolor='" + form.colorbg[rowSequence%form.colorbg.length] + "'";
      }
    
      
        val2return = "<td id='"+objectID+"_"+record.id+"_"+ fieldSequence +"' "
                     + " class='" + field.dataClass + "'" 
                     + " align='" + field.align + "'"
                     + cellBG
                     + ">"
                         + val_2show
                         + "</td>";
   
        return val2return;
    }
    function f_submitFilter(filterButton){
        alert(filterButton.name);
    }

    function f_removeFilter(filterButton){
        alert(filterButton.name);
    }



vDV.registerViewer( "objview", vDV.generateObjView) ;


