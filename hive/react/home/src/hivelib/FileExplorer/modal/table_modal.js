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
export const columnsReorder = (current_table_types , column_orders, type_columns) => {

   /// brief : 30 + 0 + 50
   /// add values of _attribute orders to get an avarage of the order across all types/file types.

   let columns_reorder;
   let new_order = {};

   current_table_types.forEach( (type ,index) => {
       let order = column_orders[type].order;
       if(index === 0){
           new_order._id = -1000;
       }
       type_columns.forEach( col => {
           if(col !== "_id"){
               if(Object.keys(new_order).includes(col)){
                  new_order[col] = new_order[col] + order[col];
               }else{
                   new_order[col] =  {}
                   new_order[col] = order[col];
               }
           }
       })
   })

   /// converting from KEY VALUE into an ARRAY
   columns_reorder = Object.keys(new_order).map(function(key) {
            return [key, new_order[key]];
   });
   columns_reorder.sort( (a,b)=> {
       if(a[1] === b[1] ){
            if (a[0] < b[0]) {
                return -1;
            } else if (a[0] > b[0]) {
                return 1;
            } else if (a[0] === b[0]) {
               // names must be equal
                return 0;
            }
       }
       return a[1] - b[1];

   })

   /// columns_reorder filter out order numbers
   /// [["_id", 0],["_dir", 2],["brief", 4]]
   /// ["_id","_dir","brief"]
   columns_reorder = columns_reorder.map(el => el[0]);
   columns_reorder = columns_reorder.filter(el => el !== "_type");
   columns_reorder.unshift("_type");
   return columns_reorder;
}