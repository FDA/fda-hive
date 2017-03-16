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

function valgoHexagonAligner(loadedID, formname, dvname) {

    //this.selectedAlgorithm=null;

    valgoCore.call(this, "aligner", loadedID, formname, dvname);

    this.algoProc = null;
    this.callbackModified = null;

    this.dsAlignmentAlgorithmChoice='dsAlignmentAlgorithmChoice';

    this.generate = function (preloadedQryIDs) {

        this.vjDS.add("infrastructure: Query help document", "dsHelpQuerySequences", "http://help/hlp.view.sequence.reads.html");
        this.vjDS.add("infrastructure: Subjects help document", "dsHelpReferenceGenomes", "http://help/hlp.obj.referenceGenome.html");
        this.vjDS.add("infrastructure: Algorithms help document", "dsHelpAlgorithms", "http://help/hlp.algo.alignmentAlgorithm.html");


        var dv = this.dvname + "Query" + "BriefViewer";
        this.vjDV.add(dv, 400, 100).frame = 'notab';
        this.vjDS.add("Prepating Query Sequences Subset","ds" + this.dvname + "Query" + "Selected", "static:// ");
        vjPAGE.initShortSelectedSequenceListTab(dv, "ds" + this.dvname + "Query" + "Selected", "selected reads", "Reads", "dna");
        this.vjDV[dv].render();
        this.vjDV[dv].load();

        dv = this.dvname + "Query" + "Viewer";
        this.vjDV.add(dv, 450, 450);
        var queryview=vjPAGE.initSequenceListViewerTab(dv, "sequence reads", "dna", null, this.formName, false, "!genomic&prop_name=category", "list", "function:vjObjFunc('onCheckedSequenceItem','" + this.objCls + "')", undefined ,undefined ,undefined ,undefined ,[{name:'add',prohibit_new:true,hidden:true}]);
        if (preloadedQryIDs) queryview.callbackRendered = "function:vjObjFunc('onRenderSequenceItem','" + this.objCls + "')";
        this.vjDV[dv].add("help", "help", "tab", [new vjHelpView({ data: 'dsHelpQuerySequences' })]);
        this.vjDV[dv].render();
        this.vjDV[dv].load();


        dv = this.dvname + "Subject" + "BriefViewer";
        this.vjDV.add(dv, 400, 100).frame = 'notab';
        this.vjDS.add("Preparing Reference Sequence Subsets", "ds" + this.dvname + "Subject" + "Selected", "static:// ");
        vjPAGE.initShortSelectedSequenceListTab(dv, "ds" + this.dvname + "Subject" + "Selected", "selected reads", "Selected references", "dna");
        this.vjDV[dv].render();
        this.vjDV[dv].load();
        //this.vjDV.locate(this.dvname + "SubjectBriefViewer.selected reads.0").debugMode=true;


        dv = this.dvname + "Subject" + "Viewer";
        this.vjDV.add(dv, 450, 450);
        vjPAGE.initSequenceListViewerTab(dv, "reference genomes", "dna", null, this.formName, false, "genomic&prop_name=category", "list", "function:vjObjFunc('onCheckedSequenceItem','" + this.objCls + "')", undefined ,undefined ,undefined ,undefined ,[{name:'add',prohibit_new:true,hidden:true}]);
        this.vjDV[dv].add("help", "help", "tab", [new vjHelpView({ data: 'dsHelpReferenceGenomes' })]);
        this.vjDV[dv].render();
        this.vjDV[dv].load();

        dv = this.dvname + "Algorithm" + "Viewer";
        this.vjDV.add(dv, 550, 250);
        this.vjDS.add("Porting Algnment Algorithms", this.dsAlignmentAlgorithmChoice, "http://objects/svc-align-choice.csv");
        var viewerAlgorithmSelection = new vjTableView({
            data: 'dsAlignmentAlgorithmChoice',
            formObject: document.forms[this.formName],
            iconSize: 32,
            cols: [{ name: 'id', hidden: true }, { name: 'icon', hidden: true }, { name: 'description', wrap: true}],
            //selectCallback:"javascript:vjObjEvent('onSelectedAlignmentAlgorithm','"+this.clsObj+"',this,node)",
            selectCallback: "function:vjObjFunc('onSelectedAlignmentAlgorithm','" + this.objCls + "')",
//            inclusionObjRegex : this.alignerInclusion,
            isok: true
        });

        this.vjDV[dv].add("alignment algorithm", "process", "tab", [viewerAlgorithmSelection]);
        this.vjDV[dv].add("help", "help", "tab", [new vjHelpView({ data: 'dsHelpAlgorithms' })]);
        this.vjDV[dv].render();
        this.vjDV[dv].load();

        var o = gObject(this.dvname + "Algorithm" + "BriefViewer");
        if (o) o.innerHTML = "<img id='" + this.dvname + "Algorithm-image' src='img-algo/svc-align-hexagon.gif' height=64 border=0 />";

        //this.makeSubmitButton( "        ALIGN!        " );
    };

    this.update = function (proc) {
        this.algoProc = proc;
//        var inputQuery;
        if (this.selfUpdate) {
            this.algoProc.setValue("alignSelector", this.selectedAlgorithm);
            //this.algoProc.setValue("slice", this.selectedAlgorithm == "svc-align-hexagon" ? 200000 : 990000);
            var chunkSize = 200000;
            if (this.selectedAlgorithm == "svc-align-hexagon")    { chunkSize = 1000000;}
            else if (this.selectedAlgorithm == "svc-align-blast") { chunkSize = 4000;   }
            else if (this.selectedAlgorithm == "svc-align-bowtie"){ chunkSize = 1000000;}
            else if (this.selectedAlgorithm == "svc-align-tophat"){ chunkSize = 800000; }
            else if (this.selectedAlgorithm == "svc-align-magic") { chunkSize = 1000000;}
            else if (this.selectedAlgorithm == "svc-align-bwa") { chunkSize = 1000000;  }
            else if (this.selectedAlgorithm == "svc-align-blat") { chunkSize = 1000000;  }
            this.algoProc.setValue("slice", chunkSize);
            this.algoProc.setValue("subject", this.selectedSubject);
            this.algoProc.setValue("query", this.selectedQuery);
        }
        this.selectedAlgorithm = this.algoProc.getValue("alignSelector");
        //alert(this.selectedAlgorithm +  '==' + this.algoProc.getValue("alignSelector"));
        this.selectedSubject = this.algoProc.getValue("subject");
        this.selectedQuery = this.algoProc.getValue("query");


        this.vjDS["ds" + this.dvname + "Subject" + "Selected"].reload((isok(this.selectedSubject) ? "http://?cmd=propget&mode=csv&ids=" + this.selectedSubject :"static:// "), true);
        this.vjDS["ds" + this.dvname + "Query" + "Selected"].reload((isok(this.selectedQuery) ? "http://?cmd=propget&mode=csv&ids=" + this.selectedQuery : "static:// "), true);
        gObject(this.dvname + "Algorithm-image").src = "img-algo/" + this.selectedAlgorithm + ".gif";

        this.selfUpdate = false;
//        if (!this.selectedQuery) {
//            var listIds = docLocValue("query");
//        }

        if (isok(this.selectedAlgorithm) && isok(this.selectedSubject) && isok(this.selectedQuery))
            return 1;
        return 0;
    };

    this.onCheckedSequenceItem = function (viewer, node) {
        var lstIDs = viewer.accumulate((viewer.name == 'list') ? "node.checked" : "node.leafnode && node.checked", "node.id").join(";");
        if (viewer.tab.name == 'reference genomes') {
            this.selectedSubject = lstIDs; //this.algoProc.setValue("subject",lstIDs);//this.selectedSubject=lstIDs;
        }
        else {
            this.selectedQuery = lstIDs; //this.selectedQuery=lstIDs;
            var lstNames = viewer.accumulate((viewer.name == 'list') ? "node.checked" : "node.leafnode && node.checked", "node.name").join(" ");
            lstNames = lstNames.substring(0, 64);
            this.algoProc.setValue("name", lstNames);
        }


        this.selfUpdate = true;
        this.update(this.algoProc);
        if (this.callbackModified)
            funcLink(this.callbackModified, viewer, this);
        this.selfUpdate = false;

        if (!gKeyCtrl)
            this.vjVIS.closeall();
    };

    this.onSelectedAlignmentAlgorithm = function (viewer, node) {
        this.selectedAlgorithm = node.id;
        this.selfUpdate = true;
        this.algoProc.setSvcProc(node.id, null, (this.selectedAlgorithm.indexOf("svc-align-hexagon") != -1 ? "dna-hexagon" : "dna-alignx"));

        //gObject("DIV-"+this.dvname+"Algorithm-image").src=node.icon;
        this.vjVIS.closeall();

    };

    this.onRenderSequenceItem = function (viewer, ds) {
        qryIds = docLocValue("query");
        if(qryIds.length){
            qryIdList = qryIds.split(",");
            var lstNames="";
            for (var selectedQ = 0; selectedQ < qryIdList.length; ++selectedQ) {
                lstNames += viewer.accumulate("node.id==" + qryIdList[selectedQ], "node.name").join(" ") + " ";
                lstNames = lstNames.substring(0, 64);
            }
            if(isok(lstNames)) this.algoProc.setValue("name", lstNames);
        }
    };

}


