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

$.widget("cart.analysisSubmit", {
    
    options: {
        analysisTypes: [],
        features: [],
        responseVars: [],
        defaultFeatureTypeMap: {},
        defaultValues: {
            dropThreshold: "0.65",
            crossValFolds: 5
        }
    },
    
    _getSelectedFeatures: function() {
        var features = [];
        this.element.find(".selected-features")
            .children()
            .each(function() {
                features.push({
                    featureName: $(this).find(".feature-name").val(),
                    featureType: $(this).find(".feature-type").val()
                });
            });
        return features;
    },
    
    _getFormData: function() {
        var formData = {};
        
        formData.analysisType = this.element.find(".analysis-type").val();
        formData.features = this._getSelectedFeatures();
        formData.responseVar = this.element.find(".response-var").val();
        formData.dropThreshold = parseFloat(this.element.find(".drop-threshold").val());
        
        formData.validationType = this.element.find(".validation-type").val();
        formData.validationOptions = {};
        if (formData.validationType === "Cross-validation") {
            formData.validationOptions.folds = parseInt(this.element.find(".cross-val-folds").val());
        } else if (formData.validationType === "Holdout Validation") {
            formData.validationOptions.holdoutPercent = parseFloat(this.element.find(".holdout-percent").val());
        }
        
        return formData;
    },
    
    _addFeature: function() {
        var featureBlock = $(
                "<div class='cart-block'>" +
                "  <div class='cart-label'></div>" +
                "  <select class='feature-name'></select>" +
                "  <select class='feature-type'>" + 
                "    <option value='Categorical'>Categorical</option>" +
                "    <option value='Continuous'>Continuous</option>" +
                "  </select>" + 
                "  <button type='button' class='remove-button'>-</button>" +
                "  <button type='button' class='add-button'>+</button>" +
                "</div>"
        );
        
        var featureSelect = featureBlock.find(".feature-name");
        $.each(this.options.features, function(index, value) {
            $("<option/>").val(value).text(value).appendTo(featureSelect);
        });
        
        this.element.find(".selected-features").append(featureBlock);
    },
    
    _checkValidForm: function(formData) {
        var problem = {
            isValid: true,
            message: ""
        };
        
        if (formData.features.length <= 0) {
            problem.isValid = false;
            problem.message += "Must select at least one feature\n";
        }
        
        if (isNaN(formData.dropThreshold) || formData.dropThreshold < 0 || formData.dropThreshold > 100) {
            problem.isValid = false;
            problem.message += "Drop Threshold must be a number between 0 and 100\n";
        }
        
        if (formData.validationType === "Cross-validation") {
            if (isNaN(formData.validationOptions.folds) || formData.validationOptions.folds < 0) {
                problem.isValid = false;
                problem.message += "Cross-validation folds must be a non-negative integer\n";
            }
        } else if (formData.validationType === "Holdout Validation") {
            if (isNaN(formData.validationOptions.holdoutPercent) ||formData.validationOptions.holdoutPercent < 0 || formData.validationOptions.holdoutPercent > 100) {
                problem.isValid = false;
                problem.message += "Percent held out must be between 0 and 100\n";
            }
        }
        
        return problem;
    },
    
    _startAnalysis: function() {
        var formData = this._getFormData();
        var problem = this._checkValidForm(formData);
        if (!problem.isValid) {
            Object.assign(formData, problem);
        }
        this.element.trigger("startanalysis", formData);
    },
    
    _buildValidationOptions: function() {
        var validOptions = $(this.element).find(".validation-options");
        validOptions.empty();
        
        var validationType = this.element.find(".validation-type").val();
        if (validationType === "Cross-validation") {
            var crossValBlock = $(
                "<div class='cart-label'>Cross-validation folds</div>" +
                "<input class='cross-val-folds' />"
            ).appendTo(validOptions);
            validOptions.find('.cross-val-folds')
                .val(this.options.defaultValues.crossValFolds);
        } else if (validationType === "Holdout Validation") {
            $(
                "<div class='cart-label'>Percent held out</div>" +
                "<input class='holdout-percent' />"
            ).appendTo(validOptions);
        }
    },
    
    _create: function() {
        this.element.addClass("analysis-submit");
        
        this._divContainer = $("<div />").appendTo(this.element);
        
        $(  
            "<div class='cart-block' style='font-weight: bold;margin:5px;'>Analysis</div>" +
            "<div class='cart-block'>" +
            "  <div class='cart-label'>Analysis Type</div>" +
            "  <select class='analysis-type'></select>" +
            "</div>" +
            "<div class='cart-block'>" +
            "  <div class='cart-label'>Features</div>" +
            "</div>" +
            "<div class='selected-features'>" +
            "</div>" +
            "<br />" +
            "<div class='cart-block'>" +
            "  <div class='cart-label'>Response Variable</div>" +
            "  <select class='response-var'></select>" +
            "</div>" +
            "<div class='cart-block'>" +
            "  <div class='cart-label'>Drop Threshold</div>" +
            "  <input class='drop-threshold' />" +
            "</div>" +
            "<div class='cart-block'>" +
            "  <div class='cart-label'>Validation Type</div>" +
            "  <select class='validation-type'>" +
            "    <option value='Cross-validation'>Cross-validation</option>" +
            "  </select>" +
            "</div>" +
            "<div class='validation-options'>" +
            "</div>" +
            "<input type='button' value='Start Analysis' class='start-analysis' />"
        ).appendTo(this._divContainer);
        
        this._on(this.element.find(".validation-type"), {
            change: this._buildValidationOptions
        });
        
        this._on(this.element.find(".start-analysis"), {
            click: this._startAnalysis
        });

        this._on({
            "click.add-button": this._addFeature,
            "click.remove-button": function(event) {
                if ($(event.currentTarget).parent().parent().children().length > 1) {
                    $(event.currentTarget).parent().remove();
                }
            },
            "change.feature-name": function(event) {
                var featureName = $(event.currentTarget).val();
                var featureType = this.options.defaultFeatureTypeMap[featureName];
                if (featureType !== undefined) {
                    $(event.currentTarget).parent().find(".feature-type").val(featureType);
                }
            }
        });
        
    },
    
    _init: function() {
        this._addFeature();
        this._buildValidationOptions();
        
        this._divContainer.find(".drop-threshold")
            .val(this.options.defaultValues.dropThreshold);
        
        var analysisSelect = this.element.find(".analysis-type");
        $.each(this.options.analysisTypes, function(index, value) {
            $("<option/>").val(value).text(value).appendTo(analysisSelect);
        });
        
        var featureSelect = $(this.element).find(".feature");
        $.each(this.options.features, function(index, value) {
            $("<option/>").val(value).text(value).appendTo(featureSelect);
        });
        
        var responseVarSelect = $(this.element).find(".response-var");
        $.each(this.options.responseVars, function(index, value) {
            $("<option/>").val(value).text(value).appendTo(responseVarSelect);
        });
    },
    
    _destroy: function() {
        this.element.removeClass("analysis-submit");
        this._divContainer.remove();
    }
    
});

