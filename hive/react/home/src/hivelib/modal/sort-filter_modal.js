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
///////////////////////////
/// Sort alphabetically ///
//////////////////////////
export const sortAlphAndCount = (array, name = false , count = false) => {
    //simple sort
    if(typeof name !== 'string' && typeof count !== 'string') return array.sort();
    array.sort((a,b) => {

        //a|b[count] can be a number, string representing number, array ,object => number
        function countToNun(n){
            if(Array.isArray(n)) return n.length;
            if(typeof n === 'number') return n;
            if(typeof n === 'string') return parseInt(n);
            if(typeof n === 'object') return Object.keys(n).length;
            return 0;
        }

        let a_count = countToNun(a[count]);
        let b_count = countToNun(b[count]);

        if(count && ( ( (a_count === 0 || b_count === 0) && a_count !== b_count ) || !name ) ){
           return a_count < b_count ? 1 : a_count > b_count ? -1 : 0;
        }
        if(!count || (count && ((a_count === 0 && a_count === b_count) || (a_count !== 0 && b_count !== 0))) ){
           let a_name = typeof a[name] === 'number' ? a[name].toString() : a[name];
           let b_name = typeof b[name] === 'number' ? b[name].toString() : b[name];

           // Non case sensitive sort
           if(typeof a_name === 'string' && typeof b_name === 'string'){
               return a_name.toLowerCase() > b_name.toLowerCase() ? 1 : a_name.toLowerCase() < b_name.toLowerCase() ? -1 : 0;
           }
           if(typeof a_name === 'string') return 1;
           if(typeof b_name === 'string') return -1;
           return 0;
        }
        return 0 ;

    })

    return array;
}

export const filterDuplicatesArr = (arr) =>{
    return arr.filter((item, index) => arr.indexOf(item) === index);
}

//////////////////
/// Filter Out ///
//////////////////
export const filterOut = ( arr , query ) =>{
    return arr.filter( el => query.includes(el._type))
}