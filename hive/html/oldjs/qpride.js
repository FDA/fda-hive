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

var req;
    
    var QPride_statList=new Array ("Any","Waiting","Processing","Running","Suspended","Done","Killed","Program Error","System Error","Unknown");
    var QPride_knownFormatList =  ",.csv,.mat,.txt,.html,.doc,.jpg,.gif,.bmp,.xls,.ppt,.out,.fa,.fasta,.fastq,.cpp,...";
    var QPride_hideFormatList = ",.tbl,-out.csv,...";
    var QPride_trackDataList=",errors,console,form,$summary.txt,...";
    
    var QPride_delayCheck=3000;
    var QPride_delayCheckBG=1000;
    var QPride_Finished=false;
    var QPride_DoneFunc="";
    var QPride_DoneJumpDelay=0;
    var QPride_lastForm="";
    
    var QPride_downloadedForm="";
    var QPride_formVisibleVariables="";
    var QPride_tableClass="HIVE_table";
    var QPride_variableClass="HIVE_variable";
    var QPride_valueClass="HIVE_value";
    var QPride_windowTitle="HIVE";

    var QPride_checkFunc="";
    var QPride_downloadedDataNamesFunc=QPride_downloadedDataNames;
    var QPride_dataNamesList=new Array();
    
    var QPride_dataActionFunctions=new Array();
    QPride_dataActionFunctions.push( {name:".csv", func :"QPride_showTable", icon: "Show Table"} );
    QPride_dataActionFunctions.push( {name:".mat", func :"QPride_showTable", icon: "eye"} );
      
        
        
    function QPride_refreshRequest()
    {
        linkCmd("-qpCheck&raw=1&req="+req+"&svc="+docLocValue("svc"),0,QPride_check);
    }
    function QPride_check(container, txt) 
    {
        var ar=txt.split(",");
        var reqName=ar[1];
        req=parseInt(ar[2]);
        var stat=parseInt(ar[3]);
        var time=parseInt(ar[4]);
        var progress=parseInt(ar[5]);
        var progress100=parseInt(ar[6]);
        var grpCnt=parseInt(ar[7]);
        
        QPride_showReqStatus(reqName, req,stat,time,progress, progress100, grpCnt);
        
        var title="";if(req)title+=req+": ";
        title+=QPride_windowTitle;
        setLocationTitle(title);
        
        
        if(stat>=4)
            QPride_Finished=true;

        QPride_getDataNames();
        QPride_trackDataBlobs();
        QPride_trackReqInfo();
        
        if(QPride_checkFunc) QPride_checkFunc(reqName, req,stat,time,progress, progress100,grpCnt);
        
        
        if(QPride_delayCheck && stat<4) {
            setTimeout("QPride_refreshRequest()", QPride_delayCheck);
        }
        
        
        if(stat>=4 && QPride_DoneFunc && QPride_DoneFunc!=""){
            QPride_delayCheck=0;
            setTimeout(QPride_DoneFunc+"("+req+","+stat+")",QPride_DoneJumpDelay);
        }
    } 

       
    function QPride_showReqStatus(QPride_reqName, QPride_reqID, QPride_stat, QPride_timespan, QPride_progress, QPride_progress100 )
    {
            
        v=gObject("QP_reqName");if(v)v.innerHTML=QPride_reqName;
        v=gObject("QP_reqID");if(v)v.innerHTML=QPride_reqID;
        if(document.forms["QP_htmlform"] && document.forms["QP_htmlform"].elements["req"])document.forms["QP_htmlform"].elements["req"].value=QPride_reqID;
        v=gObject("QP_progress");if(v)v.innerHTML=QPride_progress;
        v=gObject("QP_progress100");if(v)v.innerHTML=QPride_progress100;
        v=gObject("QP_timespan");if(v){
            var tt=QPride_timespan;
            var ts="";
            if(tt>24*3600){ts+=parseInt(tt/(24*3600))+"/"; tt=parseInt(tt%(24*3600));}
            {ts+=(100+parseInt(tt/(3600))+"").substring(1)+":"; tt=parseInt(tt%(3600));}
            {ts+=(100+parseInt(tt/(60))+"").substring(1)+":"; tt=parseInt(tt%(60)).toFixed(2);}
            {ts+=(100+parseInt(tt)+"").substring(1)+"";}
            v.innerHTML=ts;
        }
        v=gObject("QP_stat");if(v){
            var tt="";
            if(QPride_stat>=4 && QPride_DoneFunc)tt+="<a href='javascript:"+QPride_DoneFunc+"("+req+","+QPride_stat+")' >";
            tt+=QPride_statList[QPride_stat];
            if(QPride_stat>=4 && QPride_DoneFunc)tt+="</a>";
            v.innerHTML=tt;
        }
        v=gObject("QP_progress");if(v) {
            var t="";
            t+=QPride_formProgress( 100, "", QPride_progress , QPride_progress100, 0, QPride_stat);
            v.innerHTML=t;
        }
    }    

    function QPride_formProgress( width, hdr, prog , prog100, locreq, stat )
    {
        var t="<table class='QP_table' width='"+width+"%'><tr>";
                t+="<td bgcolor='#4282B1' align='right' width='"+prog100+"%' ><font color=#ffffff>";    
                if(prog100>=50){t+=prog+"("+prog100+"%)"; if(locreq)t+="&nbsp;<small><small>"+locreq+"</small></small>";}
                t+="</font></td><td bgcolor=#ffffff align=left width="+(100-prog100)+"%><font color='#4282B1'>";
                if(prog100<50){t+=prog+"("+prog100+"%)"; if(locreq)t+="&nbsp;<small><small>"+locreq+"</small></small>";}
                t+="</font></td>";
                if(stat<=3)t+="<td><a href='javascript:QPride_killRequest()'>abort</a></td>";
                t+="</tr></table>";
        return t;
    }
    function QPride_killRequest(lreq)
    {
        if(!lreq)lreq=req;
        linkSelf("-qpKillReq&req="+lreq);
        
    }

         
     
    var QPride_gProgSectContent=new Array();
    var QPride_gProgParams=new Array();
    function QPride_backgroundCheckRequest(sparams, txt)
    {
        
        var params=JSON.parse(sparams);
        
        var callback=params.callback;
        if( (callback!="download") &&  (typeof(callback) != "function") )
            callback=eval(callback);
        
        var tbl=new vjTable(txt, 0,vjTable_hasHeader  );
        var RQ=tbl.rows[0];
        alert(txt+"\n---------\n"+jjj(tbl)    );    
        
        
        if(!txt || txt.length==0){callback(0,0); return ;}
        
        

        RQ.stat=parseInt(RQ.stat);
        
        var po=params.progressbar ? gObject(params.progressbar) : null;
        
        if(po && params.progressbar && params.progressbar.length!=0)po.innerHTML=QPride_formProgress( 100, "", RQ.progress ,RQ.progress100 , RQ.reqID ,RQ.stat);
        if(RQ.stat>=5){
            var dataReq=RQ.reqID ;
            if(params.dataReq)dataReq=params.dataReq;

            if(po && params.progressbar && params.progressbar.length!=0)po.innerHTML=QPride_gProgSectContent[params.progressbar];
            
            if(!params.namefetch || params.namefetch.length==0 ) 
                callback(dataReq,RQ.stat, params.xtraparams);
            else {
                
                if(callback=="download")QPride_downloadBlob(params.namefetch+"&req="+dataReq);
                else linkCmd("-qpData&raw=1&default=error:%20"+dataReq+"%20"+params.namefetch+"%20not%20found&req="+dataReq+"&dname="+params.namefetch,params.xtraparams,callback);
            }
        }
        else if(QPride_delayCheckBG) {
            setTimeout("linkCmd('-qpCheck&raw=1&req="+RQ.reqID +"','"+sparams+"',QPride_backgroundCheckRequest)", QPride_delayCheckBG);
        }
    }

    function QPride_backgroundExecution(cmd, progressbar, callback, namefetch, xtraparams, dataReq )
    {    
        var params = { 'progressbar': progressbar, 'callback': callback, 'namefetch' : namefetch , 'xtraparams' : xtraparams , 'dataReq':  dataReq};
        var po=gObject(progressbar);
        if(po && progressbar && progressbar.length!=0)QPride_gProgSectContent[progressbar]=po.innerHTML;
        
        linkCmd(cmd+"&raw=1",JSON.stringify(params),QPride_backgroundCheckRequest);
    }

            

    function QPride_formDataNamesTable(dd, showengin)
    {
        var tshow="",thide="";
        
        for ( var i=0 ; i< dd.length; ++i){ if(dd[i].length<2)break;
            
            var extpos=dd[i].lastIndexOf(".");
            var ext = (extpos!=-1) ? dd[i].substring(extpos) : "";
            
            if(QPride_hideFormatList.indexOf(","+ext+",")!=-1)continue;
            
            var tt="<tr><td style='padding:5px;' width='55%'>"+dd[i]+"</td>";
            tt+="<td style='padding:5px;' width='15%'><a href='javascript:QPride_downloadBlob(\"$"+dd[i]+"\")' >Download</a></td>";
            for( var ifun=0; ifun<QPride_dataActionFunctions.length; ++ifun) {
                var act=QPride_dataActionFunctions[ifun];
                if(act.name!=ext)continue;
                tt+="<td style='padding:5px;' width='15%'><a href='javascript:"+act.func+"(\""+dd[i]+"\")' >"+act.icon+"</td>";
            }
            tt+="</tr>";
            
            if(QPride_knownFormatList.indexOf(","+ext+",")!=-1)tshow+=tt;
            else thide+=tt;
                    
        }
        
        var container= "eng"+Math.random();
        var t="<table width='100%'>";
        t+="<tbody id='"+container+"' class='sectVis'>";
        t+=tshow;
        t+="</tbody>";
        if(showengin && thide.length) {
            t+="<tr><td style='padding:5px;'>"+ovis(container+".qpride","sectHid","more...|less...")+"&nbsp;</td></tr>";
            t+="<tbody id='"+container+".qpride' class='sectHid'>";
            t+=thide;
            t+="</tbody>";
        }    
        t+="</table>";
        
        return t;
    }    
    function QPride_findDataName( dname )
    {
        for( var i=0; i<QPride_dataNamesList.length; ++i) { 
            if(QPride_dataNamesList[i]==dname)return i+1;
        }
        return 0;
    }
    function QPride_downloadedDataNames(container, txt)
    {
        QPride_dataNamesList=txt.split(/[\s,]+/);
        var o=gObject(container);
        if(o)o.innerHTML=QPride_formDataNamesTable(QPride_dataNamesList, false);    
    }
    
    function QPride_getDataNames(container, lreq)
    {
        if(!container)container="QP_data";
    
        linkCmd("-qpDataNames&raw=1&req="+(lreq ? lreq : req),container,QPride_downloadedDataNamesFunc);
    }
    
    function QPride_trackDataBlobs(dsections, szlimit)
    {
        if(!dsections)dsections=QPride_trackDataList;
        var sds=dsections.split(",");
        for( var i=0; i<sds.length; ++i) {
            var isgrp=(sds[i].substring(0,1)=="$") ? true : false;
            var dnam=((sds[i].substring(0,1)=="$") || (sds[i].substring(0,1)=="#") ) ? sds[i].substring(1) : sds[i];
            var o=gObject("QP_"+dnam);
            if(o) {
                if(dnam=="form")linkCmd("-form&raw=1&req="+req,sds[i],QPride_downloadedFormContent);
                else linkCmd("-qpData&dname="+dnam+"&dsize="+(szlimit ? szlimit : 2048)+"&req="+req+(isgrp ? "&grp=1" :""),sds[i],QPride_downloadedDataSection);    
            }
        }
    }
    
    function QPride_downloadedFormContent(container, txt)
    {
        QPride_lastForm=txt;
        var o=gObject("QP_"+container);if(!o)return;
        if(!txt || txt.length==0) {o.className="sectHid";return;}
        
        var ar=QPride_lastForm.split("\n");
        var t="<table width='100%' class='"+QPride_tableClass+"' >";
        for ( var ir=0; ir<ar.length - 1; ++ir ){
            var cls=ar[ir].split("=");
            var par=cls[0].replace(" ", "");
            var ifnd=-1;

            if(QPride_formVisibleVariables && QPride_formVisibleVariables.length){
                for ( ifnd=0; ifnd<QPride_formVisibleVariables.length; ++ifnd) {
                    if(par==QPride_formVisibleVariables[ifnd][1])break;
                }
                if(ifnd>=QPride_formVisibleVariables.length) continue;
                par=QPride_formVisibleVariables[ifnd][0];
            }
            
            t+="<tr><td width='20%' class='"+QPride_variableClass+"'>"+par+"</td><td width=10>=</td>";
            t+="<td class='"+QPride_valueClass+"'  >";
            for ( var ic=1; ic<cls.length; ++ic) { 
                if(ifnd>=0 && QPride_formVisibleVariables[ifnd].length>2  )
                    t+=QPride_formVisibleVariables[ifnd][2][parseInt(cls[1])];
                else t+=cls[1];
            }
            t+="</td></tr>";
        }
        t+="</table>";
        
        o.innerHTML=t;
        if(QPride_downloadedForm && QPride_downloadedForm!=""){
            eval(QPride_downloadedForm);
        }
    }

    function QPride_downloadedDataSection(container, txt)
    {
        var t="";
        var istbl=0;
        if(container.substring(0,1)=="$" || container.substring(0,1)=="#" ){
            t+="<table width='100%'><tr><td>";
            istbl=1;
            container=container.substring(1);
        }
        var o=gObject("QP_"+container);
        if(!txt || txt.length==0) o.className="sectHid";
        t+="<pre>"+txt+"</pre></td></tr>";
        if(istbl)t+="</table>";
        o.innerHTML=t;
    }

    function QPride_downloadBlob(blbname,filename, container, callback )
    {
        var url=0;
        if(filename){
            url = "-qpFile&raw=1&file=" + filename;
        }
        else if(blbname) { 
            var isgrp=0;
            if(blbname.substring(0,1)=="$"){blbname=blbname.substring(1);isgrp=1;}
            url="-qpData&dname="+blbname;
            if(blbname.indexOf("req=")==-1)url+="&req="+req;
            if(isgrp)url+="&grp=1";
        }
        
        if(!container){
            linkSelf(url);
            return ;
        }
    }
    

    
    function QPride_downloadedRequestInfo(container, txt)
    {
        var o=gObject(container); if(!o)return;
        if(!txt || txt.length==0){ o.className="sectHid"; return ;}
        
        var ar=txt.split("\n");
        var t="<table width='100%'>";
        for ( var ir=0; ir<ar.length; ++ir ){
            var cls=ar[ir].split("
            
            t+="<tr>";
            t+="<td width='10%'><pre>"+cls[0]+"</pre></td>";
            t+="<td width='45%'><pre>"+cls[2]+"</pre></td>";
            t+="<td width='45%'><pre>";
            for ( var ic=3; ic<cls.length; ++ic)t+=cls[ic];
            t+="</pre></td>";
            t+="</tr>";
        }
        t+="</table>";
        
        o.innerHTML=t;
    }
    function QPride_trackReqInfo()
    {
        var container="QP_reqInfo";
        var o=gObject(container); if(!o)return;
        linkCmd("-qpReqInfo&req="+req,container,QPride_downloadedRequestInfo);    
    }
    
    
    function QPride_showTable(dname)
    {
        var url="tbl.cgi?cmd=tblqry&req="+req;
        cookieSet("QPride_selectedTable",dname);
        window.open(url,"Show Graph");
    }
        


function QPride_reqaction(isgrp, reqid,act)
{
    linkCmd("req="+req+"&cmd=-"+(isgrp ? "grp" : "req")+"SetAction&action="+act);
}
        
        
function QPride_formValue(varnm, qpform)
{
    if(!qpform)qpform=QPride_lastForm;
    
    var ppos=qpform.indexOf(varnm+" = "); if(ppos==-1)return "";
    var retv=qpform.substring(ppos+varnm.length+3);
    var epos=retv.indexOf("\n");if(epos==-1)return retv;
    return retv.substring(0,epos);
}
        

        
        var QPride_svcList;
        var QPride_svcCols=25;
        var QPride_svcRows=0;
        var QPride_categList = new Array();
        
        var QPride_svcParamHeaders= new Array ( "svcID","name","title","isUp","svcType","knockoutSec","maxJobs","nice","sleepTime","maxLoops","parallelJobs","delayLaunchSec","politeExitTimeoutSec","maxTrials","restartSec","priority","cleanUpDays","runInMT","noGrabDisconnect","cmdLine","hosts","emails","categories","maxmemSoft","maxmemHard");
        
        var QPride_cfgList;
        var QPride_cfgCols=2;
        var QPride_cfgRows=0;
        
        

