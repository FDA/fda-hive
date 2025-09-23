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
var finishedENcCounter = 0;
var finishedConversions = 0;
var finishedTaxCounter = 0;
var cleanupCounter = 0;
var submitCounter = 1;
var assemblyPattern = /GCF_\d{9}\.{1}\d+/i;
var typeLetters = {"genomic": "G", "mitochondrion": "M", "chloroplast": "C", "plastid": "P", "leucoplast": "L", "chromoplast": "O"}
var oldTabsToClose = [];
var curTabs = [];

var gClrTable2 = new Array ("#74c493","#51574a","#447c69","#8e8c6d","#e4bf80","#e9d78e","#e16552","#c94a53","#e2975d","#be5168","#a34974","#f19670","#993767","#65387d","#4e2472","#9163b6","#e279a3","#e0598b","#7c9fb0","#5698c4","#9abf88");
var forceCleanup = false;


var colorArray = new Array ("#1E90FF", "#228B22", "#4682B4", "#9ACD32", "#9370DB", "#D2B48C", "#FA8072", "#DAA520", "#4B0082", "#708090")

var AminoAcids = ["A", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N", "P", "Q", "R", "S", "T", "V", "W", "Y", "*"]
var AllCodons = ["TTT", "TCT", "TAT", "TGT", "TTC", "TCC", "TAC", "TGC", "TTA", "TCA", "TAA", "TGA", "TTG", "TCG", "TAG", "TGG", "CTT", "CCT", "CAT", "CGT", "CTC", "CCC", "CAC", "CGC", "CTA", "CCA", "CAA", "CGA", "CTG", "CCG", "CAG", "CGG", "ATT", "ACT", "AAT", "AGT", "ATC", "ACC", "AAC", "AGC", "ATA", "ACA", "AAA", "AGA", "ATG", "ACG", "AAG", "AGG", "GTT", "GCT", "GAT", "GGT", "GTC", "GCC", "GAC", "GGC", "GTA", "GCA", "GAA", "GGA", "GTG", "GCG", "GAG", "GGG"]

var translation = new Object();
translation[1] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[2] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "M", "ACA": "T", "AAA": "K", "AGA": "*", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "*", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[3] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "T", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "T", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "T", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "T", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "M", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[4] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[5] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "M", "ACA": "T", "AAA": "K", "AGA": "S", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "S", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[6] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "Q", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "Q", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[9] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "N", "AGA": "S", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "S", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[10] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "C", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[11] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[12] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "S", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[13] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "M", "ACA": "T", "AAA": "K", "AGA": "G", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "G", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[14] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "Y", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "N", "AGA": "S", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "S", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[15] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "Q", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[16] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "L", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[21] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "M", "ACA": "T", "AAA": "N", "AGA": "S", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "S", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[22] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "*", "TAA": "*", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "L", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[23] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "*", "TCA": "S", "TAA": "*", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[24] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "W", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "S", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "K", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[25] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "G", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "L", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};
translation[26] = {"TTT": "F", "TCT": "S", "TAT": "Y", "TGT": "C", "TTC": "F", "TCC": "S", "TAC": "Y", "TGC": "C", "TTA": "L", "TCA": "S", "TAA": "*", "TGA": "*", "TTG": "L", "TCG": "S", "TAG": "*", "TGG": "W", "CTT": "L", "CCT": "P", "CAT": "H", "CGT": "R", "CTC": "L", "CCC": "P", "CAC": "H", "CGC": "R", "CTA": "L", "CCA": "P", "CAA": "Q", "CGA": "R", "CTG": "A", "CCG": "P", "CAG": "Q", "CGG": "R", "ATT": "I", "ACT": "T", "AAT": "N", "AGT": "S", "ATC": "I", "ACC": "T", "AAC": "N", "AGC": "S", "ATA": "I", "ACA": "T", "AAA": "K", "AGA": "R", "ATG": "M", "ACG": "T", "AAG": "K", "AGG": "R", "GTT": "V", "GCT": "A", "GAT": "D", "GGT": "G", "GTC": "V", "GCC": "A", "GAC": "D", "GGC": "G", "GTA": "V", "GCA": "A", "GAA": "E", "GGA": "G", "GTG": "V", "GCG": "A", "GAG": "E", "GGG": "G"};

