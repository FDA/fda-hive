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
#pragma once
#ifndef sLib_utils_sort_hpp
#define sLib_utils_sort_hpp

#include <slib/core/vec.hpp>

namespace slib {

        class sSort
        {

        public:
            typedef idx (* sCallbackSorter)(void * param, void * arr, idx i1, idx oper, idx i2 );
            typedef idx (* sCallbackSorterSimple)(void * param, void * arr, idx i1, idx i2 );

            typedef idx (* sCallbackSearch)(void * param, void * arr1, void * arr2);


            static idx sort_stringComparator(void * param, void * arr, idx i1, idx oper, idx i2 );
            static idx sort_stringNaturalComparator(void * param, void * arr, idx i1, idx oper, idx i2 );
            static idx sort_stringNaturalCaseComparator(void * param, void * arr, idx i1, idx oper, idx i2 );
            static void sortStringsInd(idx n, const char * * arr, idx * ind);
            static void sortStringsNaturalInd(idx n, const char * * arr, idx * ind)
            {
                sSort::sortCallback((sCallbackSorter) (sort_stringNaturalComparator), 0, n, arr, ind);
            }
            static void sortStringsNaturalCaseInd(idx n, const char * * arr, idx * ind)
            {
                sSort::sortCallback((sCallbackSorter) (sort_stringNaturalCaseComparator), 0, n, arr, ind);
            }
            static idx sort_idxDicComparator(void * param, void * arr, idx i1, idx oper, idx i2 );
            static idx sort_stringsDicID(void * param, void * arr, idx i1, idx i2 );

            struct Lenstats
            {
                idx num;
                idx sum;
            };

            enum sSortCompare {
                eSort_EQ=0,
                eSort_LT,
                eSort_LE,
                eSort_GT,
                eSort_GE
            };

            typedef enum sSearchHitType_enum {
                eSearch_Any=0,
                eSearch_First,
                eSearch_Last
            } sSearchHitType;


            #define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
            #define M 7
            #define NSTACK 64
            #define SWAPI(a,b) itemp=(a);(a)=(b);(b)=itemp;
            template <class Tobj> static void sort(idx n, Tobj * arr)
            {
                //--arr;
                idx i,ir=n-1,j,k,l=0;
                idx jstack=0;
                Tobj a,temp;
                sVec <idx > istack; istack.add(NSTACK+1);istack.set(0); //istack=lvector(1,NSTACK);
                // istack=lvector(1,NSTACK);
                for (;;) {

                    if (ir-l < M) {

                        for (j=l+1;j<=ir;j++) {
                            a=arr[j];
                            for (i=j-1;i>=l;i--) {
                                if (arr[i] <= a) break;
                                arr[i+1]=arr[i];
                            }
                            arr[i+1]=a;
                        }
                        if (jstack == 0 )
                            break;
                        ir=istack[jstack--];
                        l=istack[jstack--];
                    } else {

                        k=(l+ir) >> 1;
                        SWAP(arr[k],arr[l+1])
                        if (arr[l] > arr[ir]) {
                            SWAP(arr[l],arr[ir])
                        }
                        if (arr[l+1] > arr[ir]) {
                            SWAP(arr[l+1],arr[ir])
                        }
                        if (arr[l] > arr[l+1]) {
                            SWAP(arr[l],arr[l+1])
                        }
                        i=l+1;
                        j=ir;
                        a=arr[l+1];
                        for (;;) {
                            do i++; while (i <= ir && arr[i] < a);
                            do j--; while (j >= l+1 && arr[j] > a);
                            if (j < i) break;
                            SWAP(arr[i],arr[j]);
                        }
                        if(l+1!=j)
                            arr[l+1]=arr[j];
                        arr[j]=a;
                        jstack += 2;
                        if (jstack >= NSTACK)
                            return; ///nrerror("NSTACK too small in sort.");

                        if (ir-i+1 >= j-l) {
                            istack[jstack]=ir;
                            istack[jstack-1]=i;
                            ir=j-1;
                        } else {
                            istack[jstack]=j-1;
                            istack[jstack-1]=l;
                            l=i;
                        }
                    }
                }
            //      free_lvector(istack,1,NSTACK);
            }


