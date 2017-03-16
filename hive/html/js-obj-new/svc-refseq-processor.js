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

if (!javaScriptEngine) var javaScriptEngine = vjJS["undefined"];
javaScriptEngine.include("js/vjHiveseqView.js");

google.load("visualization", "1", {packages:["corechart"]});

var inputVal;

var allSearches = ["dsSearch0"];
var allRetrieves = [];

var viewerContainer = [];

var conversionContainer = [];
var converter = [];

var finishedMultiCounter = 0;
var finishedMultiGCCounter = 0;
var finishedGCCounter = 0;
var finishedCodonCounter = 0;
var finishedTableCounter = 0;
var finishedConversions = 0;
var finishedTaxCounter = 0;
var cleanupCounter = 0;
var submitCounter = 1;  // gets incremented during cleanup(), not when submit is pressed, so that it's consistent all the way through a search
var assemblyPattern = /GCF_\d{9}\.{1}\d+/i;
var typeLetters = {"genomic": "G", "mitochondrion": "M", "chloroplast": "C", "plastid": "P", "leucoplast": "L", "chromoplast": "O"}
var oldTabsToClose = [];
var curTabs = [];

var gClrTable2 = new Array ("#74c493","#51574a","#447c69","#8e8c6d","#e4bf80","#e9d78e","#e16552","#c94a53","#e2975d","#be5168","#a34974","#f19670","#993767","#65387d","#4e2472","#9163b6","#e279a3","#e0598b","#7c9fb0","#5698c4","#9abf88");
var forceCleanup = false;


var colorArray = new Array ("#1E90FF", "#228B22", "#4682B4", "#9ACD32", "#9370DB", "#D2B48C", "#FA8072", "#DAA520", "#4B0082", "#708090")


