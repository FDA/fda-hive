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
import JsonTab from "./tabs/json_tab";
import ProgressTab from "./tabs/progress_tab";
import ShareTab from "./tabs/share_tab";
import DownloadsTab from "./tabs/downloads_tab";
import ImageTab from "./tabs/image_tab";
import PreviewTab from "./tabs/preview_tab";
import TablePreviewTab from "./tabs/table_preview_tab";
import SequencesTab from "./tabs/sequences_tab";
import HistogramTab from "./tabs/histogram_tab";
import ACGTTab from "./tabs/acgt_tab";
import CodonTab from "./tabs/codon_tab";
import PositionalQCTab from "./tabs/positionalqc_tab";
import EigenvectorsTab from "./tabs/eigenvectors_tab";
import ProfileTab from "./tabs/profile_tab";
import AnnotaionTab from "./tabs/annotation_tab";
import DetailTab from "./tabs/detail_tab";

const components = {
        json: {
            component: JsonTab,
            title: 'JSON'
        },
        progress: {
            component: ProgressTab,
            title: 'Progress'
        },
        sharing: {
            component: ShareTab,
            title: 'Sharing'
        },
        downloads: {
            component: DownloadsTab,
            title: 'Download All'
        },
        image: {
            component: ImageTab,
            title: 'Image'
        },
        custom: {
            component: PreviewTab,
            title: 'Preview'
        },
        table_preview: {
            component: TablePreviewTab,
            title: 'Table Preview'
        },
        sequences: {
            component: SequencesTab,
            title: 'Sequences'
        },
        histogram: {
            component: HistogramTab,
            title: 'Histogram'
        },
        acgt: {
            component: ACGTTab,
            title: 'ACGT'
        },
        codon: {
            component: CodonTab,
            title: 'Codon QC'
        },
        positionalqc: {
            component: PositionalQCTab,
            title: 'Positional QC'
        },
        eigenvectors: {
            component: EigenvectorsTab,
            title: 'Eigenvectors'
        },
        profile: {
            component: ProfileTab,
            title: 'Profile'
        },
        annotation: {
            component: AnnotaionTab,
            title: 'Annotation'
        },
        detail: {
            component: DetailTab,
            title: 'Details'
        }
};

export default components;