            template <class Tobj, class Tind > static void sort(idx n, Tobj * arr, Tind * ind)
            {
                idx i,ir=n-1,j,k,l=0;
                idx jstack=0;
                //Tobj temp;
                idx inda,itemp;

                //--ind;
                //--arr;
                for(i=0;i<n;++i) ind[i]=(Tind)i;

                sVec <idx > istack; istack.add(NSTACK+1);istack.set(0); //istack=lvector(1,NSTACK);
                // istack=lvector(1,NSTACK);
                for (;;) {
                    if (ir-l < M) {
                        for (j=l+1;j<=ir;j++) {
                            inda=ind[j];
                            for (i=j-1;i>=l;i--) {
                                if (arr[ind[i]] <= arr[inda]) break;
                                ind[i+1]=ind[i];
                            }
                            ind[i+1]=(Tind)inda;
                        }
                        if (jstack == 0) break;
                        ir=istack[jstack--];
                        l=istack[jstack--];
                    } else {
                        k=(l+ir) >> 1;
                        SWAPI(ind[k],ind[l+1])
                        if (arr[ind[l]] > arr[ind[ir]]) {
                            SWAPI(ind[l],ind[ir])
                        }
                        if (arr[ind[l+1]] > arr[ind[ir]]) {
                            SWAPI(ind[l+1],ind[ir])
                        }
                        if (arr[ind[l]] > arr[ind[l+1]]) {
                            SWAPI(ind[l],ind[l+1])
                        }
                        i=l+1;
                        j=ir;
                        inda=ind[l+1];
                        for (;;) {
                            do i++; while (i <= ir && arr[ind[i]] < arr[inda]);
                            do j--; while (j >= l+1 && arr[ind[j]] > arr[inda]);
                            if (j < i) break;
                            SWAPI(ind[i],ind[j]);
                        }
                        ind[l+1]=ind[j];
                        ind[j]=(Tind)inda;
                        jstack += 2;
                        if (jstack >= NSTACK)
                            return;//nrerror("NSTACK too small in sort.");
                        if (ir-i+1 >= j-l) {
                            istack[jstack]=ir;
                            istack[jstack-1]=i;
                            ir=j-1;
                        } else {
                            istack[jstack]=j-1;
                            istack[jstack-1]=l;
                            l=i;
                        }
                    }
                }
            //      free_lvector(istack,1,NSTACK);
            }

