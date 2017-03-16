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
function vjUserUsageView(viewer)
{

    var precompute = "var myTime = new Date(node.id*1000);"+
    "var months = myTime.getMonth()+1;"+
    "var date = myTime.getDate();"+
    "var hours = myTime.getHours();"+
    "var minutes = myTime.getMinutes();"+
   "node.time=months+'/'+date+' '+hours+':'+minutes;";
    if(viewer.data=='dsDiskUsage' || viewer.data == 'dsTempDiskUsage'){
        precompute += "node.value = parseInt(node.cols[1])/1024;"
    }
    else{
        precompute += "node.value = parseInt(node.cols[1]);"
    }
    var googleGraphView1= new vjGoogleGraphView ({
          name:viewer.name,
         // formObject: document.forms['form-uusage'],
          data: viewer.data,
          precompute:precompute,
          options:{ height: '400', colors:['#a0a0ff','ffa0a0'] , legend: 'top' },
          series:viewer.series,
          type:'column',
          isok:true});

    var googleGraphView2= new vjGoogleGraphView ({
        name:viewer.name,
       // formObject: document.forms['form-uusage'],
        data: viewer.data,
        precompute:precompute + "if(ir>0){node.valueAcc=parseInt(this.rows[ir-1].valueAcc) + parseInt(node.value);}else{node.valueAcc = parseInt(node.value);} " ,
        options:{ height: '400', colors:['#a0a0ff','ffa0a0'] , legend: 'bottom'},
        series:viewer.seriesAcc,
        type:'column',
        isok:true});

    return [googleGraphView1,googleGraphView2];


};


//# sourceURL = getBaseUrl() + "/js/vjUserUsageView.js"