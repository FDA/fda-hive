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
$(function () {
    $.widget("layout.shareview", $.layout.layoutmanager, {

        options: {
            data: {
                userTree: "",
                userList: "",
                objList: "",
                objDep: "",
                permList: "",
                permFlags: "",
                help: ""
            }
        },

        _onBeforeInit: function () {
            this.options.config = {
                layout: {
                    allowResize: false,
                    items: [
                        {
                            id: 'sharing',
                            top: '0',
                            left: '0%',
                            right: '100%',
                            bottom: '100%',
                            allowResize: false,
                            overflow: "auto",
                            view: {
                                name: "dataview",
                                options: {

                                }
                            }
                  }]
                }
            };

            this.options.container = "sharingView" + parseInt(Math.random() * 100000);


            var data = this.options.data;
            vjDSNew.registerWidget(data.userTree, this);
            vjDSNew.registerWidget(data.userList, this);
            vjDSNew.registerWidget(data.objList, this);
            vjDSNew.registerWidget(data.objDep, this);
            vjDSNew.registerWidget(data.permList, this);
            vjDSNew.registerWidget(data.permFlags, this);
            vjDSNew.registerWidget(data.help, this);
        },

        _onAfterInit: function () {
            this.element.children().attr("id", this.options.container);
        },

        draw: function (widget, dsStruct, content) {
            var oThis = this;

            if (!this.options.userData) {
                this.options.containerForHidden = "sharingView" + parseInt(Math.random() * 100000);
                $("body").append($(document.createElement("div")).attr("id", this.options.containerForHidden).addClass("sectHidden"));

                var usrData = new vjUserShareTreeView({
                    data: ["dsUserTree", "dsUserList", "dsObjList", "dsObjDeps"],
                    treeSourceName: "dsUserTreeViewSource",
                    formObject: document.forms["form-userList"],
                    highlightAnyFlag: true,
                    container: this.options.containerForHidden,
                    dataSourceEngine: vjDS,
                    dataViewEngine: vjDV,
                    doNotDraw: true
                });

                usrData.load();
                usrData.render();
                this.options.userData = usrData;

                this.options.permFlags = new vjTable(vjDS[this.options.data.permFlags].data, 0, vjTable_propCSV);
                this.options.permList = new vjTable(vjDS[this.options.data.permList].data, 0, vjTable_propCSV);
            }
            this.element.on("area-resize", function () {
                console.log(oThis);
                oThis.drawContent();
            });

            this.drawContent(widget, dsStruct, content);
        },
        drawContent: function (widget, dsStruct, content) {
            this.options.height = this.element.height();
            this.options.width = this.element.width();
            this.options.panel = this.element.find("#sharing-panel");
            this.options.userContentDiv = "user-group-content";
            this.options.objPermContentDiv = "obj-perm-content";
            this.options.depPermContentDiv = "dep-perm-content";

            var rowHeight = this.options.height - 90;

            this.options.panel.empty();
            this.options.panel.append($(document.createElement("div"))
                .addClass("row")
                .attr("id", "rowLoc")
            );
            var appendTo = $("#rowLoc");
            appendTo.append($(document.createElement("div"))
                .addClass("treesCols sharingcols")
                .append($(document.createElement("div"))
                    .addClass("boxBasic")
                    .attr({
                        "style": "height: " + rowHeight + "px;"
                    })
                    .append($(document.createElement("div"))
                        .addClass("row ban_top")
                        .append($(document.createElement("div"))
                            .addClass("col-xs-12 float_right")
                            .append($(document.createElement("h5"))
                                .text("Group List")
                            )
                            .append($(document.createElement("form"))
                                .append($(document.createElement("input"))
                                    .attr("type", "search")
                                    .attr("placeholder", "Search ...")
                                    .addClass("form-control btn_group-non input__rv")
                                    .on("keyup", function () {
                                        var filter = this.value.toUpperCase();
                                        var li = $("[treeSelector='usrTreeLi']");

                                        $(li).each(function (elem, i) {
                                            var text = this.textContent;
                                            if (text.toUpperCase().indexOf(filter) > -1) {
                                                this.style.display = "";
                                            } else {
                                                this.style.display = "none";
                                            }

                                        });
                                    })
                                )
                                .append($(document.createElement("button"))
                                    .addClass("btn_search")
                                    .attr("title", "Search")
                                    .attr("type", "submit")
                                    .append($(document.createElement("i"))
                                        .addClass("rv-search")
                                    )

                                    .click(function (event) {
                                        event.preventDefault();
                                    })

                                )
                            )
                            .append($(document.createElement("div"))
                                .addClass("btn_group")
                                .append($(document.createElement("button"))
                                    .attr("href", "")
                                    .attr("title", "Help")
                                    .addClass("btn")
                                    .append($(document.createElement("i"))
                                        .addClass("rv-help")
                                    )
                                )
                            )

                        )
                    )
                    .append($(document.createElement("div"))
                        .addClass("content con-pad ")
                        .attr("id", this.options.userContentDiv)
                    )
                )
            );
            appendTo.append($(document.createElement("div"))
                .addClass("permCols sharingCols")
                .append($(document.createElement("div"))
                    .addClass("boxBasic")
                    .attr({
                        "style": "height: " + rowHeight + "px;"
                    })
                    .append($(document.createElement("div"))
                        .addClass("row ban_top")
                        .append($(document.createElement("div"))
                            .addClass("col-xs-12")
                            .append($(document.createElement("h5"))
                                .text("Objects")
                            )
                            .append($(document.createElement("div"))
                                .addClass("pull-right")
                                .append($(document.createElement("button"))
                                    .addClass("btn")
                                    .attr("relevant", "objs")
                                    .attr("title", "Reset")
                                    .text("Reset")
                                    .prepend($(document.createElement("i"))
                                        .addClass("rv-spinner")
                                    )
                                    .click(this._emptyFields)
                                )
                            )
                        )
                    )
                    .append($(document.createElement("div"))
                        .addClass("contentSharing")
                        .attr("style", "top:35px;")
                        .attr("id", this.options.objPermContentDiv)
                    )
                )
            );

            appendTo.append($(document.createElement("div"))
                .addClass("permCols sharingCols")
                .append($(document.createElement("div"))
                    .addClass("boxBasic")
                    .attr({
                        "style": "height: " + rowHeight + "px;"
                    })
                    .append($(document.createElement("div"))
                        .addClass("row ban_top")
                        .append($(document.createElement("div"))
                            .addClass("col-xs-12")
                            .append($(document.createElement("h5"))
                                .text("Dependencies")
                            )
                            .append($(document.createElement("div"))
                                .addClass("pull-right")
                                .append($(document.createElement("button"))
                                    .addClass("btn")
                                    .attr("relevant", "deps")
                                    .text("Reset")
                                    .prepend($(document.createElement("i"))
                                        .addClass("rv-spinner")
                                    )
                                    .click(this._emptyFields)
                                )
                            )
                        )
                    )
                    .append($(document.createElement("div"))
                        .addClass("contentSharing")
                        .attr("style", "top:35px;")
                        .attr("id", this.options.depPermContentDiv)
                    )
                )
            );

            this.options.panel.append($(document.createElement("br")));
            this.options.panel.append($(document.createElement('div'))
                .addClass('row')
                .append($(document.createElement("div"))
                    .addClass('col-md-12')
                    .append($(document.createElement("div"))
                        .addClass("float_right")
                        .append($(document.createElement("button"))
                            .attr("type", "button")
                            .attr("id", "deleteButton")
                            .addClass("btn red_btn btn_lg")
                            .attr("name", "deleteNowBtn")
                            .text("Clear Permissions")
                            .on("click", function () {
                                $("#deleteModal").modal("show");
                                $("body").append($(".modal-backdrop"));

                                var allIDs = "";
                                for (var i = 0; i < $("[data-toggle='objs']").children(".selected").length; i++) {
                                    var str = $("[data-toggle='objs']").children(".selected")[i].id;
                                    str = str.substring(3);
                                    allIDs += str + ",";
                                }
                                allIDs = allIDs.substring(0, allIDs.length - 1);

                                var depsIDs = "";
                                for (var i = 0; i < $("[data-toggle='deps']").children(".selected").length; i++) {
                                    var str = $("[data-toggle='deps']").children(".selected")[i].id;
                                    str = str.substring(3);
                                    depsIDs += str + ",";
                                }
                                depsIDs = depsIDs.substring(0, depsIDs.length - 1);

                                $("[relatesTo='delete']").empty();
                                $("[relatesTo='delete']").append($(document.createElement("p"))
                                    .append($(document.createElement("strong"))
                                        .text(allIDs)
                                    )
                                );

                                if (depsIDs != "") {
                                    $("[relatesTo='delete']").append($(document.createElement("p"))
                                            .text("and its dependencies")
                                        )
                                        .append($(document.createElement("p"))
                                            .append($(document.createElement("strong"))
                                                .text(depsIDs)
                                            )
                                        );
                                }
                            })
                        )
                        .append($(document.createElement("button"))
                            .attr("type", "button")
                            .attr("id", "shareButton")
                            .addClass("btn blue_btn btn_lg")
                            .attr("name", "shareNowBtn")
                            .text("Share Now")
                            .on("click", function () {
                                $("#shareModal").modal("show");
                                $("body").append($(".modal-backdrop"));

                                var allIDs = "";
                                for (var i = 0; i < $("[data-toggle='objs']").children(".selected").length; i++) {
                                    var str = $("[data-toggle='objs']").children(".selected")[i].id;
                                    str = str.substring(3);
                                    allIDs += str + ",";
                                }
                                allIDs = allIDs.substring(0, allIDs.length - 1);

                                var depsIDs = "";
                                for (var i = 0; i < $("[data-toggle='deps']").children(".selected").length; i++) {
                                    var str = $("[data-toggle='deps']").children(".selected")[i].id;
                                    str = str.substring(3);
                                    depsIDs += str + ",";
                                }
                                depsIDs = depsIDs.substring(0, depsIDs.length - 1);

                                $("[relatesTo='share']").empty();
                                $("[relatesTo='share']").append($(document.createElement("p"))
                                    .append($(document.createElement("strong"))
                                        .text(allIDs)
                                    )
                                );

                                if (depsIDs != "") {
                                    $("[relatesTo='share']").append($(document.createElement("p"))
                                            .text("and its dependencies")
                                        )
                                        .append($(document.createElement("p"))
                                            .append($(document.createElement("strong"))
                                                .text(depsIDs)
                                            )
                                        );
                                }
                            })
                        )
                    )
                )

            );

            var usrListAppend = $("#" + this.options.userContentDiv);
            var usrTreeContent = vjDS[this.options.data.userTree].data;

            var treeClasses = [["", "rv-sitemap"], ["child", "rv-sitemap"], ["grandchild", "rv-user"]];
            var curLoc = usrListAppend.append($(document.createElement("ul")));

            this.options.userData.tree.root.children[0].expanded = true;
            this.options.userData.tree.root.children[0].children[1].expanded = true;
            this._constructTree(curLoc, this.options.userData.tree.root, treeClasses);
            this._collapseTree(this.options.userData.tree.root);

            var oThis = this;

            this._on(false, $(".tree"), {
                click: "_treeClick"
            });
            this._treeClick = function (event) {
                var element = event.currentTarget;
                if (!event.ctrlKey && !$(element).hasClass("selected")) {
                    $(".tree").removeClass("selected");
                }
                $(element).toggleClass("selected");

                if (!event.preSelect) {
                    var usrId = $(element).attr("treeid");
                    var name = $(element).attr("id");
                    this._emptyFields();
                    if ($(element).hasClass("selected"))
                        this._setPermissions(usrId, name, $(element).children().hasClass("rv-user"));
                }

                if (!event.ctrlKey)
                    $("[treeselector='usrTreeUl']").removeClass("selected_inherited");
                event.preventDefault();
            };

            $("li p").click(function () {
                if ($(this).hasClass("selected"))
                    $(this).nextAll().toggleClass("selected_inherited");
            });

            var objsAppend = $("#" + this.options.objPermContentDiv);
            this._constructObjs(objsAppend, this.options.userData.objTbl, "Select users for object permissions on the right", false, "objs");

            var depsAppend = $("#" + this.options.depPermContentDiv);
            this._constructObjs(depsAppend, this.options.userData.depTbl, "Select files for dependencies permissions above", true, "deps");

            var userObjs = this._calculateUsersObjects();

            var tmpDiv = $(document.createElement("div"));
            tmpDiv.append($(document.createElement("div"))
                .addClass("delete modal fade")
                .attr("id", "deleteModal")
                .attr("role", "dialog")
                .append($(document.createElement("div"))
                    .addClass("modal-dialog")
                    .append($(document.createElement("div"))
                        .addClass("modal-content")
                        .append($(document.createElement("div"))
                            .addClass("modal-header")
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("close")
                                .attr("data-dismiss", "modal")
                                .text("×")
                            )
                            .append($(document.createElement("h4"))
                                .addClass("modal-title")
                                .append($(document.createElement("strong"))
                                    .text("Clear Permissions")
                                )
                            )
                        )
                        .append($(document.createElement("div"))
                            .addClass("modal-body")
                            .append($(document.createElement("p"))
                                .html("You are <strong>removing</strong> all permissions from objects with the following IDs:")
                            )
                            .append($(document.createElement("div"))
                                .attr("relatesTo", "delete")
                            )
                            .append($(document.createElement("p"))
                                .text("As a result, object may become inaccessible.")
                            )
                        )
                        .append($(document.createElement("div"))
                            .addClass("modal-footer")
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("btn btn-third")
                                .attr("data-dismiss", "modal")
                                .text("CANCEL")
                            )
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("btn btn-secondary red_btn")
                                .attr("data-dismiss", "modal")
                                .attr("mmodal-button", "delete")
                                .text("CLEAR")
                            )
                        )
                    )
                )
            );
            tmpDiv.append($(document.createElement("div"))
                .addClass("share modal fade")
                .attr("id", "shareModal")
                .attr("role", "dialog")
                .append($(document.createElement("div"))
                    .addClass("modal-dialog")
                    .append($(document.createElement("div"))
                        .addClass("modal-content")
                        .append($(document.createElement("div"))
                            .addClass("modal-header")
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("close")
                                .attr("data-dismiss", "modal")
                                .text("×")
                            )
                            .append($(document.createElement("h4"))
                                .addClass("modal-title")
                                .append($(document.createElement("strong"))
                                    .text("SHARE")
                                )
                            )
                        )
                        .append($(document.createElement("div"))
                            .addClass("modal-body")
                            .append($(document.createElement("p"))
                                .html("You are <strong>changing</strong> permissions for objects with the following IDs:")
                            )
                            .append($(document.createElement("p"))
                                .append($(document.createElement("strong"))
                                    .attr("relatesTo", "share")
                                )
                            )
                        )
                        .append($(document.createElement("div"))
                            .addClass("modal-footer")
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("btn btn-third")
                                .attr("data-dismiss", "modal")
                                .text("CANCEL")
                            )
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("btn btn-secondary blue_btn")
                                .attr("data-dismiss", "modal")
                                .attr("mmodal-button", "share")
                                .text("SHARE")
                            )
                        )
                    )
                )
            );
            tmpDiv.append($(document.createElement("div"))
                .addClass("errpr modal fade")
                .attr("id", "errorModal")
                .attr("role", "dialog")
                .append($(document.createElement("div"))
                    .addClass("modal-dialog")
                    .append($(document.createElement("div"))
                        .addClass("modal-content")
                        .append($(document.createElement("div"))
                            .addClass("modal-header")
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("close")
                                .attr("data-dismiss", "modal")
                                .text("×")
                            )
                            .append($(document.createElement("h4"))
                                .addClass("modal-title")
                                .append($(document.createElement("strong"))
                                    .text("ERROR")
                                )
                            )
                        )
                        .append($(document.createElement("div"))
                            .addClass("modal-body")
                            .append($(document.createElement("p"))
                                .html("You need to choose the appropriate permissions")
                            )
                        )
                        .append($(document.createElement("div"))
                            .addClass("modal-footer")
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("btn btn-third")
                                .attr("data-dismiss", "modal")
                                .append($(document.createElement("strong"))
                                    .text("OK")
                                )
                            )
                        )
                    )
                )
            );
            $("body").append(tmpDiv);

            this._on(false, $("button[mmodal-button='share']"), {
                click: "_onShare"
            });
            this._on(false, $("button[mmodal-button='delete']"), {
                click: "_onDelete"
            });
        },
        _constructTree: function (curLoc, node, treeClasses) {
            if (node.depth == 0) {
                for (var i = 0; i < node.children.length; i++) {
                    this._constructTree(curLoc, node.children[i], treeClasses);
                }
                return;
            }

            var iClass = "rv-chevron-right rotate_90";
            if (!node.children || node.children.length == 0) iClass = "rv-user";

            var index = node.depth - 1;
            if (index > treeClasses.length - 1) index = treeClasses.length - 1;

            var ii = $(document.createElement("i"))
                .attr("style", "margin-right:8px;")
                .addClass(iClass)
                .attr("name", node.name);
            var li = $(document.createElement("li"))
                .append($(document.createElement("p"))
                    .addClass("tree treeNode")
                    .attr("style", "padding-left:" + (20 * node.depth) + "px;")
                    .text(" " + node.title)
                    .attr("id", node.name)
                    .attr("treeId", node.id)
                    .attr("treeSelector", "usrTreeLi")
                );
            li.children()
                .prepend(ii);
            curLoc.append(li);

            if (!node.children || node.children.length < 1) {
                return;
            }

            ii.click(function () {
                var selId = $(this).attr("name");
                $("[name='" + selId + "-ul").toggle();
                $(this).toggleClass("down");
                event.stopPropagation();
            });

            var ul = $(document.createElement("ul"))
                .attr("name", node.name + "-ul")
                .attr("treeSelector", "usrTreeUl");
            li.append(ul);

            for (var i = 0; i < node.children.length; i++) {
                this._constructTree(ul, node.children[i], treeClasses);
            }
            return;
        },
        _collapseTree: function (node) {
            if (!node.children || node.children.length == 0) return;

            for (var i = 0; i < node.children.length; i++) {
                this._collapseTree(node.children[i]);
            }

            if (node.expanded == false)
                $("#" + node.name).parent().children("ul").toggle();
            else
                $("#" + node.name).parent().find("i.rv-chevron-right").first().toggleClass("down");
        },
        _constructObjs: function (appendLoc, objsTbl, phrase, selectAll, prefix) {
            var objToName = {};

            for (var i = 0; i < objsTbl.rows.length; i++) {
                objToName[objsTbl.rows[i].id] = objsTbl.rows[i]._brief;
            }

            var objectsDiv = $(document.createElement("div"))
                .addClass("tabs");

            var i = 0;
            for (var key in objToName) {
                objectsDiv.append($(document.createElement("li"))
                    .append($(document.createElement("a"))
                        .attr("data-toggle", prefix)
                        .append($(document.createElement("div"))
                            .addClass("row sides-pad")
                            .attr("id", "id-" + key)
                            .on("click", function () {
                                $(this).toggleClass("selected");
                            })
                            .append($(document.createElement("div"))
                                .addClass("col-xs-2 nod-pad")
                                .append($(document.createElement("p"))
                                    .append($(document.createElement("strong"))
                                        .text(key)
                                    )
                                )
                            )
                            .append($(document.createElement("div"))
                                .addClass("col-xs-10 nod-pad")
                                .append($(document.createElement("p"))
                                    .html(objToName[key])
                                )
                            )
                        )
                    )
                );
                i++;
            }

            var selectAdd;

            if (selectAll) {
                selectAdd = $(document.createElement("div"))
                    .addClass("col-xs-3 no-pad")
                    .attr("style", "margin-left:15px;")
                    .append($(document.createElement("a"))
                        .addClass("pull-right")
                        .attr("id", prefix + "SelectAll")
                        .attr("style", "font-size:12px;margin-top:2px;")
                        .text("Select All")
                        .on("click", function () {
                            $(this).closest(".row").prev().prev().find(".sides-pad").click();
                        })
                    )
            }

            appendLoc.append($(document.createElement("div"))
                .addClass("top-pad")
                .append(objectsDiv)
                .append($(document.createElement("hr")))
                .append($(document.createElement("div"))
                    .addClass("row")
                    .append($(document.createElement("div"))
                        .addClass("col-xs-8 no-pad")
                        .append($(document.createElement("p"))
                            .addClass("des-ckbox sides-pad")
                            .text(phrase)
                        )
                    )
                    .append(selectAdd)
                )
            );
            this._constructPermHtml(appendLoc, prefix);
        },
        _constructPermHtml: function (appendLoc, pref) {
            var prmOptions = this.options.permFlags.rows[1].options.split(",");
            var prmOptionsHtmlObj = $(document.createElement("div"))
                .addClass("col-xs-9 no-pad");

            for (var i = 0; i < prmOptions.length; i++) {
                prmOptionsHtmlObj.append($(document.createElement("div"))
                    .addClass("radio")
                    .append($(document.createElement("input"))
                        .attr("type", "radio")
                        .attr("name", "radio" + pref)
                        .attr("id", "radio" + pref + "_" + i)
                        .attr("value", "obj_radio")
                    )
                    .append($(document.createElement("label"))
                        .attr("for", "radio" + pref + "_" + i)
                        .append($(document.createElement("p"))
                            .append($(document.createElement("strong"))
                                .text(prmOptions[i])
                            )
                        )
                    )
                );
            }

            appendLoc.append($(document.createElement("div"))
                .attr("id", "accordion")
                .addClass("panel-group")
                .append($(document.createElement("div"))
                    .addClass("panel panel-default")
                    .append($(document.createElement("div"))
                        .addClass("panel-heading")
                        .append($(document.createElement("a"))
                            .addClass("panel-title")
                            .append($(document.createElement("p"))
                                .addClass("switch float_right")
                                .attr("data-toggle", "collapse")
                                .attr("href", "#" + pref + "_app")
                            )
                        )
                    )
                    .append($(document.createElement("div"))
                        .attr("id", pref + "_app")
                        .addClass("panel-collapse collapse in")
                        .append($(document.createElement("div"))
                            .addClass("con-pad")
                            .append($(document.createElement("div"))
                                .addClass("row")
                                .append($(document.createElement("div"))
                                    .addClass("col-xs-3 no-pad")
                                    .append($(document.createElement("p"))
                                        .text(" Applies To:")
                                    )
                                )
                                .append(prmOptionsHtmlObj)
                            )
                        )
                    )
                )
            );

            var pWithI = $(document.createElement("p"))
                .text("   Deny");
            pWithI.prepend($(document.createElement("i"))
                .addClass("rv-close-fat")
                .attr("area-hidden", "true")
                .attr("style", "color:#c30000;padding-left:20px;margin-right:4px;font-size: 12px;")
            );
            pWithI.prepend(" Allow ");
            pWithI.prepend($(document.createElement("i"))
                .addClass("rv-checkmark")
                .attr("area-hidden", "true")
                .attr("style", "color:#0275d8;margin-right:4px;")
            );

            var permWithExp = $(document.createElement("div"))
                .addClass("con-pad")
                .append($(document.createElement("div"))
                    .addClass("row")
                    .append($(document.createElement("div"))
                        .addClass("col-xs-6 col-xs-offset-3 no-pad")
                        .append(pWithI)
                    )
                );

            for (var i = 0; i < this.options.permList.rows.length; i++) {
                var elem = $(document.createElement("div"))
                    .addClass("row")
                    .append($(document.createElement("div"))
                        .addClass("col-xs-3 no-pad")
                        .append($(document.createElement("div"))
                            .addClass("checkbox")
                            .append($(document.createElement("input"))
                                .attr("id", this.options.permList.rows[i].perm + "_" + pref)
                                .attr("type", "checkbox")
                                .attr("name", pref + "_ckbx")
                                .data("checked", 0)
                                .on("click", function (elem) {
                                    switch ($(this).data('checked')) {
                                        case 0:
                                            $(this).data('checked', 1);
                                            $(this).prop('indeterminate', false);
                                            $(this).prop('checked', true);
                                            break;
                                        case 1:
                                            $(this).data('checked', 2);
                                            $(this).prop('indeterminate', true);
                                            $(this).prop('checked', false);
                                            break;
                                        default:
                                            $(this).data('checked', 0);
                                            $(this).prop('indeterminate', false);
                                            $(this).prop('checked', false);
                                    }
                                })
                            )
                            .append($(document.createElement("label"))
                                .attr("for", this.options.permList.rows[i].perm + "_" + pref)
                                .append($(document.createElement("p"))
                                    .append($(document.createElement("strong")).text(this.options.permList.rows[i].Permission))
                                )
                            )
                        )
                    )
                    .append($(document.createElement("div"))
                        .addClass("col-xs-9 no-pad")
                        .append($(document.createElement("p"))
                            .addClass("des-ckbox")
                            .text(this.options.permList.rows[i].Explanation)
                        )
                    );
                permWithExp.append(elem);
            }

            appendLoc.append($(document.createElement("div"))
                .addClass("panel panel-default")
                .append($(document.createElement("div"))
                    .addClass("panel-heading")
                    .append($(document.createElement("a"))
                        .addClass("panel-title")
                        .append($(document.createElement("p"))
                            .addClass("switch float_right")
                            .attr("data-toggle", "collapse")
                            .attr("href", "#" + pref + "List_app")
                        )
                    )
                )
                .append($(document.createElement("div"))
                    .attr("id", pref + "List_app")
                    .addClass("panel-collapse collapse in")
                    .append(permWithExp)
                )
            );
        },
        
        _calculateUsersObjects: function () {
            var usrTree = this.options.userData.tree;
            var objTbl = this.options.userData.objTbl;
            var depTbl = this.options.userData.depTbl;

            var userDirect = [];
            var allBits = {
                admin: 0,
                share: 0,
                browse: 0,
                read: 0,
                write: 0,
                exec: 0,
                del: 0,
                download: 0
            };
            var totalObjsPerm = 0;
            for (var i = 0; i < objTbl.rows.length; i++) {
                var _perm = objTbl.rows[i]._perm;

                for (var j = 0; j < _perm.group.length; j++) {
                    if (userDirect.indexOf(_perm.group[j]) < 0) {
                        userDirect.push(_perm.group[j]);
                        totalObjsPerm++;

                        for (var key in _perm.bits[j]) {
                            allBits[key] += _perm.bits[j][key];
                        }
                    }
                }
            }

            allBits = {
                admin: 0,
                share: 0,
                browse: 0,
                read: 0,
                write: 0,
                exec: 0,
                del: 0,
                download: 0
            };
            totalObjsPerm = 0;
            for (var i = 0; i < depTbl.rows.length; i++) {
                var _perm = depTbl.rows[i]._perm;

                for (var j = 0; j < _perm.group.length; j++) {
                    if (userDirect.indexOf(_perm.group[j]) < 0) {
                        totalObjsPerm++;

                        for (var key in _perm.bits[j]) {
                            allBits[key] += _perm.bits[j][key];
                        }
                    }
                }
            }

            for (var i = 0; i < userDirect.length; i++) {
                var event = jQuery.Event("click");
                event.ctrlKey = true;
                event.preSelect = true;
                $("[treeId='" + userDirect[i] + "']").trigger(event);
            }
        },
        _emptyFields: function (event) {
            if (event) {
                var loc = $(event.target).attr("relevant");

                $("[data-toggle='" + loc + "']").children().removeClass("selected");

                $("input[name='radio" + loc + "']").prop("checked", false);

                $("input[name='" + loc + "_ckbx']").each(function (element) {
                    $(this).data('checked', 0);
                    $(this).prop('indeterminate', false);
                    $(this).prop('checked', false);
                });
            }

            $("[data-toggle='objs']").children().removeClass("selected");
            $("[data-toggle='deps']").children().removeClass("selected");

            $("input[name='radioobjs']").prop("checked", false);
            $("input[name='radiodeps']").prop("checked", false);

            $("div .checkbox").children("input").each(function (element) {
                $(this).data('checked', 0);
                $(this).prop('indeterminate', false);
                $(this).prop('checked', false);
            });

        },
        _setPermissions: function (userId, name, isChildNode) {
            var objTbl = this.options.userData.objTbl;
            var objArr = [];
            var depTbl = this.options.userData.depTbl;
            var depArr = [];

            var allBits = {
                admin: 0,
                share: 0,
                browse: 0,
                read: 0,
                write: 0,
                exec: 0,
                del: 0,
                download: 0
            };
            var anySelected = false;
            var totalObjsPerm = 0;
            for (var i = 0; i < objTbl.rows.length; i++) {
                objArr.push(objTbl.rows[i].id);

                var _perm = objTbl.rows[i]._perm;
                for (var j = 0; j < _perm.group.length; j++) {
                    if (_perm.group[j] == userId) {
                        $("#id-" + objTbl.rows[i].id).click();
                        totalObjsPerm++;

                        for (var key in _perm.bits[j]) {
                            allBits[key] += _perm.bits[j][key];
                        }
                        anySelected = true;
                    }
                }
                if (_perm.flag[0] && _perm.flag[0].down && anySelected)
                    $("[for='radioobjs_0']").click();
            }
            for (var key in allBits) {
                if (allBits[key] == totalObjsPerm && totalObjsPerm != 0) {
                    $("#" + key + "_objs").click();
                } else if (allBits[key] == totalObjsPerm * -1 && totalObjsPerm != 0) {
                    $("#" + key + "_objs").click();
                    $("#" + key + "_objs").click();
                }
            }
            if (totalObjsPerm == 0) {
                for (var i = 0; i < objArr.length; i++)
                    $("#id-" + objArr[i]).click();
            }


            allBits = {
                admin: 0,
                share: 0,
                browse: 0,
                read: 0,
                write: 0,
                exec: 0,
                del: 0,
                download: 0
            }
            totalObjsPerm = 0;
            anySelected = false;
            for (var i = 0; i < depTbl.rows.length; i++) {
                depArr.push(depTbl.rows[i].id);

                var _perm = depTbl.rows[i]._perm;

                for (var j = 0; j < _perm.group.length; j++) {
                    if (_perm.group[j] == userId) {
                        $("#id-" + depTbl.rows[i].id).click();
                        totalObjsPerm++;

                        for (var key in _perm.bits[j]) {
                            allBits[key] += _perm.bits[j][key];
                        }
                        anySelected = true;
                    }
                }
                if (_perm.flag[0] && _perm.flag[0].down && anySelected)
                    $("[for='radiodeps_0']").click();
            }
            for (var key in allBits) {
                if (allBits[key] == totalObjsPerm && totalObjsPerm != 0) {
                    $("#" + key + "_deps").click();
                } else if (allBits[key] == totalObjsPerm * -1 && totalObjsPerm != 0) {
                    $("#" + key + "_deps").click();
                    $("#" + key + "_deps").click();
                }
            }
            if (totalObjsPerm == 0) {
                for (var i = 0; i < depArr.length; i++)
                    $("#id-" + depArr[i]).click();
            }

            if (isChildNode) {
                $("input[type=radio]").attr("disabled", true);
                $("input[type=radio]#radioobjs_0").attr("disabled", false);
                $("input[type=radio]#radiodeps_0").attr("disabled", false);
            } else {
                $("input[type=radio]").attr("disabled", false);
            }
        },
        _updateTreeInfo: function () {
            var selectedNodes = $(".treeNode.selected");
            var toReturn = [];

            for (var i = 0; i < selectedNodes.length; i++) {
                var node = selectedNodes[i];
                var id = node.id;

                var treeNode = this.options.userData.tree.findByName(node.id);
                if (treeNode) {
                    treeNode.selected = 1;
                    treeNode.treeid = $(node).attr("treeid");
                    if (treeNode.children && treeNode.children.length) {
                        for (var ii = 0; ii < treeNode.children.length; ii++)
                            treeNode.children[i].selected = 2;
                    }
                    toReturn.push(treeNode);
                }
            }

            return toReturn;
        },
        
        _onShare: function () {

            var allSelectedNodes = this._updateTreeInfo();
            var url = "permset&ids=" + docLocValue("ids");

            var curUser = decodeURIComponent(window.getCookie("last_login"));
            var warnUser = false;
            var groups = "";
            for (var i = 0; i < allSelectedNodes.length; i++) {
                if(this._findChild(allSelectedNodes[i], curUser, "email")) warnUser = true;
                groups += allSelectedNodes[i].treeid + ",";
            }
            if (groups) {
                groups = groups.substring(0, groups.length - 1);
                url += "&groups=" + groups;
            }

            var allChecked = "";
            var anyDeny = false;
            var objCheckboxes = $("[name='objs_ckbx']");

            objCheckboxes.each(function () {
                if ($(this).data("checked") == 0)
                    return;
                else if ($(this).data("checked") == 2)
                    anyDeny = true;

                var categ = $(this).attr("id").substring(0, $(this).attr("id").indexOf("_"));
                allChecked += categ + "|";
            });
            if (allChecked) {
                allChecked = allChecked.substring(0, allChecked.length - 1);
                url += "&perm=" + allChecked;
                var objsFlags = "allow";
                if (anyDeny) objsFlags = "deny";

                if ($("#radioobjs_0").prop("checked") || $("#radioobjs_1").prop("checked")) objsFlags += "|down";
                else objsFlags += "|up";

                objsFlags += "|active";
                url += "&flag=" + objsFlags;
            }

            if ($("[data-toggle='deps']").children("div.selected").length) {
                var allDeps = "";

                $("[data-toggle='deps']").children("div.selected").each(function () {
                    var objId = $(this).attr("id");
                    allDeps += objId.substring(3) + ",";
                });
                allDeps = allDeps.substring(0, allDeps.length - 1);
                url += "&optIds=" + allDeps;

                var depsChecked = "";
                anyDeny = false;

                $("[name='deps_ckbx']").each(function () {
                    if ($(this).data("checked") == 0)
                        return;
                    else if ($(this).data("checked") == 2)
                        anyDeny = true;

                    var categ = $(this).attr("id").substring(0, $(this).attr("id").indexOf("_"));
                    depsChecked += categ + "|";
                });
                if (depsChecked) {
                    depsChecked = depsChecked.substring(0, depsChecked.length - 1);
                    url += "&optPerm=" + depsChecked;
                    var objsFlags = "allow";
                    if (anyDeny) objsFlags = "deny";

                    if ($("#radiodeps_0").prop("checked") || $("#radiodeps_1").prop("checked")) objsFlags += "|down";
                    else objsFlags += "|up";

                    objsFlags += "|active";
                    url += "&optFlag=" + objsFlags;
                }
            }
            
            if(warnUser)
                this._warnUser(url);
            else{
              linkCmd(url);
              alert("Done!");
              window.location.reload();
            }
        },
        
        _onDelete: function () {

            var allSelectedNodes = this._updateTreeInfo();
            var url = "permset&ids=" + docLocValue("ids");

            var curUser = decodeURIComponent(window.getCookie("last_login"));
            var warnUser = false;
            var groups = "";
            for (var i = 0; i < allSelectedNodes.length; i++) {
                if(this._findChild(allSelectedNodes[i], curUser, "email")) warnUser = true;
                groups += allSelectedNodes[i].treeid + ",";
            }
            if (groups) {
                groups = groups.substring(0, groups.length - 1);
                url += "&groups=" + groups;
            }
            url += "&perm=&flag=allow|down";

            if(warnUser)
                this._warnUser(url);
            else{
              linkCmd(url);
              alert("Done!");
              window.location.reload();
            }
        },
        
        _warnUser: function(url){
            $("body").append($(document.createElement("div"))
                .addClass("delete modal fade")
                .attr("id", "warnModal")
                .attr("role", "dialog")
                .append($(document.createElement("div"))
                    .addClass("modal-dialog")
                    .append($(document.createElement("div"))
                        .addClass("modal-content")
                        .append($(document.createElement("div"))
                            .addClass("modal-header")
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("close")
                                .attr("data-dismiss", "modal")
                                .text("×")
                            )
                            .append($(document.createElement("h4"))
                                .addClass("modal-title")
                                .append($(document.createElement("strong"))
                                    .text("Changing your own permissions")
                                )
                            )
                        )
                        .append($(document.createElement("div"))
                            .addClass("modal-body")
                            .append($(document.createElement("p"))
                                .html("You are <strong>changing</strong> your own permissions.")
                            )
                            .append($(document.createElement("p"))
                                .text("Do you want to continue?")
                            )
                        )
                        .append($(document.createElement("div"))
                            .addClass("modal-footer")
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("btn btn-third")
                                .attr("data-dismiss", "modal")
                                .text("NO")
                            )
                            .append($(document.createElement("button"))
                                .attr("type", "button")
                                .addClass("btn btn-secondary red_btn")
                                .attr("data-dismiss", "modal")
                                .attr("mmodal-button", "delete")
                                .text("YES")
                                .on("click", function(){
                                    linkCmd(url);
                                    alert("Done!");
                                    window.location.reload();
                                })
                            )
                        )
                    )
                )
            );
            
            $("#warnModal").modal("show");
        },
        
        _findChild: function(where, what, how){
            if(where[how] == what) return true;
            
            for(var i = 0; i < where.children.length; i++){
                if(this._findChild(where.children[i], what, how)) return true;
            }
            return false;
        }
    });

}(jQuery));