            template <class Tobj, class Tind > static void sortabs(idx n, Tobj * arr, Tind * ind)
            {
                idx i,ir=n-1,j,k,l=0;
                idx jstack=0;
                //Tobj temp;
                idx inda,itemp;

                //--ind;
                //--arr;
                for(i=0;i<n;++i) ind[i]=(Tind)i;

                sVec <idx > istack; istack.add(NSTACK+1);istack.set(0); //istack=lvector(1,NSTACK);
                // istack=lvector(1,NSTACK);
                for (;;) {
                    if (ir-l < M) {
                        for (j=l+1;j<=ir;j++) {
                            inda=ind[j];
                            for (i=j-1;i>=l;i--) {
                                if (sAbs(arr[ind[i]]) <= sAbs(arr[inda])) break;
                                ind[i+1]=ind[i];
                            }
                            ind[i+1]=(Tind)inda;
                        }
                        if (jstack == 0) break;
                        ir=istack[jstack--];
                        l=istack[jstack--];
                    } else {
                        k=(l+ir) >> 1;
                        SWAPI(ind[k],ind[l+1])
                        if (sAbs(arr[ind[l]]) > sAbs(arr[ind[ir]])) {
                            SWAPI(ind[l],ind[ir])
                        }
                        if (sAbs(arr[ind[l+1]]) > sAbs(arr[ind[ir]])) {
                            SWAPI(ind[l+1],ind[ir])
                        }
                        if (sAbs(arr[ind[l]]) > sAbs(arr[ind[l+1]])) {
                            SWAPI(ind[l],ind[l+1])
                        }
                        i=l+1;
                        j=ir;
                        inda=ind[l+1];
                        for (;;) {
                            do i++; while (i <= ir && sAbs(arr[ind[i]]) < sAbs(arr[inda]));
                            do j--; while (j >= l+1 && sAbs(arr[ind[j]]) > sAbs(arr[inda]));
                            if (j < i) break;
                            SWAPI(ind[i],ind[j]);
                        }
                        ind[l+1]=ind[j];
                        ind[j]=(Tind)inda;
                        jstack += 2;
                        if (jstack >= NSTACK)
                            return;//nrerror("NSTACK too small in sort.");
                        if (ir-i+1 >= j-l) {
                            istack[jstack]=ir;
                            istack[jstack-1]=i;
                            ir=j-1;
                        } else {
                            istack[jstack]=j-1;
                            istack[jstack-1]=l;
                            l=i;
                        }
                    }
                }
            //      free_lvector(istack,1,NSTACK);
            }



            template <class Tobj, class Tind > static void sortCallback(sCallbackSorter compare , void * param, idx n, Tobj * arr, Tind * ind)
            {
                idx i,ir=n-1,j,k,l=0;
                idx jstack=0;
                //Tobj temp;
                idx inda,itemp;

                //--ind;
                //--arr;
                for(i=0;i<n;++i) ind[i]=(Tind)i;

                sVec <idx > istack; istack.add(NSTACK+1);istack.set(0); //istack=lvector(1,NSTACK);
                // istack=lvector(1,NSTACK);
                for (;;) {
                    if (ir-l < M) {
                        for (j=l+1;j<=ir;j++) {
                            inda=ind[j];
                            for (i=j-1;i>=l;i--) {
                                if (compare ( param, arr, ind[i], eSort_LE, inda) ) break;
                                ind[i+1]=ind[i];
                            }
                            ind[i+1]=(Tind)inda;
                        }
                        if (jstack == 0) break;
                        ir=istack[jstack--];
                        l=istack[jstack--];
                    } else {
                        k=(l+ir) >> 1;
                        SWAPI(ind[k],ind[l+1])
                        if (compare(param,arr, ind[l], eSort_GT,ind[ir]) ) {
                            SWAPI(ind[l],ind[ir])
                        }
                        if (compare(param,arr, ind[l+1],eSort_GT, ind[ir]) ) {
                            SWAPI(ind[l+1],ind[ir])
                        }
                        if (compare(param,arr, ind[l], eSort_GT, ind[l+1])) {
                            SWAPI(ind[l],ind[l+1])
                        }
                        i=l+1;
                        j=ir;
                        inda=ind[l+1];
                        for (;;) {
                            do i++; while (i <= ir && compare( param ,arr, ind[i], eSort_LT, inda ) );
                            do j--; while (j >= l+1 && compare( param ,arr, ind[j], eSort_GT, inda));
                            if (j < i) break;
                            SWAPI(ind[i],ind[j]);
                        }
                        ind[l+1]=ind[j];
                        ind[j]=(Tind)inda;
                        jstack += 2;
                        if (jstack > NSTACK) return;//nrerror("NSTACK too small in sort.");
                        if (ir-i+1 >= j-l) {
                            istack[jstack]=ir;
                            istack[jstack-1]=i;
                            ir=j-1;
                        } else {
                            istack[jstack]=j-1;
                            istack[jstack-1]=l;
                            l=i;
                        }
                    }
                }
            //      free_lvector(istack,1,NSTACK);
            }

