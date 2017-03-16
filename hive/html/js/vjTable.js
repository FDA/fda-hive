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
    This object is represented as an array

 vjTable :
     { rows { cols: Array, columnname1: value1,columnname2: value2  }
      hdr { name: 'headercolumn' }
    }


*/


var vjTable_hasHeader=0x0001;
var vjTable_collapsePropFormat=0x0002;
var vjTable_inheritPathPropFormat=0x0004;
var vjTable_propCSV=vjTable_hasHeader|vjTable_collapsePropFormat;


function vjTable( content, colfmt, parsemode, skiprows, cntrows, strDelimiterList, keepempty, cntHeaderRows )
{
    // This will parse a delimited string into an array of
    // arrays. The default delimiter is the comma, but this
    // can be overridden in the fifth argument.
    this.CSVToArray = function( arr, strData, colfmt, skiprows, cntrows, strDelimiterList, keepempty )
    {
        if (!strData || !strData.length) {
            return [];
        }
        // Check to see if the delimiter is defined. If not,
        // then default to comma.
        strDelimiterList = (strDelimiterList || ",");

        // Create a regular expression to parse the CSV values.
        var objPattern = new RegExp(
            (
                    // Delimiters.
                    "([" + strDelimiterList + "]|\\r?\\n|\\r|^)" +

                    // Quoted fields.
                    "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +

                    // Standard fields.
                    "([^\"" + strDelimiterList + "\\r\\n]*))"
            ),
            "gi"
            );


        // Create an array to hold our data. Give the array
        // a default empty first row.
        //var arr = [[]];

        // Create an array to hold our individual pattern
        // matching groups.
        var arrMatches = null;

        var strPattern = new RegExp("[" + strDelimiterList + "]");

        // Special case: strData starts with empty cell. We
        // cannot match it using (^|,|\r\n)(...*) because then
        // we will keep matching the empty string at the start
        // of content forever, with arrMatches.index always 0,
        // resulting in an infinite loop.
        if (strData.length && strData.charAt(0).match(strPattern)) {
            arr[arr.length - 1].push("");
        }

        // Keep looping over the regular expression matches
        // until we can no longer find a match.
        for(var it=0 ; arrMatches = objPattern.exec( strData ) ;  ) {

            // Get the delimiter that was found.
            var strMatchedDelimiter = arrMatches[ 1 ];

            // Check to see if the given delimiter has a length
            // (is not the start of string) and if it matches
            // field delimiter. If id does not, then we know
            // that this delimiter is a row delimiter.
            if ( strMatchedDelimiter.length && !strMatchedDelimiter.match(strPattern) ) {
                // Since we have reached a new row of data,
                // add an empty row to our data array.
                arr.push( [] );
                ++it;
            }

            // Now that we have our delimiter out of the way,
            // let's check to see which kind of value we
            // captured (quoted or unquoted).
            var strMatchedValue;
            if (arrMatches[ 2 ]){
                // We found a quoted value. When we capture
                // this value, unescape any double quotes.
                strMatchedValue = arrMatches[ 2 ].replace(
                    new RegExp( "\"\"", "g" ),
                    "\""
                    );
            } else {
                // We found a non-quoted value.
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
            // Now that we have our value string, let's add
            // it to the data array.
            arr[ arr.length - 1 ].push( strMatchedValue );
        }
        for ( var ir = 0; ir < arr.length; ++ir) {
            if (!keepempty && arr[ir].length <= 1) {
                // do we consider empty rows
                arr.splice(ir, 1);
                --ir;
                continue;
            }
        }
        if(skiprows) {
            // skip some rows ?
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
//                iCol=newhdr.length;
                newhdr.push({name: rows[ir].name });
                colMap[""+ rows[ir].name ]=newhdr.length;
            }

            if(!newrows[iNum][rows[ir].name])
                newrows[iNum][rows[ir].name]=rows[ir].value;
            else {

                if( !(newrows[iNum][rows[ir].name] instanceof Array) )
                    newrows[iNum][rows[ir].name]=new Array(newrows[iNum][rows[ir].name]);   // turn into array
                newrows[iNum][rows[ir].name].push(rows[ir].value);
            }
            if((parsemode&vjTable_inheritPathPropFormat) && isok(rows[ir].path))
                newrows[iNum][rows[ir].name+"."+rows[ir].path]=rows[ir].value;
        }

        this.rows=newrows;
        this.hdr=newhdr;
        //this.arr=new Array();
        for ( var ir=0; ir<this.rows.length; ++ir){
            this.rows[ir].cols=new Array();
            for ( var ic=0; ic<this.hdr.length; ++ic){
                var iCol=colMap[this.hdr[ic].name]-1;
                this.rows[ir].cols[iCol]=this.rows[ir][this.hdr[ic].name];
            }
        }


        //alert(jjj(this.rows));
        //alert(jjj(this.hdr)+"\n\n\n"+jjj(this.arr));

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
        
        //alert(jjj(this.hdr));
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
                    if (this.rows[ifound].name //&& typeof(this.rows[ifound].name)=="string"
                            && this.rows[ifound].name.match(addrows[ic].name) && !addrows[ic].noMatch) //this last flag is for special cases (when 2 names might match, but are not the same)
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

            if ((howmanyfound == 0 || !addrows[ic].prohibit_new) && allownew  && typeof(addrows[ic].name)=="string"){
                if( !objCpy ) {
                    this.rows.push( addrows[ic] );
                }
                else {
                    this.rows.push(cpyObj(addrows[ic]));
                }
            }
        }
    };

