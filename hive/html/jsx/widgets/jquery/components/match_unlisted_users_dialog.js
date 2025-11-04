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
function unlistedUsersDialog(props){
    this.type = props.type ? props.type : null;
    this.labelNames = props.labelNames ? props.labelNames : null
    this.viewer = props.viewer ? props.viewer : null;
    this.values = props.values ? props.values : null;
    this.userList = props.userList ? props.userList : null;
    this.userNotFound = props.userNotFound ? props.userNotFound : null;
    this.curProjectContact = props.curProjectContact ? props.curProjectContact : null;
    let that = this;
    
    
    this.getLabelNames = function(){
        if(this.type){
            vjDS.add("","labelNames","http:
            vjDS.labelNames.register_callback(function(){
                this.labelNames = vjDS.labelNames.data;
                if(typeof this.labelNames === 'string'){ 
                    return true;
                }
                console.error('Variable "this.labelNames" in conctructor "unlistedUsersDialog" is not a string');
                return false;
            })
            vjDS.labelNames.reload("http:
        }else{
            let reason = 'Function "getFieldNames" in conctructor "unlistedUsersDialog" requires fields';
            console.error(reason);
            return false;
        }
    }
    
    this.unKnownUsersDialogLabelSwitch = async function(){
        
      let hasLabel = await this.hasLabelName();
      if(hasLabel) this.parsedLabelNames =  ArrayToObjArray(CSVToArray(this.labelNames));
      this.unKnownUsersDialog();
      console.log(this.parsedLabelNames);
        
    }
    
    this.hasLabelName = async function(){
        if(!this.labelNames && !vjDS.labelNames){
            let hasLabel = await this.getLabelNames();
            return hasLabel;
        }else{
            if(typeof this.labelNames === 'string') return true;
            console.error('Variable "this.labelNames" in conctructor "unlistedUsersDialog" is not a string'); 
            return false;
        }
    }
    
    this.substituteFieldName = function(field_name){
        let field = this.parsedLabelNames.find(function(field){
            if(field.name === field_name ) return field.title;
        })
        if(field.title) return field.title;
        return field_name;
    }
    
    this.getUserList = async function(){
        vjDS.userList.register_callback(function(){
            userList = vjDS.userList.data;
            userList = CSVToArray(userList );
            let userObj = {};
            for(var i = 0; i < userList.length ; i++){
                 if(userList[i].indexOf('id') === -1 && userList[i].indexOf('name') === -1 &&  userList[i].length > 1){
                    userObj[userList[i][0]] = userList[i][2];
                }
            }
            userList = userObj;

        })
        vjDS.userList.reload("http:
    }; 
    
    this.checkUsersExist = function(){
        let userNotFound = {};

        if(!this.userList) return  userNotFound;

        var userListNames = Object.values(this.userList);
        var userListIds = Object.keys(this.userList);
        for(var prop in this.curProjectContact){
            let propInfo = this.curProjectContact[prop];
            if(prop === 'pi' || prop === 'research_contacts_name' || prop === 'hiveContact'){
                var found_values = [];
                var not_found_values = [];
                let list = typeof propInfo === 'object' && !Array.isArray(propInfo) ? Object.values(propInfo) : propInfo.split(',');

                for(var i = 0 ; i < list.length ; i++){
                    var name = list[i].trim()
                    if(userListNames.includes(name)){
                        let index = userListNames.indexOf(name);
                        found_values.push(userListIds[index]);
                    }else if(userListIds.includes(name)){
                        found_values.push(name);
                    }else{
                        not_found_values.push(name) ;
                    }
                }
                this.curProjectContact[prop] = [...found_values, ...not_found_values].toString()
                if(not_found_values.length > 0 )  userNotFound[prop] = not_found_values;
            }
        }
        
        this.userNotFound = userNotFound;
        return userNotFound ;
    }
    
    this.unKnownUsersDialog = function() {

        let usrlstIDs = Object.keys(this.userList);
        let usrlstNames = Object.values(this.userList);
        
        
        
        
        let form = $(document.createElement("form"));

        function createSelectOptions (name){
            var selectOptions = $(document.createElement("datalist"))
                    .attr("id", name)
                    .attr("name",name)
                    .append($(document.createElement("option"))
                        .text('-1 - REMOVE Name')
                    );
            for(var i = 0; i < usrlstNames.length; i++){
                var txt = usrlstIDs[i] + " - " + usrlstNames[i];
                selectOptions.append($(document.createElement("option"))
                        .text(txt)
                );
            }
            return selectOptions;
        }

        let userNotFoundFields = Object.keys(this.userNotFound);

        function fields (id,usrs) {
            let field_label = that.parsedLabelNames ? that.substituteFieldName(id) : id;
            
            let fields = $(document.createElement("div"))
            for(var i = 0; i < usrs.length; i++){
                var forVal = `${id}__${usrs[i]}`
                fields.append($(document.createElement("fieldset"))
                                .append($(document.createElement("label"))
                                        .attr('class','hv-dialog-container-section__label')
                                        .attr("for",forVal)
                                        .text(usrs[i] + ' ( ' + field_label + ' ) ')
                                )
                                .append($(document.createElement("input"))
                                        .attr('list',forVal)
                                        .attr("name",forVal)
                                        .attr('autocomplete',"off")
                                )
                                .append(createSelectOptions(forVal))
                )
            }
            return fields;
        }

        for(var i = 0; i < userNotFoundFields.length ; i++){
            var txt = userNotFoundFields[i]
            form.append($(document.createElement("div"))
                .attr("id" , txt)
               .append(fields(txt,this.userNotFound[txt]))                
            )
        }

        $("body").append(
            $(document.createElement("div"))
                .attr("id", "dialog-user")
                .attr("Users", "Select Users")
                .append (
                        $(document.createElement("h4"))
                        .attr('class','hv-dialog-container-title')
                        .text("Following names were not found in existing user list.")
                )
                .append (
                        $(document.createElement("p"))
                        .attr('class','hv-dialog-container-dscpt')
                        .text("Please match name with name from user list. If no match is found create user account for following users and refresh the dialog box.")
                )
                .append(
                       $(document.createElement("div"))
                       .attr('class','hv-dialog-container-actions')
                       .append (
                               $(document.createElement("button"))
                               .attr('class','hv-dialog-container-button hv-base-button')
                               .attr('id','user-btn-refresh')
                               .text("\u21BB refresh")
                       )

                        .append (
                               $(document.createElement("button"))
                               .attr('class','hv-dialog-container-button hv-link-button')
                               .attr('id','user-btn-account')
                               .text("create user account")
                       )

                )
                .append(form)
        );

        var userBtnRefresh = document.getElementById('user-btn-refresh');
        userBtnRefresh.addEventListener('click', function(){
            this.getUserList().then(function(){
                var dialogUser = document.getElementById('dialog-user');
                var selectionTags =  dialogUser.querySelectorAll('datalist');
                $.each(selectionTags,function(i,tag){
                    var tagValue = tag.value;
                    $(tag).empty();
                    console.log(this.userList)
                    debugger; 
                    $.each(this.userList, function(id,name){
                        var txt = id + " - " + name;
                        $(tag).append($(document.createElement("option"))
                            .text(txt)
                        );
                    })
                    $(tag).val(tagValue)
                });
            });

        });

        var userBtnAccount = document.getElementById('user-btn-account');
        userBtnAccount.addEventListener('click', function(){
            var loc = window.location;
            var url = loc.protocol + "
            linkURL(url , true)
        })

        $("#dialog-user").dialog({
            modal: true,
            width: 500,
            buttons: [
                        {
                          text: "OK",
                          class: "hv-btn hv-btn-minor",
                          click: function() {
                            $(this).dialog("close");
                            that.values = this.querySelectorAll("input");
                            that.recordUnlistedUsers();

                          }
                        },
                        {
                          text: "Cancel",
                          class: "hv-btn hv-btn-minor",
                          click: function() {
                            $(this).dialog("close");
                          }
                        }
                    ],
            close: function() {
                        $(this).dialog("close");
                        $(this).dialog("destroy").remove();
                    }
            
        });
    }
    
    this.recordUnlistedUsers = function () {
        var changeList = Array.from(this.values)
        var fieldsToUpdate = []
        for (var i = 0; i < changeList.length; i++){
            var fieldName = changeList[i].name.split('__');
            var field = fieldName[0];
            var name = fieldName[1];
            var subid = changeList[i].value.length === 0 ? name : changeList[i].value.split(' - ')[0];

            var curFieldValue = this.curProjectContact[field].split(',');
            var nameIndex = curFieldValue.indexOf(name);
            if(subid > -1 || subid !== '-1'){
                if(fieldsToUpdate.indexOf(field) === -1){
                    fieldsToUpdate.push(field);
                }
                if(nameIndex != -1){
                    curFieldValue.splice(nameIndex,1,subid);
                }
            }else if(subid === -1 || subid === '-1'){
                if(nameIndex != -1){
                    curFieldValue.splice(nameIndex,1);
                }   
            }
            this.curProjectContact[field] = curFieldValue.join(',')
        }
        if(fieldsToUpdate.length > 0){
            for(let i = 0; i < fieldsToUpdate.length; i++){
                let field = fieldsToUpdate[i];
                this.viewer.nodeTree.findByFieldName(field).value = this.curProjectContact[field];
            }
            this.viewer.redraw();
        }
    }
    
}