$.widget("cart.confusionMatrix", {
    
    options: {
        truePositives: 0,
        falsePositives: 0,
        falseNegatives: 0,
        trueNegatives: 0,
        width: 500,
        height: 500,
        trueColor: "#8DDBB1",
        falseColor: "#FFC2C2",
        positiveName: "Yes",
        negativeName: "No",
        percentMode: false
    },
    
    _create: function() {
        this.element.addClass("confusion-matrix");
        
        this._divContainer = $("<div />").appendTo(this.element);
        $(
            "<table style='width: 100%; height: 100%;'>" +
            "  <tr style='height: 10%;'>" +
            "    <th></th>" +
            "    <th></th>" +
            "    <th colspan='2'>True Class</th>" +
            "  </tr>" +
            "  <tr style='height: 10%;'>" +
            "    <th></th>" +
            "    <th></th>" +
            "    <th class='positive-header'></th>" +
            "    <th class='negative-header'></th>" +
            "  </tr>" +
            "  <tr style='height: 40%;'>" +
            "    <th rowspan='2'>Predicted Class</th>" +
            "    <th class='positive-header' style='text-align: center;'></th>" +
            "    <td class='true-positives'></td>" +
            "    <td class='false-positives'></td>" +
            "  </tr>" +
            "  <tr style='height: 40%;'>" +
            "    <th class='negative-header' style='text-align: center;'></th>" +
            "    <td class='false-negatives'></td>" +
            "    <td class='true-negatives'></td>" +
            "  </tr>" +
            "</table>"
        ).appendTo(this._divContainer);
        
        this.element.find("th").css({
            "text-align": "center"
        });
        this.element.find("td").css({
            "text-align": "center",
            "border": "1px solid black",
            "width": "40%"
        });
    },
    
    _init: function() {
        this._divContainer.css({
            width: this.options.width,
            height: this.options.height
        });
        
        var total = this.options.truePositives + this.options.falsePositive
            + this.options.falseNegatives + this.options.trueNegatives;
        
        var tp = this.options.truePositives;
        var fp = this.options.falsePositives;
        var fn = this.options.falseNegatives;
        var tn = this.options.trueNegatives;
        
        if (this.options.percentMode) {
            var total = tp + fp + fn + tn;
            
            tp = (tp * 100.0 / total).toFixed(1) + "%";
            fp = (fp * 100.0 / total).toFixed(1) + "%";
            fn = (fn * 100.0 / total).toFixed(1) + "%";
            tn = (tn * 100.0 /total).toFixed(1) + "%";
        }
        
        this.element.find(".true-positives").text(tp)
            .css("background-color", this.options.trueColor);
        this.element.find(".false-positives").text(fp)
            .css("background-color", this.options.falseColor);
        this.element.find(".false-negatives").text(fn)
            .css("background-color", this.options.falseColor);
        this.element.find(".true-negatives").text(tn)
            .css("background-color", this.options.trueColor);
        
        this.element.find(".positive-header").text(this.options.positiveName);
        this.element.find(".negative-header").text(this.options.negativeName);
    },
    
    _destroy: function() {
        this.element.removeClass("confusion-matrix");
        this._divContainer.remove();
    }
    
});

