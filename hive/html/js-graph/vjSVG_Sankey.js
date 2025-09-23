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

function vjSVG_Sankey(serie,source)
{

    vjSVG_Plot.call(this, source);
    this.fillN='#D0D0D0 '
        ,this.strokeN='#D0D0D0 '
        ,this.fillOpacityN=1
        ,this.strokeWidthN=0.5
        ,this.strokeOpacityN=0.5
    ;
    this.fillH='#A8A8A8'
        ,this.strokeH='#A8A8A8'
        ,this.fillOpacityH=1
        ,this.strokeWidthH=0.7
        ,this.strokeOpacityH=0.7
    ;
    this.fillS='#566e7e'
        ,this.strokeS='#566e7e'
        ,this.fillOpacityS=1
        ,this.strokeWidthS=1
        ,this.strokeOpacityS=1
    ;

    this.state_Highlighted= {fill:this.fillH , 'fill-opacity' :this.fillOpacityH , stroke:this.strokeH, 'stroke-opacity':this.strokeOpacityH, 'stroke-width':this.strokeWidthH};
    this.state_Neutral = {fill:this.fillN,'fill-opacity':this.fillOpacityN, stroke:this.strokeN, 'stroke-opacity':this.strokeOpacityN, 'stroke-width':this.strokeWidthN};
    this.state_Selected = {fill:this.fillS,'fill-opacity':this.fillOpacityS, stroke:this.strokeS, 'stroke-opacity':this.strokeOpacityS, 'stroke-width':this.strokeWidthS};

    this.linkMouseOver = function(ir,svgObj,evt){
        var obj=this.getObjbySVG( svgObj );
        
        svgObj.style["stroke-width"]=4;
        svgObj.style["stroke-opacity"]=0.8;

        var clone=this.getSVGbySankeyNode( this.getSankeyNodebyObj( obj.cloneFrom ) );

        if(clone){
            this.cloneMouseOver(null,clone,evt);
        }

        clone=this.getSVGbySankeyNode( this.getSankeyNodebyObj( obj.cloneTo ) );
        if(clone){
            this.cloneMouseOver(null,clone,evt);
        }
    };

    this.linkMouseMove = function(ir,svgObj,evt){
        var obj=this.getObjbySVG(svgObj);

        if ( this.ownerSVG && this.ownerSVG.getElementById("toolTipBox")!=null) {
            var toolt = this.ownerSVG.getElementById("toolTipBox");
            toolt.parentElement.removeChild(toolt);
        }

        var cl_from = this.getSankeyNodebyObj(obj.cloneFrom);
        var cl_to = this.getSankeyNodebyObj(obj.cloneTo);

        var title="Event: "+obj.event+ " (";

        if(obj.event=="Bifurcation"){
            title+= cl_from._branchStart;
        }
        else{
            title+= cl_from._branchEnd;
        }
        title +=")";

        var text="Between "+ cl_from._branchID+ " and "+cl_to._branchID + "\n" ;
        text+=this.getBranchStatText(obj.event=="Bifurcation"?cl_from:cl_to);
        text+=cl_from._support + " positions supporting the clone\n";

        this.computeToolTip({
            x : evt.offsetX + 10,
            y : evt.offsetY - 10
        }, text, svgObj.parentElement.id, {
            title : {
                content : title
            },
            ellipsizeWidth:80,
            backgroundColor : "white",
            fontweight : "normal"
        }, {
            mouseout:svgObj.onmouseout
        });
    };

    this.linkMouseOut = function(ir,svgObj,evt){
        svgObj.style["stroke-width"]=2;
        svgObj.style["stroke-opacity"]=0.2;

        if (this.ownerSVG.getElementById("toolTipBox")!=null) {
            var toolt = this.ownerSVG.getElementById("toolTipBox");
            toolt.parentElement.removeChild(toolt);
        }

        var obj=this.getObjbySVG(svgObj);

        var clone=this.getSVGbySankeyNode( this.getSankeyNodebyObj( obj.cloneFrom ) );
        if(clone){
            this.cloneMouseOut(null,clone,evt);
        }

        clone=this.getSVGbySankeyNode( this.getSankeyNodebyObj( obj.cloneTo ) );
        if(clone){
            this.cloneMouseOut(null,clone,evt);
        }

    };

    this.linkMouseClick = function(ir,svgObj,evt){
        var obj = this.getObjbySVG(svgObj);

        var clone=this.getSVGbySankeyNode( this.getSankeyNodebyObj( obj.cloneFrom ) );
        if(clone){
            this.cloneMouseClick(null,clone,evt);
        }

        clone=this.getSVGbySankeyNode( this.getSankeyNodebyObj( obj.cloneTo ) );
        if(clone){
            this.cloneMouseClick(null,clone,evt);
        }
    };
    this.getStateStyles = function (obj, state)
    {
        var t_objState = 0;
        switch (state) {
        case"normal":{
                t_objState = obj["state-normal"];
            break;
        }
        case"highlighted":{
            t_objState = obj["state-highlighted"];
            break;
        }
        case"selected":{
            t_objState = obj["state-selected"];
            break;
        }
        default:{
                t_objState = obj["state-normal"];
            break;
        }
        }
        return t_objState;
    };

    this.setHighlightedAttributes = function (svgObj,state, cnt) {
        var obj=this.getObjbySVG(svgObj);

        if(obj.brush.fill)shFColor=obj.brush.fill;
        if(obj.pen.stroke)shSColor=shadeColor(obj.pen.stroke,0.7,"7f7f7f");
        var fill={value:"",opacity:""}, stroke={value:"",width:"",opacity:""};

        var t_objState = this.getStateStyles(obj,state);

        if(t_objState){
            objState = t_objState;
        }
        fill={value:objState.fill,opacity:objState['fill-opacity']};
        stroke={value:objState['stroke'],width:objState['stroke-width'],opacity:objState['stroke-opacity']};

        this.setSVGCloneState (svgObj,state);

        svgObj.style["stroke"]=stroke.value;
        svgObj.style["stroke-width"]=stroke.width;
        svgObj.style["stroke-opacity"]=stroke.opacity;
        svgObj.style["fill"]=fill.value;
        svgObj.style["fill-opacity"]=fill.opacity;
    };
    this.highlightSVGClone = function(svgEl, state) {
        if( !this.isSVGsimilarity(svgEl) ) {
            if( this.isMosaic() ){
                var obj=this.getObjbySVG(svgEl),grp = 0, svgEl_c = 0;
                for( var i = 0 ; i < obj.children.length ; ++i ) {
                    grp = obj.children[i];
                    var cnt = {v:0};
                    for( var c = 0 ; c < grp.children.length ; ++c ) {
                        svgEl_c = this.getSVGbyObj(grp.children[c]);
                        this.setHighlightedAttributes(svgEl_c,state,cnt);
                    }
                }
            }
            else{
                this.setHighlightedAttributes(svgEl,state);
            }
        }
        else{
            if( this.isSVGsimilarity(svgEl) ) {
                var cnt = {v:0};
                var obj=this.getObjbySVG(svgEl).parent, svgEl_c = 0;
                for( var c = 0 ; c < obj.children.length ; ++c ) {
                    svgEl_c = this.getSVGbyObj(obj.children[c]);
                    this.setHighlightedAttributes(svgEl_c,state,cnt);
                }
            }
        }
    };

    this.cloneMouseOver = function(ir,svgObj,evt) {
        var obj= this.getObjbySVG(svgObj);

        var objcl=this.getSankeyNodebyObj(obj);


        if(this.onCloneOver) {
            if( funcLink(this.onCloneOver,objcl,ir) )
                return 0;
        }

        this.highlightSVGClone(svgObj,"highlighted");
        return 0;
    };

    this.cloneMouseOut = function (ir,svgObj,evt) {
        var eventSkipList=new Array();
        if(gObject("HighlightGroup"))eventSkipList=traverseChildren(gObject("HighlightGroup"));
        if(gObject("toolTipBox"))eventSkipList=eventSkipList.concat(traverseChildren(gObject("toolTipBox")));
        if(!eventOrigins(svgObj,evt,eventSkipList))return ;

        var obj= this.getObjbySVG(svgObj);

        if (this.ownerSVG.getElementById("toolTipBox")!=null) {
            var toolt = document.getElementById("toolTipBox");
            toolt.parentElement.removeChild(toolt);
        }
        if(this.ownerSVG.getElementById("HighlightGroup")) {
            var hgroup = document.getElementById("HighlightGroup");
            hgroup.parentElement.removeChild(hgroup);
        }

        var objcl=this.getSankeyNodebyObj(obj);
        if(this.onCloneOut) {
            if(funcLink(this.onCloneOut,objcl,ir)){
                return 0;
            }
        }

        if( this.isMosaic() ){
            obj = this.getObjbySankeyNode(objcl);
        }

        this.highlightSVGClone(svgObj,obj.selected?"selected":"normal");

        return 0;
    };

    this.cloneMouseClick = function (ir,svgObj,evt) {
        var obj=this.getObjbySVG( svgObj );

        if( this.isMosaic() ){
            if( this.isObjsimilarity(obj) ) obj = obj.getParent();
            svgObj = this.getSVGbyObj(obj);
        }

        function compareX(cord,find){return cord.x-find;};
        var ir=arraybinarySearch(obj.coordinates,evt.nX,compareX,true);


        var objcl=this.getSankeyNodebyObj(obj);
        if(this.onCloneClick) {
            if(funcLink(this.onCloneClick,objcl,ir)) {
                return 0;
            }
        }
        obj.selected = obj.selected?0:1;
        this.highlightSVGClone(svgObj,obj.selected?"selected":"normal");
        return 0;
    };

    this.cloneMouseMove = function (ir,svgObj,evt) {
        if (this.ownerSVG.getElementById("toolTipBox")!=null) {
            var toolt = document.getElementById("toolTipBox");
            toolt.parentElement.removeChild(toolt);
        }
        var obj=this.getObjbySVG( svgObj );
        function compareX(cord,find){return cord.x-find;};
        var ir=arraybinarySearch(obj.coordinates,evt.nX,compareX,true);


        var objcl=this.getSankeyNodebyObj(obj);
        if(this.onCloneMove) {
            funcLink(this.onCloneMove,objcl,ir);
        }


        if(svgObj.onmouseout)handler=svgObj.onmouseout;
        var coords = 0, colors = new Array(), isSimilariyGap = false;
        if( this.isMosaic() ){
            colors = this.colors;
            coords = this.getSimSVGDictbySimPos(obj, ir) ;
            if(!coords) isSimilariyGap = true;
        }
        if( !this.isMosaic() || isSimilariyGap){
            coords = [];
            parentID=svgObj.id;
            colors[0]=obj.brush.fill;
            coords[0] = {iSim:0, x:svgObj.pathSegList.getItem(ir).x,y:svgObj.pathSegList.getItem(ir).y};
        }
        this.showCirclePont(this.ownerSVG.id,coords,colors,null,{mouseout:svgObj.onmouseout,click:svgObj.onclick});

        var objcl = this.getSankeyNodebyObj( obj );
        var pos_obj = this.getPositionbyObj( obj, ir );
        if( !pos_obj ) return 0;
        var title = objcl._branchID + " (" + pos_obj.coord + ","
                + pos_obj.base + "," + pos_obj.cov + ")";
        var text='';
        var legend_colors = new Array();
        if (this.isMosaic() && !isSimilariyGap) {
            for(var it = 0, is = 0  ; it < coords.length ; ++it ){
                is = parseInt(coords[it].iSim);
                legend_colors.push(colors[is]);
                if(text.length)text+="\n";
                text+=''+this.References[""+(is+1)]+": "+(pos_obj.sim[is]*100).toFixed(2)+"%";
            }
            if(text.length)
                this.computeToolTip({x:evt.offsetX+10,y:evt.offsetY-10},text,this.ownerSVG.id,{title:{content:title},colors:legend_colors,backgroundColor:"white",fontweight:"normal"},{mouseout:handler});
        }
        else {
            if(objcl._branchStart != 'none'){
                text+="Bifurcation: (From:"+objcl._bifurcateID + ", At:" + objcl._branchStart+ ")\n";
                text += this.getBranchStatText(objcl);
            }
            text+="Range:["+objcl._trueStart +","+objcl._trueEnd + "]\n";
            text+="Merged To:"+objcl._mergeID +"\n";
            if(text.length)
                this.computeToolTip({x:evt.offsetX+10,y:evt.offsetY-10},text,this.ownerSVG.id,{title:{content:title},backgroundColor:"white",fontweight:"normal"},{mouseout:handler});
        }
        if(!text.length){
            text="Position not covered";
            this.computeToolTip({x:evt.offsetX+10,y:evt.offsetY-10},text,this.ownerSVG.id,{title:{content:title},backgroundColor:"white",fontweight:"normal"},{mouseout:handler});
        }
    };
    
    this.getBranchStatText = function(obj) {
        var res = "";
        if( obj._branchStats ) {
            if( !(obj._branchStats.pvalue<0 || obj._branchStats.pvalue>1) ) {
                res+="P value: " + (obj._branchStats.pvalue?obj._branchStats.pvalue:"<1E-10")+"\n";
            }
            if( !(obj._branchStats.bayes<0 || obj._branchStats.bayes>1) ) {
                res+="Inference: " + obj._branchStats.bayes+"\n";
            }
        }
        return res;
    };

    this.showCirclePont = function(parentid,coordinates,colors,radius,handlers){
        if(document.getElementById("HighlightGroup")) {
            var hgroup = document.getElementById("HighlightGroup"); 
            hgroup.parentElement.removeChild(hgroup);
        }
        if(!isok(radius)) radius=6;
        var svgChd = document.createElementNS("http:
        svgChd.setAttribute("id","HighlightGroup");

        for(var ih = coordinates.length -1 ; ih >=0 ; --ih ){
            var HighlightCircle = document.createElementNS("http:
            HighlightCircle.setAttribute("cx",coordinates[ih].x);
            HighlightCircle.setAttribute("cy",coordinates[ih].y);
            HighlightCircle.setAttribute("r",radius);
            HighlightCircle.style["stroke-width"]=radius/1.5;
            HighlightCircle.style["stroke"]=colors[coordinates[ih].iSim];
            HighlightCircle.style["fill"]="white";
            if(isok(handlers)){
                for(ii in handlers){
                    HighlightCircle.addEventListener(ii,handlers[ii]);
                }
            }
            svgChd.appendChild(HighlightCircle);
        }
        document.getElementById(parentid).appendChild(svgChd);
        return svgChd;
    };


    this.createCloneConnections=function(cloneA1,cloneB1,state,isLink){
        var cordX1=0,cordY1=0,cordX2=0,cordY2=0,indY=0;
        var cloneA = this.getObjbySankeyNode(cloneA1);
        if(!cloneA)return ;
        var cloneB = this.getObjbySankeyNode(cloneB1);
        if(!cloneB)return ;
        if(cloneA==cloneB)return;

        var lengthA=cloneA.coordinates.length;
        var lengthB=cloneB.coordinates.length;
        var lengthBy=lengthB;
        
        
        var cloneAy=cloneA,cloneBy=cloneB;
        var cloneAy1=cloneA1,cloneBy1=cloneB1;
        if(cloneA.coordinates[0].y>cloneB.coordinates[0].y){
            cloneAy=cloneB;
            cloneBy=cloneA;
            lengthAy=lengthB;
            lengthBy=lengthA;
            
            cloneAy1=cloneB1;
            cloneBy1=cloneA1;
        }
        if(!state){
            if( cloneAy1.parent == cloneBy1 ) {
                cordX1=cloneAy.coordinates[0].x;
                cordY1=cloneAy.coordinates[0].y;
                cordX2=cordX1;
                indY=arraybinarySearch(cloneBy.coordinates,cordX2,function(a,b){return a.x-b;},true);
                cordY2=cloneBy.coordinates[lengthBy - 1 - indY].y;
            }
            else if ( cloneBy1.parent == cloneAy1 ) {
                cordX1=cloneBy.coordinates[0].x;
                cordY1=cloneBy.coordinates[lengthBy-1].y;
                cordX2=cordX1;
                indY=arraybinarySearch(cloneAy.coordinates,cordX2,function(a,b){return a.x-b;},true);
                cordY2=cloneAy.coordinates[indY].y;
            }

        }
        else{ 
            if( cloneAy1.merge == cloneBy1 ) {
                cordX1 = cloneAy1._branchEnd;
                indY=arraybinarySearch(cloneBy1._positions,cordX1,function(a,b){return a.coord-b;},true);
                cordY1=cloneBy.coordinates[lengthBy-1-indY].y;
                cordX1=cloneBy.coordinates[lengthBy-1-indY].x;

                cordX2 = cloneAy1._trueEnd;
                indY=arraybinarySearch(cloneAy1._positions,cordX2,function(a,b){return a.coord-b;},true);
                cordY2=cloneAy.coordinates[indY].y;
                cordX2=cloneAy.coordinates[indY].x;
            }
            else if ( cloneBy1.merge == cloneAy1 ) {
                cordX1 = cloneBy1._branchEnd;
                indY=arraybinarySearch(cloneAy1._positions,cordX1,function(a,b){return a.coord-b;},true);
                cordY1=cloneAy.coordinates[indY].y;
                cordX1=cloneAy.coordinates[indY].x;

                cordX2 = cloneBy1._trueEnd;
                indY=arraybinarySearch(cloneBy1._positions,cordX2,function(a,b){return a.coord-b;},true);
                cordY2=cloneBy.coordinates[lengthBy-1-indY].y;
                cordX2=cloneBy.coordinates[lengthBy-1-indY].x;
            }
        }

        var linkSource=new Object();
        linkSource.coordinates = [{x:cordX1,y:cordY1},{x:cordX2,y:cordY2}];
        linkSource.event=state?'Merge':'Bifurcation';
        linkSource.cloneFrom=cloneA;
        linkSource.cloneTo=cloneB;


        linkSource.lineCmd='L';
        
        var link= new vjSVG_trajectory(linkSource);
        var link_color = "grey";
        if(isLink)
            link_color = state?"red":"green";
        link.pen={"stroke-opacity":0.2,"stroke-width":2,"stroke":link_color,"stroke-linejoin":"round"};
        link.svgID=link.objID;
        link.handler = {
                'onmouseover' : "function:vjObjFunc('linkMouseOver','" + this.objID + "')",
                'onmouseout': "function:vjObjFunc('linkMouseOut','" + this.objID + "')",
                "onclick":"function:vjObjFunc('linkMouseClick','" + this.objID + "')",
                "onmousemove" : "function:vjObjFunc('linkMouseMove','"+this.objID+"')"
        };
        return link;
    };

    this.createClone = function( cls, node){

        if(!node._positions || !node._positions.length) {
            return 0;
        }

        var svgNode=new Object();
        svgNode.coordinates = 0;svgNode.children = new Array();

        var coordArr = new Array(2*node._positions.length) , curPos = 0 , curCov = 0, clMaxY = node._weight,
               clShiftY = node.y, revx = 0, revy = 0, y =0 ;
        for(var ip=0;ip<node._positions.length;++ip){
            curPos = node._positions[ip].coord;
            curCov=node._positions[ip].cov;

            coordArr[ip]= {x:(curPos-this.min.x)*this.scaleCrd.x, y: curCov};

            revx=coordArr[ip].x;
            if(this.mode=="absolute"){
                y=(coordArr[ip].y+clShiftY)*this.scaleCrd.y;
                revy=serie.shift;
            }
            if(this.mode=="relative"){
                y=(clMaxY*this.center+(1-this.center)*coordArr[ip].y+clShiftY)*this.scaleCrd.y;
                revy=(clMaxY*this.center-(this.center*coordArr[ip].y)+clShiftY)*this.scaleCrd.y;
            }
            if(this.mode=="placed"){
                y=(coordArr[ip].y+clShiftY)*this.scaleCrd.y;
                revy=(coordArr[ip].y-coordArr[ip].w+clShiftY)*this.scaleCrd.y;
            }
            coordArr[this.getRevI(coordArr,ip)]={x:revx,y:revy};
            coordArr[ip].y=y;
            if (!coordArr[ip].z) coordArr[ip].z=0;
            coordArr[ip].z=(coordArr[ip].z=this.min.z)*this.scaleCrd.z;
        }

        svgNode.coordinates = coordArr;
        svgNode.closed=true;
        svgNode.lineCmd='L';
        if(!svgNode.coordinates.length)return 0;
        var myTraj=new vjSVG_trajectory ( svgNode) ;
        myTraj["state-normal"]={fill:this.isMosaic()?this.fillN:this.colors[node.tablenode.irow] , 'fill-opacity' :this.fillOpacityN , stroke:this.strokeN, 'stroke-opacity':this.strokeOpacityN, 'stroke-width':this.strokeWidthN};
        myTraj["state-highlighted"]={fill:this.isMosaic()?this.fillH:lightenColor(this.colors[node.tablenode.irow]) , 'fill-opacity' :this.fillOpacityH , stroke:this.strokeH, 'stroke-opacity':this.strokeOpacityH, 'stroke-width':this.strokeWidthH};
        myTraj["state-selected"]={fill:this.isMosaic()?this.fillS:oppositeColor(this.colors[node.tablenode.irow]) , 'fill-opacity' :this.fillOpacityS , stroke:this.strokeS, 'stroke-opacity':this.strokeOpacityS, 'stroke-width':this.strokeWidthS};
        myTraj.brush={fill:this.isMosaic()?this.fillN:this.colors[node.tablenode.irow],"fill-opacity":this.fillOpacityN};
        myTraj.svgID=myTraj.objID;
        myTraj.getParent = function(){
            return this.parent;
        };
        node._svgID = myTraj.objID;
        myTraj._branchID = node._branchID;
            myTraj.handler = {
                    'onmouseover' : "function:vjObjFunc('cloneMouseOver','" + this.objID + "')",
                    'onmouseout': "function:vjObjFunc('cloneMouseOut','" + this.objID + "')",
                    "onclick":"function:vjObjFunc('cloneMouseClick','" + this.objID + "')",
                    "onmousemove" : "function:vjObjFunc('cloneMouseMove','"+this.objID+"')"
            };
        myTraj.pen={"stroke-opacity":this.strokeOpacityN};


        if( this.isMosaic() ){
            myTraj.brush['fill-opacity']=1;
            myTraj["state-normal"]={fill:this.fillN , 'fill-opacity' : myTraj.brush['fill-opacity'] , stroke:this.strokeN, 'stroke-opacity':this.strokeOpacityN, 'stroke-width':this.strokeWidthN};
            myTraj["state-highlighted"]={fill:this.fillH , 'fill-opacity' :myTraj.brush['fill-opacity'] , stroke:this.strokeH, 'stroke-opacity':this.strokeOpacityH, 'stroke-width':this.strokeWidthH};
            myTraj["state-selected"]={fill: this.fillS , 'fill-opacity' :myTraj.brush['fill-opacity'] , stroke: this.strokeS, 'stroke-opacity':this.strokeOpacityS, 'stroke-width':this.strokeWidthS};

            var stack_arr = cpyObj(svgNode,myTraj.coordinates.slice(myTraj.coordinates.length/2).reverse(),{cpyObjParams:{deep:1}});

            for ( var iSim = 0 ; iSim <= this.sankey.similaritiesMaxIndex && node._similarities ; ++iSim) {
                if( node._similarities[iSim] ) {
                    this.createSimilarity(stack_arr, node, parseInt(iSim), myTraj);
                }
            }
        }

        return myTraj;
    };

    this.fillReverseArray = function(array, shiftArr, includeLeftZero, includeRightZero) {
        if(!array.length) return 0;
        var length = array.length;
        array.length *= 2;
        var rev_i = 0 ;
        for (var i = 0 ; i < length ; ++i ) {
            rev_i = this.getRevI(array,i);
            array[rev_i] = { x : array[i].x , y : shiftArr[i]};
        }
        if(includeRightZero!==undefined) {
            array.splice(length,0,includeRightZero,includeRightZero);
        }
        if(includeLeftZero!==undefined) {
            array.splice(0,0,includeLeftZero);
            array.push(includeLeftZero);
        }
    };

    this.createSVGsimilarity = function (simObjArray, svgTraj, color) {

        if(!simObjArray.coordinates.length)return 0;
        simObjArray.closed=true;
        simObjArray.lineCmd='L';
        var myTraj=new vjSVG_trajectory ( simObjArray) ;
        myTraj.brush={fill:color,"fill-opacity":this.fillOpacityN};
        myTraj.svgID=myTraj.objID;
        myTraj._branchID = svgTraj._branchID;;
        myTraj._iSim = svgTraj._iSim;

        myTraj.handler = {
                'onmouseover' : "function:vjObjFunc('cloneMouseOver','" + this.objID + "');",
                'onmouseout': "function:vjObjFunc('cloneMouseOut','" + this.objID + "');",
                "onclick":"function:vjObjFunc('cloneMouseClick','" + this.objID + "');",
                "onmousemove" : "function:vjObjFunc('cloneMouseMove','"+this.objID+"');"
        };

        myTraj.pen={"stroke-opacity":this.strokeOpacityN,"stroke" : color,"stroke-width" : this.strokeWidthN,};

        myTraj["state-normal"]={fill:color , 'fill-opacity' :this.fillOpacityN , stroke:color, 'stroke-opacity':this.strokeOpacityN, 'stroke-width':this.strokeWidthN};
        myTraj["state-highlighted"]={fill:lightenColor(color) , 'fill-opacity' :this.fillOpacityH , stroke:color, 'stroke-opacity':this.strokeOpacityH, 'stroke-width':this.strokeWidthH};
        myTraj["state-selected"]={fill:oppositeColor(color) , 'fill-opacity' :this.fillOpacityS , stroke:oppositeColor(color), 'stroke-opacity':this.strokeOpacityS, 'stroke-width':this.strokeWidthS};

        myTraj.parent = svgTraj;
        myTraj.getParent = function(){
            return this.parent.parent;
        };
        svgTraj.children.push(myTraj);
        return myTraj;
    };

    this.createSimilarity = function (svgCoordinate, node, iSim, svgTraj ) {

        var svgGrp = new vjSVG_group({_branchID:node._branchID, _iSim:iSim});
        svgGrp.svgID = svgGrp.objID;
        svgGrp.handler = {};
        var color = this.colors[iSim];
        var similArray = node._similarities[iSim];
        var simObjArray = new Object();
        simObjArray.children = new Array();
        if(!svgTraj.map) svgTraj.map = new Array() ;

        var curCoordArr= new Array();
        var shifts = new Array();
        var cur_ind , pos, sim, prev_pos = similArray.slice(-1)[0];
        for ( var ip = 0; ip < similArray.length; ++ip) {
            pos = similArray[ip].pos;
            sim = similArray[ip].sim;

            if( ip && pos - prev_pos > 1 ) {
                this.fillReverseArray(curCoordArr, shifts);
                simObjArray.coordinates = curCoordArr;
                var c_traj = this.createSVGsimilarity (simObjArray, svgGrp, color);
                c_traj._start = prev_pos - shifts.length + 1;
                for( var i = prev_pos - shifts.length +1 ; i <= prev_pos ; ++i ) {
                    if( !svgTraj.map[i] ) svgTraj.map[i] = new Object();
                    svgTraj.map[i][iSim] = c_traj ;
                }

                shifts = new Array();
                curCoordArr = new Array();
                simObjArray.coordinates = new Array();
            }

            cur_ind = curCoordArr.push ( {x:svgCoordinate[pos].x} ) - 1;

            var stackedY = svgCoordinate[pos].y;

            curCoordArr[cur_ind].y=stackedY+(sim?parseFloat(sim):0)*this.getPositionCoverage(svgTraj.coordinates,pos);
            shifts.push(stackedY);

            svgCoordinate[pos].y += curCoordArr[cur_ind].y - stackedY;

            prev_pos = pos ;
        }
        this.fillReverseArray(curCoordArr, shifts);

        simObjArray.coordinates = curCoordArr;
        c_traj = this.createSVGsimilarity (simObjArray, svgGrp, color);
        c_traj._start = prev_pos - shifts.length + 1;
        for( var i = prev_pos - shifts.length + 1 ; i <= prev_pos ; ++i ) {
            if( !svgTraj.map[i] ) svgTraj.map[i] = new Object();
            svgTraj.map[i][iSim] = c_traj ;
        }

        svgGrp.parent = svgTraj;
        svgTraj.children.push(svgGrp);

        return svgTraj;
    };


    this.bezierEndCoordinate =function (cordArray,shiftCord,end){
        var halfLength=Math.floor(cordArray.length/2),
         length=cordArray.length;
        var fI=end?halfLength-1:0,
         rI=end?halfLength:length-1,
         cfI=end?fI-1:fI+1,
         crI=end?rI+1:rI-1;
        var step=Math.abs(cordArray[fI].x-cordArray[cfI].x),
         distF=Math.abs(cordArray[cfI].y-shiftCord.y),
         distR=Math.abs(cordArray[crI].y-shiftCord.y);
        var dist=distF-distR;
        var extrF1={x:0,y:0,z:0};
         extrF2={x:0,y:0,z:0},
         extrF3={x:0,y:0,z:0},
         extrR1={x:0,y:0,z:0},
         extrR2={x:0,y:0,z:0},
         extrR3={x:0,y:0,z:0};

        extrF1.x=cordArray[fI].x-(step/2*(end?0:1));
        extrF1.y=cordArray[cfI].y;
        extrF2.x=cordArray[fI].x-(step/2*(end?0:1));
        extrF2.y=shiftCord.y+distF/2;
        extrF3.x=cordArray[fI].x-(step/2*(end?0:1));
        extrF3.y=shiftCord.y;

        extrR1.x=cordArray[rI].x+(step/2*(end?-1:1));
        extrR1.y=cordArray[crI].y;
        extrR2.x=cordArray[rI].x+(step/2*(end?-1:1));
        extrR2.y=shiftCord.y+distR/2;
        extrR3.x=cordArray[rI].x+(step/2*(end?-1:1));
        extrR3.y=shiftCord.y-dist;


        cordArray[fI].y=shiftCord.y;
        cordArray[fI].x=shiftCord.x+(end?step:-step);

        cordArray[rI].x=shiftCord.x+(end?step:-step);
        cordArray[rI].y=shiftCord.y-dist;
        if(end){
            cordArray.splice(rI+1,0,extrR3,extrR2,extrR1);
            cordArray.splice(fI,0,extrF1,extrF2,extrF3);
        }
        else{
            cordArray.splice(rI,0,cordArray[rI-1],extrR1,extrR2,extrR3);
            cordArray.splice(fI+1,0,extrF3,extrF2,extrF1);
        }

    };

    this.getRevI = function (arr,i){
        return arr.length-i-1;
    };
    this.getArrRevI = function (arr,i){
        return arr[this.getRevI(arr,i)];
    };
    this.getPositionCoverage = function (arr,i){
        return arr[i].y-this.getArrRevI(arr,i).y;
    };


    this.isMosaic = function() {
        if(this.sankey._isMosaic && !this._hideMosaic)
            return true;
        return false;
    };

    this.isSVGsimilarity = function (svg) {
        var obj = this.getObjbySVG(svg);
        if(!obj )return ;
        return this.isObjsimilarity(obj);
    };

    this.isObjsimilarity = function (obj) {
        if(!obj)return ;
        if( obj._iSim != undefined )
            return true;
        return false;
    };


    this.getSankeyNodebyObj = function (obj) {
        if(!obj)return;
        var id = obj._branchID;
        if(!id) return ;
        return this.findCloneByID(id);
    };

    this.getSankeyNodebySVG = function (svg) {
        var obj = this.getObjbySVG(svg);
        if(!obj) return ;
        return this.getSankeyNodebyObj(obj);
    };

    this.getObjbySankeyNode = function (node) {
        if(!node || !node._svgID) return ;
        return vjObj[node._svgID];
    };

    this.getSVGbySankeyNode = function (node) {
        var obj = this.getObjbySankeyNode(node);
        if(!obj) return ;
        return this.getSVGbyObj(obj);
    };

    this.getSVGbyObj = function (obj) {
        if(!obj || !obj.svgID) return ;
        var svgID = obj.svgID;
        return gObject(svgID);
    };

    this.getObjbySVG = function (svgObj) {
        if(!svgObj || !svgObj.id) return ;
        return vjObj[svgObj.id.replace("_group","")];
    };

    this.findCloneByID = function (cloneID){
        if( this.sankey )
            return this.sankey.findCloneByID(cloneID);
        else
            return;
    };
    this.findSVGbyID = function (cloneID,array){
        var node = this.findCloneByID(cloneID);
        if(!node) return ;
        return this.getSVGbySankeyNode(node);
    };

    this.getSimSVGDictbyClonePos = function ( cloneObj, pos ) {
        if( !this.isMosaic() || !cloneObj || !cloneObj.map)return ;
        if( pos > cloneObj.map.length) return ;
        return cloneObj.map[pos];
    };

    this.getSimSVGDictbySimPos = function ( simObj, pos ) {
        var cloneObj = simObj.getParent();
        if( !cloneObj ) return ;
        return this.getSimCoordDictbyClonePos( cloneObj, simObj._start + pos );
    };

    this.getPositionbyObj = function ( obj, ir ) {
        var objcl = this.getSankeyNodebyObj( obj );
        if( !objcl ) return ;
        var start = obj._start?obj._start:0;
        return objcl._positions[ ir + start ];
    };

    this.getSimCoordDictbyClonePos = function ( cloneObj, pos ) {
        var sim_obj_arr = this.getSimSVGDictbyClonePos ( cloneObj, pos );
        if( !sim_obj_arr ) return ;
        var coords = new Array(), svg_el = 0, sim_start = 0, path_el;
        for( var i in sim_obj_arr ) {
            svg_el = this.getSVGbyObj(sim_obj_arr[i]);
            if( !svg_el ) continue;
            sim_start = 0;
            if(sim_obj_arr[i]._start) sim_start = sim_obj_arr[i]._start;
            path_el = svg_el.pathSegList.getItem( pos - sim_start );
            if( !path_el ) {
                return ;
            }
            coords.push({iSim:i,x:path_el.x,y:path_el.y, cov:this.getPositionCoverage(sim_obj_arr[i].coordinates,pos - sim_start)});
        }

        coords.sort(function(a,b){return b.cov - a.cov;});
        return coords;
    };

    this.getSVGCloneState = function (clone)
    {
        var res = clone.getAttribute("display-state");
        if(!res || res==="undefined"){
            res = "normal";
        }
        return res;
    };

    this.setSVGCloneState = function (clone,state)
    {
        clone.setAttribute("display-state",state);
    };

    this.isCloneInState = function (clone,state)
    {
        switch (state) {
        case"normal":{
            return this.isCloneNormal(clone);
        }
        case"highlighted":{
            return this.isCloneHighlighted(clone);
        }
        case"selected":{
            return this.isCloneSelected(clone);
        }
        default:{
            console.error("not known state",console.trace());
            return this.isCloneNormal(clone);
        }
        }
        return false;
    };

    this.isCloneNormal = function (clone)
    {
        var res = true;
        if( this.getSVGCloneState(clone)!="normal" ){
            return false;
        }
        clone = this.getObjbySVG(clone);
        if( this.isMosaic() && this.isSVGsimilarity(clone) ){
            clone = clone.getParent();
        }
        for( var p = 0 ; p < clone.children.length ; ++p ) {
            for(var i = 0 ; i < clone.children[p].children.length ; ++i) {
                if( !this.isCloneNormal( this.getSVGbyObj(clone.children[p].children[i])) ){
                    return false;
                }
            }
        }
        return res;
    };

    this.isCloneHighlighted = function (clone)
    {
        if( this.getSVGCloneState(clone)=="highlighted" ){
            return true;
        }
        clone = this.getObjbySVG(clone);
        if( this.isMosaic() && this.isSVGsimilarity(clone) ){
            clone = clone.getParent();
        }
        for( var p = 0 ; p < clone.children.length ; ++p ) {
            for(var i = 0 ; i < clone.children[p].children.length ; ++i) {
                if( this.isCloneHighlighted( this.getSVGbyObj(clone.children[p].children[i]) ) ){
                    return true;
                }
            }
        }
        return false;
    };

    this.isCloneSelected = function (clone)
    {
        if( this.getSVGCloneState(clone)=="selected" ){
            return true;
        }
        clone = this.getObjbySVG(clone);
        if( this.isMosaic() && this.isSVGsimilarity(clone) ){
            clone = clone.getParent();
        }
        for( var p = 0 ; p < clone.children.length ; ++p ) {
            for(var i = 0 ; i < clone.children[p].children.length ; ++i) {
                if( this.isCloneSelected( this.getSVGbyObj(clone.children[p].children[i]) ) ){
                    return true;
                }
            }
        }
        return false;
    };
    
    this.createLegend = function (rows,source,t_legendGroup) {
        
        var fontsize = 9;
        var symbolsize = 20;
        var linegap = 3;
        var legendOffset = 0.02;
        var scale_x = 0.2 - legendOffset;
        var scale_correction = 1-0.2;
        var legendGroup = t_legendGroup;
        if(!legendGroup){
            legendGroup = new vjSVG_group({
                translation:{x:1+legendOffset,y:0,z:0},
                scale:{x:scale_x/scale_correction,y:1,z:1}});
            legendGroup.svgID = legendGroup.objID;
            this._legendGroup = legendGroup;
        }
        
        var legendScale = {x:this.ownerView.getChartScale().x*scale_x,y:this.ownerView.getChartScale().y};
        var linegapScale = linegap/legendScale.y;
        var symbolsizeScale = {x:symbolsize/legendScale.x,y:this.ownerView.scene.scaleSize(symbolsize)};

        function boxHeight(last_record_y) {
           return 0.1 + (1-(last_record_y))*0.9;
        }
        
        var legendTitleGroup = new vjSVG_group({
            translation:{x:0,y:0.9,z:0},
            scale:{x:1,y:0.1,z:1}});
        legendGroup.children.push(legendTitleGroup);
        
        
        var legendBodyGroup = new vjSVG_group({
            translation:{x:0,y:0,z:0},
            scale:{x:1,y:0.9,z:1}});
        
        var last_record_y = this.legendBody(legendBodyGroup, legendScale, linegapScale, symbolsizeScale);
        
        var legendBox=new vjSVG_box ( mergeObj(this.legend.box,{
            brush:{fill:'white',"fill-opacity":0.6},
            pen:{"stroke-opacity":0.8,"stroke-width":1,"stroke":"grey","stroke-linejoin":"round"}
        },{crd:{x:0,y:1-boxHeight(last_record_y),z:0},height: boxHeight(last_record_y),width:1}) );
        legendGroup.children = [legendBox].concat( legendGroup.children );

        var isArrows = this._isLegendUpArrow || this._isLegendDownArrow;
        
        var legendTitle = new vjSVG_text({
            crd:{x:0.1, y:0.5},
            text:this.isMosaic()?"References":"Clones",
            ellipsizeWidth:(scale_correction/(isArrows?1.25:1)-0.1),
            font:{'font-size':(fontsize+2)+"px",'font-weight':'bold', 'dominant-baseline':'middle'}
            });
        
        var legendArrowUp=new vjSVG_symbol ( {
            definition:"arrowhead",
            size:(symbolsizeScale.y/1.5)/legendTitleGroup.scale.y,
            crd:{x:0.7, y:0.5},
            handler : {"onclick":"function:vjObjFunc('legendArrowUpClick','" + this.objID + "');"},
            brush:{cursor:'pointer',fill:"blue","fill-opacity":0.6}}
        );
        if(!this._isLegendUpArrow){
            delete legendArrowUp.handler;
            legendArrowUp.brush.fill = "grey";
            delete legendArrowUp.brush.cursor;
        }
        var legendArrowDown=new vjSVG_symbol ({
            definition:"arrowhead",
            size:(symbolsizeScale.y/1.5)/legendTitleGroup.scale.y,
            crd:{x:0.85, y:0.5},
            rotation:{
                crd:{x:0.85, y:0.5},
                vec:{x:0,y:0,z:1},
                angle:180
            },
            handler : {"onclick":"function:vjObjFunc('legendArrowDownClick','" + this.objID + "');"},
            brush:{cursor:'pointer',fill:"blue","fill-opacity":0.6}}
        );
        if(!this._isLegendDownArrow){
            delete legendArrowDown.handler;
            legendArrowDown.brush.fill = "grey";
            delete legendArrowDown.brush.cursor;
        }
        
        legendTitleGroup.children.push(legendTitle);
        if(isArrows){
            legendTitleGroup.children.push(legendArrowUp);
            legendTitleGroup.children.push(legendArrowDown);
        }
        
        legendGroup.children.push(legendBodyGroup);
        if(source)source.children=source.children.concat(legendGroup);
    };
    
    this.legendArrowDownClick = function (empty,svgEl,mouseClick,svgID) {
        if(!this._legendGroup)return;
        var svgLegendGroup = gObject(this._legendGroup.svgID+"_group");
        if(!svgLegendGroup) return;
        svgLegendGroup.parentElement.removeChild(svgLegendGroup);
        for(var i = 0 ; i < this._legendGroup.children.length ; ++i) delete this._legendGroup.children[i];
        this._legendGroup.children = new Array();
        
        this._legendPageStart+=this._legendPageCnt;
        
        this.createLegend(undefined,undefined,this._legendGroup);
        this._legendGroup.render(this.ownerView.scene);
    };
    
    this.legendArrowUpClick = function (empty,svgEl,mouseClick,svgID) {
        if(!this._legendGroup)return;
        var svgLegendGroup = gObject(this._legendGroup.svgID+"_group");
        if(!svgLegendGroup) return;
        svgLegendGroup.parentElement.removeChild(svgLegendGroup);
        for(var i = 0 ; i < this._legendGroup.children.length ; ++i) delete this._legendGroup.children[i];
        this._legendGroup.children = new Array();
        
        this._legendPageStart-=this._legendPageCnt;
        if(this._legendPageStart<0)this._legendPageStart=0;
        
        this.createLegend(undefined,undefined,this._legendGroup);
        this._legendGroup.render(this.ownerView.scene);
    };
    
    this.legendBody = function (source, legendScale, linegapScale, symbolsizeScale) {
        var fontsize = 9;
        var symbolsize = 20;
        function getSymbol_coord(y) {
            return {x:(symbolsizeScale.x/2)*1.5,y:y};
        }
        function getText_coord(y) {
            return {x:(3/2*symbolsizeScale.x) ,y:y};
        }
        
        var rows = 0;
        if( !this.isMosaic() ) {
            rows = this.sankeyTbl.rows;
        }
        else {
            rows = this.refTbl.rows.slice(1);
        }
        var legendRecodSymbol,legendRecodText, cur_y = 1 - (linegapScale);
        if(!this._legendPageStart)this._legendPageStart = 0;
        if(!this._legendPageCnt)this._legendPageCnt = 0;
        var t_legendPageCnt = 0;
        this._isLegendDownArrow = false;
        this._isLegendUpArrow = false;
        
        if(!this._legendPageStart ){
            if(this.isMosaic()) {
                legendRecodSymbol=new vjSVG_symbol ( {
                    definition:"rectangle",
                    size:symbolsize/2+"px",
                    crd:getSymbol_coord(cur_y),
                    isUnscaled:true,
                    brush:{fill:this.fillN,"fill-opacity":0.9}});
                
                source.children.push(legendRecodSymbol);
                legendRecodText = new vjSVG_text({
                    crd:getText_coord(cur_y),
                    ellipsizeWidth:(1-2*symbolsizeScale.x),
                    font:{'font-size':fontsize+"px", 'dominant-baseline':'middle'}
                }); 
                legendRecodText.text = "References below threshold";
                source.children.push(legendRecodText);
                cur_y -= (linegapScale+symbolsizeScale.y);
            }
        }
        else {
            this._isLegendUpArrow = true;
        }
        var length = this.isMosaic()?rows.length-1:rows.length;
        for ( var i = this._legendPageStart ; i < length ; ++i) {
            var cur_text = 0, cur_color_index = 0;
            if(this.isMosaic()) {
                cur_color_index = rows[i].id-1;
                cur_text = rows[i].Reference;
            }
            else{
                cur_color_index = i;
                cur_text = rows[i][this.sankey.columnDefinition.branchID];
            }
            legendRecodSymbol=new vjSVG_symbol ( {
                definition:"rectangle",
                size:symbolsize/2+"px",
                crd:getSymbol_coord(cur_y),
                isUnscaled:true,
                brush:{fill:this.colors[cur_color_index],"fill-opacity":0.9}});
            
            source.children.push(legendRecodSymbol);
            legendRecodText = new vjSVG_text({
                crd:getText_coord(cur_y),
                ellipsizeWidth:(1-2*symbolsizeScale.x),
                font:{'font-size':fontsize+"px", 'dominant-baseline':'middle'}
            }); 
            legendRecodText.text = cur_text;

            source.children.push(legendRecodText);
            cur_y -= (linegapScale+symbolsizeScale.y);
            ++t_legendPageCnt;
            if(cur_y < symbolsizeScale.y/2) {
                if(i<length-1) {
                    this._isLegendDownArrow = true;
                }
                cur_y = 0;
                break;
            }
        }
        if(this._legendPageCnt<t_legendPageCnt)
            this._legendPageCnt = t_legendPageCnt;
        return cur_y;
    };
    
    if(this.mode===undefined)this.mode="relative";
    if(this.shift===undefined)this.shift=0;
    if(this.center===undefined)this.center=0.5;

    this.legend = {};
    
    this.averageSankeySections = function() {
        function avarageSingleSankey(_that,node){
            if(!node._positions)return;
            var sectionsCoord = new Array();
            for( var i in node.children ) {
                if( node.children[i]._bifurcateID==node._branchID && node.children[i]._branchStart>node._start ) {
                    sectionsCoord.push(node.children[i]._branchStart);
                }
            }
            for( var i in node.merged ) {
                if( node.merged[i]._mergeID==node._branchID && node.merged[i]._branchEnd<node._end ) {
                    sectionsCoord.push(node.merged[i]._branchEnd);
                }
            }
            sectionsCoord.sort(function(a,b){return a-b;});
            var sum=0, last_position = 0, pos_cnt = 0, av = 0;
            for(var i = 0, j = 0 ; i < node._positions.length ; ++i) {
                if( j < sectionsCoord.length && node._positions[i].coord > sectionsCoord[j]) {
                    av = parseInt(sum/pos_cnt);
                    while (node._positions[last_position].coord < sectionsCoord[j] && i-last_position ) {
                        node._positions[last_position].cov = av;
                        ++last_position;
                    }
                    last_position = i;
                    while( j < sectionsCoord.length && sectionsCoord[j] < node._positions[i].coord)++j; 
                    sum = 0; pos_cnt = 0;
                }
                sum+=node._positions[i].cov;
                ++pos_cnt;
            }
            av = parseInt(sum/pos_cnt);
            while (last_position<node._positions.length && node._positions[last_position].coord <= node._positions[node._positions.length-1].coord ) {
                node._positions[last_position].cov = av;
                ++last_position;
            }
        }
        this.sankey.enumerate(avarageSingleSankey, this,null,null,this.sankey.root);
    };
    
    this.constructLegend = function(serie) {
        if(serie.name != this.collection[0].name) return;
        if(!this._constrcuted) {
            this.constructSankey(serie);
        }
        this.createLegend(this.refTbl.rows,this);
    };
    
    this.constructorFunction=new Object();
    
    this.constructSankey = function (serie) {
        this.sankeyTbl = serie.tblArr;
        this.sankey = new vjSankey(this.sankeyTbl,null,this.tile);
        if(this.average){
            this.averageSankeySections();
        }
        this.refTbl = new vjTable(vjDS[this.collection[1].name].data,0,vjTable_hasHeader);
        this.References={};
        for(var ii = 0 ; ii < this.refTbl.rows.length ; ++ii ) {
            this.References[this.refTbl.rows[ii].id] = this.refTbl.rows[ii].Reference;
        }
        
        if(!this.colors_computed){
    
            var colors = this.colors ? this.colors.slice(0) : vjGenomeColors.slice(1) ;
            var c_d = 1 + this.sankey.similaritiesMaxIndex/colors.length , n_d = 0;
            var color_max = this.sankeyTbl.rows.length>this.sankey.similaritiesMaxIndex?this.sankeyTbl.rows.length:this.sankey.similaritiesMaxIndex;
            for (var it = 0; it <= color_max; ++it) {
                n_d = (it/colors.length)/c_d * 360 ;
                this.colors[it] = hueshiftColor(colors[(it) % (colors.length)],n_d);
            }
            this.colors = this.colors.slice(0,color_max + 1);
    
            this.colors_computed = true;
        }
        this._constrcuted = true;
    };
    
    this.constructorFunction['sankey']=function(serie){
        if(serie.name != this.collection[0].name) return;
        if(!this._constrcuted)this.constructSankey(series);
        var width=this.max.x-this.min.x;
    
        var trajArray=new Array();

        this.sankey.enumerate("var traj = vjObjEvent('createClone','"+this.objID+"',node,'"+width+"',"+this.sankey.similaritiesCnt+");if(traj)params.push(traj);",trajArray);
       
        
        var boxArray=new Array();
        for(var iT=0; iT < trajArray.length ; ++iT){
            var clone=this.getSankeyNodebyObj(trajArray[iT]);
    
            var cloneM = clone.merge;
            var cloneB = clone.parent;
    
            var boxM,boxB;
            if( cloneM && cloneM.depth>0 && !this.isNdrawMergeLines ){
                boxM=this.createCloneConnections(clone,cloneM,1,cloneM!=cloneB);
                if( boxM )
                    boxArray.push(boxM);
            }
            if( cloneB && cloneB.depth>0 && !this.isNdrawBifurcateLines ){
                boxB=this.createCloneConnections(clone,cloneB,0,cloneM!=cloneB);
                if( boxB )
                    boxArray.push(boxB);
            }
        }
        trajArray=trajArray.concat(boxArray);
        this.children=this.children.concat(trajArray);
        this._constrcuted = false;
    };
};
