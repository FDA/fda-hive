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
$(function()
{

$.widget("cart.patientViewGraph",
{
    
    draw: function(patientId)
    {
        
    },
    
    toImage: function()
    {
        return Plotly.toImage(this._canvasId, {format: 'png', width: this.getWidth(), height: this.getHeight()});
    },
    
    getHeight: function()
    {
        return document.getElementById(this._canvasId).offsetHeight;
    },
    
    getWidth: function()
    {
        return document.getElementById(this._canvasId).offsetWidth;
    },
    
    _init: function()
    {
        this._canvasId = null;
    }
    
});
    
$.widget("cart.demoTable", $.cart.patientViewGraph,
{

    options:
    {
        height: 200,
        leftMargin: 300,
        rightMargin: 100,
        topMargin: 0,
        bottomMargin: 0,
        width: 900
    },
    
    draw: function(patientId)
    {
        patientData = this._patientDemoDic[patientId];
        var tableValues = [
            ["Patient ID: " + patientData["USUBJID"], "Study: " + patientData["STUDYID"],
             "Sex: " + patientData["SEX"], "Ethnicity: " + patientData["ETHNIC"],
             "Cause of Death: " + patientData["CAUSE_OF_DEATH_MAX"]],
            ["Country: " + patientData["COUNTRY"],  "Age: " + patientData["AGE"],
             "Race: " + patientData["RACE"], "Death Status: " + patientData["STATUS_MAX"]]
        ];
        var tableData = [
        {
            type: "table",
            cells:
            {
                values: tableValues,
                align: ["left", "left"]
            }
        }];
        var tableLayout =
        {
            margin:
            {
                l: this.options.leftMargin,
                r: this.options.rightMargin,
                b: this.options.bottomMargin,
                t: this.options.topMargin
            }
        };
        

        $("#" + this._canvasId).css("height", this.options.height + "px");
        $("#" + this._canvasId).css("width", this.options.width + "px");
        Plotly.newPlot(this._canvasId, tableData, tableLayout);
    },
    
    loadDictionary: function(demoDic)
    {
        this._patientDemoDic = demoDic;
    },
    
    loadCSV: function(csv)
    {
        this._patientDemoDic = CartLib.csvToSumDic(csv);
    },
    
    _init: function()
    {
        this._canvasId = "cart-demoTable" +  parseInt(Math.random() * 100000);
        this._patientDemoDic = {};
        $("<div>").prop("id", this._canvasId)
            .appendTo(this.element);
    }
    
});
    
$.widget("cart.timelineGraph", $.cart.patientViewGraph, 
{
    
    options:
    {
        size: 80,
        severityToColor:
        {
            1: "rgb(2, 1, 181)",
            2: "rgb(212, 0, 22)",
            3: "rgb(14, 185, 19)",
            4: "rgb(83, 4, 93)"
        },
        aeToGraph: ["CRS_TOX", "NEURO_TOX", "CARDIAC_TOX"],
        leftMargin: 300,
        rightMargin: 100,
        topMargin: 0,
        bottomMargin: 0,
        width: 900,
        timeRange: [-14, 30]
    },

    loadDictionary: function(aeDic)
    {
        this._patientAEDic = aeDic;
    },
    
    loadCSV: function(csv)
    {
        this._patientAEDic = CartLib.csvToTSDic(csv);
    },
    
    setAEToGraph: function(list)
    {
        this._aeToGraph = list;
    },
    
    
    draw: function(patientId)
    {
        var patientData = this._patientAEDic[patientId];
        
        var aeList = this._aeToGraph.clone().reverse();
        var tickVals = aeList.map(function(val, index)
        {
            return index;
        });
        
        var startDay = this.options.timeRange[0];
        var endDay = this.options.timeRange[1];
        
        var aeLayout =
        {
            margin:
            {
                t: this.options.topMargin,
                b: this.options.bottomMargin,
                l: this.options.leftMargin,
                r: this.options.rightMargin
            },
            xaxis:
            {
                autorange: false,
                range: [startDay, endDay],
                zeroline: false,
                title: "Study Days Since Initial Treatment"
            },
            yaxis:
            {
                autorange: false,
                range: [-1, aeList.length],
                zeroline: false,
                ticktext: aeList, 
                tickvals: tickVals
            },
            showlegend: true,
            legend:
            {
                x: 1,
                y: 0.7,
                xanchor: 'left',
                yanchor: 'top'
            },
            annotations:[
            {
                xref: 'paper',
                yref: 'paper',
                xanchor: 'left',
                yanchor: 'bottom',
                text: 'Analysis Toxicity Grade',
                showarrow: false,
                x: 1,
                y: 0.7
            }
            ],
            shapes: []
        };
        
        if (patientData !== undefined)
        {
        for (var i = 0; i < aeList.length; ++i)
        {
            if (patientData[aeList[i]] === undefined)
            {
                continue;
            }
            var ae = patientData[aeList[i]];
            
            var sevToColor = this.options.severityToColor;
            
            var numOfAEToGraph = this._aeToGraph.length > 0 ? this._aeToGraph.length : 1;
            var graphHeight = (this.options.size * numOfAEToGraph) + this.options.bottomMargin + this.options.topMargin;
            
            var barWidth = 50 / graphHeight;
            
            for (var j = 0; j < ae.length; ++j)
            {
                aeLayout.shapes.push(
                {
                    fillcolor: sevToColor[ae[j].severity],
                    line: {width: 0},
                    opacity: 1,
                    type: 'rect',
                    x0: Math.round(ae[j].startDay),
                    x1: Math.round(ae[j].endDay),
                    xref: 'x',
                    y0: i - (barWidth/2),
                    y1: i + (barWidth/2),
                    yref: 'y'
                });
            }
        }
        }
        
        
        var aeData = [];
        for (var severity in this.options.severityToColor)
        {
            aeData.push(
            {
                x: [-2],
                y: [-2],
                mode: "lines+markers",
                type: "scatter",
                xaxis: "x",
                yaxis: "y",
                name: severity,
                line: { color: this.options.severityToColor[severity] }
            });
        }
        $("#" + this._canvasId).css("height", graphHeight + "px");
        $("#" + this._canvasId).css("width", this.options.width + "px");
        Plotly.newPlot(this._canvasId, aeData, aeLayout);
    },
    
    _init: function()
    {
        this._canvasId = "cart-timelineGraph" +  parseInt(Math.random() * 100000);
        $("<div>").prop("id", this._canvasId)
            .appendTo(this.element);
        this._aeToGraph = this.options.aeToGraph;
        this._patientAEDic = {};
    }
    
});

$.widget("cart.testGraph", $.cart.patientViewGraph,
{
    
    options:
    {
        lineColor: 'rgb(55, 128, 191)',
        sizePerGraph: 80,
        testsToGraph: [ "IL6", "IFNG", "IL12" ],
        leftMargin: 300,
        rightMargin: 100,
        bottomMargin: 50,
        topMargin: 0,
        marginBetweenGraphs: 25,
        width: 900,
        timeRange: [-14, 30]
    },
    
    loadDictionary: function(labDic)
    {
        this._patientLabDic = labDic;
    },
    
    loadCSV: function(csv)
    {
        this._patientLabDic = CartLib.csvToTSDic(csv);
    },
    
    setTestsToGraph: function(list)
    {
        this._testsToGraph = list;
    },
    
    draw: function(patientId)
    {
        var patientData = this._patientLabDic[patientId];
        var testsToGraph = this._testsToGraph.clone().reverse();
        
        var labData = this._genLabData(patientData, testsToGraph);
        var labLayout = this._genLabLayout(testsToGraph, this.options.timeRange, patientId);

        $("#" + this._canvasId).css("height", this._calcCanvasHeight() + "px");
        $("#" + this._canvasId).css("width", this.options.width + "px");
        Plotly.newPlot(this._canvasId, labData, labLayout);
    },
    
    _genLabData: function(patientData, testsToGraph)
    {
        var labData = [];

        for (var i = 0; i < testsToGraph.length; ++i)
        {
            var test = patientData[testsToGraph[i]];
            
            var x = [];
            var y = [];
            
            if (test !== undefined)
            {
                test.sort(function(a, b)
                {
                    if (a.studyDay < b.studyDay)
                    {
                        return -1;
                    }
                    else if (a.studyDay > b.studyDay)
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                });
                
                for (var j = 0; j < test.length; ++j)
                {
                    if (!Number.isNaN(test[j].result) &&  test[j].result !== undefined)
                    {
                        x.push(test[j].studyDay);
                        y.push(test[j].result);
                    }
                }
            }
            
            labData.push(
            {
                x: x,
                y: y,
                mode: "lines+markers",
                type: "scatter",
                xaxis: "x" + (i + 1),
                yaxis: "y" + (i + 1),
                name: testsToGraph[i],
                line:
                {
                    color: this.options.lineColor
                }
            });
        }
        
        return labData;
    },
    
    _genLabLayout: function(testsToGraph, timeRange, patientId)
    {
        var labLayout =
        {
            showlegend: false,
            margin:
            {
                t: this.options.topMargin, 
                b: this.options.bottomMargin,
                l: this.options.leftMargin,
                r: this.options.rightMargin
            },
            annotations: []
        };
        
        var pxlBuf = this.options.marginBetweenGraphs;
        var pxlGraph = this.options.sizePerGraph;
        
        var canvasHeight = this._calcCanvasHeight();
        
        for (var i = 1; i < testsToGraph.length + 1; ++i)
        {
            var units = this.options.testToUnits[patientId][testsToGraph[i-1]];
            var title = testsToGraph[i-1];
            if (units !== undefined)
            {
                title += "<br>(" + units + ")";
            }
            
            var pxlStartY = (pxlGraph + pxlBuf) * (i-1) + pxlBuf;
            var pxlEndY =  (pxlGraph + pxlBuf) * i;
            var paperStartY = pxlStartY / canvasHeight;
            var paperEndY = pxlEndY / canvasHeight;
            
            labLayout["xaxis"+i] = 
            {
                anchor: "y"+i, 
                domain: [0.0, 1.0],
                range: timeRange,
                zeroline: false
            };
            
            if (i > 1)
            {
                labLayout["xaxis"+i].showticklabels = false;    
            }
            
            labLayout["yaxis"+i] =
            {
                anchor: "x"+i,
                domain: [paperStartY, paperEndY],
                side: "right",
                zeroline: false
            };
            
            labLayout["annotations"].push(
            {
                text: title,
                xanchor: 'right',
                yanchor: 'bottom',
                x: 0,
                y: (paperStartY + paperEndY) / 2,
                xref: 'paper',
                yref: 'paper',
                showarrow: false
            });
        }
        
        return labLayout;
    },
    
    _calcCanvasHeight: function()
    {
        var numOfGraphs = this._testsToGraph.length > 0 ? this._testsToGraph.length : 1;
        return (numOfGraphs * (this.options.marginBetweenGraphs + this.options.sizePerGraph)) + this.options.bottomMargin;
    },
        
    _init: function()
    {
        this._canvasId = "cart-testGraph" +  parseInt(Math.random() * 100000);
        $("<div>").prop("id", this._canvasId)
            .appendTo(this.element);
        this._patientLabDic =  {};
        this._testsToGraph = this.options.testsToGraph;
    }
    
});
    
$.widget("cart.cart_report",
{
    
    _getDemoQry: function(studies)
    {
        var demoQry = {
            "query_type": "PATIENT_LEVEL",
            "global_filters": [
                {
                    "col": "STUDYID",
                    "condition": "isin",
                    "value": studies
                }
            ],
            "sub_queries": [
                {
                    "domain": "DEMOGRAPHICS",
                    "keep_columns": [],
                    "converts": [],
                    "filters": [],
                    "interpolate": null,
                    "rescale": null,
                    "agg_function": null,
                    "agg_show_all": null,
                    "agg_converts": []
                },
                {
                    "domain": "DEATH",
                    "keep_columns": [],
                    "converts": [],
                    "filters": [],
                    "interpolate": null,
                    "rescale": null,
                    "agg_function": "first",
                    "agg_show_all": null,
                    "agg_converts": []
                }
            ]
        }
        return demoQry;
    },
    
    _getLabQry: function(studies, timeRange)
    {
        var labQry = {
            query_type: "TIME_SERIES",
            global_filters: [
                {
                    col: "STUDYID",
                    condition: "isin",
                    value: studies
                },
                {
                    col: "TS_DATE_RANGE",
                    condition: "between",
                    value: timeRange
                }
            ],
            sub_queries: [
                {
                    domain: "LAB",
                    keep_columns: [],
                    converts: [
                    {
                        "mthd": "NUMERIC",
                        "args": null,
                        "col": "LBORRES"
                      },
                      {
                        "mthd": "CENSOR",
                        "args": null,
                        "col": "LBORRES"
                      }
                    ],
                    filters: [],
                    interpolate: null,
                    rescale: null,
                    agg_function: null,
                    agg_show_all: null,
                    agg_converts: []
                }
            ]
        }
        return labQry;
    },
    
    _getAEQry: function(studies, timeRange)
    {
        var aeQry =
                {
            query_type: "RAW",
            query: {
                    domain: "ADVERSE_EVENTS",
                    keep_columns: [],
                    converts: [],
                    filters: [
                            {
                            "col"      : "STUDYID",
                            "condition": "isin",
                            "value"    : studies
                            },
                            {
                             "col"      : "AESCAT_MAP",
                             "condition": "isin",
                             "value"    : ["CRS_TOX", "NEURO_TOX", "CARDIAC_TOX"]
                            },
                            {
                             "col"      : "ae_study_day",
                             "condition": "between"     ,
                             "value"    : timeRange
                            }

                    ],
                    interpolate: null,
                    rescale: null,
                    agg_function: null,
                    agg_show_all: null,
                    agg_converts: []
                }
        };
        return aeQry;
    },
    
    _getAESpecQry: function(studies, timeRange)
    {
        var aeQry =
        {
            query_type: "RAW",
            query: {
                domain: "ADVERSE_EVENTS",
                keep_columns: [],
                converts: [],
                filters: [
                        {
                        "col"      : "STUDYID",
                        "condition": "isin",
                        "value"    : studies
                        },
                        {
                         "col"      : "ae_study_day",
                         "condition": "between"     ,
                         "value"    : timeRange
                        }

                ],
                interpolate : null,
                rescale     : null,
                agg_function: null,
                agg_show_all: null,
                agg_converts: []
            }
            
        };
        return aeQry;
    },
    
    _drawFilterSubmission: function(cartReportId)
    {
        var oThis = this;

        var prodClass =
        {
            "ALL": ["CCTL019B2202","CCTL019B2205J","CCTL019B2101J","CCTL019C2201","KTE-C19-101", "KTE-C19-102", "KTE-C19-103", "KTE-C19-104", "KITE-585-501", "015001","017001"],
            "CD19_CART": ["CCTL019B2202","CCTL019B2205J","CCTL019B2101J","CCTL019C2201","KTE-C19-101","KTE-C19-102", "KTE-C19-103", "KTE-C19-104","015001","017001"],
            "MM_CART": ["KITE-585-501"]
            
        };
        var reportType =
        {
            "Report-1": [],
            "Report-2": [],
            "Report-3": []
            
        };

        $("#cartReport").append("<div class='cart-block' style='font-weight: bold;margin:5px;'>Report Submission</div>")
            .appendStudySelect(prodClass, 11)
            .appendTimeSelect(-14, 30);

        $("<input type='button' value='Submit' class='cart-button' />")
            .click(function()
            {
                var studies = $("#cartReport").find(".cart-studySel").val() || prodClass[$("#cartReport").find(".cart-prodClassSel").val()];
                var start = parseInt($("#cartReport").find(".cart-startSel").val());
                var end = parseInt($("#cartReport").find(".cart-EndSel").val());
                oThis._dateRangeFilter = [start, end];
                
                var demoQry = oThis._getDemoQry([studies]);
                var labQry = oThis._getLabQry([studies], [start, end]);
                var aeQry = oThis._getAEQry([studies], [start, end]);
                var aeSpecQry = oThis._getAESpecQry([studies], [start, end]);
                
                var demoStr = "qpbg_cart_query:
                var labStr = "qpbg_cart_query:
                var aeStr = "qpbg_cart_query:
                var aeSpecStr = "qpbg_cart_query:
                
                vjDS["dsReportDemo"].reload(demoStr, true);
                vjDS["dsReportLab"].reload(labStr, true);
                vjDS["dsReportAE"].reload(aeStr, true);
                vjDS["dsReportAESpec"].reload(aeSpecStr, true);
            })
            .appendTo("#cartReport");
    },
    
    _drawGraphs: function(patientId)
    {    
        $("#tableCanvas").demoTable("draw", patientId);
        $("#labCanvas").testGraph("draw", patientId);
        $("#aeSpecCanvas").timelineGraph("draw", patientId);
    },
    
    _drawPatientView: function(cartReportId)
    {    
        var oThis = this;
        
        $("#cartReport").html("");
        
        var graphWidth = 1100;
        var leftMargin = 300;
        var rightMargin = 200;
        
        var mergedAEDic = {};
        for (var aePat in this._patientAEDic)
        {
            mergedAEDic[aePat] = {};
            for (var ae in this._patientAEDic[aePat])
            {
                mergedAEDic[aePat][ae] = this._patientAEDic[aePat][ae];
            }
            for (var ae in this._patientAESpecDic[aePat])
            {
                mergedAEDic[aePat][ae] = this._patientAESpecDic[aePat][ae];
            }
        }
        var mergedAEColList = this._aeSpecColList.concat(this._aeColList);
        
        $("#" + cartReportId).append("Patient: ");
        var patientSelect = $("<select id='patientSelect'></select>");
        for (id in this._patientDemoDic)
        {
            patientSelect.append("<option value='" + id + "'>" + id + "</option>");
        }
        patientSelect.change(function()
        {
            var id = $(this).val();
            oThis._drawGraphs(id);
            oThis._createDownloadImage(downloadLink, "tableCanvas", "labCanvas", "aeSpecCanvas");
        });
        $("#" + cartReportId).append(patientSelect);
        
        $("<button>").prop("type", "button")
            .text("<")
            .click(function()
            {
                var curIndex = patientSelect.prop("selectedIndex");
                var totalItems = patientSelect.find("option").length;
                patientSelect.prop("selectedIndex", (curIndex + totalItems - 1) % totalItems);
                oThis._drawGraphs(patientSelect.val());
                oThis._createDownloadImage(downloadLink, "tableCanvas", "labCanvas", "aeSpecCanvas");
            })
            .appendTo("#" + cartReportId);
        $("<button>").prop("type", "button")
            .text(">")
            .click(function()
            {
                var curIndex = patientSelect.prop("selectedIndex");
                var totalItems = patientSelect.find("option").length;
                patientSelect.prop("selectedIndex", (curIndex + 1) % totalItems);
                oThis._drawGraphs(patientSelect.val());
                oThis._createDownloadImage(downloadLink, "tableCanvas", "labCanvas", "aeSpecCanvas");
            })
            .css("margin-right", "20px")
            .appendTo("#" + cartReportId);
        
        var downloadLink = $("<a>Download Patient Report</a>").appendTo("#" + cartReportId);
        
        $("<a>Return to Report Submission</a>")
            .click(function()
            {
                oThis._initVariables();
                $("#cartReport").empty();
                oThis._drawFilterSubmission("cartReport");
            })
            .css("float", "right")
            .appendTo("#" + cartReportId);
        
        var tableCanvas = $("<div id='tableCanvas'></div>").demoTable(
            {
                height: 200,
                leftMargin: leftMargin,
                rightMargin: rightMargin,
                topMargin: 0,
                bottomMargin: 0,
                width: graphWidth
            })
            .appendTo("#" + cartReportId);
        tableCanvas.demoTable("loadDictionary", this._patientDemoDic);
        
        var labOptions = $("<div>").addClass("cart-options")
            .css("margin-left", "30px")
            .css("display", "none");
        $("<button>").text("Lab Options")
            .prop("type", "button")
            .addClass("cart-collapsible")
            .addClass("cart-collapsed")
            .click(function()
            {
                labOptions.toggle();
                $(this).toggleClass("cart-collapsed");
                $(this).toggleClass("cart-open");
            })
            .appendTo("#" + cartReportId);
        labOptions.appendTo("#" + cartReportId);
        
        var labColSel = $("<div>");
        
        var labCanvas = $("<div id='labCanvas'></div>").appendTo("#" + cartReportId);
        labCanvas.testGraph(
        {
            lineColor: 'rgb(55, 128, 191)',
            sizePerGraph: 60,
            testsToGraph: [ "IL6", "IFNG", "IL12" ],
            leftMargin: leftMargin,
            rightMargin: rightMargin,
            bottomMargin: 50,
            topMargin: 0,
            marginBetweenGraphs: 25,
            width: graphWidth,
            timeRange: this._dateRangeFilter,
            testToUnits: this._testToUnits
        });
        labCanvas.testGraph("loadDictionary", this._patientLabDic);
        
        labColSel.columnSelect(
            {
                colList: this._labColList,
                selectedOnStart: [ "IL6", "IFNG", "IL12" ],
                funcOnChange: function()
                {
                    var cols = labColSel.columnSelect("getSelectedColumns");
                    labCanvas.testGraph("setTestsToGraph", cols);
                    labCanvas.testGraph("draw", patientSelect.val());
                    oThis._createDownloadImage(downloadLink, "tableCanvas", "labCanvas", "aeSpecCanvas");
                },
                dropdownWidth: 150,
                dropdownLabel: "Lab Test:"
            })
            .appendTo(labOptions);
                
        var aeOptions = $("<div>").addClass("cart-options")
            .css("margin-left", "30px")
            .css("display", "none");
        $("<div>").text("Adverse Event Options")
            .addClass("cart-collapsible")
            .addClass("cart-collapsed")
            .click(function()
            {
                aeOptions.toggle();
                $(this).toggleClass("cart-collapsed");
                $(this).toggleClass("cart-open");
            })
            .appendTo("#" + cartReportId);
        aeOptions.appendTo("#" + cartReportId);
        
        var aeColSel = $("<div>").appendTo(aeOptions);
        
        var aeSpecCanvas = $("<div id='aeSpecCanvas'></div>").appendTo("#" + cartReportId);
        aeSpecCanvas.timelineGraph(
        {
            aeToGraph: [ "CRS_TOX", "NEURO_TOX", "CARDIAC_TOX", "Nausea", "Hypotension", "Hypertension" ],
            size: 40,
            severityToColor:
            {
                1: "rgb(2, 1, 181)",
                2: "rgb(212, 0, 22)",
                3: "rgb(14, 185, 19)",
                4: "rgb(83, 4, 93)"
            },
            leftMargin: leftMargin,
            rightMargin: rightMargin,
            topMargin: 0,
            bottomMargin: 50,
            width: graphWidth,
            timeRange: this._dateRangeFilter
        });
        aeSpecCanvas.timelineGraph("loadDictionary", mergedAEDic);

        aeColSel.columnSelect(
            {
                selectedOnStart: [ "CRS_TOX", "NEURO_TOX", "CARDIAC_TOX", "Nausea", "Hypotension", "Hypertension" ],
                colList: mergedAEColList,
                funcOnChange: function()
                {
                    var cols = aeColSel.columnSelect("getSelectedColumns");
                    aeSpecCanvas.timelineGraph("setAEToGraph", cols);
                    aeSpecCanvas.timelineGraph("draw", patientSelect.val());
                    oThis._createDownloadImage(downloadLink, "tableCanvas", "labCanvas", "aeSpecCanvas");
                },
                dropdownWidth: 150,
                dropdownLabel: "Adverse Event:"
            })
            .appendTo(aeOptions);
        
        this._drawGraphs(Object.keys(this._patientDemoDic)[0]);
        
        this._createDownloadImage(downloadLink, "tableCanvas", "labCanvas", "aeSpecCanvas");
    },
    
    _createDownloadImage: function(downloadLink, tableCanvas, labCanvas, aeSpecCanvas)
    {
        var tableWidth = $("#" + tableCanvas).demoTable("getWidth");
        var tableHeight = $("#" + tableCanvas).demoTable("getHeight");
        
        var labWidth = $("#" + labCanvas).testGraph("getWidth");
        var labHeight = $("#" + labCanvas).testGraph("getHeight");
        
        var aeSpecWidth = $("#" + aeSpecCanvas).timelineGraph("getWidth");
        var aeSpecHeight = $("#" + aeSpecCanvas).timelineGraph("getHeight");
        
        var tmpCanvas = document.createElement('canvas');
        tmpCanvas.width = labWidth;
        tmpCanvas.height = tableHeight + labHeight + aeSpecHeight;
        tmpCtx = tmpCanvas.getContext("2d");
        
        var tableImgLoaded = false;
        var labImgLoaded = false;
        var aeSpecImgLoaded = false;
        
        function updateDownloadLinkIfReady()
        {
            if (tableImgLoaded && labImgLoaded && aeSpecImgLoaded)
            {
                downloadLink.prop("href", tmpCanvas.toDataURL("image/png"))
                    .prop("download", "download");
            }
        }
        
        $("#" + tableCanvas).demoTable("toImage").then(function(dataURL)
        {
            var tableImage = new Image();
            tableImage.src = dataURL;
            tableImage.onload = function()
            {
                tmpCtx.drawImage(tableImage, 0, 0);
                tableImgLoaded = true;
                updateDownloadLinkIfReady();
            }
            return $("#" + labCanvas).testGraph("toImage");
        }).then(function(dataURL)
        {
            var labImage = new Image();
            labImage.src = dataURL;
            labImage.onload = function()
            {
                tmpCtx.drawImage(labImage, 0, tableHeight);
                labImgLoaded = true;
                updateDownloadLinkIfReady();
            }
            return $("#" + aeSpecCanvas).timelineGraph("toImage");
        }).then(function(dataURL)
        {
            var aeSpecImage = new Image();
            aeSpecImage.src = dataURL;
            aeSpecImage.onload = function()
            {
                tmpCtx.drawImage(aeSpecImage, 0, tableHeight + labHeight);
                aeSpecImgLoaded = true;
                updateDownloadLinkIfReady();
            }
        });
    },
    
    _initVariables: function()
    {
        this._patientDemoDic = {};
        this._patientLabDic = {};
        this._patientAEDic = {};
        this._patientAESpecDic = {};
        this._demoQryDone = false;
        this._labQryDone = false;
        this._aeQryDone = false;
        this._aeSpecQryDone = false;        
        this._labColList = [];
        this._aeColList = [];
        this._aeSpecColList = [];
        this._dateRangeFilter = [-14, 30];
    },
    
    _getAEDic: function(content, aeDic, colSet)
    {
        var csv = CSVToArray(content);
        
        var patientIndex = csv[0].indexOf("USUBJID");
        var testTypeIndex = csv[0].indexOf("AEDECOD");
        var studyDayIndex = csv[0].indexOf("ae_study_day");
        var endDayIndex = csv[0].indexOf("ae_end_study_day");
        var resultIndex = csv[0].indexOf("AETOXGR");

        for (var i = 1; i < csv.length; ++i)
        {
            if (csv[i].length <= 1)
            {
                continue;
            }
            var patient = csv[i][patientIndex];
            var adverseEvent = csv[i][testTypeIndex];
            var studyDay = parseFloat(csv[i][studyDayIndex]);
            
            var endDay;
            if (csv[i][endDayIndex] === undefined)
            {
                endDay = studyDay + 1;
            }
            else
            {
                endDay = parseFloat(csv[i][endDayIndex]);
            }
            
            var result = parseInt(csv[i][resultIndex]);
            
            if (aeDic[patient] === undefined)
            {
                aeDic[patient] = {};
            }
            if (aeDic[patient][adverseEvent] === undefined)
            {
                aeDic[patient][adverseEvent] = [];
            }
            aeDic[patient][adverseEvent].push(
            {
                adverseEvent: adverseEvent,
                startDay: studyDay,
                endDay: endDay,
                severity: result
            });
            
            colSet.add(adverseEvent);
        }
    },
    
    _getAESpecDic: function(content, aeDic, colSet)
    {
        var csv = CSVToArray(content);
        
        var patientIndex = csv[0].indexOf("USUBJID");
        var testTypeIndex = csv[0].indexOf("AESCAT_MAP");
        var studyDayIndex = csv[0].indexOf("ae_study_day");
        var endDayIndex = csv[0].indexOf("ae_end_study_day");
        var resultIndex = csv[0].indexOf("AETOXGR");

        for (var i = 1; i < csv.length; ++i)
        {
            if (csv[i].length <= 1)
            {
                continue;
            }
            var patient = csv[i][patientIndex];
            var adverseEvent = csv[i][testTypeIndex];
            var studyDay = parseFloat(csv[i][studyDayIndex]);
            
            var endDay;
            if (csv[i][endDayIndex] === undefined)
            {
                endDay = studyDay + 1;
            }
            else
            {
                endDay = parseFloat(csv[i][endDayIndex]);
            }
            
            var result = parseInt(csv[i][resultIndex]);
            
            if (aeDic[patient] === undefined)
            {
                aeDic[patient] = {};
            }
            if (aeDic[patient][adverseEvent] === undefined)
            {
                aeDic[patient][adverseEvent] = [];
            }
            aeDic[patient][adverseEvent].push(
            {
                adverseEvent: adverseEvent,
                startDay: studyDay,
                endDay: endDay,
                severity: result
            });
            
            colSet.add(adverseEvent);
        }
    },
    
    _create: function()
    {
        this._initVariables();
        
        var oThis = this;
        $(document.createElement('div'))
            .attr("id", "cartReport")
            .appendTo(this.element);
        
        vjDS.add("Submitting report demographic query", "dsReportDemo", "static:
        vjDS["dsReportDemo"].register_callback(function(structor, content)
        {
            oThis._patientDemoDic = CartLib.csvToSumDic(content);
            oThis._demoQryDone = true;
            if (oThis._demoQryDone && oThis._labQryDone && oThis._aeQryDone && oThis._aeSpecQryDone)
            {
                oThis._drawPatientView("cartReport");
            }
        });
        
        vjDS.add("Submitting report lab query", "dsReportLab", "static:
        vjDS["dsReportLab"].register_callback(function(structor, content)
        {
            
            
            var labDic = {};
            var colSet = new Set();
            
            var csv = CSVToArray(content);
            
            oThis._testToUnits = {};
            
            var patientIndex = csv[0].indexOf("USUBJID");
            var testTypeIndex = csv[0].indexOf("test");
            var studyDayIndex = csv[0].indexOf("study_day");
            var resultIndex = csv[0].indexOf("result");
            var unitsIndex = csv[0].indexOf("units");

            for (var i = 1; i < csv.length; ++i)
            {
                if (csv[i].length <= 1)
                {
                    continue;
                }
                var patient = csv[i][patientIndex];
                var testType = csv[i][testTypeIndex];
                var studyDay = csv[i][studyDayIndex];
                var result = csv[i][resultIndex];
                var units = csv[i][unitsIndex];
                
                if (labDic[patient] === undefined)
                {
                    labDic[patient] = {};
                }
                if (labDic[patient][testType] === undefined)
                {
                    labDic[patient][testType] = [];
                }
                labDic[patient][testType].push(
                {
                    studyDay: parseFloat(studyDay),
                    result: parseFloat(result)
                });
                
                colSet.add(testType);
                if (oThis._testToUnits[patient] == undefined)
                {
                    oThis._testToUnits[patient] = {};
                }
                if (oThis._testToUnits[patient][testType] == undefined)
                {
                    oThis._testToUnits[patient][testType] = units;
                }
            }
            
            oThis._patientLabDic = labDic;
            oThis._labColList = Array.from(colSet);
            oThis._labColList.sort();
            
            oThis._labQryDone = true;
            if (oThis._demoQryDone && oThis._labQryDone && oThis._aeQryDone && oThis._aeSpecQryDone)
            {
                oThis._drawPatientView("cartReport");
            }
        });
        
        vjDS.add("Submitting report adverse events query", "dsReportAE", "static:
        vjDS["dsReportAE"].register_callback(function(structor, content)
        {
            oThis._patientAEDic = {};
            var colSet = new Set();
            oThis._getAEDic(content, oThis._patientAEDic, colSet);
            oThis._aeColList = Array.from(colSet);
            oThis._aeColList.sort();
            
            oThis._aeQryDone = true;
            if (oThis._demoQryDone && oThis._labQryDone && oThis._aeQryDone && oThis._aeSpecQryDone)
            {
                oThis._drawPatientView("cartReport");
            }
        });
        
        vjDS.add("Submitting specific report adverse events query", "dsReportAESpec", "static:
        vjDS["dsReportAESpec"].register_callback(function(structor, content)
        {
            oThis._patientAESpecDic = {};
            var colSet = new Set();
            oThis._getAESpecDic(content, oThis._patientAESpecDic, colSet);
            oThis._aeSpecColList = Array.from(colSet);
            oThis._aeSpecColList.sort();
            
            oThis._aeSpecQryDone = true;
            if (oThis._demoQryDone && oThis._labQryDone && oThis._aeQryDone && oThis._aeSpecQryDone)
            {
                oThis._drawPatientView("cartReport");
            }
        });
        
        this._drawFilterSubmission("cartReport");
    }

});

});