var valgoGenomeColors = new Array();
for (var it = 0; it < gClrTable2.length; ++it) valgoGenomeColors[it] = pastelColor(gClrTable2[(it + 1) % (gClrTable2.length)]); // make colors for those


function valgoHexagonHitList(loadedID, formname, dvname, dvinfo)
{
    valgoCore.call(this, "hitlist", loadedID, formname, dvname);

    this.callbackSelected = null;
    this.callbackCheck = null;
    this.dvinfo = dvinfo;
    this.algoProc = null;

    this.selectedGenomes = null;
    this.selfUpdate = false;

    this.generate = function (selectedTab, autoclick) {

        this.vjDS.add("infrastructure: Coagulation of Reference Hits","ds" + this.dvname, "static:// ");
        this.vjDS.add("infrastructure: Coagulation of Reference Hits","ds" + this.dvname + "Histogram", "http://?cmd=objFile&filename=histogram.csv&ids="+loadedID);
        this.vjDS.add("infrastructure: Hit list help document", "dsHelpHitList", "http://help/hlp.view.results.alignment.html");
        this.autoclick = autoclick;

        var dv = this.dvname + "Viewer";
        this.vjDV.add(dv, 350, 350).selected = selectedTab ? selectedTab : 0;

        var defaultText = 'select a reference genome to see detail information';

        var panelHitList = vjPAGE.initBasicObjListPanel(this.dvname, this.formName);
        panelHitList.selectedCounter = '';
        var viewerHitList = new vjTableView({
            data: "ds" + this.dvname + "",
            formObject: document.forms[this.formName],
            //checkable:true,
            maxTxtLen: 32,
            name: 'reference_list',
            defaultEmptyText: 'no reference sequences to show',
            selectCallback: "function:vjObjFunc('onSelectedHitListItem','" + this.objCls + "')",
            checkCallback: "function:vjObjFunc('onCheckReferenceGenomes','" + this.objCls + "')",
            bgColors: ['#f2f2f2', '#ffffff'],
            checkColors: valgoGenomeColors,
            colIDColors: valgoGenomeColors,
            checkable: this.checkable,
            //            callbackRendered: "function:vjObjFunc('onLoadedHitList','" + this.objCls + "')",
            cols: [
                  { name: 'Hits', type: 'largesci' },
                  { name: 'Hits Unique', type: 'largenumber', hidden: true },
                  { name: 'Length', type: 'largenumber' },
                  { name: 'Reference', maxTxtLen: 15 },
                  { name: 'name', hidden: true },
                  { name: 'value', hidden: true }
            ],

            precompute: "if(node.id==0 || node.id=='+')node.styleNoCheckmark=true;",
            isok: true
        });

        var viewerHitPie = new vjGoogleGraphView({
            data: "ds" + this.dvname + "",
            options: {
                width: 300, height: 300, chartArea: { height: '95%', width: '95%' },
                legend: 'top',
                colors: valgoGenomeColors,
                is3D: true,
                pieSliceText: 'label'
            },
            series: [{ label: true, name: 'Reference', title: 'Reference Genes' }, { name: 'Hits', title: 'Hits'}],
            type: 'pie',
            //rows:[{id:'+',hidden:true}],
            precompute: "if(node.id=='+' && node.Reference=='total')node.hidden=true;",
            selectCallback: "function:vjObjFunc('onSelectedHitListItem','" + this.objCls + "')",
            isok: true
        });


        this.vjDV[dv].add("piechart", "pie", "tab", [viewerHitPie]);
        this.vjDV[dv].add("list", "table", "tab", [panelHitList, viewerHitList]);

        //this.vjDV[dv].add("download", "download", "download", [viewerHitList]);


        this.viewer = this.vjDV.locate(dv + ".1._active");
        this.vjDV[dv].render();
        this.vjDV[dv].load();



        if (this.dvinfo) {
            dv = this.dvinfo + "Viewer";
            this.vjDV.add(dv, 800, 350);


            this.vjDS.add("Composing per Nucleotide Alignment View","ds" + this.dvname + "alView", "static://preview:loading", null, "Element #,#,Direction,Start,Alignment,End,Sequence,Repeats,Position,PosHigh,Motif Start,Motif End");
            var alNav = [{ name: 'rgSub', path: '/Options/Search On/rgSub', title: 'reference', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '4', description: 'Show only reads that touch the position' },
                            { name: 'rgInt', path: '/Options/Search On/rgInt', title: 'comparison', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '5', description: 'Show only reads that touch the position' },
                            { name: 'rgQry', title: 'read',path: '/Options/Search On/rgQry', type: 'checkbox', isSubmitable: true, value: true, align: 'right', order: '6', description: 'Show only reads that touch the position'}];
            var panelAlignmentViewer = vjPAGE.initAlignNavigatorPanel(this.dvname + "alView", this.formName);
            panelAlignmentViewer.rowSpan = 3;
            for (var nav = 0; nav < alNav.length; ++nav)
                panelAlignmentViewer.rows.push(alNav[nav]);

            var alignViewer = new vjTableView({
                data: "ds" + this.dvname + "alView",
                iconSize: 0,
                rowSpan: 3,
                formObject: document.forms[this.formName],
                bgColors: ['#efefff', '#efefff', '#efefff', '#ffffff', '#ffffff', '#ffffff'],
                defaultEmptyText: defaultText,
                dataTypeAppliesToHeaders: true,
                cols: [{ name: new RegExp(/.*/), isNmatchCol: true },
                       { name: "Element #", hidden: true },
                       { name: "Direction", hidden: true },
                       { name: "#", hidden: true },
                      // { name: "Sequence", maxTxtLen : 32 , hidden: true },
                       { name: "Alignment", type: 'alignment', isNmatchCol: false },
                       { name: new RegExp(/Motif.*/), hidden: true}
                       ,{ name: new RegExp(/PosHigh/), hidden: true}],
                precompute: "node.bgcolor=(node.cols[2].indexOf('-')==-1  ?'#e4e4ff' : '#dfffdf');if(node['Motif Start']){node.matchSubstring={start: parseInt(node['Motif Start']), end:parseInt(node['Motif End'])};node['Motif End']=parseInt(node['Motif End'])+parseInt(node.Start);}",
                matchColor: '#FF9900',
                maxTxtLen: 4096,
                isok: true
            });
            //this.vjDV[dv].add("alignments","dna","tab", [ panelAlignmentViewer, new vjHTMLView ( { data:"ds"+this.dvname+"alView", composer: 'preview'} ) ] );
            this.vjDV[dv].add("alignments", "dna", "tab", [panelAlignmentViewer, alignViewer]);

            //this.viewer = this.vjDV.locate(dv + ".0.0");

            this.vjDS.add("Visualizing Alignments Stacking", "ds" + this.dvname + "alStack", "static://preview:loading");//, null, "#,Name,+/-,Start,Sequence,End,High");
            this.vjDS.add("Accumulating Mutation Bias Diagram", "ds" + this.dvname + "alMutBias", "static://preview:loading", null, "Position,Count-A,Count-C,Count-G,Count-T");//, null, "#,Name,+/-,Start,Sequence,End,High");

            var panelStackViewer = vjPAGE.initAlignNavigatorPanel(this.dvname + "alStack", this.formName);
            panelStackViewer.data.push("ds" + this.dvname + "alMutBias");
            panelStackViewer.excludedDataRefresh.push(2);

            var mutBiasViewer = new vjGoogleGraphView({
                data: "ds" + this.dvname + "alMutBias",
                name: "mutation bias",
                icon: 'graph',
                formObject: document.forms[this.formName],
                series: [{ name: 'Position' },
                         { name: 'Count-A'},
                         { name: 'Count-C'},
                         { name: 'Count-G'},
                         { name: 'Count-T'}
                    ],

                options: { width: 800, height: 120, vAxis: { title: 'Mutation Bias' },
                    hAxis: {title:'Distribution of read position mapping current reference position'}, lineWidth: 1, legend: 'top', chartArea: { top: 20, left: 70, width: '100%', height: "60%" },
                    //colors: ['blue', 'green']
                    colors: vjPAGE.colorsACGT,
                    focusTarget: 'category',
                    isStacked:true
                    },
                //selectCallback: "function:vjObjFunc('onZoomProfileOperation','" + this.objCls + "')",
                type: 'column',
                isok: true
            });
            var stackViewer = new vjTableView({
                data: "ds" + this.dvname + "alStack",
                name: 'stack',
                iconSize: 0,
                bgColors: ['#efefff', '#ffffff'],
                defaultEmptyText: defaultText,
                precompute: "node.bgcolor=(node.cols[2].indexOf('-')==-1 ?'#f2f2f2' : '#f2f2f2');if(node.cols[9])node.PosHigh=parseInt(node.cols[9]);if(node.cols[10]){node.matchSubstring={start: parseInt(node.cols[10]), end:parseInt(node.cols[11])};}",//node.cols[10]=parseInt(node.cols[10])+parseInt(node.cols[4]);}",
                matchColor: '#FF9900',
                dataTypeAppliesToHeaders:true,
                cols: [{ name: new RegExp(/.*/), type: 'alignment', hidden: true, isNmatchCol: true }
                       ,{ name: new RegExp(/\S/), hidden: false, isNmatchCol: true  }
                       ,{ name: 'Element #', hidden: true }
                       ,{ col: 1, hidden: true }
                       ,{ col: 2, hidden: true }
                       ,{ col: 3, hidden: false }
                       ,{ col: 4, isNmatchCol: false}
                       ],
                maxTxtLen: 4096,
                isok: true
            });
            this.vjDV[dv].add("stack", "dna", "tab", [panelStackViewer, mutBiasViewer, stackViewer]).viewtoggles = 1;


            this.vjDS.add("Importing Sequence Hit Table", "ds" + this.dvname + "alMatch", "static:// ");
            var panelMatchViewer = vjPAGE.initBasicObjListPanel(this.dvname + "alMatch", this.formName);
            var matchViewer = new vjTableView({
                data: "ds" + this.dvname + "alMatch",
                iconSize: 0,
                bgColors: ['#efefff', '#ffffff'],
                defaultEmptyText: defaultText,
                startAutoNumber: 1,
                maxTxtLen: 32,
                isok: true
            });
            this.vjDV[dv].add("hit table", "table", "tab", [panelMatchViewer, matchViewer]);


            this.vjDS.add("infrastructure: Constructing download links for Aligner", "ds" + this.dvname + "Statistics", "static:// ");
            var statisticViewer = new vjTableView({
                data: "ds" + this.dvname + "Statistics",
                iconSize: 16,
                icon: 'download',
                newSectionWord: "-",
                defaultEmptyText: defaultText,
                geometry: { width: 500 },
                selectCallback: "function:vjObjFunc('onPerformReferenceOperation','" + this.objCls + "')",
                cols: [{ name: 'icon', type: 'icon', hidden: true }, { name: 'operation', hidden: true }, { name: 'arguments', hidden: true}],
                maxTxtLen: 32,
                isok: true
            });
            this.vjDV[dv].add("downloads", "table", "tab", [statisticViewer]);


///NCBI this.vjDS.add("infrastructure: Retrieving link from NCBI", "ds" + this.dvname + "NCBIAnnot", "static://<iframe width=1024 height=350 FRAMEBORDER=0 MARGINWIDTH=0 MARGINHEIGHT=0 SCROLLING=auto id='ncbi-search' src=''> </iframe>");
///NCBI this.vjDV[dv].add("ncbi", "help", "tab", [new vjHTMLView({ data: "ds" + this.dvname + "NCBIAnnot" })]).forceLoadAll = true; ;


            this.vjDV[dv].add("help", "help", "tab", [new vjHelpView({ data: 'dsHelpHitList' })]);
            //vjPAGE.initTaxonomyBrowserTree(dv, "taxonomy", this.formName);


            this.vjDV[dv].render();
            this.vjDV[dv].load();
        }

    };

    this.onLoadedHitList = function ()
    {
        if(this.autoclick)
            this.viewer.mimicClickCell(this.autoClickedReference,0);
    };

    this.reload = function (loadedID, reqid, autoClickedReference) // viewer,
    {
        if (loadedID)
            this.loadedID = loadedID;
        if (!this.loadedID)
            return;

        var url = "http://?cmd=alCount&objs=" + this.loadedID+"&start=0&cnt=50&info=1";
        if(this.profilerList)url+="&childProcessedList="+this.profilerList;
        if (reqid) {
            this.reqID = reqid;
            url += "&req=" + this.reqID;
        }
        if (autoClickedReference) {
            this.autoClickedReference = autoClickedReference;
            this.viewer.callbackRendered = "function:vjObjFunc('onLoadedHitList','" + this.objCls + "')";
        }
        this.vjDS["ds" + this.dvname].reload(url, true);

        //this.vjDS["ds" + this.dvname+"histogram"].reload(url, true);

    };

    this.update = function (proc) {
        this.algoProc = proc;
        var checkedIDS = this.viewer.accumulate("node.checked", "node.id");
        //if(this.selfUpdate)return ;

        //var ids=this.algoProc.getValue("subSet");
        //var loadID=this.algoProc.getValue("parent_proc_ids");
        //if(this.loadedID=loadID;
        //alert(ids+" dd "+this.selfUpdate)
        //alerJ('d',);

        //var params={selCnt:0, computedProfiles:this.algoProc.viewer.attached("file")};
        //if(loadID && loadID!=this.loadedID){
        //    this.reload(loadID)
        //}
        //else
        //if(ids && !this.selfUpdate){
        //alerJ('t',this.viewer)
        //this.viewer.enumerate("if(params.computedProfiles['SNPprofile-'+node.id+'.csv']){node.checked=1;params.selCnt++;}else node.checked=0; alerJ('d-'node.id,node) ",params);
        //this.viewer.enumerate("alert(params.computedProfiles['SNPprofile-'+node.id+'.csv'])",params);

        //this.viewer.refresh();
        //}
        return isok(checkedIDS) ? 1 : 0;
    };


    this.onSelectedHitListItem = function (viewer, node)
    {
        if (this.dvinfo) {
/// NCBI
            /*
            if (parseInt(node.id) > 0) {
                var term = node.Reference; if (!isok(term)) return;
                var pos = term.lastIndexOf("."); if (pos != -1) term = term.substring(0, pos);
                gObject("ncbi-search").src = "http://www.ncbi.nlm.nih.gov/nuccore?term=" + term;
            }
            */
            //            var alHigh = docLocValue("alHigh", '50', this.vjDS["ds" + this.dvname + "alStack"].url);
            var viewer = this.vjDV.locate(this.dvinfo + "Viewer.downloads.0");
            var t = "download,icon,operation,arguments\n";
            if (parseInt(node.id) == 0) {
                viewer.prefixHTML = "There were <b>" + viewer.formDataValue("" + node.Hits, 'largenumber', node) + "</b> sequences that did not match any of the reference genomes.";
                t += "Unaligned Reads,download,alFasta";
            } else {
                //alert(this.selectedAlgorithm);

                viewer.prefixHTML = "<b>" + viewer.formDataValue("" + node.Hits, 'largenumber', node) + "</b> sequences were aligned to <b>" + node.Reference + "</b>";
                t += "Hit list,download,hitlist,\nAligned Reads,download,alFasta,\n" +
                    "Hit Table,download,alMatch,\nAlignments,download,alView,\nAlignments in SAM format,download,alSam,\n" +
                    "Stack,download,alStack,\nStack of non perfect alignments,download,alStack,&alNonPerfOnly=1\n" +
                    "Stack of alignments with mutation on specified position ,download,alStack,&alVarOnly=1\n" +
                    "Mutation bias,download,alMutBias,\n";
                    // get

                    var filelist=this.algoProc.viewer.attached("file");
                    //alerJ('length='+filelist,filelist);
                    var tAux="";
                    for( ff in filelist ){
                        if(ff.indexOf(".csv")==-1&&ff.indexOf(".txt")==-1&&ff.indexOf(".sam")==-1&&ff.indexOf(".fpkm_tracking")==-1)continue;
                        // checks if this file needs to be added....
                        tAux+=ff+",download,objFile,&ids="+this.loadedID+"&filename="+ff+"\n";
                    }
                    if(tAux.length)
                        t+="-Auxiliary information,,,\n"+tAux;

            }
            this.referenceID = node.id;


            this.vjDS["ds" + this.dvname + "Statistics"].reload("static://" + t, true);
            this.vjDS["ds" + this.dvname + "alView"].reload("http://" + this.makeReferenceOperationURL(viewer, node, 'alView') + "&cnt=20", false);// &alHigh=50
            this.vjDS["ds" + this.dvname + "alMatch"].reload("http://" + this.makeReferenceOperationURL(viewer, node, 'alMatch') + "&cnt=20", false);
            this.vjDS["ds" + this.dvname + "alStack"].reload("http://" + this.makeReferenceOperationURL(viewer, node, 'alStack') + "&cnt=20", false); // &alHigh=50
            this.vjDS["ds" + this.dvname + "alMutBias"].reload("http://" + this.makeReferenceOperationURL(viewer, node, 'alMutBias') + "&cnt=20", false); // &alHigh=50
            this.vjDV.locate(this.dvinfo + "Viewer._active.").load();
            //this.vjDS["ds" + this.dvname + "alStackSubject"].reload("http://" + this.makeReferenceOperationURL(viewer, node, 'alStackSubj') + "&cnt=1&rangeStart=0&rangeEnd=100&High=50", true);

            //alert(node.Reference + " - " + node.Reference.split(" ").join("|"))
            //            viewer = this.vjDV.locate(this.dvinfo + "Viewer.taxonomy.0");
            //            viewer.onUpdate(viewer.container, null, node.Reference.split(" ").join("|"));
        }

        this.selfUpdate = 1;
        if (this.callbackSelected)
            funcLink(this.callbackSelected, this.viewer, node);
        this.selfUpdate = 0;
    };

    this.onCheckReferenceGenomes = function (viewer, node)
    {
//        alerJ('a',this.algoProc)
        this.algoProc.setValue("subSet", viewer.accumulate("node.checked", "node.id").join(";"));


        //if(this.modeActive) {
        var lstNamesArr = viewer.accumulate("node.checked", "node.Reference");
        lstNames = ""; //lstNamesArr[0];
        var toprint = 3;
        for (var il = 0; il < lstNamesArr.length && il < toprint; ++il)
            lstNames += lstNamesArr[il].replace(">", "") + " ";
        if (lstNamesArr.length > toprint) lstNames += " and " + (lstNamesArr.length - toprint) + " more ...";
        lstNames = lstNames.substring(0, 64);
        //alert(lstNames);
        this.algoProc.setValue("name", lstNames);
        //}

        this.selfUpdate = 1;
//        if (this.callbackModified)
 //           funcLink(this.callbackModified, viewer, this);
        if (this.callbackChecked)
            funcLink(this.callbackChecked, viewer, this);
        this.selfUpdate = 0;
    };



    this.makeReferenceOperationURL = function (viewer, node, oper, args) {
        if (oper == 'hitlist'){
            var url = urlExchangeParameter(this.viewer.getData(0).url.substring(7), "cnt", '0');
            return url;
        }
//        else if (oper == 'alStack' || oper == 'alView' || oper == 'alMatch' || oper == 'alMutBias') {
//            var url = vjDS[this.viewer.getData(0).name + oper].url;
//            url = urlExchangeParameter(url, "cnt", '0');
//            if (args)
//                url += args;
//            return url;
//        }
        var url = "?cmd=" + oper + "&objs=" + this.loadedID + "&req=" + this.reqID;
        if (node.id == 0) {
            url += "&found=0";
            this.vjDV[this.dvinfo + "Viewer"].select(0, true);
        } else if (node.id == '+') {
            url += "&found=1";
        } else {
            url += "&mySubID=" + (this.referenceID) + "&found=1";
        }
        url+=args;
        return url;
    };
    this.onPerformReferenceOperation = function (viewer, node, ir, ic) {
        var oper = node.operation;
        var args = node.arguments;
        var url;
        if (oper == 'alStack' || oper == 'alView' || oper == 'alMatch' || oper == 'alMutBias' ) {
                url = vjDS[this.viewer.getData(0).name + oper].url;
                url = urlExchangeParameter(url, "cnt", '0');
//                url = urlExchangeParameter(url, "found", '1');
            if (args)
                url += args;
            url = url.substring(7);
        }
        else
            url = this.makeReferenceOperationURL(viewer, node, node.operation, node.arguments);
        url = urlExchangeParameter(url, "down", '1');
        document.location = url;

    };


}





















