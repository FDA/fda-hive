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
/*
 sampleTableView={

 checkable
 newSectionWord: appearance of these words in the first column makes them to be rendered as th ... sections

 className: 'DV_table',
 startAutoNumber: '',
 skipRows: 0,
 prefixHTML: 'text', // prefix text for table
 appendHTML: 'text', // psotfix tex for the table
 allowVariableReplacement: false, in the values
 appendCols : [{header: name, cell: text }]
 clickLink: javascript: or url
 exclusionObjRegex: {file-name: null, id: /![0-9]/g}
 iconSize: 24
 defaultIcon : 'rec'
 this: "dsMenuAct",
 defaultEmptyText: "" ,   //      'no element to show'
 maxTxtLen: 32,        // max number of text will be shown in a cell, the rest is interpreted by ...
 isStickyHeader:true    //puts the header into seperate div (and table) so scrolling doesn't affect header.
 isReOrderable : false  //columns can be reordered

 cols:[   // serial number of the column to be customized
      { name: name of the column to be customized,
        link: url or javascript,
        align: right | left,
        type: largenumber, percent,
        wrap: true|false,
        hidden: true|false
      }],

 rows:[ { //serial number of the row to customized
       checked,
       styleColor,
       styleNoCheckmark,
       url: url or javascript
       }]
  selectCallback: function,         //   "function:vjObjFunc('onSelectedItem','" + this.objCls + "')"
  checkCallback: function,          //   "function:vjObjFunc('onCheckedItem','" + this.objCls + "')"
  callbackRendered: function,       //    "function:vjObjFunc('onLoadedItem','" + this.objCls + "')"
  precompute: "if(node.id==0 || node.id=='+')node.styleNoCheckmark=true;",
 };



 */

function vjMobileTableView(viewer) {

    vjTableView.call(this, viewer); // Inherit default behaviors from TableView

    this.waitOnLoadCallback=function(a,b,c,d)
    {
        var headerdivH=document.getElementById('seconddiv').style.height;
        var tabdivH=document.getElementById('Running').style.height;

        var superdivH=document.getElementById('super').style.height;
        var loadH=screen.height-headerdivH-tabdivH+"px";
        for(var i=0; i<this.data.length; i++)
            this.dataSourceEngine[this.data[i]].refreshOnEmpty=true;
        this.div=this.container?gObject(this.container):null;
        if(!this.div)return ;
        var top=this.div.scrollTop;
        if( !this.doNotShowRefreshIcon) {
            if(this.div.style.overflow=='auto')this.div.style.overflow='hidden';
            this.div.innerHTML ="<div id='"+this.container+"_loading' style='position:static;width:100%;height:100%'><div class='progressingMobile'>&nbsp;</div></div>"+this.div.innerHTML;//styles='background-image:url(\"img/progress.gif\");' width='16'></img>";
        }
//      this.div_loading=gObject(this.container+"_loading");
    };
}



function vjProcessMobileView(viewer) {

    vjMobileTableView.call(this, viewer); // inherit default behaviours of the

    this.generateTableViewText=function(tbl)
    {
        var tt="</br>";
        if (this.prefixHTML && this.prefixHTML.length) {
            tt += this.prefixHTML;
        }


        for (var x=0;x<tbl.rows.length;x++){
            var name="";
            if (!tbl.rows[x]['name'] || tbl.rows[x]['name'] == "undefined" || tbl.rows[x]['name'] == "")
                {
                    name="<No Name Entered>";
                }
            else if(tbl.rows[x]['name']) {
                if(tbl.rows[x]['name'].length>=25){
                    name=tbl.rows[x]['name'].substring(0,22)+"...";

                }
                else {
                    name=tbl.rows[x]['name'];
                }

            }

            
           var date="Created: "+formaDatetime(tbl.rows[x]['created']);
            var time=Date.now()-tbl.rows[x]['created']*1000;
            console.log("C:" +tbl.rows[x]['created']);
            console.log("N:"+ Date.now());
            var limit=36*24*60*60*1000; //days*hours in day* minutes in hour*seconds in minute*1000
            var icon="";


           if (vjHO[tbl.rows[x]['_type']]!=undefined){
                icon=makeImgSrc(vjHO[tbl.rows[x]['_type']].icon.replace('==',tbl.rows[x]['_type']));
           }
           if (time<limit)
          {            
               tt=tt+"<div id=\"row\" onclick=\"Select_Options("+x+","+tbl.rows[x]['id']+")\"><div id =\"check\" class=\"circle\"></div><div id=\"process\" class=\"divtable\" ><div class=\"processinner\" id="+tbl.rows[x]['id']+" onClick='vjObjEvent(\"onSelect\", \"" + this.objCls+"\","+x+");' ><div id=\"top\" class=\"processHeader\"><div id=\"name\"class=\"processName\">"+name+"</div>"+
              "                 <img src='"+icon+"'class=\"processImg\"></div>"+
                "<div id=\"details\" class=\"processDetails\"><div id=\"id\" class=\"processDate\">"+date+"</div>" +
                "<div id=\"rest\"class=\"processsRest\">"+tbl.rows[x]['svcTitle']+"<br />"+tbl.rows[x]['id']+"</div></div>"+
                  "<div id=bar class=\"progressbar1\"><div id="+x+" class=\"progressbarDONE\"></div></div></div></div></div>";


        }


            }
              document.getElementById(this.container).innerHTML=tt;
              if (time<limit)
              { 
                  for    (var x=0;x<    tbl.rows.length;x++){
                      if(tbl.rows[x]['progress100']){
                          var progress=tbl.rows[x]['progress100']+"%";
                          document.getElementById(x).style.width=progress;
                      }
                 
                  }
              }
            return tt;
    };
    this.onSelect = function (container,x) {
        if(this.onSelectCallback) {
            funcLink(this.onSelectCallback,this.tblArr.rows[x],x);
        }
    };

    this.refresh = function() {
        if (!this.tblArr || !this.tblArr.rows) return;
        //this.div.innerHTML =
        this.generateTableViewText(this.tblArr);
        if(this.isStickyHeader){
            this._reHeadered = false;
            this.stickyHeader();
        }

        if(this.drag)this.setDnDobjects();
    };



}
function vjMobileView(viewer) {
     vjTableView.call(this, viewer); // inherit default behaviours of the

        this.generateTableViewText=function(tbl)
        {
            var tt="</br>";
            if (this.prefixHTML && this.prefixHTML.length) {
                tt += this.prefixHTML;
            }
        };
}

function changeTabs(a,b,c){

    //------> change the tab label
    document.getElementById("runningLabel").innerHTML=a;
    document.getElementById("completedLabel").innerHTML=b;
    document.getElementById("failedLabel").innerHTML=c;
}


//# sourceURL = getBaseUrl() + "/js/vjTableViewMobile.js"