            template <class Tobj> static void sortSimpleCallback(sCallbackSorterSimple compare , void * param, idx n, Tobj * arr)
            {
                //--arr;
                idx i,ir=n-1,j,k,l=0;//,rs=0;
                idx jstack=0;
                Tobj a,temp;
                sVec <idx > istack; istack.add(NSTACK+1);istack.set(0);// 2*(n+1));istack.set(0); //istack=lvector(1,NSTACK);
                // istack=lvector(1,NSTACK);
                for (;;) {

                    if (ir-l < M) {

                        for (j=l+1;j<=ir;j++) {
                            // Unwrap one level of insertion sort inner loop to ensure that
                            // arr[j] is not overwritten while we are still comparing it to arr[i]
                            if (compare(param, arr, j-1, j) <= 0) continue;
                            a=arr[j-1];
                            for (i=j-2;i>=l;i--) {
                                if (compare (param, arr, i, j) <= 0) break;
                                arr[i+1]=arr[i];
                            }
                            arr[i+1]=arr[j];
                            arr[j]=a;
                        }
                        if (jstack == 0)
                            break;
                        ir=istack[jstack--];
                        l=istack[jstack--];

                    } else {

                        k=(l+ir) >> 1;
                        SWAP(arr[k],arr[l+1])
                        if (compare ( param, arr,l, ir)>0) {
                            SWAP(arr[l],arr[ir])
                        }
                        if (compare ( param, arr,l+1, ir)>0) {
                            SWAP(arr[l+1],arr[ir])
                        }
                        if (compare ( param, arr,l, l+1)>0) {
                            SWAP(arr[l],arr[l+1])
                        }
                        i=l+1;
                        j=ir;
                        a=arr[l+1];
                        for (;;) {
                            do i++; while (i <= ir && compare ( param, arr,i, l+1)<0);
                            do j--; while (j >= l+1 && compare ( param, arr,j, l+1)>0);
                            if (j < i) break;
                            SWAP(arr[i],arr[j]);
                        }
                        if(l+1!=j)
                            arr[l+1]=arr[j];
                        arr[j]=a;
                        jstack += 2;
                        if (jstack >= NSTACK)
                            return; ///nrerror("NSTACK too small in sort.");
                        if (ir-i+1 >= j-l) {
                            istack[jstack]=ir;
                            istack[jstack-1]=i;
                            ir=j-1;
                        } else {
                            istack[jstack]=j-1;
                            istack[jstack-1]=l;
                            l=i;
                        }
                    }
                }
            }

            template <class Tobj, class Tind> static void sortSimpleCallback(sCallbackSorterSimple compare , void * param, idx n, Tobj * arr, Tind *ind)
            {
                idx i,ir=n-1,j,k,l=0;
                idx jstack=0;
                //Tobj temp;
                idx inda,itemp;

                //--ind;
                //--arr;
                for(i=0;i<n;++i) ind[i]=i;

                sVec <idx > istack; istack.add(NSTACK+1);istack.set(0); //istack=lvector(1,NSTACK);
                // istack=lvector(1,NSTACK);
                for (;;) {
                    if (ir-l < M) {
                        for (j=l+1;j<=ir;j++) {
                            inda=ind[j];
                            for (i=j-1;i>=l;i--) {
                                if (compare ( param, arr, ind[i], inda)<=0 ) break;
                                ind[i+1]=ind[i];
                            }
                            ind[i+1]=inda;
                        }
                        if (jstack == 0) break;
                        ir=istack[jstack--];
                        l=istack[jstack--];
                    } else {
                        k=(l+ir) >> 1;
                        SWAPI(ind[k],ind[l+1])
                        if (compare(param,arr, ind[l],ind[ir])>0 ) {
                            SWAPI(ind[l],ind[ir])
                        }
                        if (compare(param,arr, ind[l+1], ind[ir])>0 ) {
                            SWAPI(ind[l+1],ind[ir])
                        }
                        if (compare(param,arr, ind[l], ind[l+1])>0 ) {
                            SWAPI(ind[l],ind[l+1])
                        }
                        i=l+1;
                        j=ir;
                        inda=ind[l+1];
                        for (;;) {
                            do i++; while (i <= ir && compare( param ,arr, ind[i], inda )<0);
                            do j--; while (j >= l+1 && compare( param ,arr, ind[j], inda)>0);
                            if (j < i) break;
                            SWAPI(ind[i],ind[j]);
                        }
                        ind[l+1]=ind[j];
                        ind[j]=inda;
                        jstack += 2;
                        if (jstack >= NSTACK) return;//nrerror("NSTACK too small in sort.");
                        if (ir-i+1 >= j-l) {
                            istack[jstack]=ir;
                            istack[jstack-1]=i;
                            ir=j-1;
                        } else {
                            istack[jstack]=j-1;
                            istack[jstack-1]=l;
                            l=i;
                        }
                    }
                }
            }

