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

var CartLib =
{
    
    csvToSumDic: function(csv)
    {
        var tableData = CSVToArray(csv);
        
        var summaryDemoDic = {};
        var patientIndex = tableData[0].indexOf("USUBJID");
        for (var i = 1; i < tableData.length; ++i)
        {
            var id = tableData[i][patientIndex];
            if (id === "" || id === null || id === undefined)
            {
                continue;
            }
            summaryDemoDic[id] = {};
            for (j = 0; j < tableData[i].length; ++j)
            {
                var colName = tableData[0][j];
                var cellData = tableData[i][j];
                if (cellData !== undefined && cellData !== null)
                {
                    summaryDemoDic[id][colName] = cellData;
                }
                else
                {
                    summaryDemoDic[id][colName] = "";
                }
            }
        }
        
        return summaryDemoDic;
    },
    
    csvToTSDic: function(csv)
    {
        var tableData = CSVToArray(csv);
        var tsDic = {};
        var patientIndex = tableData[0].indexOf("USUBJID");
        
        for (var c = 0; c < tableData[0].length; ++c)
        {
            var curPat = tableData[0][patientIndex];
            var curColName = tableData[0][c];
            var curPatCol = [];
            var isEmptyCol = true;
            for (var r = 0; r < tableData.length; ++r)
            {
                if (tableData[r][patientIndex] !== curPat)
                {
                    if (!isEmptyCol)
                    {
                        if (tsDic[curPat] === undefined)
                        {
                            tsDic[curPat] = {};
                        }
                        tsDic[curPat][curColName] = curPatCol;
                    }
                    curPat = tableData[r][patientIndex];
                    curPatCol = [];
                    isEmptyCol = true;
                }
                curPatCol.push(tableData[r][c]);
                if (tableData[r][c] !== undefined)
                {
                    isEmptyCol = false;
                }
            }
            if (!isEmptyCol)
            {
                if (tsDic[curPat] === undefined)
                {
                    tsDic[curPat] = {};
                }
                tsDic[curPat][curColName] = curPatCol;
            }
        }
        
        return tsDic;
    }
    
};

