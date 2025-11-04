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
import { Route, Routes, useLocation } from 'react-router-dom';
import { HomeTab } from './home-container';
import { MainTab } from './main-container';
import { HiveAbout } from './hive-about';
import { PortalRender } from '../portal/index';
import BcoEditor from '../bcoeditor/index';
import { Network } from '../network/index';
import { Uploader2App } from '../uploader2/index';
import { LavaApp } from '../lava/index';

function DynamicHomeTab() {
    const location = useLocation();
    //const pathWithoutQueryOrHash = location.pathname.split(/[?#]/)[0];

    return (
      <>
        {/tab=home/.test(location.search) ? <HomeTab selected='homenew'/> :
        /tab=about/.test(location.search) ? <HiveAbout selected='about'/> :
        /tab=apps/.test(location.search) ? PortalRender():
        /tab=bcoeditor/.test(location.search) ? <BcoEditor selected='bcoeditor'/> :
        /tab=network/.test(location.search) ? <Network selected='network'/> :
        /tab=uploader2/.test(location.search) ? <Uploader2App selected='uploader2'/> :
        /tab=lava/.test(location.search) ? <LavaApp selected='lava'/> :
        /tab=main/.test(location.search) ? <MainTab selected='about'/> : <HomeTab selected='homenew'/> }
      </>
    );
}

function AppContent() {
  return (
      <Routes>
        <Route path="*" element={<DynamicHomeTab />} />
      </Routes>
  );

//   return (
//     <div className="main-content">
//       <Routes>
//         <Route path="zzzzz/:path0/:path1/:path2/home" element={<HomeTab />} />
//         <Route path="/:path0/:path1/home" element={<HomeTab />} />
//         <Route path="/:path0/home" element={<HomeTab />} />
//         <Route path="*/home" element={<HomeTab />} />
//         <Route path="/" element={<HomeTab />} />
//         <Route path="/:path0/:path1/:path2/maintab" element={<HomeTab />} />
//         <Route path="/:path0/:path1/maintab" element={<HomeTab />} />
//         <Route path="/:path0/maintab" element={<HomeTab />} />
//         <Route path="/maintab" element={<MainTab />} />
//         <Route path="*" element={<MainTab />} />
//       </Routes>
//     </div>
//   );
}

export default AppContent;