$.widget("cart.modelScatter", {
    
    options: {
        xFeatureName: "",
        xFeature: [],
        yFeatureName: "",
        yFeature: [],
        predicitons: [],
        result: [],
        mode: "data",
        colors: ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd'],
        
        allFeatures: {},
        xFeatureN: "",
        yFeatureN: ""
    },
    
    switchMode: function(mode) {
        this.options.mode = mode;
        this._drawGraph();
    },
    
    setFeatures: function(xFeatureN, yFeatureN) {
        this.options.xFeatureN = xFeatureN;
        this.options.yFeatureN = yFeatureN;
        this._drawGraph();
    },
    
    _partitionFeaturesByClass: function() {
        var classes = {};
        
        var classCol;
        if (this.options.mode === "data") {
            classCol = this.options.result;
        } else if (this.options.mode === "predictions") {
            classCol = this.options.predictions;
        }
        
        for (var i = 0; i < classCol.length; ++i) {
            var className = classCol[i];
            if (classes[className] === undefined) {
                classes[className] = {
                    xFeature: [],
                    yFeature: [],
                    predictions: [],
                    result: []
                };
            }
            classes[className].xFeature.push(this.options.allFeatures[this.options.xFeatureN][i]);
            classes[className].yFeature.push(this.options.allFeatures[this.options.yFeatureN][i]);
            classes[className].predictions.push(this.options.predictions[i]);
            classes[className].result.push(this.options.result[i]);
        }
        
        return classes;
    },
    
    _partitionByCorrectitude: function(classes) {
        var newClasses = {};
        
        for (className in classes) {
            newClasses[className] = {
                correct: {
                    xFeature: [],
                    yFeature: [],
                    predictions: [],
                    result: []
                },
                incorrect: {
                    xFeature: [],
                    yFeature: [],
                    predictions: [],
                    result: []
                }
            };
            
            for (var i = 0; i < classes[className].xFeature.length; ++i) {
                if (classes[className].predictions[i] === classes[className].result[i]) {
                    newClasses[className].correct.xFeature.push(classes[className].xFeature[i]);
                    newClasses[className].correct.yFeature.push(classes[className].yFeature[i]);
                    newClasses[className].correct.predictions.push(classes[className].predictions[i]);
                    newClasses[className].correct.result.push(classes[className].result[i]);
                } else {
                    newClasses[className].incorrect.xFeature.push(classes[className].xFeature[i]);
                    newClasses[className].incorrect.yFeature.push(classes[className].yFeature[i]);
                    newClasses[className].incorrect.predictions.push(classes[className].predictions[i]);
                    newClasses[className].incorrect.result.push(classes[className].result[i]);
                }
            }
        }
        
        return newClasses;
    },
    
    _drawGraph: function() {
        var classes = this._partitionFeaturesByClass();
        
        if (this.options.mode === "predictions") {
            classes = this._partitionByCorrectitude(classes);
        }
        
        var plotlyData = [];
        
        var sortedClasses = Object.keys(classes).sort();
        for (var i = 0; i < sortedClasses.length; ++i) {
            var className = sortedClasses[i];
            
            if (this.options.mode === "data") {
                plotlyData.push({
                    name: className,
                    x: classes[className].xFeature,
                    y: classes[className].yFeature,
                    mode: "markers",
                    type: "scatter",
                    marker: {
                        size: 10,
                        symbol: "circle",
                        color: this.options.colors[i % this.options.colors.length]
                    }
                });
            } else if (this.options.mode === "predictions") {
                plotlyData.push({
                    name: className,
                    x: classes[className].correct.xFeature,
                    y: classes[className].correct.yFeature,
                    mode: "markers",
                    type: "scatter",
                    marker: {
                        size: 10,
                        symbol: "circle",
                        color: this.options.colors[i % this.options.colors.length]
                    }
                });
                plotlyData.push({
                    name: className,
                    x: classes[className].incorrect.xFeature,
                    y: classes[className].incorrect.yFeature,
                    mode: "markers",
                    type: "scatter",
                    marker: {
                        size: 10,
                        symbol: "x",
                        color: this.options.colors[i % this.options.colors.length]
                    }
                });
            }
            
        }
        
        var plotlyLayout = {
            xaxis: {
                title: this.options.xFeatureN,
                zeroline: false,
                showline: true
            },
            yaxis: {
                title: this.options.yFeatureN,
                zeroline: false,
                showline: true
            }
        };
        
        if (this.options.mode === "data") {
            plotlyLayout.title = "Data";
        } else if (this.options.mode === "predictions") {
            plotlyLayout.title = "Model Predictions";
        }
        
        Plotly.react(this._divContainer.get(0), plotlyData, plotlyLayout);
    },
    
    _create: function() {
        this.element.addClass("model-scatter");
        this._divContainer = $("<div />").appendTo(this.element);
    },
    
    _init: function() {
        this._drawGraph();
    },
    
    _destroy: function() {
        this.element.removeClass("model-scatter");
        this._divContainer.remove();
    }
    
});