$(function()
{

$.fn.appendOptions = function(list)
{
    var that = this;
    $.each(list, function(index, value)
    {
        $("<option/>").val(value).text(value).appendTo(that);
    });
    return this;
};

$.fn.appendStudySelect = function(prodClass, studySelSize, isMultiSelect)
{
    var prodClassList = Object.keys(prodClass);
    var labelText = isMultiSelect ? "Studies" : "Study";
    
    var prodClassBlock = $("<div>")
        .addClass("cart-block")
        .appendTo(this);

    var studySelBlock = $("<div>")
        .addClass("cart-block")
        .appendTo(this);

    $("<div>").addClass("cart-label")
        .text("Product Class")
        .appendTo(prodClassBlock);

    $("<div>").addClass("cart-label")
        .text(labelText)
        .appendTo(studySelBlock);

    var prodClassSel = $("<select>")
        .addClass("cart-prodClassSel")
        .appendOptions(prodClassList)
        .appendTo(prodClassBlock);

    var studySel = $("<select>")
        .addClass("cart-studySel")
          .appendOptions(prodClass[prodClassList[0]])
          .appendTo(studySelBlock);
    
    if(isMultiSelect)
    {
        studySel.prop("multiple", "multiple")
            .prop("size", studySelSize);
    }
    
    prodClassSel.change(function()
    {
        var curStudies = prodClass[prodClassSel.val()];
        studySel.empty().appendOptions(curStudies);
    });
    
    return this;
};
$.fn.appendReportTypeSelect = function(reportType)
{
    var reportTypeList = Object.keys(reportType);
    var reportTypeBlock = $("<div>")
        .addClass("cart-block")
        .appendTo(this);

    $("<div>").addClass("cart-label")
        .text("Report Type")
        .appendTo(reportTypeBlock);

    var reportTypeSel = $("<select>")
        .addClass("cart-reportTypeSel")
        .appendOptions(reportTypeList)
        .appendTo(reportTypeBlock);

    reportTypeSel.change(function()
    {
        var curReportType = reportType[reportTypeSel.val()];
        
    });
    return this;
};

$.fn.appendSelectType = function(selectVal, label)
{
    var selectTypeList = Object.keys(selectVal);
    var selectClass = (label.replace(/ .*/,'')).concat("class");
    var selectTypeBlock = $("<div>")
        .addClass("cart-block")
        .appendTo(this);

    $("<div>").addClass("cart-label")
        .text(label)
        .appendTo(selectTypeBlock);

    var typeSel = $("<select> ")
        .addClass(selectClass)
        .appendOptions(selectTypeList)
        .appendTo(selectTypeBlock);

    typeSel.change(function()
    {
        var curSelectType = selectVal[typeSel.val()];
        
    });
    
    return this;
};


$.fn.appendBiVarSelectGrp = function(selectVal, label)
{
    
    var selectTypeList = Object.keys(selectVal.bivariate_json);
        
    var selectClass = (label.replace(/ .*/,'')).concat("class");
    var selectTypeBlock = $("<div>")
        .addClass("cart-block")
        .text("")
        .appendTo(this);


    var typeSel = $("<select>")
        .addClass(selectClass)
        .appendOptions(selectTypeList)
        .appendTo(selectTypeBlock);

    selectValList= ["raw_pvals", "adjusted_pvals"];
    selectClass ="pvalClass"

    var valSel = $("<select>")
        .addClass(selectClass)
        .appendOptions(selectValList)
        .appendTo(selectTypeBlock);

    var remove_btn = $("<button type='button' class='removeButton'>-</button>" +
                        "<button type='button' class='addButton'>+</button>" )
    .appendTo(selectTypeBlock);                    
    return this;
};



$.fn.appendTimeSelect = function(defaultStart, defaultEnd)
{
    var startBlock = $("<div>")
        .addClass("cart-block")
        .appendTo(this);

    var endBlock = $("<div>")
        .addClass("cart-block")
        .appendTo(this);
    
    $("<div>").addClass("cart-label")
        .text("Time Start")
        .appendTo(startBlock);

    $("<div>").addClass("cart-label")
        .text("Time End")
        .appendTo(endBlock);
    
    $("<input>").prop("type", "text")
        .prop("size", 3)
        .prop("value", defaultStart)
        .addClass("cart-startSel")
        .appendTo(startBlock);
    
    $("<input>").prop("type", "text")
        .prop("size", 3)
        .prop("value", defaultEnd)
        .addClass("cart-EndSel")
        .appendTo(endBlock);    
    
    return this;
};

$.fn.appendTimeRange = function(defaultStart, defaultEnd, label)
{
    var startBlock = $("<div>")
        .addClass("cart-block")
        .appendTo(this);


    $("<div>").addClass("cart-label")
        .text(label)
        .appendTo(startBlock);

        var start = (label.replace(/ .*/,'')).concat("-startSel");
        var end =  (label.replace(/ .*/,'')).concat("-endSel");

        $("<input>").prop("type", "text")
        .prop("size", 3)
        .prop("value", defaultStart)
        .addClass(start)
        .appendTo(startBlock);
    
    $("<input>").prop("type", "text")
        .css("margin-left", "20px")
        .prop("size", 3)
        .prop("value", defaultEnd)
        .prop("margin", 5 )
        .addClass(end)
        .appendTo(startBlock);    
    
    return this;
};

$.widget("cart.columnSelect",
{
    
    options:
    {
        colList: [],
        selectedOnStart: [],
        funcOnChange: null,
        dropdownWidth: null,
        dropdownLabel: "Field:"
    },
    
    getSelectedColumns: function()
    {
        return this._selectedColumns;
    },
    
    _addColumn: function(column, shouldCallFunc)
    {
        var oThis = this;
        
        this._selectedColumns.push(column);
        
        var container = $("<div>").addClass("cart-removeableGroup")
            .css("display", "inline")
            .css("margin-right", "5px")
            .appendTo(oThis._listBlock);
        $("<div>").text(column)
            .addClass("cart-selectedCol")
            .css("display", "inline")
            .appendTo(container);
        $("<button>").prop("type", "button")
            .text("-")
            .css("display", "inline")
            .click(function()
            {
                var indexToRemove = $(this).parent().index()
                oThis._selectedColumns.splice(indexToRemove, 1);
                oThis.options.funcOnChange();
                $(this).parent().remove();
            })
            .appendTo(container);
        
        if (shouldCallFunc)
        {
            oThis.options.funcOnChange();
        }
    },
    
    _init: function()
    {
        var oThis = this;
        
        this._selectedColumns = [];
        this._addBlock = null;
        this._listBlock = null;
        
        this._addBlock = $("<div>")
            .addClass("cart-addBlock")
            .css("display", "inline")
            .appendTo(this.element);
    
        this._listBlock = $("<div>")
            .addClass("cart-listBlock")
            .css("display", "inline-block")
            .appendTo(this.element);
        
        $("<div>").addClass("cart-label")
            .text(this.options.dropdownLabel)
            .css("display", "inline")
            .appendTo(this._addBlock);
        
        var colSel = $("<select>").addClass("cart-colSel")
            .appendOptions(this.options.colList)
            .css("display", "inline")
            .appendTo(this._addBlock);
        
        if (this.options.dropdownWidth !== null)
        {
            colSel.css("width", this.options.dropdownWidth + "px");
        }
        
        $("<button>").prop("type", "button")
            .text("+")
            .click(function()
            {
                var column = oThis._addBlock.find(".cart-colSel").val();
                oThis._addColumn(column, true);
            })
            .appendTo(this._addBlock);
        
        $("<div class='cart-label'>").appendTo(this._addBlock);
        
        $.each(this.options.selectedOnStart, function(index, value)
        {
            oThis._addColumn(value, false);
        });
    }

});

});