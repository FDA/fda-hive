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
function vjHoneyComb ()
{
    this.ok=true;


    this.recordViewNode=function(viewer,node)
    {
        if(!viewer.vjHCAssociatedRecordViewer)return ;
        var rv=vjDV.locate(viewer.vjHCAssociatedRecordViewer);
        if(!rv)return ;
        var dsSpecName=rv.data[0];
        var dsRecordName=rv.data[1];

        if(!node){
            vjDS[dsSpecName].url="static:
            vjDS[dsRecordName].url="static:
            vjDS[dsSpecName].load();
            vjDS[dsRecordName].load();
            return ;
        }

        if(node._type)rv.objType=node._type;
        else if(viewer.objType) rv.objType=viewer.objType;

        if(node.id)rv.hiveId=node.id;
        else rv.hiveId=viewer.objType;

        vjDS[dsSpecName].url="http:
        vjDS[dsRecordName].url="http:
        vjDS[dsSpecName].load();
        vjDS[dsRecordName].load();
    };



    this.permissionComputer=function(view, node, perm_key)
    {
        perm_key = perm_key || "_perm";
        if(!node[perm_key])return;
        if( typeof(node[perm_key][0]) !='string')return;

        var perm=new Object({group:new Array(),bits:new Array(),flag:new Array(),view:new Array() } );
        node[perm_key]=verarr(node[perm_key]);
        for(var ip=0;ip<node[perm_key].length;++ip){
            var spl=node[perm_key][ip].split(',');
            perm.group.push(spl[0]);
            perm.view.push(spl[1]);

            var bt=new Object();
            var btarr=spl[2].split("|");
            for (var ib=0;ib<btarr.length; ++ib){
                if(!bt[btarr[ib]])bt[btarr[ib]]=1;
                else ++bt[btarr[ib]];
            }
            perm.bits.push(bt);

            bt=new Object();
            btarr=spl[3].split("|");
            for (var ib=0;ib<btarr.length; ++ib){
                if(!bt[btarr[ib]])bt[btarr[ib]]=1;
                else ++bt[btarr[ib]];
            }
            perm.flag.push(bt);
        }
        node[perm_key]=perm;

    };

    this.permissionAccumulator = function(permAr, permTot, grpfilt) {
        if(!permTot)
            permTot = new Object({
                group : new Array(),
                bits : new Object(),
                flag : new Object(),
                view : new Array()
            });
        for(var ip = 0; ip < permAr.length; ++ip) {
            if(permAr[ip] && permAr[ip].group) {
                for(var ix = 0; ix < permAr[ip].group.length; ++ix) {
                    if(grpfilt) {
                        for(var ig = 0; ig < grpfilt.length && permAr[ip].group[ix] != grpfilt[ig]; ++ig)
                            ;
                        if(ig >= grpfilt.length) {
                            continue;
                        }
                    }
                    permTot.group.push(permAr[ip].group[ix]);
                    permTot.view.push(permAr[ip].view[ix]);
                    for(ff in permAr[ip].bits[ix]) {
                        if(!permTot.bits[ff]) {
                            permTot.bits[ff] = 0;
                        }
                        permTot.bits[ff] += permAr[ip].bits[ix][ff];
                    }
                    for(ff in permAr[ip].flag[ix]) {
                        if(!permTot.flag[ff]) {
                            permTot.flag[ff] = 0;
                        }
                        permTot.flag[ff] += permAr[ip].flag[ix][ff];
                    }
                }
            }
        }
        return permTot;
    }
}


var vjHC=new vjHoneyComb();

function vjUserShareTreeSource(sourcebase)
{
    vjDataSource.call(this, sourcebase);

    this.isUserShareTreeSource = true;
    this.tree = null;

    this.findTree = null;

    this.load=function() {
        if (this.state == "done" || this.state == "err") {
            return true;
        }
        if (this.findTree && !this.tree) {
            this.tree = this.findTree();
        }
        this.load_callback();
        return true;
    }

    this.unload=function() {
        this.tree = null;
        this.state = "";
    }

    this.load_callback = function() {
        if (this.tree) {
            this.call_callbacks("arrived");
            this.state = "done";
            this.timeDone=new Date();
            this.call_refresh_callbacks();
        } else {
            this.state = "";
        }
    }
}