$.widget("cart.rocGraph", {
    
    options: {
        truePositiveRate: [],
        falsePositiveRate: [],
        currentClassifierTPR: 0,
        currentClassifierFPR: 0,
        auc: 0
    },
    
    _create: function() {
        this.element.addClass("roc-graph");
        this._divContainer = $("<div />").appendTo(this.element);
    },
    
    _init: function() {
        var plotlyData = [{
            name: "ROC curve",
            x: this.options.falsePositiveRate,
            y: this.options.truePositiveRate,
            mode: "lines",
            type: "scatter",
            fill: 'tozeroy'
        },
        {
            name: "Current classifier",
            x: [this.options.currentClassifierFPR],
            y: [this.options.currentClassifierTPR],
            text: ["(" + this.options.currentClassifierFPR.toFixed(2) + ", " +
                         this.options.currentClassifierTPR.toFixed(2) + ")"],
            textposition: "bottom",
            mode: "markers+text",
            type: "scatter"
        },
        {
            name: "AUC",
            x: [0.8],
            y: [0.2],
            mode: "text",
            text: ["AUC = " + this.options.auc.toFixed(2)],
            textposition: "bottom",
            showlegend: false
        }];
        
        var plotlyLayout = {
            xaxis: {
                title: "False positive rate",
                range: [-0.2, 1.2],
                tickvals: [0, 0.2, 0.4, 0.6, 0.8, 1],
                zeroline: false,
                showline: true
            },
            yaxis: {
                title: "True positive rate",
                range: [-0.2, 1.2],
                tickvals: [0, 0.2, 0.4, 0.6, 0.8, 1],
                zeroline: false,
                showline: true
            }    
        };
        
        Plotly.react(this._divContainer.get(0), plotlyData, plotlyLayout);
    },
    
    _destroy: function() {
        this.element.removeClass("roc-graph");
        this._divContainer.remove();
    }
    
});