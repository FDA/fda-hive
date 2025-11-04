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

function valgodmSNVDisCore(cls, loadedID, formname) {
    this.vjDS = vjDS;
    this.vjDV = vjDV;

    this.loadedID = loadedID;
    this.formName = formname;

    this.objCls = "dmSnpdis-" + cls + "-" + Math.random();
    vjObj.register(this.objCls, this);


    this.callbackLoaded = null;

};


function valgodmSNVDisAnalysis(loadedID, formname) {
    this.snpset = "Snpset";
    this.proteome = "Proteome";
    this.analysis = "Analysis";
    this.run = "Submitter";
    this.prm="Parameters";
    this.selectedHitListCallback = null;
    this.callbackSubmitComputation = null;
    this.callbackSubmitedComputation = null;

    valgodmSNVDisCore.call(this, "input", loadedID, formname);

    this.algoProcViewer = null;

    this.uploadFileBody=function(id,name,header){
        var t='<table><tr><td><i>'+header+'</i></td></tr>'+
            '<tr><td><input style="width:200px;" id="'+sanitizeElementId(id)+'" type="file" name="'+sanitizeElementId(name)+'" /><br/><br/></td></tr>'+
            '</table>';
        return t;
    };
    this.uploadTextBody=function(id,name,header){
        var t='<table><tr><td><i>'+header+'</i></td></tr>'+
            '<tr><td><textarea rows="3" cols="60" id="' + sanitizeElementId(id) + '" value=""  name="' + sanitizeElementId(name) + '" ></textarea></td></tr></table>';
        return t;
    };
    this.pValBody=function(id,name,header){
        var t=header+'<input id="'+sanitizeElementId(id)+'" style="width:95px;" type="text" name="'+sanitizeElementId(name)+'" value="">'+
        '<i style="font-size:80%;">e.g. 0.0000001 </i><br/><br/><br/><br/>';
        return t;
    };
    this.specInputBody=function(id,name,header){
        var t=header+"<input class='wickEnabled' id='"+sanitizeElementId(id)+"' type='text' name='"+sanitizeElementId(name)+"' />"+
        "<i>e.g. PF00870</i><br/><br/><br/><br/>";
        return t;
    };

    this.generate = function () {
        var hidIn='<input type=hidden name="svc" id="svc" value="dmSnpdis" />'+
        '<input type=hidden name="cmd" id="cmd" value="dmSnpdis" />'+
        '<input type=hidden name="type" id="type" value="" />'+
        '<input type=hidden name="input" id="input" value="" />'+
        '<input type=hidden name="snpset" id="snpset" value="" />'+
        '<input type=hidden name="proteome" id="proteome" value="" />'+
        '<input type=hidden name="pval" id="pval" value="" />'+
        '<input type=hidden name="elemName" id="elemName" value="" />'+
        '<input type=hidden name="siteset" id="siteset" value="" />'+
        '<input type=hidden name="bind_source" id="bind_source" value="" />'+
        '<input type=hidden name="cdd_bind_type" id="cdd_bind_type" value="" />'+
        '<input type=hidden name="uni_bind_type" id="uni_bind_type" value="" />'+
        '<input type=hidden name="act_source" id="act_source" value="" />'+
        '<input type=hidden name="nlinked_type" id="nlinked_type" value="" />';
        var snplist = "UniProtKB, date of access 06/28/2012,www.uniprot.org/manual/variant\n" +
        "dbSNP, build 135,www.ncbi.nlm.nih.gov/projects/SNP/\n" +
        "COSMIC, version 59,www.sanger.ac.uk/genetics/CGP/cosmic/\n" +
        "NCI-60, -,www.sanger.ac.uk/genetics/CGP/NCI60/";
        var proteomlist = "UniProtKB/Swiss-Prot, date of access 06/28/2012,20228,www.uniprot.org/uniprot/?query=organism%3A9606+AND+reviewed%3Ayes+AND+keyword%3A181";
        vjDS.add("dsSnpsets", "static:
        vjDS.add("dsProteomeSet", "static:
        vjDS.add("dsHelp", "innerHTML:
        vjDS.add("dsHidIn", "static:
        vjDS.add("dsUserTextProteom", "static:
        vjDS.add("dsUserFileProteom", "static:

        var helpViewer = new vjHTMLView({
            data: 'dsHelp',
            geometry: { width: "100%" },
            isok: true
        });

        var snvSetsViewer = new vjTableView({
            icon: 'list',
            data: 'dsSnpsets',
            name: 'SNV sets',
            formObject: document.forms[formname],
            cols: [{ name: new RegExp(/.*/), hidden: false },
                    { name: new RegExp(/.*Link.*/), hidden: false, url: this.dmSnpdis_linkTosource}],
            checkable: true,
            multiSelect: true,
            bgColors: ['#f2f2f2', '#ffffff'],
            styleColors: ['#AAAAAA'],
            geometry: { width: '100%' },
            selectCallback:"function:vjObjFunc('onSelectedSNVs','"+this.objCls+"')",
            isok: true
        });

        var proteomeSetsViewer = new vjTableView({
            icon: 'list',
            data: 'dsProteomeSet',
            name: 'set',
            formObject: document.forms[formname],
            cols: [{ name: new RegExp(/.*/), hidden: false },
                    { name: new RegExp(/.*Link.*/), hidden: false, url: this.dmSnpdis_linkTosource}],
            checkable: true,
            multiSelect: true,
            bgColors: ['#f2f2f2', '#ffffff'],
            styleColors: ['#AAAAAA'],
            geometry: { width: '100%' },
            selectCallback:"function:vjObjFunc('onSelectedUserProt','"+this.objCls+"')",
            proteomeSetsViewer:maxTxtLen= 24,
            isok: true
        });

        var uploadTextProteomeViewer = new vjHTMLView({
            data: 'dsUserTextProteom',
            name: 'text',
            icon: 'keyboard',
            hidden:false,
            formObject: document.forms[formname],
            selectCallback:"function:vjObjFunc('onSelectedUserProt','"+this.objCls+"')",
            isok: true
        });
        var uploadFileProteomeViewer = new vjHTMLView({
            data: 'dsUserFileProteom',
            name: 'file',
            icon: 'list',
            hidden:true,
            formObject: document.forms[formname],
            selectCallback:"function:vjObjFunc('onSelectedUserProt','"+this.objCls+"')",
            isok: true
        });
        var hidInViewer = new vjHTMLView({
            data:'dsHidIn',
            hidden:true,
        });
        var dv="dv"+this.snpset;
        vjDV.add(dv,"100%", 380).selected = 1;
        vjDV[dv].add("help", 0, "tab", [helpViewer]);
        vjDV[dv].add("nsSNV sources", "table", "tab", [snvSetsViewer]);
        vjDV[dv].render();
        vjDV[dv].load();
        var vjSnp = vjDV[dv].tabs[1].viewers[0];
        vjSnp.enumerate('node.checked=true;');
        vjSnp.refresh();
        vjSnp.onCheckmarkSelected(vjSnp.formObject.elements[1].name, vjSnp.formObject, 0);

        dv="dv"+this.proteome;
        vjDV.add(dv, "100%", 380).selected = 1;
        vjDV[dv].add("help", 0, "tab", [helpViewer]);
        vjDV[dv].add("Proteome Sources", "table", "tab", [proteomeSetsViewer]);
        vjDV[dv].add("Upload proteome", "upload", "tab", [uploadTextProteomeViewer, uploadFileProteomeViewer]).viewtoggles = -1;
        vjDV[dv].render();
        vjDV[dv].load();
        var vjPrt=vjDV[dv].tabs[1].viewers[0];
        vjPrt.enumerate('node.checked=true;');
        vjPrt.refresh();
        vjPrt.onCheckmarkSelected(vjPrt.formObject.elements[1].name, vjPrt.formObject,0);


        var sitetree="Name,hierarchy\nbinding sites,/\nUniProt,/binding sites/\nCa binding,/binding sites/UniProt/\nChemical group binding,/binding sites/UniProt/\n"+
        "DNA binding,/binding sites/UniProt/\nNP binding,/binding sites/UniProt/\nCDD,/binding sites/\n2Fe-2S,/binding sites/CDD/\nacetylation,/binding sites/CDD/\n"+
        "binding(surface),/binding sites/CDD/\nchemical binding,/binding sites/CDD/\ncleavage,/binding sites/CDD/\nDNA binding,/binding sites/CDD/\n"+
        "glycosylation,/binding sites/CDD/\ninhibition,/binding sites/CDD/\nion binding,/binding sites/CDD/\nlipid-binding,/binding sites/CDD/\n"+
        "metal-binding,/binding sites/CDD/\nmodified,/binding sites/CDD/\nnucleotide binding,/binding sites/CDD/\npolypeptide binding,/binding sites/CDD/\n"+
        "posttranslational modification,/binding sites/CDD/\nother,/binding sites/CDD/\nactive sites,/\nUniProt,/active sites/\nCDD,/active sites/\nN-linked glycosylation sites,/\n"+
        "N-linked glycosylation loss,/N-linked glycosylation sites/\nnot affecting the site,/N-linked glycosylation sites/";
        vjDS.add("dsSites", "static:
        vjDS.add("dsPathNames", "http:
        vjDS.add("dsDomSpec", "static:
        vjDS.add("dsDomPval", "static:
        vjDS.add("dsPathPval", "static:


        vjDS.add("dsDomains", "static:
        vjDS.add("dsProteinSets", "static:
        vjDS.add("dsUserTextDomains", "static:
        vjDS.add("dsUserTextProteinSets", "static:
        vjDS.add("dsUserFileDomains", "static:
        vjDS.add("dsUserFileProteinSets", "static:

        vjDS.add("dsHelp", "innerHTML:

        var helpViewer = new vjHTMLView({
            data: 'dsHelp',
            geometry: { width: "100%" },
            isok: true
        });

        var siteTreeViewer = new vjTreeView({
            name: 'hierarchy',
            icon: 'tree',
            data: 'dsSites',
            hierarchyColumn: 'hierarchy',
            showRoot: false,
            showLeaf: true,
            checkLeafs: true,
            checkBranches: true,
            hideEmpty: true,
            showChildrenCount: true,
            hideEmpty: false,
            formObject: document.forms[formname],
            checkPropagate: true,
            selectedNode: '/',
            precompute: "row.path=(row.hierarchy ? row.hierarchy : '/') +row['Name'];",
            postcompute: "node.checked=true",
            checkCallback:"function:vjObjFunc('onSelectedSites','"+this.objCls+"')",
            isok: true
        });

        var domPvalViewer = new vjHTMLView({
            data: 'dsDomPval',
            name: 'by significance',
            icon: 'graph',
            hidden: false,
            selectCallback:"function:vjObjFunc('onSelectedDomains','"+this.objCls+"')",
            formObject: document.forms[formname],
            isok    : true
        });

        var domSpecViewer = new vjHTMLView({
            data: 'dsDomSpec',
            name: 'by Name',
            icon: 'recLock',
            hidden:true,
            selectCallback:"function:vjObjFunc('onSelectedDomains','"+this.objCls+"')",
            formObject: document.forms[formname],
            isok: true
        });

        var pathSpecViewer = new vjTableView({
            data    : 'dsPathNames',
            name: 'by Name',
            icon: 'table',
            hidden: false,
            bgColors: ['#f2f2f2', '#ffffff'],
            formObject: document.forms[formname],
            selectCallback:"function:vjObjFunc('onSelectedProtSet','"+this.objCls+"')",
            multiSelect: false,
            isok: true
        });
        var pathPvalViewer = new vjHTMLView({
            data: 'dsPathPval',
            name: 'by significance',
            icon: 'graph',
            selectCallback:"function:vjObjFunc('onSelectedProtSet','"+this.objCls+"')",
            formObject: document.forms[formname],
            hidden:true,
            isok: true
        });

        var userTextDomsViewer = new vjHTMLView({
            data: 'dsUserTextDomains',
            name: 'text',
            icon: 'keyboard',
            hidden:true,
            formObject: document.forms[formname],
            selectCallback:"function:vjObjFunc('onSelectedUserDoms','"+this.objCls+"')",
            geometry: { width: "100%" },
            isok: true
        });
        var userFileDomsViewer = new vjHTMLView({
            data: 'dsUserFileDomains',
            name: 'file',
            icon: 'list',
            hidden: false,
            selectCallback:"function:vjObjFunc('onSelectedUserDoms','"+this.objCls+"')",
            formObject: document.forms[formname],
            geometry: { width: "100%" },
            isok: true
        });

        var userTextPathViewer = new vjHTMLView({
            data: 'dsUserTextProteinSets',
            name: 'text',
            icon: 'keyboard',
            hidden: true,
            selectCallback:"function:vjObjFunc('onSelectedUserPaths','"+this.objCls+"')",
            formObject: document.forms[formname],
            geometry: { width: "100%" },
            isok: true
        });
        var userFilePathViewer = new vjHTMLView({
            data: 'dsUserFileProteinSets',
            name: 'file',
            icon: 'list',
            hidden: false,
            selectCallback:"function:vjObjFunc('onSelectedUserPaths','"+this.objCls+"')",
            formObject: document.forms[formname],
            geometry: { width: "100%" },
            isok: true
        });


        dv = "dv"+this.analysis;
        vjDV.add(dv,"100%", 300).selected = 1;
        vjDV[dv].add("help", 0, "tab", [helpViewer]);
        vjDV[dv].add("Sites", "actsites", "tab", [siteTreeViewer]);
        vjDV[dv].add("Domains", "domsites", "tab", [ domSpecViewer, domPvalViewer]).viewtoggles = -1;
        vjDV[dv].add("Upload custom domains", "upload", "tab", [ userTextDomsViewer, userFileDomsViewer]).viewtoggles = -1;
        vjDV[dv].add("Protein sets", "protpaths", "tab", [ pathSpecViewer, pathPvalViewer]).viewtoggles = -1;
        vjDV[dv].add("Upload protein sets", "upload", "tab", [ userTextPathViewer, userFilePathViewer]).viewtoggles = -1;
        vjDV[dv].render();
        vjDV[dv].load();

        dv="dv"+this.prm;
        this.vjDS.add( "ds"+this.prm+"Vals" ,"http:
        this.vjDS.add( "ds"+this.prm+"Spec" ,"http:
        var viewerParams=new vjRecordView( {
                    data: this.loadedID ? ["ds"+this.prm+"Spec", "ds"+this.prm+"Vals" ] : [ "ds"+this.prm+"Spec"],
                    formObject:document.forms[this.formName],
                    objID: this.loadedID ? this.loadedID : "svc-dmSnvDis" ,
                    showRoot: true,
                    autoStatus:3,
                    autoDescription:0,
                    RVtag:"RV",
                    objType: "svc-seqalign",
                    cmdPropSet:"dna.cgi?cmd=-qpProcSubmit&svc=dmSnpdis",
                    accumulateWithNonModified:true,
                    constructionPropagateDown:2,
                    autoexpand:0,
                    showRoot:false,
                    hideControls:true,
                    autoDescription:0,
                    readonlyMode: this.loadedID ? true : false ,
                    callbackRendered :"function:vjObjFunc('onParamsRecordLoaded','"+this.objCls+"')",
                    isok:true});

        vjDV.add(dv,"100%",300);
        this.vjDV[dv].add("parameters","dna","tab", [ viewerParams ] );
        vjDV[dv].render();
        vjDV[dv].load();

        dv="dv"+this.run;
        o=gObject( dv) ;
        if(o){o.innerHTML=""+
            "<input type=button onClick='vjObjEvent(\"onSumbitComputation\",\""+this.objCls+"\")' name='BUTTON-"+dv+"_submitter' style='width:50;height:10' value='    SUBMIT!    ' />"+
            "";
            o.className="sectVis";
        }
    };
    this.dmSnpdis_linkTosource=function(vv, tbl, ir, ic) {
        var tmpurl = "http:
        window.open(tmpurl);
    };

    this.onSumbitComputation=function()
    {
        if(this.callbackSubmitComputation)
            funcLink(this.callbackSubmitComputation,this,this.algoProcViewer);
        else
            this.algoProcViewer.setValues(null, true, "function:vjObjFunc('onRedirectProcSubmitted','"+this.objCls+"')");
    };

    this.onRedirectProcSubmitted=function (param, text )
    {
        var reqID=0, objID=0;
        var nums=isok(text) ? text.split(",") : new Array();
        if(nums.length>=2) {
               reqID=parseInt(nums[0]);
               objID=parseInt(nums[1]);
        }
        if(!reqID || !objID) {
            alert("Error: could not submit the computation request !\n"+text);
            return ;
        }
        this.loadedID=objID;

        if(this.callbackSubmitedComputation)
            funcLink(this.callbackSubmitedComputation,this,this.algoProcViewer);
        else
            document.location="?cmd=prot-dmSnpdis&id="+this.loadedID;
    };
    this.onParamsRecordLoaded=function( viewer, text)
    {
        this.algoProcViewer=viewer;
        if(!this.loadedID)return ;

        var vals=this.algoProcViewer.accumulate( "node.fld.name=='query'","node.value" );
        this.vjDS["ds"+this.qry+"Selected" ].reload((isok(vals) ? "http:

        vals=this.algoProcViewer.accumulate( "node.fld.name=='subject'","node.value" );
        this.vjDS["ds"+this.sub+"Selected"].reload((isok(vals) ? "http:

        if(this.prg) {
            vals=this.algoProcViewer.accumulate( "node.fld.name=='reqID'","node.value" );
            this.vjDS["ds"+this.prg].reload((isok(vals) ? "http:
        }

        if(this.callbackLoaded)
            funcLink(this.callbackLoaded,this,this.algoProcViewer);
    };

    this.onSumbitComputation=function()
    {
        var url = "";
        strcmd = "Sites";
        var dv="dv"+this.snpset;
        var snpset = vjDV[dv].tabs[1].viewers[0].accumulate("node.checked", "node.Name");
        if (snpset.length == 0) {
            alert("Please select set of SNPs");
            return;
        }
        this.algoProcViewer.changeElementValue('snp_sets',snpset,0);
        gObject("snpset").value = snpset;
        dv="dv"+this.proteome;
        if (vjDV[dv].selected == 1) {
            var proteomeset = vjDV[dv].tabs[1].viewers[0].accumulate("node.checked", "node.Source");
            if (proteomeset.length == 0) {
                alert("Please select Proteome");
                return;
            }
            this.algoProcViewer.changeElementValue('proteome_method','sets',0);
            this.algoProcViewer.changeElementValue('proteome_sets',proteomeset,0);

            gObject("proteome").value =proteomeset;
        }
        else {
            this.algoProcViewer.changeElementValue('proteome_method','up',0);
            if (vjDV[dv].tabs[2].viewers[0].hidden==false) {
                if (document.getElementById("ProteomeText").value == "") {
                    alert("Please provide Protein AC numbers as proteome");
                    return;
                }
                else{
                    gObject("proteome").value = "fromtext";
                    this.algoProcViewer.changeElementValue('proteome_up_method','text',0);
                    this.algoProcViewer.changeElementValue('proteome_up_text','text',0);
                }
            }
            else {
                if (document.getElementById("ProteomeFile").value == "") {
                    alert("Please provide a file as proteome");
                    return;
                }
                else{
                    gObject("proteome").value = "fromfile";
                    this.algoProcViewer.changeElementValue('proteome_up_method','file',0);
                    this.algoProcViewer.changeElementValue('proteome_up_file','text',0);
                }
            }
        }

        var tabViewer = vjDV["dv"+this.analysis].tabs[vjDV["dv"+this.analysis].selected];
        if (tabViewer.name == "Sites") {
            var siteset = tabViewer.viewers[0].accumulate("node.checked", "node.Name");
            this.algoProcViewer.changeElementValue('analysis','functionalSites',0);

            if (siteset.length == 1) {
                alert("Please selecte set of sites");
                return;
            }
            else {
                var rootsites = tabViewer.viewers[0].accumulate("node.checked && node.depth==1", "node.Name");
                gObject("siteset").value = rootsites;
                alert(rootsites);
                for (ii = 0; ii < rootsites.length; ++ii) {
                    if (rootsites[ii].indexOf("binding") != -1) {
                        var bind_source =tabViewer.viewers[0].accumulate("node.checked && node.depth==2 && node.parent.name=='binding sites'", "node.Name");
                        gObject("bind_source").value = bind_source;
                        var bind_source_notselected = tabViewer.viewers[0].accumulate("!node.checked && node.depth==2 && node.parent.name=='binding sites'", "node.Name");
                        if (bind_source_notselected == 0) {
                            gObject("bind_source").value ="all";
                        }
                        for (jj = 0; jj < bind_source; ++jj) {
                            if (bind_source[jj].indexOf("UniProt") != -1) {
                                var bind_uni_notselected = tabViewer.viewers[0].accumulate("!node.checked && node.depth==3 && node.parent.name=='UniProt'", "node.Name");
                                if (bind_uni_notselected.length != 0) {
                                    var uni_bind_type =tabViewer.viewers[0].accumulate("node.checked && node.depth==3 && node.parent.name=='UniProt'", "node.Name");
                                    gObject("uni_bind_type").value = uni_bind_type;
                                }
                            }
                            if (bind_source[jj].indexOf("CDD") != -1) {
                                var bind_cdd_notselected = tabViewer.viewers[0].accumulate("!node.checked && node.depth==3 && node.parent.name=='CDD'", "node.Name");
                                if (bind_cdd_notselected.length != 0) {
                                    var cdd_bind_type = tabViewers.viewers[0].accumulate("node.checked && node.depth==3 && node.parent.name=='CDD'", "node.Name");
                                    gObject("cdd_bind_type").value = cdd_bind_type;
                                }
                            }
                        }
                    }
                    if (rootsites[ii].indexOf("active") != -1) {
                        var act_source = tabViewer.viewers[0].accumulate("node.checked && node.depth==2 && node.parent.name=='active sites'", "node.Name");
                        gObject("act_source").value = act_source;
                    }
                    if (rootsites[ii].indexOf("N-linked") != -1) {
                        var nlinked_type = tabViewer.viewers[0].accumulate("node.checked && node.depth==2 && node.parent.name=='N-linked glycosylation sites'", "node.Name");
                        gObject("nlinked_type").value = nlinked_type;
                    }
                }
            }

            gObject("cmd").value = "FunctionalSites";
        }
        else if (tabViewer.name == "Domains") {
            this.algoProcViewer.changeElementValue('analysis','domains',0);
            if (tabViewer.viewers[0].hidden == false) {
                gObject("type").value = "specific";
                var dmname = document.getElementById("domain_name").value;
                if (dmname == "" || !dmname) {
                    alert("Please provide name or Pfam ID of the domain you interested in");
                    return;
                }
                gObject("elemName").value = dmname;
            }
            else if (tabViewer.viewers[1].hidden == false) {
                gObject("type").value = "significance";
                var pval = document.getElementById("domPval").value;
                if (pval == "" || !pval) {
                    alert("Please provide p-value cutoff");
                    return;
                }
                else if (isNaN(pval)) {
                    alert("Please numerical values");
                    return;
                }
                else if (pval > 1 || pval < 0) {
                    alert("p-value within the range of [0,1]");
                    return;
                }
                gObject("pval").value = pval;
            }

            gObject("cmd").value = "Domains";
        }
        else if (tabViewer.name == "Protein sets") {
            this.algoProcViewer.changeElementValue('analysis','protein_sets',0);
            if (tabViewer.viewers[0].hidden == false) {
                gObject("type").value = "specific";
                var pthname = tabViewer.viewers[0].accumulate("node.selected", "node.Name");
                if (pthname == "" || !pthname) {
                    alert("Please selected one of the listed pathways");
                    return;
                }
                gObject("elemName").value = pthname;
            }
            else if (tabViewer.viewers[1].hidden == false) {
                gObject("type").value = "significance";
                var pval =  document.getElementById("pathPval").value;
                if (pval == "" || !pval) {
                    alert("Please provide p-value cutoff");
                    return;
                }
                else if (isNaN(pval)) {
                    alert("Please numerical values");
                    return;
                }
                else if (pval > 1 || pval < 0) {
                    alert("p-value within the range of [0,1]");
                    return;
                }
                gObject("pval").value = pval;
            }
            gObject("cmd").value = "ProteinSets";
        }
        else if (tabViewer.name == "Upload custom domains") {
            this.algoProcViewer.changeElementValue('analysis','customdomains',0);
            if (tabViewer.viewers[0].hidden == false) {
                if (document.getElementById("dom_text").value == "") {
                    alert("Please provide protein ranges");
                    return;
                }
                gObject("input").value = "dom_text";
            }
            else if (tabViewer.viewers[1].hidden == false) {
                if (document.getElementById("dom_file").value == "") {
                    alert("Please provide a file");
                    return;
                }

                gObject("input").value = "dom_file";
            }
            gObject("cmd").value = "CustomDomains";
        }
        else if (tabViewer.name == "Upload protein sets") {
            this.algoProcViewer.changeElementValue('analysis','customProteinsSets',0);
            if (tabViewer.activeView == 0) {
                if (document.getElementById("path_text").value == "") {
                    alert("Please provide protein sets");
                    return;
                }

                gObject("input").value = "path_text";
            }
            else if (tabViewer.viewers[1].hidden == false) {
                if (document.getElementById("path_file").value == "") {
                    alert("Please provide a file");
                    return;
                }
                gObject("input").value = "path_file";
            }
            gObject("cmd").value = "CustomProteinSets";
        }
        else {
            alert("ERROR: UNEXPECTED");
        }
        if(this.callbackSubmitComputation)
            funcLink(this.callbackSubmitComputation,this,this.algoProcViewer);
        else
            this.algoProcViewer.setValues(null, true, "function:vjObjFunc('onRedirectProcSubmitted','"+this.objCls+"')");
    }
}

function valgodmSNVDisProgress(loadedID, formname) {
    this.prg = "Progress";
    this.callbackDoneComputing = null;

    valgodmSNVDisCore.call(this, "progress", loadedID, formname);

    this.generate = function () {
        this.vjDS.add("ds" + this.prg, "static:
        this.vjDV.add("dv" + this.prg + "Viewer", 400, 300).frame = 'notab';
        vjPAGE.initStandardProgressViewer("dv" + this.prg + "Viewer", "ds" + this.prg, this.formname, this.callbackDoneComputing, true);
        this.vjDV["dv" + this.prg + "Viewer"].render();
        this.vjDV["dv" + this.prg + "Viewer"].load();
    }

}
function valgoSNVDisHitList(loadedID, formname) {
    this.generate=function (){
    }
}
