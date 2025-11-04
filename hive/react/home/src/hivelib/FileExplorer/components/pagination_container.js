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
import React from "react";
import { Pagination } from "antd";

const PaginationView =  (props) => {
    const { pagerInfo , onPagerSelect, tableSpinner } = props;
    const total = !pagerInfo ? 0 : pagerInfo.total;
    let currentPage = 1;

    let newPagerInfo = !pagerInfo ? {} : {
        total: pagerInfo.total,
        start: pagerInfo.start,
        cnt: pagerInfo.cnt
    };

    const onChange = (page, pageSize) => {
        if (tableSpinner) return;
        newPagerInfo.start = pageSize * page - pageSize;
        onPagerSelect(newPagerInfo);
    };

    const onShowSizeChange = (current,size) => {
        if( size > pagerInfo.total){
            newPagerInfo.start = 0;
        }
        const mod = pagerInfo.total % size;
        const diff = pagerInfo.total - pagerInfo.start;
        if(diff < mod){
            newPagerInfo.start = pagerInfo.total - mod;
        }
        newPagerInfo.cnt = size;
        onPagerSelect(newPagerInfo);
    };

    currentPage = (newPagerInfo.start + newPagerInfo.cnt)/newPagerInfo.cnt;
    currentPage = Math.floor(currentPage);
    //console.log('----- Pagination rerender')
    return(
        <Pagination
            current={currentPage}
            onChange={onChange}
            onShowSizeChange={onShowSizeChange}
            size="small"
            total={total}
            showSizeChanger
            showQuickJumper
            pageSize={newPagerInfo.cnt}
            pageSizeOptions={['10','20','50','100','1000']}
        />
    );
}

export const PaginationContainer = React.memo(PaginationView);