vjHO.register('svc-refseq-processor').Constructor=function ()
{
    var id;
    vjDS.add("", "dsSearch0", "static://");
    
    /*var collapser = $("[tab-id=searchTab" + submitCounter +"]");
    collapser.on("click",
            function(event) {
                console.log("hello");
                
        }); */
    /*function getBrowser() {
          if( navigator.userAgent.indexOf("Chrome") != -1 ) {
            return "Chrome";
          } else if( navigator.userAgent.indexOf("Opera") != -1 ) {
            return "Opera";
          } else if( navigator.userAgent.indexOf("MSIE") != -1 ) {
            return "IE";
          } else if( navigator.userAgent.indexOf("Firefox") != -1 ) {
            return "Firefox";
          } else {
            return "unknown";
          }
        }

    browser = getBrowser();
    if (browser == "IE"){
        alert("This webpage is not compatible with Internet Explorer. Please use Chrome or Firefox for best results.");
    } */


    var taxIDPanel = new vjPanelView({
        data:["dsSearch0"],
        rows: [
                {name: 'changeFile', type:'select', options: [[0,"RefSeq"], [1,"GenBank"]], value:0, title: " ", align:'left', showTitle: false, readonly:false, order:1, },
                {name: 'dnaType', type:'select', options: [["genomic", "genomic"], ["mitochondrion", "mitochondrion"], ["chloroplast", "chloroplast"], ["plastid", "plastid"],["leucoplast", "leucoplast"],["chromoplast", "chromoplast"]], value: "genomic", title: " ", align:'left', showTitle: false, readonly:false, order:2},
                {name: 'valueForFilter', type:"text", title: "Query for search", showTitle: true, align: 'left' , order : 3, path: "/valueForFilter"},
                {name: 'depthFilter', type:"checkbox", title: "Deep Search", showTitle: true, align: 'left' , value: true, order : 4},
                {name: 'newSearchBox', order:99, title:"Add Additional Search", icon:'Add_query', isSubmitable: true, url: onAddSearchBox},
                {name:'apply', order:100, title: 'Submit All' , icon:'submit' , isSubmitable: true, url: onUpdate }
                // This is where the dropdowns, checkboxes, etc are defined
                ],
        formObject: document.forms[formName],
        onKeyPressCallback: smartSearchFunc,
        iconSize:20,
        isok: true
    });
    
    viewerContainer.push(taxIDPanel);

   
    var gencodes = new Object();
    gencodes[0] = ["TTT", "TTC", "TTA", "TTG", "CTT", "CTC", "CTA", "CTG", "ATT", "ATC", "ATA", "ATG", "GTT", "GTC", "GTA", "GTG", "TAT", "TAC", "TAA", "TAG", "CAT", "CAC", "CAA", "CAG", "AAT", "AAC", "AAA", "AAG", "GAT", "GAC", "GAA", "GAG", "TCT", "TCC", "TCA", "TCG", "CCT", "CCC", "CCA", "CCG", "ACT", "ACC", "ACA", "ACG", "GCT", "GCC", "GCA", "GCG", "TGT", "TGC", "TGA", "TGG", "CGT", "CGC", "CGA", "CGG", "AGT", "AGC", "AGA", "AGG", "GGT", "GGC", "GGA", "GGG"];
    gencodes[1] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TAG', 'TGA'];
    gencodes[2] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATA', 'ATG', 'TGA', 'TGG', 'AGA', 'AGG', 'TAA', 'TAG'];
    gencodes[3] = ['TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'CTA', 'CTC', 'CTG', 'CTT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATA', 'ATG', 'TGA', 'TGG', 'TAA', 'TAG'];
    gencodes[4] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGA', 'TGG', 'TAA', 'TAG'];
    gencodes[5] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGA', 'AGC', 'AGG', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATA', 'ATG', 'TGA', 'TGG', 'TAA', 'TAG'];
    gencodes[6] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAA', 'TAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TGA'];
    gencodes[9] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGA', 'AGC', 'AGG', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAG', 'AAA', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGA', 'TGG', 'TAA', 'TAG'];
    gencodes[10] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGA', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TAG'];
    gencodes[11] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TAG', 'TGA'];
    gencodes[12] = ['CTA', 'CTC', 'CTT', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'CTG', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TAG', 'TGA'];
    gencodes[13] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'AGA', 'AGG', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATA', 'ATG', 'TGA', 'TGG', 'TAA', 'TAG'];
    gencodes[14] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGA', 'AGC', 'AGG', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAG', 'AAA', 'AAC', 'AAT', 'CAA', 'CAG', 'TAA', 'TAC', 'TAT', 'ATG', 'TGA', 'TGG', 'TAG'];
    gencodes[16] = ['CTA', 'CTC', 'CTG', 'CTT', 'TAG', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TGA'];
    gencodes[21] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGA', 'AGC', 'AGG', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAG', 'AAA', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATA', 'ATG', 'TGA', 'TGG', 'TAA', 'TAG'];
    gencodes[22] = ['CTA', 'CTC', 'CTG', 'CTT', 'TAG', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TCA', 'TGA'];
    gencodes[23] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TAG', 'TGA', 'TTA'];
    gencodes[24] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGA', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AGG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGA', 'TGG', 'TAA', 'TAG'];
    gencodes[25] = ['CTA', 'CTC', 'CTG', 'CTT', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'TGA', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TAG'];
    gencodes[26] = ['CTA', 'CTC', 'CTT', 'TTA', 'TTG', 'AGA', 'AGG', 'CGA', 'CGC', 'CGG', 'CGT', 'AGC', 'AGT', 'TCA', 'TCC', 'TCG', 'TCT', 'CTG', 'GCA', 'GCC', 'GCG', 'GCT', 'GGA', 'GGC', 'GGG', 'GGT', 'CCA', 'CCC', 'CCG', 'CCT', 'ACA', 'ACC', 'ACG', 'ACT', 'GTA', 'GTC', 'GTG', 'GTT', 'ATA', 'ATC', 'ATT', 'TGC', 'TGT', 'GAC', 'GAT', 'GAA', 'GAG', 'TTC', 'TTT', 'CAC', 'CAT', 'AAA', 'AAG', 'AAC', 'AAT', 'CAA', 'CAG', 'TAC', 'TAT', 'ATG', 'TGG', 'TAA', 'TAG', 'TGA'];


    //for example here, we will get an empty results sub object
    this.fullview=function(node, whereToAdd)
    {
        id = docLocValue("id");
        
        vjDS.add("", "dsSearch", "static://");
        vjDS["dsSearch"].register_callback(onSearchResults);

        var filesStructureToAdd = [{
            tabId: 'searchTab' + submitCounter,
            tabName: "Search",
            multiView: true,
            callback: function (){
                closeOldTabs();
                $("[data-id='right']").children("[title='Open']").click();
                },
            position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'75%'},
            viewerConstructor: {
                views: [{instance:taxIDPanel, size: '45'}],
                layoutType: "stack",
                orientation: "vertical"
            },
            autoOpen: ["computed"]
        }];
        

        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.moveTab("resultsTable", {top:"0%", left: "20%", right: "75%", bottom: "100%"}, 0);
        
        $("#parameters").closest("li").hide();
        //$("#progress").closest("li").hide();
        $("#whatNext").closest("li").hide();
    };

   function removeSearchBox(viewer) {
       var indexToRemove = -1;
       
       for (var i=1; i < viewerContainer.length; i++) {
           var curV = viewerContainer[i];
           if (curV.divIdToCheck == viewer.id) {
               indexToRemove = i;
               break;
           }
       }
       if (indexToRemove != -1) {
           viewerContainer.splice(indexToRemove,1);
       }       
   };
    
    function onAddSearchBox(viewer, node){
        var instance = $("#searchTab" + submitCounter + "-tab").dataviews('instance');
        
        var newdsname = "dsSearch" + submitCounter + allSearches.length.toString();
        
        vjDS.add("", newdsname, "static://");
        allSearches.push(newdsname);
        
        var shorttaxIdPanel = new vjPanelView({
            data: newdsname,
            rows: [
                   {name: 'changeFile', type:'select', options: [[0,"RefSeq"], [1,"GenBank"]], value:0, title: " ", align:'left', showTitle: false, readonly:false, order:1},
                   {name: 'dnaType', type:'select', options: [["genomic", "genomic"], ["mitochondrion", "mitochondrion"], ["chloroplast", "chloroplast"], ["plastid", "plastid"],["leucoplast", "leucoplast"],["chromoplast", "chromoplast"]], value: "genomic", title: " ", align:'left', showTitle: false, readonly:false, order:2},
                   {name: 'valueForFilter', type:"text", title: "Query for search", showTitle: true, align: 'left' , order : 3, path: "/valueForFilter"},
                   {name: 'depthFilter', type:"checkbox", title: "Deep Search", showTitle: true, align: 'left' , value: true, order : 4}
                   // This is where the dropdowns, checkboxes, etc are defined
                   ],
           formObject: document.forms[formName],
           onKeyPressCallback: smartSearchFunc,
              iconSize:20,
              divIdToCheck: newdsname,
           isok: true
       });
        
        instance.add({
            instance: shorttaxIdPanel, 
            overflowX: 'auto',
            overflowY: '20',
            overflow: "auto",
            size: '45',
            allowClose: true,
            onClose: removeSearchBox,
        });
        
        viewerContainer.push(shorttaxIdPanel);
    };
    
    function clearEmptySearches(){
        var inputVal;
        var emptyIndices = [];
        
        for (var index = 0; index<viewerContainer.length;index++){
            inputVal = viewerContainer[index].tree.root.children[2].value;
            if (inputVal == null){
                emptyIndices.push(index);
            }
        }
        
        for (var i = 0; i<emptyIndices.length; i++){
            indexToRemove = emptyIndices[i];
            viewerContainer.splice(indexToRemove,1);
        }
    };
    
    function closeOldTabs(){
        for (var index = 0; index<oldTabsToClose.length; index++){
            algoWidgetObj.closeTab(oldTabsToClose[index]);
        }
    };
    
    function onUpdate(viewer, node, irow){
        $("[data-id='right']").children("[title='Close']").click();
        var sourceVal;
        var inputVal;
        var dnaTypeVal;
        var searchTypeVal;
        var convertdsName;
        var converturl;
        var id = docLocValue("id");
        
        clearEmptySearches();
        //closeOldTabs();
        if (submitCounter > 1){
            algoWidgetObj.removeTabs("taxonomyTab" + (submitCounter-1), "results");
        }

        for (var index = 0; index<viewerContainer.length; index++){
            
            sourceVal = viewerContainer[index].tree.root.children[0].value ? "genbank" : "refseq";
            dnaTypeVal = viewerContainer[index].tree.root.children[1].value;
            inputVal = viewerContainer[index].tree.root.children[2].value;
        
            var type = "t";
            if (sourceVal == "genbank"){
                type += "G";
                //converturl = "http://?cmd=ionCodonDB&ionCodon=ionCodon/ionCodon.ion&id=" + id + "&genbank=genbank";
            } else { // sourceVal == "refseq"
                type += "R";
                //converturl = "http://?cmd=ionCodonDB&ionCodon=ionCodon/ionCodon.ion&id=" + id + "&genbank=refseq";
            }
            
            type += typeLetters[dnaTypeVal]
            converturl = "http://?cmd=ionCodonDB&ionCodon=ionCodon/ionCodon.ion&id=" + id + "&type=" + type;
            
            if (!isNaN(inputVal)){
                converturl = converturl + "&taxid=" + vjDS.escapeQueryLanguage(inputVal);
            }
            else if (assemblyPattern.test(inputVal)){
                converturl = converturl + "&assembly=" + vjDS.escapeQueryLanguage(inputVal);
            }
            else {
                converturl = converturl + "&name=" + vjDS.escapeQueryLanguage(inputVal);
            }
                    
            convertdsName = "dsConvert" + submitCounter + index.toString();
            converturl = converturl + "&raw=1" + "&regex=0";  // when submitting the query for real, don't use regex
            vjDS.add("", convertdsName, converturl);
            vjDS[convertdsName].reload(converturl, true);
            conversionContainer.push(convertdsName);
            vjDS[convertdsName].register_callback(buildConversion);
            
        }
    };
    
    function onUpdate2(viewer,node,irow) {
        // viewer.rebuildTree();
        //viewerContainer.unshift(viewer);


        //var fileVal = viewer.tree.root.children[0].value ? "all_CDS.tab" : "all_species.tab";
        var sourceVal;
        var searchTypeVal;
        var deepSearch;
        var organellar;
        var url;
        var filesStructureToAdd = [];
        var convertdsName;
        var converturl;
        var id = docLocValue("id");
    

        for(var index = 0; index<viewerContainer.length; index++){
            sourceVal = viewerContainer[index].tree.root.children[0].value ? "genbank" : "refseq";
            dnaTypeVal = viewerContainer[index].tree.root.children[1].value;
            inputVal = viewerContainer[index].tree.root.children[2].value;
            deepSearch = viewerContainer[index].tree.root.children[3].value;
            
            
            if (sourceVal == "genbank"){
                url = "http://?cmd=ionTaxidCollapse&svcType=svc-refseq-processor&objId=" + id + "&fileSource=genbank_species.tsv";
            } else { // sourceVal == "refseq"
                url = "http://?cmd=ionTaxidCollapse&svcType=svc-refseq-processor&objId=" + id + "&fileSource=refseq_species.tsv";
            }
                  
            
            url = url + "&taxid=" + vjDS.escapeQueryLanguage(converter[index].taxid);
            
            /*if (searchTypeVal == 1){
                url = url + "&filterInColName=Assembly&filterIn=" + inputVal;
            } */
            var filter_col_names = ["Organellar"];
            var filter_col_values = [dnaTypeVal];
            
            if (assemblyPattern.test(inputVal)){
                filter_col_names.push("Assembly");
                filter_col_values.push(inputVal);
            }
            url += "&filterInColName=" + vjDS.escapeQueryLanguage(JSON.stringify(filter_col_names)) + "&filterIn=" + vjDS.escapeQueryLanguage(JSON.stringify(filter_col_values));
            
            url = url + "&searchDeep=" + (deepSearch ? "true" : "false");
            
            url = url + "&raw=1";
            
            
            var dsName = "dsRetrieve" + submitCounter + index.toString();
            
            vjDS.add("Retrieving information for search " + inputVal, dsName, url)
            allRetrieves.push(dsName);
            vjDS[dsName].register_callback(putTaxonomyTree);
            vjDS[dsName].register_callback(reformatText);
            vjDS[dsName].register_callback(graphGC);
            vjDS[dsName].register_callback(graphMultipleGC);
            vjDS[dsName].register_callback(graphReorderedCodons);
            vjDS[dsName].register_callback(graphMultipleCodons);
            vjDS[dsName].reload(url, true);
            
        
        }  
        
        //algoWidgetObj.addTabs(filesStructureToAdd, "results");
        //algoWidgetObj.moveTab("resultsTable", {top:"0%", left: "20%", right: "75%", bottom: "100%"}, 0);
        algoWidgetObj.closeTab("searchTab" + submitCounter);
    };

    
    
    function buildConversion (viewer, content, requestInfo){
        finishedConversions += 1;
        if (finishedConversions < conversionContainer.length){
            return;
        }
        
        var placeholder = new Object();
        placeholder.taxid = 0;
        placeholder.assembly = "None";
        placeholder.name = "None";
        placeholder.display = "No Data Available";
        for (var index = 0; index < viewerContainer.length; index++){
            converter[index] = placeholder; //initialize the array with something
        }
        
        for (var index = 0; index < viewerContainer.length; index++){
            var inputVal = viewerContainer[index].tree.root.children[2].value;
            var position;

            
            if (!isNaN(inputVal)){  // if it's a number, it's the first spot
                position = 0;
            }
            else if (assemblyPattern.test(inputVal)){  // if it's an assembly, it's the second spot
                position = 1;
            }
            else {  // if it's a string name, it's the last spot
                position = 2;
            }
            

            var splitstring = vjDS[conversionContainer[index]].data.split("\n");
            var n = 0;
            
            for (var i = 0; i < splitstring.length; i++){
                var data = new Object();
                s = splitstring[i].split(",");
                //console.log(splitstring[i]);
                if (s[position] == inputVal){
                    if (isNaN(inputVal) && !assemblyPattern.test(inputVal)){  //user searches for name (isNaN and not assemb)
                        data.taxid = s[0];
                        data.assembly = s[1];
                        data.name = s[2];
                        if (viewerContainer[index].tree.root.children[1].value != "genomic"){
                            data.display = s[2] + " (" + viewerContainer[index].tree.root.children[1].value + " " + s[0] + ")";
                        }
                        else {
                            data.display = s[2] + " (" + s[0] + ")";
                        }
                        
                        converter[index] = data;
                        //converter.push(data);
                        //console.log(data);
                        break;
                    }
                    else if (assemblyPattern.test(inputVal)){ //user searches for assembly
                        data.taxid = s[0];
                        data.assembly = s[1];
                        data.name = s[2];
                        if (viewerContainer[index].tree.root.children[1].value != "genomic"){
                            data.display = s[2] + " (" + viewerContainer[index].tree.root.children[1].value + " " + s[1] + ")";
                        }
                        else {
                            data.display = s[2] + " (" + s[1] + ")";
                        }
                        converter[index] = data;
                        //converter.push(data);
                        //console.log(data);
                        break;
                    }
                    else {  //user searches for taxID
                        data.taxid = s[0];
                        data.assembly = s[1];
                        data.name = s[2];
                        if (viewerContainer[index].tree.root.children[1].value != "genomic"){
                            data.display = s[0] + " (" + viewerContainer[index].tree.root.children[1].value + " " + s[2] + ")";
                        }
                        else {
                            data.display = s[0] + " (" + s[2] + ")";
                        }
                        converter[index] = data;
                        break;    
                        
                    } // end else (user searched for taxid)
                }  //end if (s[position] == inputVal)
            } //end for i<splitstring.length
            
            
        } //end for index<viewerContainer.length
        
        var displayNames = [];
        for (var index = 0; index<converter.length; index++){
            displayNames.push(converter[index].display);
        }
        
        clearNoResultSearches();
        
        if (forceCleanup == true){
            cleanup();
            return;
        }
        
        var duplicates = hasDuplicates(displayNames);
        if (duplicates == true){
            fixDuplicateDisplayNames(displayNames);
        }
        
        
        
        onUpdate2(null,null,null);
        
    };
    
    function hasDuplicates(arr){
        return (new Set(arr)).size !== arr.length;
    };

    function fixDuplicateDisplayNames(displayNames){
        for (var index = 0; index<converter.length; index++){
            name = displayNames[index];
            namePositions = [];
            for (var i = 0; i<converter.length; i++){
                if (converter[i].display == name){
                    namePositions.push(i)
                }
            }
            
            if (namePositions.length > 1){
                for (var i = 0; i<namePositions.length; i++){
                    /*if (converter[index].display == "No Data Available"){
                        break;
                    } */
                    var pos = namePositions[i];
                    var sourceVal = viewerContainer[pos].tree.root.children[0].value ? "genbank" : "refseq";
                    var deepSearch = viewerContainer[pos].tree.root.children[3].value;
                    var appendage = " (";
                    
                    if (sourceVal == "genbank"){
                        appendage += "GB";
                    } else {
                        appendage += "RS";
                    }
                    
                    if (deepSearch == true){
                        appendage += "-DS)";
                    } else {
                        appendage += ")";
                    }
                    
                    var newname = name.substring(0, name.length-1) + appendage + name.substring(name.length-1, name.length);
                    converter[pos].display = newname;
                } // end for    
            } // end if
        }
    };
    
    function clearNoResultSearches(){
        var emptyIndices = [];
        var emptySearches = [];
        
        for (var index = 0; index<converter.length;index++){
            if (converter[index].display == "No Data Available"){
                emptyIndices.push(index);
            }
        }
        
        for (var i = 0; i<emptyIndices.length; i++){
            var searchTerm = "";
            searchTerm += viewerContainer[emptyIndices[i]].tree.root.children[2].value + " - ";
            var sourceVal = viewerContainer[emptyIndices[i]].tree.root.children[0].value ? "genbank" : "refseq";
            searchTerm += sourceVal + " - ";
            searchTerm += viewerContainer[emptyIndices[i]].tree.root.children[1].value;
            emptySearches.push(searchTerm);            
        }
        
        for (var i = emptyIndices.length -1; i >= 0; i--){ //go backwards to not splice too early
            indexToRemove = emptyIndices[i];
            viewerContainer.splice(indexToRemove,1);
            converter.splice(indexToRemove,1);
        }
        
        var failedString = "The following search(es) failed:\n";
        if (emptyIndices.length > 0){
            //alert("The following searches failed:");
            //console.log("The following searches failed:");
            for (var i = 0; i<emptyIndices.length; i++){
                failedString += "  " + emptySearches[i] + "\n";
                //console.log(emptySearches[i]);
            }
            
            failedString += "This could mean either there was no valid data for that search, or a technical problem occurred.\n";
            alert(failedString);
        }
        
        
        if (viewerContainer.length == 0){
            forceCleanup = true;
        }
        
    };
    
    
    var lastContainer;
    function smartSearchFunc (container, e, nodepath, elname){
        var node = this.tree.findByPath(nodepath);
        var el = this.findElement(node.name, container);
        var id = docLocValue("id");
        for (var index = 0; index < viewerContainer.length; index++){
            for (var i = 0; i < 4; i++){
                viewerContainer[index].rows[i].value = viewerContainer[index].tree.root.children[i].value;
            }
        } // need to do this to protect the values from being reset by rebuildTree()

        var inputVal, sourceVal, dnaTypeVal;
        for (var i = 0; i < viewerContainer.length; i++){
            if (viewerContainer[i].container == this.container){
                inputVal = viewerContainer[i].tree.root.children[2].value;
                sourceVal = viewerContainer[i].tree.root.children[0].value ? "genbank" : "refseq";
                dnaTypeVal = viewerContainer[i].tree.root.children[1].value;
                if (!isNaN(inputVal)){
                    var useDropdown = false;
                    if (i == 0){
                        viewerContainer[i].rows.splice(7);
                    }
                    else {
                        viewerContainer[i].rows.splice(5);
                    }
                    viewerContainer[i].rebuildTree();
                }
                else if (assemblyPattern.test(inputVal)){
                    var useDropdown = false;
                    if (i == 0){
                        viewerContainer[i].rows.splice(7);
                    }
                    else {
                        viewerContainer[i].rows.splice(5);
                    }
                    viewerContainer[i].rebuildTree();
                }
                else {
                    var useDropdown = true;
                }
                break
            }
        }
        
        if (el.value && el.value.length >=3 && useDropdown == true && sourceVal && dnaTypeVal){
            // var nUrl = "http://?cmd=ionTaxidByName&limit=20&name="+el.value + String.fromCharCode(e.which) +"&raw=1";
            // var nUrl = "http://?cmd=ionCodonDB&ionCodon=ionCodon/ionCodon.ion&id=" + id + "&type=tGG" + ... ;
            var type = "t";
            if (sourceVal == "genbank"){
                type += "G";
                //converturl = "http://?cmd=ionCodonDB&ionCodon=ionCodon/ionCodon.ion&id=" + id + "&genbank=genbank";
            } else { // sourceVal == "refseq"
                type += "R";
            }
            
            type += typeLetters[dnaTypeVal]
            var nUrl = "http://?cmd=ionCodonDB&ionCodon=ionCodon/ionCodon.ion&id=" + id + "&type=" + type + "&name=" + vjDS.escapeQueryLanguage(el.value) + "&regex=1";
            searchVal = vjDS.escapeQueryLanguage(el.value) + String.fromCharCode(e.which);
            lastContainer = this.container;
            vjDS["dsSearch"].reload(nUrl, true);
        }
    };
    
    var searchVal;
    function onSearchResults (viewer, content, requestInfo){
        var table = new vjTable (content);
        
        var containerId = lastContainer; //viewer.container;
        var index = null;
        for(var i = 0; i < viewerContainer.length; i++){
            if (viewerContainer[i].container == containerId){
                index = i;
                break;
            }
        }
        
        if (index == 0){
            viewerContainer[index].rows.splice(6);
        } else if (index == null){
            return;
        } else {
            viewerContainer[index].rows.splice(4);
        }
        
        viewerContainer[index].rows[2].value = searchVal;
        var seenNames = {};
        
        for(var i = 1; ((i < table.rows.length) && (i <=50)); i++) {
            var taxID = table.rows[i].cols[0];
            var name = table.rows[i].cols[2];
            if (seenNames[name]) {
                continue; // show only 1 dropdown entry per name - independent of assembly/taxid
            } else {
                viewerContainer[index].rows.push({name: taxID, taxName: name, order:i ,title: name, isSubmitable: true, path: "/valueForFilter/"+taxID, taxID: taxID, url:onUpdateText});
                seenNames[name] = true;
            }
        }
        
        /*for (var i = 0; i < 5; i++){
            viewerContainer[index].rows[i].value = viewerContainer[index].tree.root.children[i].value;
        }*/
        
        viewerContainer[index].rebuildTree();  //uncomment this later, but it's causing the searchTypeVal to get reset
        //viewerContainer[index].refresh(); 
        
        /*taxIDPanel.rows.splice(7);
        taxIDPanel.rows[1].value = searchVal;
        
        for(var i = 1; i < table.rows.length; i++){
            taxIDPanel.rows.push({name:table.rows[i].cols[1], order:i ,title: table.rows[i].cols[1], isSubmitable: true, path: "/valueForFilter/"+table.rows[i].cols[1], taxID: table.rows[i].cols[0], url:onUpdateText});
        }
        
        taxIDPanel.rebuildTree();
        taxIDPanel.refresh(); */
    };
    
    function onUpdateText (viewer, node, irow){
        //viewer.tree.root.children[1].value = node.taxID;
        viewer.tree.root.children[2].value = node.taxName;
        viewer.refresh(); // display value is node.name
        //viewer.tree.root.children[1].value = node.taxID; // actual searched value is taxID
        //onUpdate(viewer, node, irow);  // needs to not start the search immediately, just autofill the values
    };
    
    function putTaxonomyTree (viewer, content, requestInfo){
        finishedTaxCounter += 1;
        if (finishedTaxCounter < viewerContainer.length){
            return;
        }
        
        var taxIdList = [];
        for (index = 0; index<converter.length; index++){
            taxIdList.push(converter[index].taxid);
        }
        
        filesStructureToAdd = [];
        
        var taxonomy = new vjD3TaxonomyControl({formName: "algoForm", taxIds: taxIdList});
        
        curTabs.push("taxonomyTab" + submitCounter);
        
        filesStructureToAdd.push({
           tabId: "taxonomyTab" + submitCounter,
           tabName: "Taxonomy",
           tabOrder: 1,
           position: {posId: 'taxonomy', top:'0%', bottom:'50%', left:'20%', right:'65%'},
           viewerConstructor: {
              instance: taxonomy[0]
          },
          autoOpen: ["closed"]
      });  //adding this tab here breaks when there are multiple searches executed without reloading-- fix w/ Kate 
      
      algoWidgetObj.addTabs(filesStructureToAdd, "results");
      algoWidgetObj.closeTab("searchTab" + submitCounter);
    };
    
    
    
    function reformatText (viewer, content, requestInfo){
        finishedTableCounter += 1;
        if (finishedTableCounter < viewerContainer.length){
            return;
        }
        
        var codons = ["TTT", "TTC", "TTA", "TTG", "CTT", "CTC", "CTA", "CTG", "ATT", "ATC", "ATA", "ATG", "GTT", "GTC", "GTA", "GTG", "TAT", "TAC", "TAA", "TAG", "CAT", "CAC", "CAA", "CAG", "AAT", "AAC", "AAA", "AAG", "GAT", "GAC", "GAA", "GAG", "TCT", "TCC", "TCA", "TCG", "CCT", "CCC", "CCA", "CCG", "ACT", "ACC", "ACA", "ACG", "GCT", "GCC", "GCA", "GCG", "TGT", "TGC", "TGA", "TGG", "CGT", "CGC", "CGA", "CGG", "AGT", "AGC", "AGA", "AGG", "GGT", "GGC", "GGA", "GGG"];
        var filesStructureToAdd = [];
         
        
        for (index = 0; index<viewerContainer.length; index++){
            var data = new Object();
            var formattedString = "";
    
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i < splitstring.length; i++){
                s = splitstring[i].split(",");
                data[s[0]] = s[1];
            }
    
            
            var maxes = new Object();
            maxes[0] = 0, maxes[1] = 0, maxes[2] = 0, maxes[3] = 0;
            var column = 0;
            
            for (var i = 0; i < codons.length; i++){
                column = i%4;
                if (data[codons[i]].toString().length > maxes[column]){
                    maxes[column] = data[codons[i]].toString().length;
                }        
            }
    
            
            for (var j = 0; j < codons.length; j++){
                formattedString += codons[j] + "&nbsp;";
                
                var freq = ((1000*data[codons[j]])/data["\"#codon\""]);
                if (freq < 10){
                    formattedString += "&nbsp;";
                }
                formattedString += freq.toFixed(2) + "&nbsp;";
                
                
                formattedString += "(" + data[codons[j]].toString() + ")";
                column = j%4;
                var diff = maxes[column] - data[codons[j]].toString().length
                
                for (var k = 0; k < diff; k++){
                    formattedString += "&nbsp;";
                }
                
                if ((j+1) < codons.length){  // don't need extra space after the last line
                    if (column == 3){
                        formattedString += "\n";
                    } else { 
                        formattedString += "&nbsp;&nbsp;";
                        //formattedString += "\n";  //only use when you want to get each one row by row, not in usual table format
                    }
                    
                    if ((j+1)%16 == 0){
                        formattedString += "\n"; //comment out if you want there to not be an extra space after every 4 rows
                    }
                }
                
            }
            
            
            var tableDSName = "dsTable" + submitCounter + index.toString();
            
            vjDS.add("", tableDSName, "static://" + formattedString);
            
            curTabs.push(tableDSName + '-cut');
            
            filesStructureToAdd.push({
                  tabId: tableDSName + '-cut',
                  tabName: converter[index].display + " Codon Usage Table",
                 position: {posId: 'CodonUsageTable', top:'0%', bottom:'50%', left:'20%', right:'65%'},
                 viewerConstructor: {
                     dataViewer: "vjTextView",
                     dataViewerOptions:{
                         data: tableDSName,
                        name: 'basicGraph',
                        width: "800px"
                     }
                 },
                 autoOpen: ["computed"]
             });
        }
        
         algoWidgetObj.addTabs(filesStructureToAdd, "results");
         algoWidgetObj.closeTab("searchTab" + submitCounter);
    };
    
    
    
    
    function graphReorderedCodons(viewer, content, requestInfo){
        finishedCodonCounter += 1;
        if (finishedCodonCounter < viewerContainer.length){
            return;
        }
        
        var filesStructureToAdd = [];
        
        for (var index = 0; index<viewerContainer.length; index++){
            var reorderedDataString = "id," + converter[index].display +"\n";
            var translationTable = 0; //0 is the default order, same as the table; can get it as a variable once I have a function for it
            var data = new Object();
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i < splitstring.length; i++){
                s = splitstring[i].split(",");
                data[s[0]] = s[1];
            }
            
    
            
            for (var i = 0; i < gencodes[translationTable].length; i++){
                reorderedDataString += gencodes[translationTable][i] + "," + data[gencodes[translationTable][i]] + "\n";
            }
    
            
            
            freqDSName = "dsFreq" + submitCounter + index.toString();
            vjDS.add("Reordering codons", freqDSName, "static://" + reorderedDataString);
            
            curTabs.push(freqDSName + '-codon');
    
            filesStructureToAdd.push({
               tabId: freqDSName + '-codon',
               tabName: converter[index].display + " Codon Frequencies",
               position: {posId: 'bottomGraph', top:'50%', bottom:'100%', left:'20%', right:'99%'},
               viewerConstructor: {
                   dataViewer: "vjGoogleGraphView",
                   dataViewerOptions:{
                          data: freqDSName,
                       name: 'basicGraph',
                       type: "column",
                       scaleTo: 1000,
                       title: converter[index].display + " Codon Frequencies (per 1000 codons)",
                       options:{
                               title: "graph graph",
                               curveType: 'function',
                               //colors:vjPAGE.colorsACGT,
                               colors: colorArray,
                            chartArea :{height:'75%', width: '75%' },
                            vAxis: { viewWindow: {min: 0}},
                            legend: {position: 'none'}
                       },
                       maxRowPerSeries: 64,
                       series:[ {name : "id", label: true}, {name : converter[index].display, type: "number"}]
                       }
               },
               autoOpen: ["computed"]
           });
            
        }
    
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.closeTab("searchTab" + submitCounter);
        
        cleanup();
    };
    
    function graphGC(viewer, content, requestInfo){
        finishedGCCounter += 1;
        if (finishedGCCounter < viewerContainer.length){
            return;
        }
        
        var filesStructureToAdd = [];
        
        for (var index = 0; index<viewerContainer.length; index++){
            var newString = "id," + converter[index].display + "\n";
            var data = new Object();
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i < splitstring.length; i++){
                s = splitstring[i].split(",");
                data[s[0]] = s[1];
            }
            
            newString += "GC%," + data["\"GC%\""] + "\n";
            newString += "Codon Position 1 GC%," + data["\"GC1%\""] + "\n";
            newString += "Codon Position 2 GC%," + data["\"GC2%\""] + "\n";
            newString += "Codon Position 3 GC%," + data["\"GC3%\""] + "\n";
            
            dsGCName = "dsGC" + submitCounter +  index.toString();
            vjDS.add("Reordering codons", dsGCName, "static://" + newString);
            
            curTabs.push(dsGCName + '-gc');
            
            filesStructureToAdd.push({
                tabId: dsGCName + '-gc',
                tabName: converter[index].display + " GC%",
                position: {posId: 'topRightGraph', top:'0%', bottom:'50%', left:'65%', right:'99%'},
                viewerConstructor: {
                    dataViewer: "vjGoogleGraphView",
                    dataViewerOptions:{
                        data: dsGCName,
                        name: 'basicGraph',
                        type: "column",
                        title: converter[index].display + " GC%",
                        options:{
                            title: "graph graph",
                            curveType: 'function',
                            //colors:vjPAGE.colorsACGT,
                            colors: colorArray,
                            vAxis: { viewWindow: {min: 0}},
                            legend: {position: 'none'},
                            chartArea :{height:'75%', width: '75%' }
                        },
                        maxRowPerSeries: 4,
                        series:[ {name : "id", label: true}, {name : converter[index].display, type: "number"}]
                    }
                },
                autoOpen: ["computed"]
            });
        }
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.closeTab("searchTab" + submitCounter);
        
        cleanup();
    };
    
    function graphMultipleCodons(viewer, content, requestInfo){
        finishedMultiCounter += 1;
        if (finishedMultiCounter < viewerContainer.length){
            return;
        }
        if (viewerContainer.length == 1){
            cleanup();
            return;  // don't plot a multiple graph if there's only one search
        }
        
        var combinedString = 'id,';
        var dataContainer = [];
        var dataseries = [{name: "id", label: true}];
        var translationTable = 0; //0 is default order, same as table; change when you have real values to fill in
        
        /* initialize dataContainer for each search; prep the combinedString with the right headers; create entries in dataseries for each search */ 
        for (var index = 0; index<viewerContainer.length; index++){
            dataContainer[index] = new Object();
            combinedString += converter[index].display + ",";
            dataseries.push({name: converter[index].display, type: "number"});
        }
        combinedString = combinedString.substring(0,combinedString.length-1) + "\n";  // remove trailing comma, add newline
        
        
        for (var index = 0; index<allRetrieves.length; index++){
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i<splitstring.length; i++){
                var s = splitstring[i].split(",");
                dataContainer[index][s[0]] = s[1];
            }
            
        }
        //console.log(dataContainer);
        
        
        for (var i = 0; i < gencodes[translationTable].length; i++){
            combinedString += gencodes[translationTable][i] + ",";
            for (var index = 0; index<allRetrieves.length; index++){
                combinedString += ((1000*parseFloat(dataContainer[index][gencodes[translationTable][i]]))/parseFloat(dataContainer[index]["\"#codon\""])).toFixed(2).toString() + ",";
            }
            combinedString = combinedString.substring(0,combinedString.length-1) + "\n";
        }
        //console.log(combinedString);
        
        multiDSName = "multiDSTable" + submitCounter;
        vjDS.add("Reordering codons", multiDSName, "static://" + combinedString);        
        
        curTabs.push(multiDSName + '-multiCUT');

        var filesStructureToAdd = [{
           tabId: multiDSName + '-multiCUT',  
           tabName: "Combined Codon Frequencies",
           position: {posId: 'bottomGraph', top:'50%', bottom:'100%', left:'20%', right:'99%'},
           viewerConstructor: {
               dataViewer: "vjGoogleGraphView",
               dataViewerOptions:{
                      data: multiDSName,
                   name: 'basicGraph',
                   type: "column",
                   title: "Codon Frequencies (per 1000 codons)",
                   options:{
                           title: "graph graph",
                           curveType: 'function',
                           //colors:vjPAGE.colorsACGT,
                           colors: colorArray,
                        chartArea :{height:'75%', width: '75%' },
                        vAxis: { viewWindow: {min: 0}}
                   },
                   maxRowPerSeries: 64,
                   series:dataseries
                   }
           },
           autoOpen: ["computed"]
       }];
    
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.closeTab("searchTab" + submitCounter);
        //console.log(dataseries);
        cleanup();
    };
    
    function graphMultipleGC(viewer, content, requestInfo){
        finishedMultiGCCounter += 1;
        if (finishedMultiGCCounter < viewerContainer.length){
            return;
        }
        if (viewerContainer.length == 1){
            cleanup();
            return;  // don't plot a multiple graph if there's only one search
        }
        
        var combinedString = 'id,';
        var dataContainer = [];
        var dataseries = [{name: "id", label: true}];
        
        /* initialize dataContainer for each search; prep the combinedString with the right headers; create entries in dataseries for each search */ 
        for (var index = 0; index<viewerContainer.length; index++){
            dataContainer[index] = new Object();
            combinedString += converter[index].display + ",";
            dataseries.push({name: converter[index].display, type: "number"});
        }
        combinedString = combinedString.substring(0,combinedString.length-1) + "\n";  // remove trailing comma, add newline
        
        
        for (var index = 0; index<allRetrieves.length; index++){
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i<splitstring.length; i++){
                var s = splitstring[i].split(",");
                dataContainer[index][s[0]] = s[1];
            }
            
        }
        
        var gclines = ["GC%", "Codon Position 1 GC%", "Codon Position 2 GC%", "Codon Position 3 GC%"];
        var gcdatanames = ["\"GC%\"", "\"GC1%\"", "\"GC2%\"", "\"GC3%\""]
        
        for (var i = 0; i < gclines.length; i++){
            combinedString += gclines[i] + ",";
            for (var index = 0; index<allRetrieves.length; index++){
                combinedString += dataContainer[index][gcdatanames[i]].toString() + ",";
            }
            combinedString = combinedString.substring(0,combinedString.length-1) + "\n";
        }
        

        //console.log(combinedString);
        
        multiDSGCName = "multiDSGCTable" + submitCounter;
        vjDS.add("Plotting multi GC", multiDSGCName, "static://" + combinedString);
        
        curTabs.push(multiDSGCName + '-multiGC');

        var filesStructureToAdd = [{
           tabId: multiDSGCName + '-multiGC',  
           tabName: "Combined GC%",
           //position: {posId: 'codon', top:'10%', bottom:'55%', left:'20%', right:'100%'},
           position: {posId: 'topRightGraph', top:'0%', bottom:'50%', left:'65%', right:'99%'},
           viewerConstructor: {
               dataViewer: "vjGoogleGraphView",
               dataViewerOptions:{
                      data: multiDSGCName,
                   name: 'basicGraph',
                   type: "column",
                   title: "GC Content",
                   options:{
                        title: "graph graph",
                        curveType: 'function',
                        //colors:vjPAGE.colorsACGT,
                        colors: colorArray,
                        vAxis: { viewWindow: {min: 0}},
                        chartArea :{height:'75%', width: '75%' }
                   },
                   maxRowPerSeries: 4,
                   series:dataseries
                   }
           },
           autoOpen: ["computed"]
       }];
    
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.closeTab("searchTab" + submitCounter);
        //console.log(dataseries);
        
        cleanup();
    };
    
    function cleanup(){
        cleanupCounter += 1;
        if (forceCleanup == false){
            if (cleanupCounter < 4){  // 4 because cleanup() is called by 4 callback functions- if you add another, change to 5
                return;
            }
        }
        

        
        
        algoWidgetObj.removeTabs("searchTab" + submitCounter, "results");
        submitCounter++;
        
        var taxIDPanel = new vjPanelView({
            data:["dsSearch0"],
            rows: [
                    {name: 'changeFile', type:'select', options: [[0,"RefSeq"], [1,"GenBank"]], value:0, title: " ", align:'left', showTitle: false, readonly:false, order:1, },
                    {name: 'dnaType', type:'select', options: [["genomic", "genomic"], ["mitochondrion", "mitochondrion"], ["chloroplast", "chloroplast"], ["plastid", "plastid"],["leucoplast", "leucoplast"],["chromoplast", "chromoplast"]], value: "genomic", title: " ", align:'left', showTitle: false, readonly:false, order:2},
                    {name: 'valueForFilter', type:"text", title: "Query for search", showTitle: true, align: 'left' , order : 3, path: "/valueForFilter"},
                    //{name: 'searchType', type:'select', options: [[0,"Species"], [1,"Assembly"], [2,"Taxid"]], value:0, title: "Search Type", align:"left", showTitle: true, readonly:false, order:3, isSubmitable: true},
                    {name: 'depthFilter', type:"checkbox", title: "Deep Search", showTitle: true, align: 'left' , value: true, order : 4},
                    {name: 'newSearchBox', order:99, title:"Add Additional Search", icon:'Add_query', isSubmitable: true, url: onAddSearchBox},
                    {name:'apply', order:100, title: 'Submit All' , icon:'submit' , isSubmitable: true, url: onUpdate }
                    ],
            formObject: document.forms[formName],
            onKeyPressCallback: smartSearchFunc,
            iconSize:20,
            isok: true
        });
        
        
        id = docLocValue("id");
        
        vjDS.add("", "dsSearch", "static://");
        vjDS["dsSearch"].register_callback(onSearchResults);
        
        if (forceCleanup == true){
            var filesStructureToAdd = [{
                tabId: 'searchTab' + submitCounter,
                tabName: "Search",
                multiView: true,
                tabOrder: 0,  // always insert at the first position
                callback: function (){
                    closeOldTabs();
                    $("[data-id='right']").children("[title='Open']").click();
                    },
                position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {
                    views: [{instance:taxIDPanel, size: '45'}],
                    layoutType: "stack",
                    orientation: "vertical"
                },
                autoOpen: ["computed"]
            }];
        } else {
            var filesStructureToAdd = [{
                tabId: 'searchTab' + submitCounter,
                tabName: "Search",
                multiView: true,
                tabOrder: 0,  // always insert at the first position
                callback: function (){
                    closeOldTabs();
                    $("[data-id='right']").children("[title='Open']").click();
                    },
                position: {posId: 'resultsTable', top:'0', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {
                    views: [{instance:taxIDPanel, size: '45'}],
                    layoutType: "stack",
                    orientation: "vertical"
                },
                autoOpen: ["closed"]
            }];
        }
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        //algoWidgetObj.moveTab("searchTab" + submitCounter, {top:"0%", left: "20%", right: "75%", bottom: "100%"}, 0);
        algoWidgetObj.moveTab("resultsTable", {top:"0%", left: "20%", right: "100%", bottom: "100%"}, 0);
        
        allSearches = ["dsSearch0"];
        allRetrieves = [];

        viewerContainer = [];
        viewerContainer.push(taxIDPanel);

        conversionContainer = [];
        converter = [];
        
        //console.log(oldTabsToClose);
        closeOldTabs();
        oldTabsToClose = oldTabsToClose.concat(curTabs);
        curTabs = [];

        finishedMultiCounter = 0;
        finishedMultiGCCounter = 0;
        finishedGCCounter = 0;
        finishedCodonCounter = 0;
        finishedTableCounter = 0;
        finishedConversions = 0;
        cleanupCounter = 0;
        
        forceCleanup = false;
        
        
        
    };
    
};

//# sourceURL = getBaseUrl() + "/js-obj-new/svc-refseq-processor.js"
