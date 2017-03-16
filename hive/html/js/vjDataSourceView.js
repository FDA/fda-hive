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
var gDatasourceViewerDebug=0;

function vjDatasourceView(viewer)
{
    //this.prefixHTML="<table width='100%'><tr><td><img border='0' src='img/progress.gif' width='16' /></td><td>Loading data from server</td></tr></table>";
    this.delayRefresh=100;
    this.delayHide=1000;

    vjProgressView.call(this,viewer); // inherit default behaviours of the MenuViewer

//    this.postcompute = "node.Progress = node.Progress+'|'+node.status; ";

    this.action_onDepth = 0;
    this.refreshOnEmpty = true;
    this.baseComposerFunction=this.composerFunction;
    this.cols=[{ name: new RegExp(/.*/),hidden:true},
               { name: 'name', title:'Loading',hidden:false },
               { name: 'stat', type: 'reqstatus',hidden:true },
//               { name: 'status', type: 'reqstatus', hidden:true },
               { name:'progress100', title: 'Status',  type:'qp_progress' ,width:200,hidden:false }
               ];
    this.iconSize=24;


    this.composerFunction=function (viewer, content )
    {
        //var t="Title,Status,Url\n";
//        var t = "";
        var timeNow=new Date();
        var t="name,progress100,stat,reqID,grpID\n";
        var icnt=0,icntNotDone=0;
        for (var dnam in this.dataSourceEngine ) {
            var ds=this.dataSourceEngine[dnam];

            if(content){
                ++icntNotDone;
                t +=  content;
                break;
            }

            if( !gDatasourceViewerDebug ) {
                if(!ds.url)continue;
                if(ds.url.indexOf("http://")!=0 && ds.url.indexOf("qp_")!=0 && ds.url.indexOf("qpbg_")!=0)continue;
                if(!ds.title || ds.title.indexOf("infrastructure:")==0)continue;
    //            if(!isok(ds.state))continue;
                if(ds.url.indexOf("http://help/")==0 ) continue;
                if(!isok(ds.state) ) continue;//st='pending';
            }

            var st=ds.state;
            var timeDiff = 0;

            if(st=='load')
                ++icntNotDone;
            var status;
            var reqID=""; grpID = "";
            if(!ds.RQ){
                if(st=='load') {
                    st="-1";
                    status = 3;
                }

                else if(st=='done') {
                    st="100";
                    status = 5;
                }
                else if(st=='err') {
                    st="100";
                    status = 6;
                }

                //var tmEnd= (ds.timeDone) ?  ds.timeDone : timeNow;
                //var tmStart= (ds.timeLoad) ?  ds.timeLoad : timeNow;
                timeDiff=timeNow.getTime()-(ds.timeDone ? ds.timeDone : ds.timeLoad ).getTime();
                timeDiff/=1000;
                if(st!='-1' && timeDiff>10)continue;
            }
            else{
                st = ds.RQ.progress100;
                timeDiff=timeNow.getTime()/1000-Int(ds.RQ.actTime );
                if( Int(st)<=0 || !Int(st) ) {
                    st = "0";
                }
                ++icntNotDone;
                status = ds.RQ.stat;
                reqID = ds.RQ.reqID;
                grpID = ds.RQ.grpID;
            }


            t+=""+ds.title+","+st+","+status+","+reqID+","+grpID+"\n";//+timeDiff+"\n"; //,'"+protectFields(ds.url)+"'\n";
            ++icnt;
        }

//alert(icnt + "\n"+t)
        if( !(this.current_content && this.current_content==t ) ) {
            this.baseComposerFunction(viewer,t);
            this.current_content = t;
        }

        if(icntNotDone)
            setTimeout("vjObjEvent('refreshContent','"+this.objCls+"')",this.delayRefresh);
        else
            vjDatasourceViewerDiv_hide('dvDatasourceDiv');
            //setTimeout("vjDatasourceViewerDiv_hide('dvDatasourceDiv')",this.delayHide);
    };


    this.refreshContent = function ()
    {
        this.getData(0).reload(null, true);
    };

}



var gvjDatasourceIsShown=false;
function vjDatasourceViewerDiv_hide(layername)
{

    if(!layername)layername="dvDatasourceDiv";
    gObjectSet(layername,"-","-","-","hide","-","-");
    gvjDatasourceIsShown=0;

}
function vjDatasourceViewerDiv_show(x,y, layername)
{
    if(gvjDatasourceIsShown)return ;
    ++gvjDatasourceIsShown;

    this.display = function(x,y,layername){
        if (x == 'center') x = gPgW;// gMoX-(0.4*gMoX);
        if(y=='center')y=4*gPgH/5-30;
        if (x == 'mouse') x = gMoX-200;// gMoX-(0.4*gMoX);
        if(y=='mouse')y=gMoY-20;
        if (x == 'auto') x = parseInt(gPgW/2-200);// gMoX-(0.4*gMoX);
        if(y=='auto')y=10;

        if(x<0)x=gMoX;
        if(y<0)y=gMoY;
        if(!layername)layername="dvDatasourceDiv";
        gObjectSet(layername,x,y,'-','show','-','-',100);
    };

    display(x,y,layername);

    if( vjDS.dsDatasourceViewDatasource )
        vjDS.dsDatasourceViewDatasource.reload(null,true);
    //setTimeout("gObjectSet('"+layername+"',"+x+","+y+",'-','show','-','-',true);",this.delayRefresh);


}


function vjDatasourceViewer_init()
{
     var ds = vjDS.add("infrastructure: Datasource List","dsDatasourceViewDatasource","static://");
     ds.refreshOnEmpty = true;
     vjDV.add("dvDatasourceViewer", 400, 100).frame='none';
     var viewerDatasourceView=new vjDatasourceView({
         data:'dsDatasourceViewDatasource',
         bgColors: ['#efefff', '#ffffff'],
         doNotShowRefreshIcon:true,
         defaultEmptyText: "no data is currently being retrieved",
         maxTxtLen: 80,
        isok: true});
     vjDV.dvDatasourceViewer.add("datasource","table","tab", [viewerDatasourceView]);
     vjDV.dvDatasourceViewer.render();
     vjDV.dvDatasourceViewer.load();

}

//# sourceURL = getBaseUrl() + "/js/vjDataSourceView.js"