/*
 * this.customizeRows=function( addrows , allownew ) { for( var ir=0; ir<addrows.length;
 * ++ir) {
 *
 * var ifound=parseInt(addrows[ir].row), ff; if(isNaN(ifound)) { if(allownew ) {
 * ifound=this.rows.length; this.rows[ifound]={cols:new Array()}; } else
 * ifound=ir; } if(ifound<0 || ifound>=this.rows.length)continue;
 *
 * for ( ff in addrows[ir] ) { this.rows[ifound][ff]=addrows[ir][ff]; } } }
 */
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
                // ensure syncing only overwrites first column if we have multiple
                // columns with same name
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
            for( is=0; is<this.hdr.length ; ++is ) { // look for duplicate column name
                 if(colnameToAppend==this.hdr[is].name )
                     break;
            }
            
            if(is<this.hdr.length) { // when found duplication
                continue;
            }
            else  { 
                this.hdr.push(secondTable.hdr[ic]);
            }
            var ir;
            for(ir=0; ir<secondTable.rows.length; ++ir){ // adding rows
                var valueToAdd = secondTable.rows[ir][colnameToAppend];
                if (ir > this.rows.length-1){ // when table to append has more rows than the current => set the row elements from current table to 0 
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
            // in case the table to append has less rows than the current
            // => add emtpy value 
            for (; ir<this.rows.length; ++ir){
                this.rows[ir][colnameToAppend]=0;
                this.rows[ir].cols.push(0);
            }
        }
    };
    
    this.arrayToCSV = function (){
        var strToReturn = "";
        
        if (this.hdr.length > 0){ // if there is a separate row header
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
            
    var _arr=this.CSVToArray( new Array ( new Array() ) , content, colfmt, skiprows, cntrows, strDelimiterList, keepempty  );

    var startRow=0;
    this.hdr=new Array();
    this.rows=new Array();
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
            // ensure this.rows[ir][name] is set to *first* column with that name
            for (var ic = this.hdr.length-1; ic >= 0; --ic) {
                this.rows[ir][this.hdr[ic].name]=this.rows[ir].cols[ic];
            }
        }
        if( (parsemode&vjTable_collapsePropFormat) && this.hdr.length>=4 && this.hdr[0].name=="id" && this.hdr[1].name=="name" && this.hdr[2].name=="path" && this.hdr[3].name=="value" )
            this.convertPropToHorizontal( this.rows );
    }
}

//# sourceURL = getBaseUrl() + "/js/vjTable.js"