            template<class Tobj, class Tind> static void reorder(idx n, Tobj * arr, Tind * ind)
            {
                Tobj temp;
                Tind itemp, itemp1;
                for(int i = 0; i < n; i++) {
                    while( ind[i] != i ) {
                        SWAP(arr[ind[i]], arr[i])
                        itemp1 = ind[i];
                        SWAPI(ind[i], ind[itemp1])
                    }
                }
            }

            #undef M
            #undef NSTACK
            #undef SWAP
            #undef SWAPI

            template<class Tobj > static idx binarySearch(sCallbackSearch compare, void * param, Tobj * cmp, idx n, Tobj * arr, idx * ind = 0, sSearchHitType hitType = eSearch_Any, bool approximate = false)
            {
                idx mid = 0, min = 0, max = n - 1, cmpcli = -1, cmpres = 0;
                bool isExactHit = false;
                while( max >= min ) {
                    mid = min + ((max - min) / 2);
                    cmpres = compare(param, cmp, &arr[ind ? ind[mid] : mid]); //do not change the order of comparison
                    if( cmpres > 0 )
                        min = mid + 1;
                    else if( cmpres < 0 )
                        max = mid - 1;
                    else {
                        isExactHit = true;
                        if( hitType == eSearch_Last && max != mid ) {
                            min = mid;
                        } else if( hitType == eSearch_First && min != mid ) {
                            max = mid;
                        } else {
                            cmpcli = mid;
                            break;
                        }
                    }
                }
                if( !isExactHit && approximate && (min > max) ) {
                    if( max < 0) return 0;
                    if( min >= n ) return n-1;
                    idx cmpmin = compare(param, cmp, &arr[ind ? ind[min] : min]), //do not change the order of comparison
                        cmpmax = compare(param, cmp, &arr[ind ? ind[max] : max]); //do not change the order of comparison
                    if( cmpmin < 0 )
                        cmpmin = -cmpmin;
                    if( cmpmax < 0 )
                        cmpmax = -cmpmax;
                    cmpres = cmpmin;
                    cmpcli = min;
                    if( cmpmax < cmpres ) {
                        cmpcli = max;
                        cmpres = cmpmax;
                    }
                    if( min ) {
                        idx cmpmin1 = compare(param, cmp, &arr[ind ? ind[min - 1] : (min - 1)]); //do not change the order of comparison
                        if( cmpmin1 < 0 )
                            cmpmin1 = -cmpmin1;
                        if( cmpres > cmpmin1 ) {
                            cmpres = cmpmin1;
                            cmpcli = min - 1;
                        }
                    }
                    if( max + 1 < n ) {
                        idx cmpmax1 = compare(param, cmp, &arr[ind ? ind[max + 1] : (max + 1)]); //do not change the order of comparison
                        if( cmpmax1 < 0 )
                            cmpmax1 = -cmpmax1;
                        if( cmpres > cmpmax1 ) {
                            cmpcli = max - +1;
                        }
                    }
                }
                return cmpcli;
            }
    };
} // namespace slib

#endif