function valgoHexagonPopulator(loadedID, formName, dvname, dvinfo)
{
    valgoCore.call(this, "populator", loadedID, formName, dvname);

    //this.selectedHitListCallback=null;
    this.dvinfo = dvinfo;
    this.datasets = new Array("popHierarchy", "popCoverage");//, "popConsesus"); // "profNoise", "profNoiseIntegral",

    if(this.sortedSubjIndex!==undefined){

    }

    this.generate = function () {
        this.isok = true;

        this.vjDS.add("Summarizing Next-Gen Clone Discovery Component: " + this.datasets[0].substring(3), "ds" + this.dvname + this.datasets[0], "static://",null,"Name,Start,End,Merge,Hierarchy");
        this.vjDS.add("Summarizing Next-Gen Clone Discovery Component: " + this.datasets[1].substring(3), "ds" + this.dvname + this.datasets[1], "static://",null,"Name,Start,End,Merge,Father,Coverage,max,average");
//        this.vjDS.add("Summarizing Next-Gen Clone Discovery Component: " + this.datasets[2].substring(3), "ds" + this.dvname + this.datasets[2], "static://");

        this.vjDS.add("infrastructure: Population analysis tool help document", "dsHelpPopulator", "http://help/hlp.algo.snpProfile.algorithmParameters.html");
        //this.vjDS.add("Accumulating SNPs", "ds" + this.dvname + "SNPcalls", "static://");
        this.vjDS.add("Building downloading table","ds"+this.dvname+"popDownloads","static://");
        var serie1=new vjDataSeries({
//          data: vjDS.dsScatterData,
         title:"clone series",
         name:"ds"+this.dvname+"popClones",
//         parser:'sankeyParser',
         url:"static://",
         center:0.5,
         symbolSize:"0.17",
         isNXminBased: true,
         byX : true,
         columnDefinition:{branchID:'Clone ID',start:'showStart',end:'showEnd',weight:'Max Coverage',mergeID:'Merged ID',bifurcateID:'Bifurcated ID',coverage:'Coverage'},
         type:'sankey',
         id:this.dvname+'cloneSerie',
         isok:true});

        var serie2=new vjDataSeries({
//          data: vjDS.dsScatterData,
         title:"Normalized clone series",
         name:"ds"+this.dvname+"popClonesNorm",
//         parser:'sankeyParser',
         isNXminBased: true,
         byX : true,
         url:"static://",
         center:0.5,
         symbolSize:"0.17",
         columnDefinition:{branchID:'Clone ID',start:'showStart',end:'showEnd',weight:'Max Coverage',mergeID:'Merged ID',bifurcateID:'Bifurcated ID',coverage:'Coverage'},
         type:'sankey',

         id:this.dvname+'cloneSerieNorm',
         isok:true});



        var rowlist = new Object([ {
            name : 'minCloneCov',
            path : '/Filters/Coverage',
            title : '0',
            size : '1',
            type : 'text',
            description : 'Specify coverage threshold',
            forceUpdate : true,
            isSubmitable : true,
            align:'left',
            prefix : 'Coverage'
        }, {
            name : 'minCloneLen',
            path : '/Filters/minCloneLen',
            title : '0',
            type : 'text',
            size : '1',
            description : 'Specify length threshold',
            forceUpdate : true,
            isSubmitable : true,
            align:'left',
            prefix : 'Length'
        }, {
            name : 'minCloneSup',
            path : '/Filters/Support',
            title : '0',
            size : '1',
            type : 'text',
            description : 'Specify supporting mutation threshold',
            forceUpdate : true,
            isSubmitable : true,
            align:'left',
            prefix : 'Support'
        }, {
            name : 'mergeHidden',
            type : 'checkbox',
            isSubmitable : true,
            align:'left',
            title : 'Merge',
            description : 'Merge hidden clones to their parents',
            order : '1'
        }, {
            name : 'showSimil',
            type : 'checkbox',
            isSubmitable : true,
            align:'left',
            title : 'Similarities',
            description : 'Show per clone similarities to references',
            value : true,
            order : '2'
        }, {
            name :'refresh',
            align:'right',
            order : -1,
            title: 'Reset' ,
            description: 'refresh the content of the control to retrieve up to date information' ,
            url : "function:vjObjFunc('onResetCloneDS','"+this.objCls+"')",
            icon : 'refresh'
        }, {
            name:'submit',
            align:'right',
            type : 'submitter'
        }
                ]);


        var rowlist2 = new Object([ {
            name : 'back',
            icon : 'search',
            type:'',
            iconSize : '12',
            align : 'left',
            description : 'zoom out',
            url : "function:vjObjFunc('ZoomOutReg','" + this.objCls + "');"
        }, {
            name : 'showStart',
            title : ' ',
            type : 'text',
            size : '4',
            description : 'Specify start position',
            forceUpdate : true,
            isSubmitable : true,
            align:'left',
            prefix : 'Range: '
        }, {
            name : 'showEnd',
            title : ' ',
            type : 'text',
            size : '4',
            description : 'Specify end position',
            forceUpdate : true,
            isSubmitable : true,
            align:'left',
            prefix : '- &nbsp'
        }, {
            name:'submit',
            align:'left',
            type : 'submitter',
            description: 'zoom in'
        }
            ]);

        var rowlist3 = new Object([ {
            name : 'back',
            icon : 'search',
            type:'',
            iconSize : '12',
            align : 'left',
            description : 'zoom out',
            url : "function:vjObjFunc('ZoomOutNorm','" + this.objCls + "');"
        }, {
            name : 'showStart',
            title : ' ',
            type : 'text',
            size : '4',
            description : 'Specify start position',
            forceUpdate : true,
            isSubmitable : true,
            align:'left',
            prefix : 'Range: '
        }, {
            name : 'showEnd',
            title : ' ',
            type : 'text',
            size : '4',
            description : 'Specify end position',
            forceUpdate : true,
            isSubmitable : true,
            align:'left',
            prefix : '- &nbsp'
        }, {
            name:'submit',
            align:'left',
            type : 'submitter',
            description: 'zoom in'
        }
            ]);


        var viewerPanel = new vjPanelView({
            data : [ "dsVoid", "ds" + this.dvname + this.datasets[0],"ds"+this.dvname+"popClones","ds"+this.dvname+"popClonesNorm" ],
            formObject : document.forms[formName],
            iconSize : 24,
//            hideViewerToggle : true,
            rows : rowlist,
            isok : true
        });
        viewerPanel.rows[viewerPanel.rows.length-1].url="javascript:vjObjEvent(\"onUpdate\",\""+viewerPanel.objCls+"\")";

        var viewerPanelReg = new vjPanelView({
            data : [ "dsVoid","ds"+this.dvname+"popClones"],
            formObject : document.forms[formName],
            iconSize : 24,
            rows : rowlist2,
            datastack:true,
            isok : true
        });
        var viewerPanelNorm = new vjPanelView({
            data : [ "dsVoid","ds"+this.dvname+"popClonesNorm" ],
            formObject : document.forms[formName],
            iconSize : 24,
            rows : rowlist3,
            datastack:true,
            isok : true
        });

        var viewerSummary=new vjTreeView( {
            name:'hierarchy',
            icon:'tree',
            data:"ds" + this.dvname + this.datasets[0],
            hierarchyColumn: 'Hierarchy',
            showRoot: true,
            showLeaf : true ,
            checkLeafs: true,
            checkBranches: true,
            hideEmpty: false,
            icons: { leaf: 'img/dna.gif' },
            showChildrenCount: true,
//            checkPropagate:true,
            formObject:document.forms[this.formName],
            checkBranchCallback:"function:vjObjFunc('onCloneCheckOperation','"+this.objCls+"');",
            checkLeafCallback:"function:vjObjFunc('onCloneCheckOperation','"+this.objCls+"')",
//            linkLeafCallback: selectCallback ? selectCallback : vjHC.recordViewNode  ,
            autoexpand:'all',
            maxTxtLen:this.maxTxtLen,
//            vjHCAssociatedRecordViewer:recordviewer+".details.0",
            precompute: "row.path=(row.Hierarchy ? row.Hierarchy : '/') +row['Name'];",
//            postcompute:"if(node.name"
            //postcompute:"if(node.name=='inpos.fa')node.uncheckable=true;alert(node.name + ' - ' + node.uncheckable);",
//            objectsDependOnMe:[ dvname+'.'+tabname+'.'+cntViewersIn ],
            isok:true });

        var plot1=new vjSVG_Plot();
        plot1.add(serie1);
        plot1.colors=valgoGenomeColors.slice(1);
        plot1.cloneHierarchyViewer=this.dvname + "Viewer.Summary.1";
        var plot2=new vjSVG_Plot();
        plot2.add(serie2);
        plot2.cloneHierarchyViewer=this.dvname + "Viewer.Summary.1";
        plot2.colors=valgoGenomeColors.slice(1);

        var viewerClones=new vjSVGView ( {
//             data:['dsScatterData' ] ,
            chartArea:{width:'70%',height:"80%"},
            plots:[ plot1 ],
            Axis:{
                x:{title:"Position",showGrid:true,showArrow:false},
                y:{title:"SNP",showArrow:false,showTicks:true,showGrid:true,showTitle:false}
            },

            // add any number of additional parameters to make your viewer versatile and customizeable
            isok:true});
        var viewerClonesNorm=new vjSVGView ( {
//          data:['dsScatterData' ] ,
         chartArea:{width:'70%',height:"80%"},
         plots:[ plot2 ],
         Axis:{
             x:{title:"Position",showGrid:true,showArrow:false},
             y:{title:"SNP",showArrow:false,showTicks:true,showGrid:true,showTitle:false}
         },

         // add any number of additional parameters to make your viewer versatile and customizeable
         isok:true});
        var viewerDownloads = new vjTableView({
            data: "ds" + this.dvname + "popDownloads",
            iconSize: 16,
            icon: 'download',
            defaultEmptyText: "select reference genome to see available downloads",
            geometry: { width: "95%" },
            selectCallback: "function:vjObjFunc('onPerformPopulationOperation','" + this.objCls + "')",
            cols: [{ name: 'icon', type: 'icon', hidden: true }, { name: 'operation', hidden: true }, { name: 'blob', hidden: true}],
            maxTxtLen: 32,
            isok: true
        });
//        var viewerClones = new vjSVGView({
//            data: "ds" + this.dvname + "popHierarchy",
//            formObject: document.forms[this.formName],
//            maxTxtLen: 48,
//            defaultEmptyText: 'no information to show',
//            newSectionWord: "-",
//            bgColors: ['#f2f2f2', '#ffffff'],
//            isok: true
//        });

        var dv = this.dvname + "Viewer";
        this.vjDV.add(dv, 450, 650).selected=0;//.frame='notab';


        this.vjDV[dv].add("Summary", "table", "tab", [viewerPanel,viewerSummary]);
        this.vjDV[dv].add("help", "help", "tab", [new vjHelpView({ data: 'dsHelpPopulator' })]);
        this.vjDV[dv].render();
        this.vjDV[dv].load();


        var inf = this.dvinfo + "Viewer";
        this.vjDV.add(inf, 950, 650).selected=1;
        this.vjDV[inf].add("shankey diagram", "table", "tab", [viewerPanelReg,viewerClones]);
        this.vjDV[inf].add("normalized shankey diagram", "table", "tab", [viewerPanelNorm,viewerClonesNorm]);
        this.vjDV[inf].add("Downloads", "table", "tab", [viewerDownloads]);
        this.vjDV[inf].add("help", "help", "tab", [new vjHelpView({ data: 'dsHelpPopulator' })]);
        this.vjDV[inf].render();
        this.vjDV[inf].load();

    };


    this.ZoomOutReg=function(){
        var curds=vjDS["ds"+this.dvname+"popClones"];
        if(isok(curds.urlStack)){
            if(curds.urlStack.length){
                curds.reload(curds.urlStack.pop(),true);
            }
        }
    };

    this.ZoomOutNorm=function(){
        var curds=vjDS["ds"+this.dvname+"popClonesNorm"];
        if(isok(curds.urlStack)){
            if(curds.urlStack.length){
                curds.reload(curds.urlStack.pop(),true);
            }
        }
    };

    this.onCloneCheckOperation = function(vv,node,op){
        var viewerSankey=vjDV.dvPopulInfoViewer.tabs[0].viewers[1],
            viewerSankeyNorm=vjDV.dvPopulInfoViewer.tabs[1].viewers[1];
        var urlSankey=vjDS[viewerSankey.data].url,
        urlSankeyNorm=vjDS[viewerSankeyNorm.data].url,
        urlHier=vjDS[vv.data].url;
        var hiddenstring=docLocValue("showClones", -1, urlSankey);
        var hidden=new Array();
        if(hiddenstring!=-1){
            hidden=hiddenstring.split(",");
        }

        var ir=-1;
        var id=parseInt(node.Name.split("_")[1]);
        for(var i=0; i<hidden.length;++i){
            if(parseInt(hidden[i])==id)
                ir=i;
        }
        if(ir>=0){
            if(!node.checked)
                hidden.splice(ir,1);
        }
        else{
            if(node.checked)
                hidden.push(id);
        }
        if(hidden.length){
            urlSankey=urlExchangeParameter(urlSankey,"showClones",hidden.join(","));
            urlSankeyNorm=urlExchangeParameter(urlSankeyNorm,"showClones",hidden.join(","));
            urlHier=urlExchangeParameter(urlHier,"showClones",hidden.join(","));
        }

        vjDS[viewerSankey.data].url=urlSankey;
        vjDS[viewerSankeyNorm.data].url=urlSankeyNorm;
        vjDS[vv.data].url=urlHier;

    };

    this.onResetCloneDS = function (viewer, pp, tt) {

        this.vjDS["ds" + this.dvname + "popClones"].reload("http://?cmd=popClones" + "&objs="+this.loadedID+"&resolution=200", true);
        this.vjDS["ds" + this.dvname + "popHierarchy"].reload("http://?cmd=popHierarchy" + "&objs="+this.loadedID+"", true);
        this.vjDS["ds" + this.dvname + "popClonesNorm"].reload("http://?cmd=popClones" + "&objs="+this.loadedID+"&normCloneCov=1&resolution=200", true);
    };

    this.makeProfileOperationURL = function (viewer, node, oper) {

        var url = "?cmd=objFile&ids=" + this.loadedID + "&filename=SNPprofile-" + this.referenceID + ".csv";

        return url;
    };


    this.reload = function (loadedID, parent_loaded_ID) {
        if (!this.isok) return;
        if (loadedID) this.loadedID = loadedID;
//        if (subID == 0 || subID == '+') return;

        this.dna_aligner_ID = parent_loaded_ID;

        var url;
        for (var i = 0; i < this.datasets.length; ++i) {
            url = "http://?cmd=" + (this.datasets[i]) + "&objs=" + this.loadedID ;

//            alert("Reloading commands");
            this.vjDS["ds" + this.dvname + this.datasets[i]].reload(url, true);
        }
        var t = "download,icon,operation,blob\n";
        t += "Summary,download,popSummary,\n" +
            "Coverage,download,popCoverage,\n" +
            "Consensus,download,popConsesus,";
        this.vjDS["ds" + this.dvname + "popDownloads"].reload("static://" + t, false);

        this.vjDS["ds" + this.dvname + "popClones"].reload("http://?cmd=popClones" + "&objs="+this.loadedID+"&resolution=200", true);

        this.vjDS["ds" + this.dvname + "popClonesNorm"].reload("http://?cmd=popClones" + "&objs="+this.loadedID+"&normCloneCov=1&resolution=200", true);

        this.vjDV.dvPopulInfoViewer.tabs[1].viewers[0].load(true);

        this.vjDV.locate(this.dvname + "Viewer._active.").load(true);
        this.vjDV.locate(this.dvinfo + "Viewer._active.").load(true);
    };

    this.makePopulationOperationURL = function (viewer, node, oper) {

        var url;
        if (node.blob)
            url = "?cmd=objFile&ids=" + this.loadedID + "&filename=" + node.blob;
        else {
            url = "?cmd=" + node.operation + "&objs=" + this.loadedID ;
            if (node.operation == 'profNoise')
                url += "&noiseProfileMax=1";
        }
        return url;
    };

    this.onPerformPopulationOperation = function (viewer, node, ir, ic) {
        var url = this.makePopulationOperationURL(viewer, node, node.operation);
        document.location = url;
    };

}
