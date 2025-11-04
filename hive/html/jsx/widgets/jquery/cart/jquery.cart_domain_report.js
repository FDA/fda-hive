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

    $.widget("cart.cart_domain_report",
        {
            options: {
                grpList: [],
                pVal   : [],
                selGrp : '',
                selRes : ''
            },

            _getSelectedGrpPvals: function () {
                var grpPval = [];

                this.element.find(".group-name")
                    .children()
                    .each(function () {
                        grpPval.push({
                            grpName: $(this).find(".feature-name").val(),
                            pVal: $(this).find(".feature-type").val()
                        });
                    });
                return grpPval;
            },

            _addGrpPVal: function (selectVal) {
                var GrpPValBlock = $(
                    "<div class='cart-block'>" +
                    "  <div class='cart-label'></div>" +
                    "  <select class='group-name'></select>" +
                    "</div>"
                );
                GrpPValBlock.find(".group-name").appendOptions(selectVal);
                this.element.find(".selected-grp").append(GrpPValBlock);
            },

            _generate_bivar_table: function (results, bivar_keys, selectedPval) {
                
                biVarJson = results.bivariate_json;


                var my_header = [];
                var col = [];
                my_header.push(["Variables"]);
                for (var i = 0; i < bivar_keys.length; i++) {
                    var key = bivar_keys[i].replace("_", " ");
                    if (col.indexOf(key) === -1) {
                        col.push(key);
                        my_header.push([key]);
                    }
                    if (i === 3) { break; }
                }

                varlist = biVarJson[bivar_keys[0]];
                var rowLen = varlist.variables.length;

                var my_values = []
                var my_colors = []

                cur_grp_col = []
                for (var vari = 0; vari < rowLen; vari++) {
                    cur_grp_col.push(varlist.variables[vari].replace("_", " "))
                };
                my_values.push(cur_grp_col);
                my_colors.push('white')

                for (var grpi = 0; grpi < bivar_keys.length; grpi++) {
                    cur_grp_col = []
                    cur_colors  = []
                    for (var vari = 0; vari < rowLen; vari++) {
                        if (selectedPval == "raw_pvals") {
                            cur_val = biVarJson[bivar_keys[grpi]].raw_pvals[vari];
                            cur_colors.push((typeof cur_val === 'number' && cur_val <= 0.1 && cur_val >= 0) ? 'lightgreen' : 'white')
                            cur_grp_col.push( (cur_val == null || cur_val == -1 ) ?  '-' : cur_val )
                        } else {
                            cur_val = biVarJson[bivar_keys[grpi]].adjusted_pvals[vari];
                            cur_colors.push((typeof cur_val === 'number' && cur_val <= 0.1 && cur_val >= 0) ? 'lightgreen' : 'white')
                            cur_grp_col.push( (cur_val == null || cur_val == -1 ) ?  '-' : cur_val )
                        }

                    };

                    my_values.push(cur_grp_col.map(x => (typeof x === 'number') ? x.toFixed(3) : x ));
                    my_colors.push(cur_colors);

                    if (grpi === 3) { break; }
                };


                var paramTablePlotly = [{
                    type: "table",
                    header: {
                        values: my_header,
                        line: { width: 1, color: 'black' },
                        fill: { color: "grey" },
                        font: { family: "Arial", size: 14, color: "white" }
                    },
                    cells: {
                        values: my_values,
                        align: "center",
                        height: 24,
                        line: { color: "black", width: 1 },
                        font: { family: "Arial", size: 12, color: ["black"] },
                        fill: { color: my_colors }
                    }
                }];

                Plotly.react("bivar-table", paramTablePlotly);

                return true;
            },

            _getDomainReport: function (domain, groupBy, study_day_range, AERange) {
                var DomainReport = {
                    "report_type": "DOMAIN REPORT",
                    "domain": domain,
                    "study_day_range": study_day_range,
                    "group_by": groupBy,
                    "ae_time_range": AERange
                }
                return DomainReport;
            },

            _drawFilterSubmission: function (cartDomReportId) {
                var oThis = this;

                var domainClass =
                {
                    "DEMOGRAPHICS"    : [],
                    "CYTOKINES"       : [],
                    "PERSISTENCE"     : [],
                    "ADVERSE_EVENTS"  : [],
                    "CONCOMITANT_MEDS": [],
                    "LYMPHODEPLETION" : [],
                    "DISEASE_BURDEN"  : [],
                    "DOSE"            : [],
                    "CELL_COMPONENTS" : [],
                    "CELL_PRODUCT"    : [],
                    "VECTOR"          : [],
                    "VECTOR_LOT"      : [],
                    "SIGNIFICANT_FEATURES_FROM_ALL_DOMAINS" : []
                };

                var groupByClass =
                {
                    "STUDYID": [],
                    "indication": []

                };

                var label1 = "Domain";
                var label2 = "Group By";
                var label3 = "Adverse Event Time Range";
                var label5 = "Study Date Range";
                $("#cartDomReport").append("<div class='cart-block' style='font-weight: bold;margin:5px;'>Report Submission</div>")
                    .appendSelectType(domainClass, label1)
                    .append("<div class='study-time-block' </div>")
                    .appendSelectType(groupByClass, label2)
                    .append("<div class='ae-time-block' </div>")
                $(".ae-time-block").appendTimeRange(0, 28, label3);
                $(".Domainclass").change(function () {
                    if ((this.value == "LAB") || (this.value == "ADVERSE_EVENTS") || (this.value == "CONCOMITANT_MEDS") || (this.value == "RESPONSE") ||
                        (this.value == "DISEASE_BURDEN") || (this.value == "DOSE") || (this.value == "CYTOKINES") || (this.value == "PERSISTENCE")  ) {
                        $(".study-time-block").empty();
                        $(".study-time-block").appendTimeRange(0, 14, label5);
                    }
                    else {
                        $(".study-time-block").empty();
                    }
                })

                $("<input type='button' value='Submit' class='cart-button' />")
                    .click(function () {

                        var domain = $("#cartDomReport").find(".Domainclass").val();

                        if ($(".Study-startSel").length) {
                            var study_time_start = parseInt($("#cartDomReport").find(".Study-startSel").val());
                            var study_time_end = parseInt($("#cartDomReport").find(".Study-endSel").val());
                            oThis._StudyRange = [study_time_start, study_time_end];
                        }
                        else {
                            var study_time_start = "NONE";
                            var study_time_end = "NONE";
                            oThis._StudyRange = [study_time_start, study_time_end];
                        }

                        var group_by = $("#cartDomReport").find(".Groupclass").val();
                        selGrp = $("#cartDomReport").find(".Groupclass").val();

                        var ae_time_start = parseInt($("#cartDomReport").find(".Adverse-startSel").val());
                        var ae_time_end = parseInt($("#cartDomReport").find(".Adverse-endSel").val());
                        oThis._AERange = [ae_time_start, ae_time_end];


                        var domain_field = oThis._getDomainReport(domain, group_by, [study_time_start, study_time_end], [ae_time_start, ae_time_end]);

                        var domainStr = "qpbg_cart_report:
                        vjDS["crs3DomainReport"].reload(domainStr, true);
                    })
                    .appendTo("#cartDomReport");
            },

            _createMultivarView: function (results) {


                $("#multivarAnalysis").append(
                    "<div class='cart-block' style='font-weight: bold;margin:5px;padding-top: 15px;'><h4>Parameter Estimates</h4></div>" +
                    "<div id='param-table'></div>" +
                    "<div class='cart-block' style='font-weight: bold;margin:5px;'><h4>Confusion Matrix</h4></div>" +
                    "<div id='confusion-matrix' style='display: inline-block'></div>" +
                    "<div id='confusion-matrix-percent' style='display: inline-block; margin-left: 50px;'></div>" +
                    "<div class='cart-block' style='font-weight: bold;margin:5px;'><h4>ROC Curve</h4></div>" +
                    "<div id='roc-plot'></div>"
                );

                if (results.multivariate_json.method.startsWith("LOGISTIC")) {
                    var paramTerms = ["Intercept"].concat(results.multivariate_json.all_data.final_features);
                    var paramEsts = results.multivariate_json.all_data.model.intercept.concat(results.multivariate_json.all_data.model.coefs);
                    var paramTableVals = [paramTerms, paramEsts.map(x => x.toPrecision(4))];
                    var paramTablePlotly = [{
                        type: "table",
                        header: {
                            values: [["<b>Term<b>"], ["<b>Estimate<b>"]],
                            line: { width: 1, color: 'black' },
                            fill: { color: "grey" },
                            font: { family: "Arial", size: 20, color: "white" }
                        },
                        cells: {
                            values: paramTableVals,
                            align: "center",
                            height: 24,
                            line: { color: "black", width: 1 },
                            font: { family: "Arial", size: 16, color: ["black"] }
                        }
                    }];
                } else if (results.multivariate_json.method.startsWith("DECISION")) {
                    var paramTableVals = ['<b>Decision Text<b>', results.multivariate_json.all_data.model.decision_text];
                    var paramTablePlotly = [{
                        type: "table",
                        header: {
                            values: [["<b>Term<b>"], ["<b>Estimate<b>"]],
                            line: { width: 1, color: 'black' },
                            fill: { color: "grey" },
                            font: { family: "Arial", size: 20, color: "white" }
                        },
                        cells: {
                            values: paramTableVals,
                            align: "center",
                            height: 30,
                            line: { color: "black", width: 1 },
                            font: { family: "Arial", size: 16, color: ["black"] }
                        }
                    }];
                } else if (results.multivariate_json.method.startsWith("RANDOM_FOREST")) {
                    var paramTableVals = ['<b>Decision Text<b>', 'Random Forest Tree Here'];
                    var paramTablePlotly = [{
                        type: "table",
                        header: {
                            values: [["<b>Term<b>"], ["<b>Estimate<b>"]],
                            align: "center",
                            line: { width: 1, color: 'black' },
                            fill: { color: "grey" },
                            font: { family: "Arial", size: 20, color: "white" }
                        },
                        cells: {
                            values: paramTableVals,
                            align: "center",
                            height: 30,
                            line: { color: "black", width: 1 },
                            font: { family: "Arial", size: 16, color: ["black"] }
                        }
                    }];
                }
                Plotly.react("param-table", paramTablePlotly);

                var tn = results.multivariate_json.all_data.test.TN;
                var fp = results.multivariate_json.all_data.test.FP;
                var fn = results.multivariate_json.all_data.test.FN;
                var tp = results.multivariate_json.all_data.test.TP;

                var tnpct = Math.round((tn / (tn + fp)) * 100).toString();
                var fppct = Math.round((fp / (tn + fp)) * 100).toString();
                var fnpct = Math.round((fn / (tp + fn)) * 100).toString();
                var tppct = Math.round((tp / (tp + fn)) * 100).toString();

                tn = tn.toString();
                fn = fn.toString();
                fp = fp.toString();
                tp = tp.toString();

                var confusionMatrix = [{
                    x: ['predicted -', 'predicted +'],
                    y: ['actual +', 'actual -'],
                    z: [
                        [0, 1],
                        [1, 0]
                    ],
                    type: 'heatmap',
                    xgap: 3,
                    ygap: 3,
                    colorscale: [[0, '#FED4C4'], [1.0, 'aqua']],
                    showscale: false
                }];

                var cm_layout = {
                    showlegend: false,
                    width: 400,
                    height: 400,
                    annotations: [
                        { font: { family: "Arial", size: 16, color: ["black"] }, x: 0, y: 0, xref: 'x', yref: 'y', text: 'FN='.concat(fn), showarrow: false },
                        { font: { family: "Arial", size: 16, color: ["black"] }, x: 1, y: 1, xref: 'x', yref: 'y', text: 'FP='.concat(fp), showarrow: false },
                        { font: { family: "Arial", size: 16, color: ["black"] }, x: 1, y: 0, xref: 'x', yref: 'y', text: 'TP='.concat(tp), showarrow: false },
                        { font: { family: "Arial", size: 16, color: ["black"] }, x: 0, y: 1, xref: 'x', yref: 'y', text: 'TN='.concat(tn), showarrow: false }
                    ],
                };

                var cmpct_layout = {
                    showlegend: false,
                    width: 400,
                    height: 400,
                    annotations: [
                        { font: { family: "Arial", size: 16, color: ["black"] }, x: 0, y: 0, xref: 'x', yref: 'y', text: 'FN='.concat(fnpct, ' %'), showarrow: false },
                        { font: { family: "Arial", size: 16, color: ["black"] }, x: 1, y: 1, xref: 'x', yref: 'y', text: 'FP='.concat(fppct, ' %'), showarrow: false },
                        { font: { family: "Arial", size: 16, color: ["black"] }, x: 1, y: 0, xref: 'x', yref: 'y', text: 'TP='.concat(tppct, ' %'), showarrow: false },
                        { font: { family: "Arial", size: 16, color: ["black"] }, x: 0, y: 1, xref: 'x', yref: 'y', text: 'TN='.concat(tnpct, ' %'), showarrow: false }
                    ],
                };

                Plotly.newPlot('confusion-matrix', confusionMatrix, cm_layout);
                Plotly.newPlot('confusion-matrix-percent', confusionMatrix, cmpct_layout);




                $("#roc-plot").rocGraph({
                    truePositiveRate: results.multivariate_json.all_data.test.tpr,
                    falsePositiveRate: results.multivariate_json.all_data.test.fpr,
                    currentClassifierTPR: results.multivariate_json.all_data.test.TP / (results.multivariate_json.all_data.test.TP + results.multivariate_json.all_data.test.FN),
                    currentClassifierFPR: results.multivariate_json.all_data.test.FP / (results.multivariate_json.all_data.test.FP + results.multivariate_json.all_data.test.TN),
                    auc: results.multivariate_json.all_data.test.auc
                });

            },

            _createReportView: function (cartDomReportId, results, reqID) {
                var oThis = this;
                $("#cartDomReport").html("");
                var groupList =
                {
                    "CRS3"  : [],
                    "CRS4"  : [],
                    "NEURO3": [],
                    "NEURO4": []
                };
                var pVals =
                {
                    "raw_pvals": [],
                    "adjusted_pvals": []
                }
                var label     = "Response Group"
                var label1    = "Group"
                var pvalLabel = "Pvalue Selection"
                var grpLabel  = "Group Selection"

                $("#cartDomReport").append("<div class='cart-block' style='font-weight: bold;margin:5px;'> Response Selection</div>")
                    .appendSelectType(groupList, label)
                    .appendSelectType(pVals, pvalLabel)
                    .appendSelectType(results.bivariate_json, grpLabel)
                    .append("<div class='cart-block' style='margin:5px;padding-bottom: 15px;'></div>")
                    .append("<input type='button' class='gen-bivarTblGraph-button' value='Generate Report' />")
                    .append("<div class='cart-block' style='font-weight: bold;margin:5px;'> <h3>Bivariate Analysis</h3></div>")
                    .append("<div class='cart-block' style='margin:5px;'></div>")
                    .append(
                        "<div class='cart-container' id='bivar-DataGrph'>" +
                        "<div class='cart-item' id='bivar-table'></div>" +
                        "<div class='cart-item' id='bivar-graph'>" +
                        "<div class='bi-grph-ctrl' id='bivar-graph-control' style='width: 15%; height: 100%; float:left; display:inline-block; '>Graph Control</div>" +
                        "<div class='graph-area' id='biVarGraph' style='width: 85%; height: 100%; float:right; display:inline-block; border: 1px solid;'>Graph Area</div>" +
                        "</div>" +
                        "</div>")
                    .append("<div class='cart-block' id='multivarAnalysis' style='font-weight: bold;margin:5px;'><h3>Multivariate Analysis</h3></div>");


                $("#bivar-graph-control").append(
                    "<div class='cart-block' style='margin:5px;'>X Variable:</div>" +
                    "<select name='x_variable' id='x_bivar' style='width: 70px;text-overflow: ellipsis;'></select>" +

                    "<div class='cart-block' style='margin:5px;'>Y Variable:</div>" +
                    "<select name='y_variable' id='y_bivar' style='width: 70px;text-overflow: ellipsis;'></select>" +

                    "<div class='cart-block' style='margin:5px;'>Graph Type:</div>" +
                    "<select name='Graph Type' id='bivar_graph_type' style='width: 70px;text-overflow: ellipsis;'></select>" +

                    "<div class='cart-block' style='margin:5px;'>Group:</div>" +
                    "<select name='Group' id='bivar_group' style='width: 70px;text-overflow: ellipsis;'></select>" +

                    "<div class='cart-block' style='margin:5px;'>Color By:</div>" +
                    "<select name='Color By' id='bivar_clr_by' style='width: 70px;text-overflow: ellipsis;'></select>" +

                    "<div class='cart-block' style='margin:5px;'></div>" +
                    "<input type='button' value='Submit' class='cart-button' id='bivarGrphBtn' style='width: auto;' />"
                );

                $(".Groupclass").prop("multiple", "true");
                $(".Groupclass").prop("size", 3);

                var selectedGrps     = [];
                var selectedPval     = "raw_pvals";

                var bivar_keys = Object.keys(results.bivariate_json);
                this._generate_bivar_table(results, bivar_keys, selectedPval);
                this._createMultivarView(results);

                this._on({
                    "click.gen-bivarTblGraph-button": function (event) {
                        selectedPval = $(".Pvalueclass :selected").text();
                        cur_grps = $(".Groupclass :selected")
                        selectedGrps = [];
                        for (i = 0; i < cur_grps.length; i++) {
                            selectedGrps.push(cur_grps[i].value)
                        }
                        this._generate_bivar_table(results, selectedGrps, selectedPval);
                        selectedGrps.length = 0;
                    },

                    "change.Pvalueclass": function (event) {
                        selectedPval = $(".Pvalueclass :selected").text();
                    },

                    "change.Groupclass": function (event) {
                        if ($(".Groupclass :selected").length > 4) {
                            $(".Groupclass").find("option").removeAttr("selected");
                            alert('You can select upto 4 Groups only');
                        }
                        $(".Groupclass :selected").each(function () {
                            if (selectedGrps.indexOf($(this).val()) === -1) {
                                selectedGrps.push($(this).val());
                            }
                        });

                    }
                });

                $(".Responseclass").change(function () {
                    selRes = this.value; 

                    if (this.value == "CRS4") {
                        vjDS["crs4DomainReport"].reload("http:

                    } else if (this.value == "NEURO3") {
                        vjDS["neuro3DomainReport"].reload("http:
                    } else if (this.value == "NEURO4") {
                        vjDS["neuro4DomainReport"].reload("http:
                    } else if (this.value == "CRS3") {
                        vjDS["crs3DomainReport"].reload("http:
                    }
                    else {
                        alert("Choose right response");
                    }

                    
                });

                vjDS["biVarDataDomainReport"].reload("http:
 

                $(".Responseclass").val(selRes);

            },

            _initVariables: function () {
                this._StudyRange  = ["NONE", "NONE"];
                this._CRSRange    = [0, 28];
                this._NtoxicRange = [0, 28];
                this.selGrp       = 'STUDYID';
                this._AERange     = [0, 28];
            },

            _create: function () {
                this._initVariables();

                var oThis = this;
                $(document.createElement('div'))
                    .attr("id", "cartDomReport")
                    .appendTo(this.element);

                var reqID;

                vjDS.add("Submitting bivar crs3"          , "crs3DomainReport"     , "static:
                vjDS.add("Submitting bivar crs4"          , "crs4DomainReport"     , "static:
                vjDS.add("Submitting bivar neuro3"        , "neuro3DomainReport"   , "static:
                vjDS.add("Submitting bivar neuro4"        , "neuro4DomainReport"   , "static:
                vjDS.add("Submitting query for bivar data", "biVarDataDomainReport", "static:

                vjDS["crs3DomainReport"].register_callback(function (structor, content) {

                    var results = JSON.parse(content);
                    reqID = vjDS["crs3DomainReport"].qpbg_params.RQ.dataID;
                    oThis._createReportView(cartDomReport, results, reqID);

                });

                vjDS["neuro3DomainReport"].register_callback(function (structor, content) {
                    var results = JSON.parse(content);
                    oThis._createReportView(cartDomReport, results, reqID);
                });

                vjDS["crs4DomainReport"].register_callback(function (structor, content) {
                    var results = JSON.parse(content);
                    oThis._createReportView(cartDomReport, results, reqID);
                });

                vjDS["neuro4DomainReport"].register_callback(function (structor, content) {
                    var results = JSON.parse(content);
                    oThis._createReportView(cartDomReport, results, reqID);
                });

                vjDS["biVarDataDomainReport"].register_callback(function (structor, content) {
                    var res = content.replace(/NaN,/g, "");
                    $("#biVarGraph").append($("#canvas")).cart_bivar_graph({ value: selGrp, data: res });

                });

                this._drawFilterSubmission("cartDomReport");

            }

        });

});