var revtranslation = new Object();
revtranslation[1] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TGA","TAG"]};
revtranslation[2] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATA","ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG","AGA","AGG"]};
revtranslation[3] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC"], "K": ["AAA","AAG"], "L": ["TTA","TTG"], "M": ["ATA","ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["CTT","CTC","CTA","CTG","ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[4] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[5] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATA","ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC","AGA","AGG"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[6] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["TAA","TAG","CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TGA"]};
revtranslation[9] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC","AAA"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC","AGA","AGG"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[10] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC","TGA"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[11] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TGA","TAG"]};
revtranslation[12] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","CTG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TGA","TAG"]};
revtranslation[13] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["AGA","AGG","GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATA","ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[14] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC","AAA"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC","AGA","AGG"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC","TAA"], "*": ["TAG"]};
revtranslation[15] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["TAG","CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TGA"]};
revtranslation[16] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","TAG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TGA"]};
revtranslation[21] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC"], "K": ["AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATA","ATG"], "N": ["AAT","AAC","AAA"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC","AGA","AGG"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[22] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","TAG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TCA","TAA","TGA"]};
revtranslation[23] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TTA","TAA","TGA","TAG"]};
revtranslation[24] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG","AGG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC","AGA"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGA","TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[25] = {"A": ["GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["TGA","GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA","CTG"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TAG"]};
revtranslation[26] = {"A": ["CTG","GCT","GCC","GCA","GCG"], "C": ["TGT","TGC"], "D": ["GAT","GAC"], "E": ["GAA","GAG"], "F": ["TTT","TTC"], "G": ["GGT","GGC","GGA","GGG"], "H": ["CAT","CAC"], "I": ["ATT","ATC","ATA"], "K": ["AAA","AAG"], "L": ["TTA","TTG","CTT","CTC","CTA"], "M": ["ATG"], "N": ["AAT","AAC"], "P": ["CCT","CCC","CCA","CCG"], "Q": ["CAA","CAG"], "R": ["CGT","CGC","CGA","CGG","AGA","AGG"], "S": ["TCT","TCC","TCA","TCG","AGT","AGC"], "T": ["ACT","ACC","ACA","ACG"], "V": ["GTT","GTC","GTA","GTG"], "W": ["TGG"], "Y": ["TAT","TAC"], "*": ["TAA","TGA","TAG"]};

gencodeids = [1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14, 15, 16, 21, 22, 23, 24, 25, 26];
gencodenames = {1: "Standard", 2: "Vertebrate Mitochondrial", 3: "Yeast Mitochondrial", 4: "Mold Mitochondrial, Protozoan Mitochondrial, Coelenterate Mitochondrial, Mycoplasma, Spiroplasma", 5: "Invertebrate Mitochondrial", 6: "Ciliate Nuclear, Dasycladacean Nuclear, Hexamita Nuclear", 9: "Echinoderm Mitochondrial, Flatworm Mitochondrial", 10: "Euplotid Nuclear", 11: "Bacterial, Archaeal, Plant Plastid", 12: "Alternative Yeast Nuclear", 13: "Ascidian Mitochondrial", 14: "Alternative Flatworm Mitochondrial", 15: "Blepharisma Macronuclear", 16: "Chlorophycean Mitochondrial", 21: "Trematode Mitochondrial", 22: "Scenedesmus obliquus Mitochondrial", 23: "Thraustochytrium Mitochondrial", 24: "Pterobranchia Mitochondrial", 25: "Candidate Division SR1, Gracilibacteria", 26: "Pachysolen tannophilus Nuclear Code"};


vjHO.register('svc-refseq-processor').Constructor=function ()
{
    var id;
    vjDS.add("", "dsSearch0", "static:
    

    


    var taxIDPanel = new vjPanelView({
        data:["dsSearch0"],
        rows: [
                {name: 'changeFile', type:'select', options: [[0,"RefSeq"], [1,"GenBank"]], value:0, title: " ", align:'left', showTitle: false, readonly:false, order:1, },
                {name: 'dnaType', type:'select', options: [["genomic", "genomic"], ["mitochondrion", "mitochondrion"], ["chloroplast", "chloroplast"], ["plastid", "plastid"],["leucoplast", "leucoplast"],["chromoplast", "chromoplast"]], value: "genomic", title: " ", align:'left', showTitle: false, readonly:false, order:2},
                {name: 'valueForFilter', type:"text", title: "Query for search", showTitle: true, align: 'left' , order : 3, path: "/valueForFilter"},
                {name: 'depthFilter', type:"checkbox", title: "Deep Search", showTitle: true, align: 'left' , value: true, order : 4},
                {name: 'newSearchBox', order:99, title:"Add Additional Search", icon:'Add_query', isSubmitable: true, url: onAddSearchBox},
                {name:'apply', order:100, title: 'Submit All' , icon:'submit' , isSubmitable: true, url: onUpdate }
                ],
        formObject: document.forms[formName],
        onKeyPressCallback: smartSearchFunc,
        iconSize:20,
        isok: true
    });
    
    viewerContainer.push(taxIDPanel);

   
    var gencodes = new Object();
    gencodes[0] = ["TTT", "TCT", "TAT", "TGT", "TTC", "TCC", "TAC", "TGC", "TTA", "TCA", "TAA", "TGA", "TTG", "TCG", "TAG", "TGG", "CTT", "CCT", "CAT", "CGT", "CTC", "CCC", "CAC", "CGC", "CTA", "CCA", "CAA", "CGA", "CTG", "CCG", "CAG", "CGG", "ATT", "ACT", "AAT", "AGT", "ATC", "ACC", "AAC", "AGC", "ATA", "ACA", "AAA", "AGA", "ATG", "ACG", "AAG", "AGG", "GTT", "GCT", "GAT", "GGT", "GTC", "GCC", "GAC", "GGC", "GTA", "GCA", "GAA", "GGA", "GTG", "GCG", "GAG", "GGG"];


    this.fullview=function(node, whereToAdd)
    {
        id = docLocValue("id");
        
        vjDS.add("", "dsSearch", "static:
        vjDS["dsSearch"].register_callback(onSearchResults);
        
        vjDS.add("", "dsSearchExplanation", "static:
        vjDS["dsSearchExplanation"].register_callback(onSearchResults);

        var filesStructureToAdd = [{
            tabId: 'searchTab' + submitCounter,
            tabName: "Search",
            multiView: true,
            callback: function (){
                closeOldTabs();
                $("[data-id='right']").children("[title='Open']").click();
                },
            position: {posId: 'resultsTable', top:'0', bottom:'80%', left:'20%', right:'75%'},
            viewerConstructor: {
                views: [{instance:taxIDPanel, size: '45'}],
                layoutType: "stack",
                orientation: "vertical"
            },
            autoOpen: ["computed"]
        },
        {
             tabId: "dsSearchExplanation0",
             tabName: "Search Explanation",
            position: {posId: 'SearchExplanation', top:'80%', bottom:'100%', left:'20%', right:'75%'},
            viewerConstructor: {
                dataViewer: "vjTextView",
                dataViewerOptions:{
                    data: "dsSearchExplanation",
                   name: 'basicGraph',
                   width: "100%",
                   height: "10vh"
                }
            },
            autoOpen: ["computed"]
        }];
        

        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.moveTab("resultsTable", {top:"0%", left: "20%", right: "75%", bottom: "100%"}, 0);
        
        $("#parameters").closest("li").hide();
        $("#progress").closest("li").hide();
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
        
        vjDS.add("", newdsname, "static:
        allSearches.push(newdsname);
        
        var shorttaxIdPanel = new vjPanelView({
            data: newdsname,
            rows: [
                   {name: 'changeFile', type:'select', options: [[0,"RefSeq"], [1,"GenBank"]], value:0, title: " ", align:'left', showTitle: false, readonly:false, order:1},
                   {name: 'dnaType', type:'select', options: [["genomic", "genomic"], ["mitochondrion", "mitochondrion"], ["chloroplast", "chloroplast"], ["plastid", "plastid"],["leucoplast", "leucoplast"],["chromoplast", "chromoplast"]], value: "genomic", title: " ", align:'left', showTitle: false, readonly:false, order:2},
                   {name: 'valueForFilter', type:"text", title: "Query for search", showTitle: true, align: 'left' , order : 3, path: "/valueForFilter"},
                   {name: 'depthFilter', type:"checkbox", title: "Deep Search", showTitle: true, align: 'left' , value: true, order : 4}
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
            inputVal = viewerContainer[index].tree.root.children[2].value.trim();
            inputVal = unescape(inputVal);
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
        if (submitCounter > 1){
            algoWidgetObj.removeTabs("taxonomyTab" + (submitCounter-1), "results");
        }

        for (var index = 0; index<viewerContainer.length; index++){
            
            sourceVal = viewerContainer[index].tree.root.children[0].value ? "genbank" : "refseq";
            dnaTypeVal = viewerContainer[index].tree.root.children[1].value;
            inputVal = viewerContainer[index].tree.root.children[2].value.trim();
            inputVal = unescape(inputVal);
        
            var type = "t";
            if (sourceVal == "genbank"){
                type += "G";
            } else {
                type += "R";
            }
            
            type += typeLetters[dnaTypeVal]
            converturl = "http:
            
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
            converturl = converturl + "&raw=1" + "&regex=1"; 
            vjDS.add("", convertdsName, converturl);
            vjDS[convertdsName].reload(converturl, true);
            conversionContainer.push(convertdsName);
            vjDS[convertdsName].register_callback(buildConversion);
            
        }
    };
    
    function onUpdate2(viewer,node,irow) {


        var sourceVal;
        var searchTypeVal;
        var deepSearch;
        var organelle;
        var url;
        var filesStructureToAdd = [];
        var convertdsName;
        var converturl;
        var id = docLocValue("id");
    

        for(var index = 0; index<viewerContainer.length; index++){
            sourceVal = viewerContainer[index].tree.root.children[0].value ? "genbank" : "refseq";
            dnaTypeVal = viewerContainer[index].tree.root.children[1].value;
            inputVal = viewerContainer[index].tree.root.children[2].value.trim();
            inputVal = unescape(inputVal);
            deepSearch = viewerContainer[index].tree.root.children[3].value;
            
            
            if (sourceVal == "genbank"){
                url = "http:
            } else {
                url = "http:
            }
                  
            
            url = url + "&taxid=" + vjDS.escapeQueryLanguage(converter[index].taxid);
            
            var filter_col_names = ["Organelle"];
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
            vjDS[dsName].register_callback(calculateENc);
            vjDS[dsName].register_callback(graphGC);
            vjDS[dsName].register_callback(graphMultipleGC);
            vjDS[dsName].register_callback(graphReorderedCodons);
            vjDS[dsName].register_callback(graphMultipleCodons);
            vjDS[dsName].reload(url, true);
            
        
        }  
        
        algoWidgetObj.closeTab("searchTab" + submitCounter);
        algoWidgetObj.closeTab("dsSearchExplanation0");
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
            converter[index] = placeholder;
        }
        
        for (var index = 0; index < viewerContainer.length; index++){
            var inputVal = viewerContainer[index].tree.root.children[2].value.trim();
            inputVal = unescape(inputVal)
            var position;

            
            if (!isNaN(inputVal)){
                position = 0;
            }
            else if (assemblyPattern.test(inputVal)){
                position = 1;
            }
            else {
                position = 2;
            }
            

            var splitstring = vjDS[conversionContainer[index]].data.split("\n");
            var n = 0;
            
            for (var i = 0; i < splitstring.length; i++){
                var data = new Object();
                s = splitstring[i].split(",");
                if (s[position].toLowerCase() == inputVal.toLowerCase()){
                    if (isNaN(inputVal) && !assemblyPattern.test(inputVal)){
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
                        break;
                    }
                    else if (assemblyPattern.test(inputVal)){
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
                        break;
                    }
                    else {
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
                        
                    }
                }
            }
            
            
        }
        
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
                }
            }
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
        
        for (var i = emptyIndices.length -1; i >= 0; i--){
            indexToRemove = emptyIndices[i];
            viewerContainer.splice(indexToRemove,1);
            converter.splice(indexToRemove,1);
        }
        
        var failedString = "The following search(es) failed:\n";
        if (emptyIndices.length > 0){
            for (var i = 0; i<emptyIndices.length; i++){
                failedString += "  " + emptySearches[i] + "\n";
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
        }

        var inputVal, sourceVal, dnaTypeVal;
        for (var i = 0; i < viewerContainer.length; i++){
            if (viewerContainer[i].container == this.container){
                inputVal = viewerContainer[i].tree.root.children[2].value;
                inputVal = unescape(inputVal);
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
            var type = "t";
            if (sourceVal == "genbank"){
                type += "G";
            } else {
                type += "R";
            }
            
            type += typeLetters[dnaTypeVal]
            var nUrl = "http:
            searchVal = vjDS.escapeQueryLanguage(el.value) + String.fromCharCode(e.which);
            lastContainer = this.container;
            vjDS["dsSearch"].reload(nUrl, true);
        }
    };
    
    var searchVal;
    function onSearchResults (viewer, content, requestInfo){
        var table = new vjTable (content);
        
        var containerId = lastContainer;
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
                continue;
            } else {
                viewerContainer[index].rows.push({name: taxID, taxName: name, order:i ,title: name, isSubmitable: true, path: "/valueForFilter/"+taxID, taxID: taxID, url:onUpdateText});
                seenNames[name] = true;
            }
        }
        
        
        viewerContainer[index].rebuildTree();
        
    };
    
    function onUpdateText (viewer, node, irow){
        viewer.tree.root.children[2].value = node.taxName;
        viewer.refresh();
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
           tabOrder: 3,
           position: {posId: 'taxonomy', top:'0%', bottom:'50%', left:'20%', right:'65%'},
           viewerConstructor: {
              instance: taxonomy[0]
          },
          autoOpen: ["closed"]
      });
      
      algoWidgetObj.addTabs(filesStructureToAdd, "results");
      algoWidgetObj.closeTab("searchTab" + submitCounter);
      algoWidgetObj.closeTab("dsSearchExplanation0");
    };
    
    
    
    function reformatText (viewer, content, requestInfo){
        finishedTableCounter += 1;
        if (finishedTableCounter < viewerContainer.length){
            return;
        }
        
        var codons = ["TTT", "TCT", "TAT", "TGT", "TTC", "TCC", "TAC", "TGC", "TTA", "TCA", "TAA", "TGA", "TTG", "TCG", "TAG", "TGG", "CTT", "CCT", "CAT", "CGT", "CTC", "CCC", "CAC", "CGC", "CTA", "CCA", "CAA", "CGA", "CTG", "CCG", "CAG", "CGG", "ATT", "ACT", "AAT", "AGT", "ATC", "ACC", "AAC", "AGC", "ATA", "ACA", "AAA", "AGA", "ATG", "ACG", "AAG", "AGG", "GTT", "GCT", "GAT", "GGT", "GTC", "GCC", "GAC", "GGC", "GTA", "GCA", "GAA", "GGA", "GTG", "GCG", "GAG", "GGG"];
        var filesStructureToAdd = [];
         
        
        for (index = 0; index<viewerContainer.length; index++){
            var data = new Object();
            var formattedString = "";
    
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i < splitstring.length; i++){
                s = splitstring[i].split(",");
                data[s[0]] = s[1];
            }
            
            
            formattedString += "Table contains " + data["\"#CDS\""] + " CDSs (" + data["\"#codon\""] + " codons)\n\n";
    
            
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
                
                if ((j+1) < codons.length){
                    if (column == 3){
                        formattedString += "\n";
                    } else { 
                        formattedString += "&nbsp;&nbsp;";
                    }
                    
                    if ((j+1)%16 == 0){
                        formattedString += "\n";
                    }
                }
                
            }
            
            
            var tableDSName = "dsTable" + submitCounter + index.toString();
            
            vjDS.add("", tableDSName, "static:
            
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
         algoWidgetObj.closeTab("dsSearchExplanation0");
         
     cleanup();
    };
    
    
    
    
    function graphReorderedCodons(viewer, content, requestInfo){
        finishedCodonCounter += 1;
        if (finishedCodonCounter < viewerContainer.length){
            return;
        }
        
        var filesStructureToAdd = [];
        codonarr = ["TTT", "TTC", "TTA", "TTG", "TCT", "TCC", "TCA", "TCG", "TAT", "TAC", "TAA", "TAG", "TGT", "TGC", "TGA", "TGG", "CTT", "CTC", "CTA", "CTG", "CCT", "CCC", "CCA", "CCG", "CAT", "CAC", "CAA", "CAG", "CGT", "CGC", "CGA", "CGG", "ATT", "ATC", "ATA", "ATG", "ACT", "ACC", "ACA", "ACG", "AAT", "AAC", "AAA", "AAG", "AGT", "AGC", "AGA", "AGG", "GTT", "GTC", "GTA", "GTG", "GCT", "GCC", "GCA", "GCG", "GAT", "GAC", "GAA", "GAG", "GGT", "GGC", "GGA", "GGG"];
        
        for (var index = 0; index<viewerContainer.length; index++){
            var reorderedDataString = "id," + converter[index].display +"\n";
            var translationTable = 0;
            var data = new Object();
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i < splitstring.length; i++){
                s = splitstring[i].split(",");
                data[s[0]] = s[1];
            }
            
    
            
            for (var i = 0; i < codonarr.length; i++){
                reorderedDataString += codonarr[i] + "," + data[codonarr[i]] + "\n";
            }
    
            
            
            freqDSName = "dsFreq" + submitCounter + index.toString();
            vjDS.add("Reordering codons", freqDSName, "static:
            
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
        algoWidgetObj.closeTab("dsSearchExplanation0");
        
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
            vjDS.add("Reordering codons", dsGCName, "static:
            
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
        algoWidgetObj.closeTab("dsSearchExplanation0");
        
        cleanup();
    };
    
    function graphMultipleCodons(viewer, content, requestInfo){
        finishedMultiCounter += 1;
        if (finishedMultiCounter < viewerContainer.length){
            return;
        }
        if (viewerContainer.length == 1){
            cleanup();
            return;
        }
        
        var combinedString = 'id,';
        var dataContainer = [];
        var dataseries = [{name: "id", label: true}];
        var translationTable = 0;
        
        for (var index = 0; index<viewerContainer.length; index++){
            dataContainer[index] = new Object();
            combinedString += converter[index].display + ",";
            dataseries.push({name: converter[index].display, type: "number"});
        }
        combinedString = combinedString.substring(0,combinedString.length-1) + "\n";
        
        
        for (var index = 0; index<allRetrieves.length; index++){
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i<splitstring.length; i++){
                var s = splitstring[i].split(",");
                dataContainer[index][s[0]] = s[1];
            }
            
        }
        
        codonarr = ["TTT", "TTC", "TTA", "TTG", "TCT", "TCC", "TCA", "TCG", "TAT", "TAC", "TAA", "TAG", "TGT", "TGC", "TGA", "TGG", "CTT", "CTC", "CTA", "CTG", "CCT", "CCC", "CCA", "CCG", "CAT", "CAC", "CAA", "CAG", "CGT", "CGC", "CGA", "CGG", "ATT", "ATC", "ATA", "ATG", "ACT", "ACC", "ACA", "ACG", "AAT", "AAC", "AAA", "AAG", "AGT", "AGC", "AGA", "AGG", "GTT", "GTC", "GTA", "GTG", "GCT", "GCC", "GCA", "GCG", "GAT", "GAC", "GAA", "GAG", "GGT", "GGC", "GGA", "GGG"]
        
        for (var i = 0; i < codonarr.length; i++){
            combinedString += codonarr[i] + ",";
            for (var index = 0; index<allRetrieves.length; index++){
                combinedString += ((1000*parseFloat(dataContainer[index][codonarr[i]]))/parseFloat(dataContainer[index]["\"#codon\""])).toFixed(2).toString() + ",";
            }
            combinedString = combinedString.substring(0,combinedString.length-1) + "\n";
        }
        
        multiDSName = "multiDSTable" + submitCounter;
        vjDS.add("Reordering codons", multiDSName, "static:
        
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
        algoWidgetObj.closeTab("dsSearchExplanation0");
        cleanup();
    };
    
    function graphMultipleGC(viewer, content, requestInfo){
        finishedMultiGCCounter += 1;
        if (finishedMultiGCCounter < viewerContainer.length){
            return;
        }
        if (viewerContainer.length == 1){
            cleanup();
            return;
        }
        
        var combinedString = 'id,';
        var dataContainer = [];
        var dataseries = [{name: "id", label: true}];
        
        for (var index = 0; index<viewerContainer.length; index++){
            dataContainer[index] = new Object();
            combinedString += converter[index].display + ",";
            dataseries.push({name: converter[index].display, type: "number"});
        }
        combinedString = combinedString.substring(0,combinedString.length-1) + "\n";
        
        
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
        

        
        multiDSGCName = "multiDSGCTable" + submitCounter;
        vjDS.add("Plotting multi GC", multiDSGCName, "static:
        
        curTabs.push(multiDSGCName + '-multiGC');

        var filesStructureToAdd = [{
           tabId: multiDSGCName + '-multiGC',  
           tabName: "Combined GC%",
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
        algoWidgetObj.closeTab("dsSearchExplanation0");
        
        cleanup();
    };
    
    function calculateENc (viewer, content, requestInfo){
        finishedENcCounter += 1;
        if (finishedENcCounter < viewerContainer.length){
            return;
        }
        
               
        var codons = ["TTT", "TCT", "TAT", "TGT", "TTC", "TCC", "TAC", "TGC", "TTA", "TCA", "TAA", "TGA", "TTG", "TCG", "TAG", "TGG", "CTT", "CCT", "CAT", "CGT", "CTC", "CCC", "CAC", "CGC", "CTA", "CCA", "CAA", "CGA", "CTG", "CCG", "CAG", "CGG", "ATT", "ACT", "AAT", "AGT", "ATC", "ACC", "AAC", "AGC", "ATA", "ACA", "AAA", "AGA", "ATG", "ACG", "AAG", "AGG", "GTT", "GCT", "GAT", "GGT", "GTC", "GCC", "GAC", "GGC", "GTA", "GCA", "GAA", "GGA", "GTG", "GCG", "GAG", "GGG"];
        var filesStructureToAdd = [];
        
        
        

        
         
        
        for (index = 0; index<viewerContainer.length; index++){
            var data = new Object();
            var formattedString = "Effective Number of Codons for each genetic code:\nFor more information about ENc, see the help toolbar at right.\n\n";
    
            var splitstring = vjDS[allRetrieves[index]].data.split(/\s+/);
            
            for (var i = 0; i < splitstring.length; i++){
                s = splitstring[i].split(",");
                data[s[0]] = parseInt(s[1]);
            }
    
            for (gencodeindex = 0; gencodeindex<gencodeids.length; gencodeindex++){
                gencode = gencodeids[gencodeindex]
        
                var n = new Object();
                var p = new Object();
                var f = new Object();
                
                for (i = 0; i<AminoAcids.length; i++){
                    aminoacid = AminoAcids[i];
                    n[aminoacid] = 0;
                }
                
                for (i = 0; i<AminoAcids.length; i++){
                    aminoacid = AminoAcids[i];
                    for (j = 0; j<revtranslation[gencode][aminoacid].length; j++){
                        codon = revtranslation[gencode][aminoacid][j];
                        n[aminoacid] += data[codon];
                    }
                }
                
                for (i = 0; i<AllCodons.length; i++){
                    codon = AllCodons[i];
                    p[codon] = data[codon]/n[translation[gencode][codon]];
                }
                
                for (i = 0; i<AminoAcids.length; i++){
                    aminoacid = AminoAcids[i];
                    psqsum = 0;
                    for (j = 0; j<revtranslation[gencode][aminoacid].length; j++){
                        codon = revtranslation[gencode][aminoacid][j];
                        psqsum += (p[codon]*p[codon]);
                    }
                    f[aminoacid] = (n[aminoacid]*psqsum-1)/(n[aminoacid]-1);
                }
                
                
                var numerators = {0:0, 1:0, 2:0, 3:0, 4:0, 5:0, 6:0, 7:0, 8:0, 9:0, 10:0}
                var denominators = {0: [], 1: [], 2: [], 3: [], 4: [], 5: [], 6: [], 7: [], 8: [], 9: [], 10: []}
                for (i = 0; i<AminoAcids.length; i++){
                    aminoacid = AminoAcids[i]
                    numcodons = revtranslation[gencode][aminoacid].length;
                    numerators[numcodons] += 1;
                    denominators[numcodons].push(f[aminoacid]);
                }
                
                
                var ENc = 0;
                
                for (i = 0; i<10; i++){
                    if (numerators[i] == 0){
                        continue;
                    }
                    
                    sum = 0;
                    for (j = 0; j<denominators[i].length; j++){
                        sum += denominators[i][j];
                    }
                    
                    ENc += (numerators[i]/(sum/denominators[i].length));
                }
                
                
                
    
                formattedString +=  gencode + ": " + ENc.toFixed(3) + " (" + gencodenames[gencode] + " code)\n";
            
            }
            
            var encDSName = "dsENc" + submitCounter + index.toString();
            
            vjDS.add("", encDSName, "static:
            
            curTabs.push(encDSName + '-enc');
            
            

            filesStructureToAdd.push({
                  tabId: encDSName + '-enc',
                  tabName: converter[index].display + " Codon Bias",
                 position: {posId: 'CodonBiasTab', top:'0%', bottom:'50%', left:'20%', right:'65%'},
                 viewerConstructor: {
                     dataViewer: "vjTextView",
                     dataViewerOptions:{
                         data: encDSName,
                        name: 'basicGraph',
                        width: "800px"
                     }
                 },
                 autoOpen: ["closed"]
             });
        
            
        }
         algoWidgetObj.addTabs(filesStructureToAdd, "results");
         algoWidgetObj.closeTab("searchTab" + submitCounter);
         algoWidgetObj.closeTab("dsSearchExplanation0");
        
        
     cleanup();
            
    }; 
        
        
    function cleanup(){
        cleanupCounter += 1;
        if (forceCleanup == false){
            if (cleanupCounter < 6){
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
        
        vjDS.add("", "dsSearch", "static:
        vjDS["dsSearch"].register_callback(onSearchResults);
        
        if (forceCleanup == true){
            var filesStructureToAdd = [{
                tabId: 'searchTab' + submitCounter,
                tabName: "Search",
                multiView: true,
                tabOrder: 0,
                callback: function (){
                    closeOldTabs();
                    $("[data-id='right']").children("[title='Open']").click();
                    },
                position: {posId: 'resultsTable2', top:'0', bottom:'100%', left:'20%', right:'100%'},
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
                tabOrder: 0,
                callback: function (){
                    closeOldTabs();
                    $("[data-id='right']").children("[title='Open']").click();
                    },
                position: {posId: 'resultsTable2', top:'0', bottom:'100%', left:'20%', right:'100%'},
                viewerConstructor: {
                    views: [{instance:taxIDPanel, size: '45'}],
                    layoutType: "stack",
                    orientation: "vertical"
                },
                autoOpen: ["closed"]
            }];
        }
        
        algoWidgetObj.addTabs(filesStructureToAdd, "results");
        algoWidgetObj.moveTab("resultsTable", {top:"0%", left: "20%", right: "100%", bottom: "100%"}, 0);
        
        allSearches = ["dsSearch0"];
        allRetrieves = [];

        viewerContainer = [];
        viewerContainer.push(taxIDPanel);

        conversionContainer = [];
        converter = [];
        
        oldTabsToClose.push("advHelp");
        oldTabsToClose.push("dsSearchExplanation0");
        closeOldTabs();
        oldTabsToClose = oldTabsToClose.concat(curTabs);
        curTabs = [];
        

        finishedMultiCounter = 0;
        finishedMultiGCCounter = 0;
        finishedGCCounter = 0;
        finishedCodonCounter = 0;
        finishedTableCounter = 0;
        finishedENcCounter = 0;
        finishedConversions = 0;
        cleanupCounter = 0;
        
        forceCleanup = false;
        
        
        
    };
    
};

