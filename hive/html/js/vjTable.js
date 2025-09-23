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


var vjTable_hasHeader=0x0001;
var vjTable_collapsePropFormat=0x0002;
var vjTable_inheritPathPropFormat=0x0004;
var vjTable_fromJSON=0x0008;
var vjTable_autoJSON=0x0010;
var vjTable_jsonStringize=0x0020;
var vjTable_propCSV=vjTable_hasHeader|vjTable_collapsePropFormat|vjTable_autoJSON;



function vjTable( content, colfmt, parsemode, skiprows, cntrows, strDelimiterList, keepempty, cntHeaderRows )
{
    this.CSVToArray = function( arr, strData, colfmt, skiprows, cntrows, strDelimiterList, keepempty )
    {
        if (!strData || !strData.length) {
            return [];
        }
        strDelimiterList = (strDelimiterList || ",");

        var objPattern = new RegExp(
            (
                    "([" + strDelimiterList + "]|\\r?\\n|\\r|^)" +

                    "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +

                    "([^\"" + strDelimiterList + "\\r\\n]*))"
            ),
            "gi"
            );



        var arrMatches = null;

        var strPattern = new RegExp("[" + strDelimiterList + "]");

        if (strData.length && strData.charAt(0).match(strPattern)) {
            arr[arr.length - 1].push("");
        }

        for(var it=0 ; arrMatches = objPattern.exec( strData ) ;  ) {

            var strMatchedDelimiter = arrMatches[ 1 ];

            if ( strMatchedDelimiter.length && !strMatchedDelimiter.match(strPattern) ) {
                arr.push( [] );
                ++it;
            }

            var strMatchedValue;
            if (arrMatches[ 2 ]){
                strMatchedValue = arrMatches[ 2 ].replace(
                    new RegExp( "\"\"", "g" ),
                    "\""
                    );
            } else {
                strMatchedValue = arrMatches[ 3 ];
            }

            if (colfmt && colfmt.length != 0) {
                for ( var ic = 0; ic < strMatchedValue.length; ++ic) {
                    if (!keepempty && (ic < strMatchedValue.length && strMatchedValue[ic].length == 0)) {
                        continue;
                    }
                    var fmt = (ic < colfmt.length) ? colfmt.charAt(ic) : "s";
                    if (fmt == "-") {
                        continue;
                    } else if (fmt == "i") {
                        strMatchedValue[ic] = parseInt(strMatchedValue[ic]);
                    } else if (fmt == "r") {
                        strMatchedValue[ic] = parseFloat(strMatchedValue[ic]);
                    } else {
                        strMatchedValue[ic] = strMatchedValue[ic];
                    }
                }
            }
            arr[ arr.length - 1 ].push( strMatchedValue );
        }
        for ( var ir = 0; ir < arr.length; ++ir) {
            if (!keepempty && arr[ir].length <= 1) {
                arr.splice(ir, 1);
                --ir;
                continue;
            }
        }
        if(skiprows) {
            arr.splice(0,skiprows);
        }
        return( arr );
    };

    this.convertPropToHorizontal=function( rows )
    {

        var newrows = new Array();
        var newhdr = new Array ( { name:"id"} ) ;
        var idmap = new Object ();
        var colMap = new  Object() ;
        colMap["id"]=1;
        for ( var ir=0; ir<rows.length; ++ir )  {

            var iNum=idmap[""+ rows[ir].id ];
            if(!iNum){
                iNum=newrows.length;
                idmap[""+ rows[ir].id ]=iNum+1;
                newrows[iNum]=new Object();
                newrows[iNum].id=rows[ir].id;
            }else --iNum;

            if(!colMap [""+ rows[ir].name ]){
                newhdr.push({name: rows[ir].name });
                colMap[""+ rows[ir].name ]=newhdr.length;
            }

            if(!newrows[iNum][rows[ir].name])
                newrows[iNum][rows[ir].name]=rows[ir].value;
            else {

                if( !(newrows[iNum][rows[ir].name] instanceof Array) )
                    newrows[iNum][rows[ir].name]=new Array(newrows[iNum][rows[ir].name]);
                newrows[iNum][rows[ir].name].push(rows[ir].value);
            }
            if((parsemode&vjTable_inheritPathPropFormat) && isok(rows[ir].path))
                newrows[iNum][rows[ir].name+"."+rows[ir].path]=rows[ir].value;
        }

        this.rows=newrows;
        this.hdr=newhdr;
        for ( var ir=0; ir<this.rows.length; ++ir){
            this.rows[ir].cols=new Array();
            for ( var ic=0; ic<this.hdr.length; ++ic){
                var iCol=colMap[this.hdr[ic].name]-1;
                this.rows[ir].cols[iCol]=this.rows[ir][this.hdr[ic].name];
            }
        }



        return newrows;
    };


    this.maxRow=function(icol , skiprow , cntrow )
    {
        var imax=skiprow;
        for (var ir=skiprow+1; ir<this.rows.length; ++ir) {
            if(cntrow && ir>=skiprow+cntrow)break;
            if(this.rows[ir][icol]>this.rows[imax][icol])imax=ir;
        }
        return imax;
    };


    this.enumerate=function( operation, params, checked )
    {
        for ( var ir = 0; ir < this.rows.length; ++ir) {
            var row = this.rows[ir];
            var node = row;
            if (checked && (!row.checked))
                continue;
            if (typeof (operation) == "function")
                operation(params, this, ir, operation);
            else if (operation.indexOf("javascript:") == 0)
                eval(operation.substring(0, 11))(params, this, ir, operation);
            else
                eval(operation);
        }
    };

    this.accumulate=function( checker, collector , params, checked )
    {
        if(!params)params=new Object();
        params.findByOperList=new Array();

        if (typeof(checker) == "function" && typeof(collector) == "function")
            this.enumerate(
                function(params, tbl, ir, operation) {
                    var node = tbl.rows[ir];
                    if (checker(node, tbl))
                        params.findByOperList.push(collector(node, tbl));
            }, params, checked);
        else
            this.enumerate( "var res="+checker+";if(res)params.findByOperList.push("+collector+");", params, checked);

        return params.findByOperList;
    };

    this.customizeColumns=function( addheaders , doAppend )
    {
        var forAllCols = -1;
        
        for( var ic=0; ic<addheaders.length; ++ic) {
            var ifound=parseInt(addheaders[ic].col ? addheaders[ic].col : ""),ff=0;
            var didFind=false;

            if( !isNaN(ifound ) ){
                if (this.hdr[ifound] == undefined) continue;
                for (ff in addheaders[ic] ) {
                    if(ff=="name" || ff=="col" ) continue;
                    this.hdr[ifound][ff]=addheaders[ic][ff];
                }
                didFind=true;
                this.hdr[ifound].validated = true;
            }
            else if( addheaders[ic].name ) {
                for( ifound=0; ifound<this.hdr.length ; ++ifound){
                    if(this.hdr[ifound].name.match( addheaders[ic].name)) {
                        for (ff in addheaders[ic] ) {
                            if(ff=="name" || ff=="col" ) continue;
                            this.hdr[ifound][ff]=addheaders[ic][ff];
                        }
                        didFind=true;
                        this.hdr[ifound].validated = true;
                    }
                }
            }
            if( (doAppend || addheaders[ic].doAppend )&& !didFind ){
                ifound=this.hdr.length;
                this.hdr[ifound]=new Object();
                for (ff in addheaders[ic] ) {
                    this.hdr[ifound][ff]=addheaders[ic][ff];
                }
                this.hdr[ifound].validated = true;
            }

            if (addheaders[ic].col=="*")
                forAllCols = addheaders[ic];
        }
        
        if (forAllCols != -1)
        {
            for (var ih = 0; ih < this.hdr.length; ih++)
            {
                if (!this.hdr[ih].validated)
                {
                    for (ff in forAllCols ) {
                        this.hdr[ih][ff]=forAllCols[ff];
                    }                    
                }
            }
        }
        
    };



    this.customizeRows = function(addrows, allownew, objCpy) {
        for ( var ic = 0; ic < addrows.length; ++ic) {
            var ifound = parseInt(addrows[ic].row ? addrows[ic].row : ""), ff=0;

            var howmanyfound = 0;
            if (!isNaN(ifound)) {
                for (ff in addrows[ic]) {
                    if (ff == "name" || ff == "row")
                        continue;
                    this.rows[ifound][ff] = addrows[ic][ff];
                }
                ++howmanyfound;
            } else if (addrows[ic].name) {
                for (ifound = 0; ifound < this.rows.length; ++ifound) {
                    if (this.rows[ifound].name
                            && this.rows[ifound].name.match(addrows[ic].name) && !addrows[ic].noMatch)
                    {
                        for (ff in addrows[ic]) {
                            if (ff == "name" || ff == "row" )
                                continue;
                            if(  this.rows[ifound]["path"] && ( addrows[ic]["_path"]) )
                                continue;
                            this.rows[ifound][ff] = addrows[ic][ff];
                        }
                        ++howmanyfound;
                    }
                }
            }
            if ((howmanyfound == 0 || !addrows[ic].prohibit_new) && !addrows[ic].prohibit_new && allownew  && typeof(addrows[ic].name)=="string"){
                if( !objCpy ) {
                    this.rows.push( addrows[ic] );
                }
                else {
                    this.rows.push(cpyObj(addrows[ic]));
                }
            }
        }
    };

    this.mangleRows = function (matchObjRegex, objToBorrow, inverted) {
        var newrows = new Array();

        for (var ir = 0; ir < this.rows.length; ++ir) {
            var didmatch = matchObj(this.rows[ir], matchObjRegex);
            if (inverted) didmatch = !didmatch;

            if (didmatch) {
                if (objToBorrow == "delete") continue;

                for (ff in objToBorrow) {
                    this.rows[ir][ff] = objToBorrow[ff];
                }
                continue;
            }
            if (objToBorrow == "delete" && !didmatch)
                newrows.push(this.rows[ir]);
        }
        if (objToBorrow == "delete")
            this.rows = newrows;

    };

    this.subsetRows = function (indexArray, inverted) {
        if(!inverted) {
            this.rows = indexArray.map( i => this.rows[i]);
        } else {
            this.rows = this.rows.filter((e,i) => !indexArray.includes(i));
        }
    };

    this.syncCols = function() {
        var namecount = {};
        for (var ic = 0; ic < this.hdr.length; ++ic) {
            var name = this.hdr[ic].name;
            if (namecount[name]) {
                namecount[name]++;
            } else {
                namecount[name] = 1;
            }
        }

        for (var ir = 0; ir < this.rows.length; ++ir) {
            var synced_repeated = {};
            for (var ic = 0; ic < this.hdr.length; ++ic) {
                var name = this.hdr[ic].name;
                if (namecount[name] > 1 && synced_repeated[name]) {
                    continue;
                } else {
                    this.rows[ir].cols[ic] = this.rows[ir][name];
                    if (namecount[name] > 1) {
                        synced_repeated[name] = true;
                    }
                }
            }
        }
    };

    this.sortTbl = function(ic,isNdesc,sortFunc){
        if(!this.rows.length)return;
        function rowSort(a,b){

            if(a.order || b.order ) {
                return (a.order - b.order);
            }
            var at=a.cols[ic],bt=b.cols[ic];
            result = cmpNatural(at,bt);
            if(isNdesc)
                result*=-1;
            return result;
        }
        if(sortFunc) {
            this.rows.sort(sortFunc);
        }
        else{
            this.rows.sort(rowSort);
        }
    };
    
    this.appendTbl=function( secondTable )
    {
        var ic,is;
        for( ic=0; ic < secondTable.hdr.length ; ++ic ) {
            var colnameToAppend = secondTable.hdr[ic].name;
            for( is=0; is<this.hdr.length ; ++is ) {
                 if(colnameToAppend==this.hdr[is].name )
                     break;
            }
            
            if(is<this.hdr.length) {
                continue;
            }
            else  { 
                this.hdr.push(secondTable.hdr[ic]);
            }
            var ir;
            for(ir=0; ir<secondTable.rows.length; ++ir){
                var valueToAdd = secondTable.rows[ir][colnameToAppend];
                if (ir > this.rows.length-1){
                    this.rows[ir] = new Object();
                    var keyArr = Object.keys(this.rows[0]);
                    for (var ik=0; ik<keyArr.length; ++ik){
                        if (keyArr[ik]=="cols"){
                            this.rows[ir]['cols'] = new Array();
                            for (var icv=0; icv<this.rows[0].cols.length; ++icv) this.rows[ir].cols.push(0);
                        }
                        else if (keyArr[ik]=='irow'){
                            this.rows[ir]['irow']=ir;
                        }
                        else this.rows[ir][keyArr[ik]]=0;
                    }
                }
                this.rows[ir][colnameToAppend]=valueToAdd;
                this.rows[ir].cols.push(valueToAdd);
            }
            for (; ir<this.rows.length; ++ir){
                this.rows[ir][colnameToAppend]=0;
                this.rows[ir].cols.push(0);
            }
        }
    };
    
    this.arrayToCSV = function (){
        var strToReturn = "";
        
        if (this.hdr.length > 0){
            console.log ("getting information from the header was not implemented");
            return "";
        }
        else{
            for (var ir = 0; ir < this.rows.length; ir++){
                for (var ic = 0; ic < this.rows[ir].cols.length; ic++){
                    if (ic > 0)
                        strToReturn += ",";
                    var tmp =  (this.rows[ir].cols[ic]).replace(/[\\"']/g, '$&$&').replace(/\u0000/g, '\\0');
                    strToReturn += "\"" + tmp + "\"";
                }
                strToReturn += "\n";
            }
        }
        return strToReturn;
    };

    var startRow=0;
    this.hdr=new Array();
    this.rows=new Array();
    
    if(content && (parsemode&vjTable_autoJSON)) {
        var ch=content.substring(0,1);
    }
    
    if(content && (parsemode&vjTable_fromJSON)) {
        var jsonObj = JSON.parse(content);
        var iRow=0;
        for (var row in jsonObj ) {
            if( typeof jsonObj[row] != "object" ) 
                continue
            this.rows[iRow]={cols: [], irow: iRow};
            
            for (var col in jsonObj[row] ) {
                var ih;for ( ih=0; ih<this.hdr.length; ++ih) {
                    if(this.hdr[ih].name==col)break;
                }
                if(ih>=this.hdr.length) {
                    this.hdr.push({name:col,rows: [col]});
                } else {
                }
                this.rows[iRow].cols.push(col);
                
                var val=jsonObj[row][col];
                if(typeof val == "object" )
                    val=JSON.stringify(val);
                this.rows[iRow][col]=val;
                
            }
            
            ++iRow;
        } 
    }
    else { 
        var _arr=this.CSVToArray( new Array ( new Array() ) , content, colfmt, skiprows, cntrows, strDelimiterList, keepempty  );
    
        if(!_arr.length)return;
    
        if( parsemode&vjTable_hasHeader ) {
            if (!cntHeaderRows) {
                cntHeaderRows = 1;
            }
    
            for(var ihdrRow=0; ihdrRow < cntHeaderRows; ihdrRow++) {
                for (var ic=0; ic<_arr[startRow].length; ++ic) {
                    var hdrCell = _arr[startRow][ic];
                    if (ihdrRow == 0) {
                        this.hdr[ic] = {
                            name: hdrCell,
                            rows: [hdrCell]
                        };
                    } else {
                        this.hdr[ic].name = this.hdr[ic].name.length ? this.hdr[ic].name + " " + hdrCell : hdrCell;
                        this.hdr[ic].rows.push(hdrCell);
                    }
                }
                ++startRow;
            }
        }
    
        for ( var ir=startRow; ir<_arr.length; ++ir)
            this.rows[ir-startRow]={ cols: _arr[ir], irow: ir-startRow };
    
        if(!this.rows.length)return;
    
        if( parsemode&vjTable_hasHeader ){
            for ( var ir=0; ir<this.rows.length; ++ir){
                for (var ic = this.hdr.length-1; ic >= 0; --ic) {
                    this.rows[ir][this.hdr[ic].name]=this.rows[ir].cols[ic];
                }
            }
            if( (parsemode&vjTable_collapsePropFormat) && this.hdr.length>=4 && this.hdr[0].name=="id" && this.hdr[1].name=="name" && this.hdr[2].name=="path" && this.hdr[3].name=="value" )
                this.convertPropToHorizontal( this.rows );
        }
    }
}

