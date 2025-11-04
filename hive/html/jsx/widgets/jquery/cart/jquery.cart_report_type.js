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

    $.widget("cart.cart_report_type",
        {
            _drawFilterSubmission: function (cartReportId) {
                var oThis = this;

                var prodClass =
                {
                    "ALL": ["CCTL019B2202", "CCTL019B2205J", "CCTL019B2101J", "CCTL019C2201", "KTE-C19-101", "KTE-C19-102", "KTE-C19-103", "KTE-C19-104", "KITE-585-501", "015001", "017001"],
                    "CD19_CART": ["CCTL019B2202", "CCTL019B2205J", "CCTL019B2101J", "CCTL019C2201", "KTE-C19-101", "KTE-C19-102", "KTE-C19-103", "KTE-C19-104", "015001", "017001"],
                    "MM_CART": ["KITE-585-501"]
                };
                var reportType =
                {
                    "--Select Report--": [],
                    "Patient Report": [],
                    "Domain Report": []
                }

                 $("#cartReportSel").append("<div class='cart-block' style='font-weight: bold;margin:5px;'>Report Selection</div>")
                    .appendReportTypeSelect(reportType);
                    
                $(".cart-reportTypeSel").change(function () {
                    var report_selection = $("#cartReportSel").append("<div id='cart-reportType-block' style='font-weight: bold;margin:5px;'</div>");
                    if (this.value == "Patient Report") {
                        $(".cart-reportType-block").empty();
                        report_selection.cart_report();
                    } else if (this.value == "Domain Report") {
                        $("#cartReportSel").append("<div class='cart-reportType-block' style='font-weight: bold;margin:5px;'</div>")
                        .cart_domain_report();
                }
                    else{
                        $(".cart-reportType-block").empty();
                    }
                })
                                
            },

            _create: function () {

                var oThis = this;
                $(document.createElement('div'))
                    .attr("id", "cartReportSel")
                    .appendTo(this.element);

                this._drawFilterSubmission("cartReportSel");
            },


        });

});