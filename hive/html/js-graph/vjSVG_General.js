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

function transferAttributes(dest,origin){
    if(typeof dest != 'object' || typeof origin != 'object')return 0;
    var ii=0;
    for(ii in origin){
        dest.setAttribute(ii,origin[ii]);
    }
}



function bumper(nodeArr,params){
    if (!isok(nodeArr))
        return;
    var weight = 1;
    function doOverlap(nodeA,nodeB) {
        return (nodeA.start<=nodeB.start)?( (nodeA.end >= nodeB.start) ?true:false ):((nodeA.start<=nodeB.end)?true:false);
    }
    if(params!==undefined){
        if(params.rule!==undefined)rule=params.rule;
        if(params.weight!==undefined)weight=params.weight;
    }
    
    else params=0;
    var placedArr=new Array();

    for(var i=0;i<nodeArr.length;++i){
        var node=nodeArr[i],places=new Array();node.place=0;
        placedArr.sort(function(a, b) {
            return ((a.place!==undefined  ? a.place : Number.MAX_VALUE) - (b.place!==undefined ? b.place : Number.MAX_VALUE));
        });
        if(!node.weight)node.weight=weight;

        for(var j=0;j<placedArr.length;++j){
            var examN=placedArr[j];
            if( doOverlap(node,examN) )
                places.push(j);
        }
        var ranges=new Array();
        if(places.length){
            var overN=placedArr[places[0]];
            if(overN.place>=node.weight){
                ranges.push({start:0,end:overN.place});
            }
            var j=1;
            for(; j<places.length; ++j){
                overN=placedArr[places[j]];
                var prevL=placedArr[places[j-1]].place+placedArr[places[j-1]].weight;
                if( !doOverlap(overN,prevL) ) continue;
                var dist=overN.place-prevL;
                if(dist>node.weight)
                    ranges.push({start:prevL,end:overN.place,id:places[j]});
            }
            ranges.push({start:placedArr[places[j-1]].place+placedArr[places[j-1]].weight,end:0,id:places[j-1]});
        }
        else
            ranges.push({start:0,end:0,id:-1});

        if(!placedArr.length || params.operation===undefined){
            node['bumped']=ranges[0].id;
            node.place=ranges[0].start;
        }
        else{
            if (typeof (params.operation) == "function")
                params.operation(params.params, node, params.operation);
            else if (operation.indexOf("javascript:") == 0)
                eval(params.operation.substring(0, 11))(params.params, this, node, params.operation);
            else
                eval(params.operation);
        }
        nodeArr[i]=node;
        placedArr.push(node);
    }
    var top_node=placedArr[placedArr.length-1];
    var top_height = top_node.place + top_node.weight;
    var layer_sep = 1 * top_height / 100 ;
    for(var iii=1;iii<nodeArr.length;++iii){
        var cnode = nodeArr[iii];
        cnode.place = cnode.place+(iii*layer_sep);
    }
};


function vjSVG_GIhandler(funcname, svgObj, evt,TsvgID) {
    if(TsvgID!==undefined){
        if(TsvgID==svgObj.id)return 0;
    }
    else
        TsvgID=svgObj.id;
    
    if(evt.offsetX==undefined){
        evt.offsetX=evt.layerX;
        evt.offsetY=evt.layerY;
    }

    var svgScreenCTM= svgObj.getScreenCTM().inverse();
    var screenMatrix = [ [ svgScreenCTM.a, svgScreenCTM.c, svgScreenCTM.e, 0 ],
            [ svgScreenCTM.b, svgScreenCTM.d, svgScreenCTM.f, 0 ],
            [ 0, 0, 1, 0 ], [ 0, 0, 0, 1 ] ];
    var InvCTM=matrixMultiplicationToVector(matrixInverse(screenMatrix),{x:evt.offsetX,y:evt.offsetY,z:0});
    var curCord=matrixMultiplicationToVector(svgObj.currentInvertedMatrix,InvCTM);
    evt.nX=curCord.x instanceof Array ?curCord.x[0]:curCord.x;
    evt.nY=curCord.x instanceof Array ?curCord.y[0]:curCord.y;
    evt.nZ=curCord.x instanceof Array ?curCord.z[0]:curCord.z;
    var obj = vjObj[svgObj.id];
    var irow=obj?(obj.irow?obj.irow:(obj.coordinates?obj.coordinates[0].irow:undefined)):undefined;
    var res=funcLink(funcname, irow, svgObj, evt,TsvgID);

    if (evt.stopPropagation)
        evt.stopPropagation();
    else
        window.event.cancelBubble = true;

    if (!obj)
        return;

    if (res > 0) {
        if (obj.parent) {
            if(obj.parent.handlers){
                var propFuncName = obj.parent.handlers[evt.type];
                if(propFuncName)
                    vjSVG_GIhandler(propFuncName, obj.parent, evt,TsvgID);
            }
        }
    }
    if (res < 0) {
        for ( var i = 0; i < obj.children.length; ++i) {
            if (obj.children[i]) {
                var propFuncName = obj.children[i].handlers[evt.type];
                if(propFuncName)
                    vjSVG_GIhandler(propFuncName, obj.children[i], evt, TsvgID);
            }
        }
    }
    return 0;
}


