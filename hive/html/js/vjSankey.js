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

function vjSankeyNode(nodeBase) {
    return vjTreeNode.call(this,nodeBase);
}


function vjSankey(tbl, columnDefinition, tile, excludeObjRegexp ) {
        this._tile = tile;
        if( typeof(this._tile)!==undefined) {
            if(typeof(this._tile)=="number") {
                this._tileThrshl = this._tile;
                if(this._tileThrshl<0)this._tileThrshl = 0;
                if(this._tileThrshl>100)this._tileThrshl = 100;
            }
            else if (typeof(this._tile) == "boolean") {
                this._isTile = this._tile;
            }
        }

    if( !columnDefinition ) {
        columnDefinition = {
            branchID : 'Clone ID',
            start : 'showStart',
            end : 'showEnd',
            trueStart : 'Start',
            trueEnd : 'End',
            weight : 'Max Coverage',
            mergeID : 'Merged ID',
            bifurcateID : 'Bifurcated ID',
            coverage : 'Coverage',
            branchStart : 'Bifurcation Pos',
            branchEnd : 'Merge Pos',
            firstDiff : 'First Diff',
            lastDiff : 'Last Diff',
            branchStats: 'Bifurcation Statistics',
            support : '# of points of support'
        };
    }
    this.columnDefinition = columnDefinition;

    tbl.enumerate("node.name = node[params.name]; if(node[params.parent] && node[params.name]!=node[params.parent])node.parent = node[params.parent];",{name:this.columnDefinition.branchID,parent:this.columnDefinition.bifurcateID});


    this.newNode = function(base) {
        return new vjSankeyNode(base);
    };

    vjTree.call(this, tbl);


    this.parseSankey = function( node ) {

        function linkSankeyObjects( _this, node) {
            var cl = _this.findCloneByID(node._mergeID);
            if( cl && node._mergeID != node._branchID) {
                node.merge = cl;
                if(!cl.merged) cl.merged = new Array();
                cl.merged.push(node);
            }
        }

        this.enumerate("params.cloneContent2Object.call(params,node)",this,null,null,node);
        this.enumerate(linkSankeyObjects, this,null,null,node);
        var simil_param = {max_length:0,max_index:0};
        this.enumerate("for(var is in node._similarities){is = parseInt(is); if(is > params.max_index){params.max_index = is;} }if(node._similaritiesCnt && node._similaritiesCnt > params.max_length) {params.max_length = node._similaritiesCnt;}",simil_param,null,null,node);
        this.similaritiesCnt = parseInt(simil_param.max_length);
        this.similaritiesMaxIndex = parseInt(simil_param.max_index);
    };

    this.cloneContent2Object = function(Obj, ir) {

        Obj._branchID = Obj[this.columnDefinition.branchID];delete Obj[this.columnDefinition.branchID];
        Obj._bifurcateID = Obj[this.columnDefinition.bifurcateID];delete Obj[this.columnDefinition.bifurcateID];
        Obj._branchStart = (parseInt(Obj[this.columnDefinition.branchStart]) < 0) ? 'none'
                : parseInt(Obj[this.columnDefinition.branchStart]);delete Obj[this.columnDefinition.branchStart];
        Obj._branchEnd = parseInt(Obj[this.columnDefinition.branchEnd]); delete Obj[this.columnDefinition.branchEnd];
        Obj._firstDiff = parseInt(Obj[this.columnDefinition.firstDiff]); delete Obj[this.columnDefinition.firstDiff];
        Obj._lastDiff = parseInt(Obj[this.columnDefinition.lastDiff]); delete Obj[this.columnDefinition.lastDiff];
        var stats=Obj[this.columnDefinition.branchStats];
        Obj._branchStats = stats?JSON.parse(stats.replace(/'/g, "\"")):""; delete Obj[this.columnDefinition.branchStats];

        Obj._start = parseInt(Obj[this.columnDefinition.start]); delete Obj[this.columnDefinition.start];
        Obj._end = parseInt(Obj[this.columnDefinition.end]);delete Obj[this.columnDefinition.end];

        Obj._trueStart = parseInt(Obj[this.columnDefinition.trueStart]);delete Obj[this.columnDefinition.trueStart];
        Obj._trueEnd = parseInt(Obj[this.columnDefinition.trueEnd]);delete Obj[this.columnDefinition.trueEnd];

        Obj._mergeID = (Obj[this.columnDefinition.mergeID] == Obj._branchID) ? 'none'
                : Obj[this.columnDefinition.mergeID]; delete Obj[this.columnDefinition.mergeID];

        Obj._coverage = Obj[this.columnDefinition.coverage]; delete Obj[this.columnDefinition.coverage];

        Obj._weight = parseInt(Obj[this.columnDefinition.weight]); delete Obj[this.columnDefinition.weight];
        Obj._support = Obj[this.columnDefinition.support]; delete  Obj[this.columnDefinition.support];;

        Obj._positions = new Array();

        if(Obj._coverage) {
            var posArr = JSON.parse(Obj._coverage.replace(/'/g, "\""));
            var siml = 0, max_siml = 0;
            for (var ip = 0; ip < posArr.length; ++ip) {
                var clpos = new Object();
                clpos.coord = posArr[ip].p;
                clpos.base = posArr[ip].l;
                clpos.cov = posArr[ip].c;
                clpos.sim = new Object();
                siml = 0;
                var t_sim_max = 0 , t_sim_index_max = 0;
                for ( var is in posArr[ip].s) {
                    this._isMosaic = true;
                    if( this._isTile === true ) {
                        if( t_sim_max < posArr[ip].s[is] ) {
                            t_sim_max = posArr[ip].s[is];
                            t_sim_index_max = parseInt(is);
                            siml = 1;
                        }
                    }
                    else if ( this._tileThrshl ) {
                        if( posArr[ip].s[is]*100 > this._tileThrshl ) {
                            clpos.sim[parseInt(is)] = posArr[ip].s[is];
                            t_sim_max += posArr[ip].s[is]*100;
                            ++siml;
                        }
                        t_sim_max += posArr[ip].s[is];
                    }
                    else {
                        clpos.sim[parseInt(is)] = posArr[ip].s[is];
                        t_sim_max += posArr[ip].s[is];
                        ++siml;
                    }
                }
                if( this._isTile && siml) {
                    clpos.sim[t_sim_index_max] = 1;
                }
                else if ( this._tileThrshl && siml ) {
                    for(var is in clpos.sim) {
                        clpos.sim[is] = clpos.sim[is]*100/t_sim_max;
                    }
                }
                if (siml) {
                    clpos.sim.length = siml;
                    if (max_siml < siml) {
                        max_siml = siml;
                    }
                } else {
                    delete clpos.sim;
                }

                Obj._positions.push(clpos);
                if (isok(ir))
                    break;
            }


            if (max_siml) {
                var sim_arr = new Object();

                var t_obj = 0, clpos = Obj._positions;
                for (var ic = 0; ic < clpos.length; ++ic) {
                    for ( var is in clpos[ic].sim) {
                        if(is =='length')continue;
                        t_obj = sim_arr[is];
                        if (!t_obj) {
                            sim_arr[is] = new Array();
                            t_obj = sim_arr[is];
                        }
                        t_obj.push({
                            'pos' : ic,
                            'sim' : clpos[ic].sim[is]
                        });
                    }
                }

                Obj._similarities = sim_arr;
                Obj._similaritiesCnt = max_siml;
            }
            if (isNumber(ir)) {
                Obj._positions = Obj._positions[0];
            }
        }
        return Obj;
    };

    this.findCloneByID = function(cloneID, node) {
        return this.findByAttribute("_branchID", cloneID, node);
    };

    if ( this.root) {
        this.parseSankey( this.root );
    }
}

