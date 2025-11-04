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
$(function() {
    $.widget("view.cart_graph_control", {
        _create: function () {
            $(this.element).append($("#graph_control"));
            $('#addTimeFieldButton').click(function (e) {
                var fieldType = $("#displayType").val();
                var field     = $("#yvalueField").val();

                $('#timeSeriesFieldCollection').append(
                        $('<div/>')
                                .addClass("timeField")
                                .append("<input type=\"text\" name=\"field\" value=\""+field+"\" />")
                                .append("<input type=\"text\" name=\"fieldType\" value=\""+fieldType+"\" />")
                                .append("<button class='removeParent' style='margin-left: 5px'>-</button>")
                );

            });

            function filterDomains(categorySelector, fieldSelector)
            {
                var domain = $(categorySelector).val();
                var options = "";
                if (domain === "ALL")
                {
                    for (var i = 0; i < tableCols.length; ++i)
                    {
                        var tableCol = tableCols[i];
                        options += '<option value="' + tableCol + '">' + tableCol + '</option>';
                    }
                }
                else if (domain == "CUSTOM_COLUMNS")
                {
                    var colsWithDomain = [];
                    for (domain in domainDict)
                    {
                        colsWithDomain = colsWithDomain.concat(domainDict[domain]);
                    }
                    for (var i = 0; i < tableCols.length; ++i)
                    {
                        var tableCol = tableCols[i];
                        if (!colsWithDomain.includes(tableCol))
                        {
                            options += '<option value="' + tableCol + '">' + tableCol + '</option>';
                        }
                    }
                }
                else
                {
                    var columns = domainDict[domain];
                    for (var i = 0; i < columns.length; ++i)
                    {
                        options += '<option value="' + columns[i] + '">' + columns[i] + '</option>';
                    }
                }

                $(fieldSelector).html(options);
            }
            
            $(document).on('click', '.removeParent', function(){
                $(this).parent().remove();
            }); 
            
            $(document).on('change', '#xvalueCategory', function ()
            {
                filterDomains("#xvalueCategory", "#xvalueField");
            });
            
            $(document).on('change', '#yvalueCategory', function ()
            {
                filterDomains("#yvalueCategory", "#yvalueField");
            });
            
            function advanceField(fieldSelector, graphButtonSelector)
            {
                var menu = document.querySelector(fieldSelector);
                var pos = menu.selectedIndex;
                pos = (pos + 1) % menu.options.length;
                menu.options[pos].selected = true;
                $(graphButtonSelector).click();
            }
            
            function prevField(fieldSelector, graphButtonSelector)
            {
                var menu = document.querySelector(fieldSelector);
                var pos = menu.selectedIndex;
                pos = (pos + menu.options.length - 1) % menu.options.length;
                menu.options[pos].selected = true;
                $(graphButtonSelector).click();
            }
            
            $("#graphNextX").click(function()
            {
                advanceField("#xvalueField", "#graphButton");
            });
            
            $("#graphPrevX").click(function()
            {
                prevField("#xvalueField", "#graphButton");
            });
            
            $("#graphNextY").click(function()
            {
                advanceField("#yvalueField", "#graphButton");
            });
            
            $("#graphPrevY").click(function()
            {
                prevField("#yvalueField", "#graphButton");
            });

        }
    })
});