function vjUserShareTreeView(viewer)
{
    if (!viewer.icons) viewer.icons = {leaf: "img/user.gif"};
    if (!viewer.className) viewer.className = "permission";
    vjTreeView.call(this, viewer);

    this.data = verarr(this.data);
    for (var i=0; i<4; i++) {
        if (!this.data[i]) {
            this.data[i] = "dsVoid";
        }
    }

    if (!this.treeSourceName)
        this.treeSourceName = null;

    if (this.treeSourceName && !vjDS[this.treeSourceName]) {
        vjDS[this.treeSourceName] = new vjUserShareTreeSource(  { title: "infrastructure: User Sharing Tree", name: this.treeSourceName, url: "static:
    }

    if (this.permsInTitle === undefined) this.permsInTitle = false;
    if (this.hideNonPermitted === undefined) this.hideNonPermitted = false;
    if (this.highlightAnyFlag === undefined) this.highlightAnyFlag = false;

    this.showRoot = 1;
    this.showLeaf = true;
    this.showChildrenCount = true;
    this.autoexpand = 1;
    this.iconSize = 18;
    this.multiSelect = true;

    this.precompute = function(view, tbl, ir) {
        var node = tbl.rows[ir];
        node.title = node.name;
        delete node.name;
        if (node.path && /\/users\/$/.test(node.path)) {
            node.expanded = true;
        }
    };

    this.composerFunction = function() {
        if (this.treeSourceName) {
            vjDS[this.treeSourceName].unload();
        }

        this.tree.empty();

        this.parseContent(this.getData(0).data);

        this.maintainPreviousData=false;

        var email2primary_grp = {};
        this.tree.enumerate(function(view, node) {
            if (node.path) {
                var mtch = node.path.match(/\/users\/(.+)/);
                if (mtch && mtch[1] && mtch[1].length) {
                    node.email = mtch[1];
                    email2primary_grp[node.email] = node;
                }
            }
        }, this);

        var user_tbl = new vjTable(this.getData(1).data, 0, vjTable_propCSV);
        user_tbl.enumerate(function(view, tbl, ir) {
            var row = tbl.rows[ir];
            var elts = row.path.split("/");
            if (elts.length) {
                var email = elts.pop();
                var grp_path = elts.join("/") + "/";
                var grp_node = view.tree.findByPath(grp_path);
                if (grp_node && grp_node.path == grp_path && email2primary_grp[email]) {
                    if (!grp_node.users) {
                        grp_node.users = [];
                    }
                    grp_node.users.push(email2primary_grp[email]);
                }
            }
        }, this);

        var obj_tbl = new vjTable(this.getData(2).data, 0, vjTable_propCSV);
        var dep_tbl = new vjTable(this.getData(3).data, 0, vjTable_propCSV);
        var perms = {};
        var num_perms = 0;

        var viewer = this;

        function get_flag_key(flag) {
            var flag_names = [], flag_name;
            for (flag_name in flag) {
                if (flag[flag_name]) {
                    flag_names.push(flag_name);
                }
            }
            flag_names.sort();
            return flag_names.join(","); 
        }

        function set_bits(dst, obj_id, bits, flag, flag_key, inherited) {
            var bit_name, flag_name;

            if (!dst._perm) {
                dst._perm = {};
            }
            if (!dst._perm[obj_id]) {
                dst._perm[obj_id] = {
                    effective_bits: {},
                    flag_bits: {}
                };
            }
            if (!inherited) {
                dst._perm[obj_id].direct_flags = {};
                dst._perm[obj_id].direct_bits = {};
                for (flag_name in flag) {
                    dst._perm[obj_id].direct_flags[flag_name] = true;
                }
                for (bit_name in bits) {
                    if (bits[bit_name]) {
                        dst._perm[obj_id].direct_bits[bit_name] = true;
                    }
                }
            }

            if (!dst._perm[obj_id].flag_bits[flag_key]) {
                dst._perm[obj_id].flag_bits[flag_key] = {};
            }
            for (bit_name in bits) {
                if (bits[bit_name]) {
                    dst._perm[obj_id].flag_bits[flag_key][bit_name] = true;
                }
            }

            if (!flag.hold) {
                for (bit_name in bits) {
                    if (bits[bit_name]) {
                        if (flag.deny) {
                            delete dst._perm[obj_id].effective_bits[bit_name];
                        } else {
                            dst._perm[obj_id].effective_bits[bit_name] = true;
                        }
                    }
                }
            }

            if (dst.users) {
                dst.users.forEach(function(user) {
                    set_bits(user, obj_id, bits, flag, flag_key, true);
                });
            }
        }

        function set_perm(grp_id, obj_id, bits, flag, flag_key) {
            var node = viewer.tree.findByAttribute("id", grp_id);
            if (node && node.id == grp_id) {
                set_bits(node, obj_id, bits, flag, flag_key, false);
                if (flag.up) {
                    for (var par = node.parent; par; par = par.parent) {
                        set_bits(par, obj_id, bits, flag, flag_key, true);
                    }
                } else if (flag.down) {
                    this.enumerate(function(view, chld) {
                        if (chld != node) {
                            set_bits(chld, obj_id, bits, flag, flag_key, true);
                        }
                    }, this, undefined, undefined, node);
                }
            }
        }

        function enum_set_perm(deny, tbl, ir) {
            var obj_node = tbl.rows[ir];
            if (!obj_node.id) {
                return;
            }
            vjHC.permissionComputer(viewer, obj_node);
            if (obj_node._perm && obj_node._perm.group) {
                for(var i=0; i<obj_node._perm.group.length; i++) {
                    var grp_id = obj_node._perm.group[i];
                    var bits = obj_node._perm.bits[i];
                    var flag = obj_node._perm.flag[i];
                    if ((flag.deny && deny) || (!flag.deny && !deny)) {
                        set_perm.call(viewer, grp_id, obj_node.id, bits, flag, get_flag_key(flag));
                    }
                }
            }
        }

        obj_tbl.enumerate(enum_set_perm);
        dep_tbl.enumerate(enum_set_perm);
        obj_tbl.enumerate(enum_set_perm, true);
        dep_tbl.enumerate(enum_set_perm, true);

        this.resetSelected(obj_tbl.accumulate(function(node) { return node.id }, function(node) { return node.id }));

        if (this.treeSourceName) {
            var this_ = this;
            vjDS[this.treeSourceName].findTree = function() { return this_.tree; };
            vjDS[this.treeSourceName].load();
        }

        this.tree.enumerate(function(view, node) {
            if (node.leafnode) {
                node.icon = null;
            } else {
                node.icon = "img/user-organization.gif";
            }
        }, this);

        if(this.postcompute)
            this.tree.enumerate(this.postcompute, this);
        if (this.doNotDraw){
            this.objTbl = obj_tbl;
            this.depTbl = dep_tbl;
            return;
        }
        this.refresh();
    };

    this.resetSelected = function(objIdList) {
        objIdList = verarr(objIdList);

        (function set_selected(node) {
            var visible_cnt = 0;
            var bits = [];
            var has_direct = false;
            if (node._perm) {
                var flag_key, bit_name, obj_id;
                for (var i = 0; i < objIdList.length; i++) {
                    var obj_id = objIdList[i];
                    if (node._perm[obj_id]) {
                        if (node._perm[obj_id].direct_bits) {
                            has_direct = true;
                        }
                        if (this.highlightAnyFlag) {
                            for (flag_key in node._perm[obj_id].flag_bits) {
                                for (bit_name in node._perm[obj_id].flag_bits[flag_key]) {
                                    if (node._perm[obj_id].flag_bits[flag_key][bit_name]) {
                                        bits.push(bit_name);
                                        visible_cnt = 1;
                                    }
                                }
                            }
                        } else {
                            for (bit_name in node._perm[obj_id].effective_bits) {
                                if (node._perm[obj_id].effective_bits[bit_name]) {
                                    bits.push(bit_name);
                                    visible_cnt = 1;
                                }
                            }
                        }
                    }
                }
            }
            node.children.forEach(function(node) {
                visible_cnt += set_selected(node);
            });
            if (bits.length || visible_cnt) {
                node.hidden = false;
                node.expanded = true;
                if (bits.length) {
                    if (has_direct) {
                        node.selected = 1;
                    } else {
                        node.selected = 2;
                    }
                    if (viewer.permsInTitle) {
                        node.title += " : can " + bits.sort().join(", ");
                    }
                }
            } else {
                if (viewer.hideNonPermitted) {
                    node.hidden = true;
                }
                node.expanded = false;
                node.selected = 0;
            }
            return visible_cnt;
        })(this.tree.root);
    };
}

function vjUserShareTreeColorView(viewer)
{
    vjHTMLView.call(this, viewer);
    this.data = [ "ds_vjUserShareTreeColorView" ];
    if (!vjDS[this.data[0]]) {
        vjDS.add("infrastructure: Retrieving User/Group Color Key", this.data[0], "static:
            "<table class='HIVE_variable'>" +
                "<tr>" +
                    "<td class='permission_selected1'>Have direct permissions</td>" +
                    "<td class='permission_selected2'>Only inherited permissions</td>" +
                "</tr>" +
            "</table>");
    }
}

