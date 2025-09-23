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

#include <ssci/bio/bioseq.hpp>
#include <ssci/math/clust/clust2.hpp>

using namespace slib;

#define BLAST_AA_MATRIX_NLETTERS 24
static const char blast_aa_matrix_letters[BLAST_AA_MATRIX_NLETTERS + 1] = "ARNDCQEGHILKMFPSTWYVBZX*";

static const idx blast_aa_matrix[][BLAST_AA_MATRIX_NLETTERS * (BLAST_AA_MATRIX_NLETTERS + 1) / 2] = {
    {
        5, -2, -1, -2, -1, -1, -1, 0, -2, -1, -1, -1, -1, -2, -1, 1, 0, -2, -2, 0, -1, -1, 0, -5,
        7, 0, -1, -3, 1, 0, -2, 0, -3, -2, 3, -1, -2, -2, -1, -1, -2, -1, -2, -1, 0, -1, -5,
        6, 2, -2, 0, 0, 0, 1, -2, -3, 0, -2, -2, -2, 1, 0, -4, -2, -3, 4, 0, -1, -5,
        7, -3, 0, 2, -1, 0, -4, -3, 0, -3, -4, -1, 0, -1, -4, -2, -3, 5, 1, -1, -5,
        12, -3, -3, -3, -3, -3, -2, -3, -2, -2, -4, -1, -1, -5, -3, -1, -2, -3, -2, -5,
        6, 2, -2, 1, -2, -2, 1, 0, -4, -1, 0, -1, -2, -1, -3, 0, 4, -1, -5,
        6, -2, 0, -3, -2, 1, -2, -3, 0, 0, -1, -3, -2, -3, 1, 4, -1, -5,
        7, -2, -4, -3, -2, -2, -3, -2, 0, -2, -2, -3, -3, -1, -2, -1, -5,
        10, -3, -2, -1, 0, -2, -2, -1, -2, -3, 2, -3, 0, 0, -1, -5,
        5, 2, -3, 2, 0, -2, -2, -1, -2, 0, 3, -3, -3, -1, -5,
        5, -3, 2, 1, -3, -3, -1, -2, 0, 1, -3, -2, -1, -5,
        5, -1, -3, -1, -1, -1, -2, -1, -2, 0, 1, -1, -5,
        6, 0, -2, -2, -1, -2, 0, 1, -2, -1, -1, -5,
        8, -3, -2, -1, 1, 3, 0, -3, -3, -1, -5,
        9, -1, -1, -3, -3, -3, -2, -1, -1, -5,
        4, 2, -4, -2, -1, 0, 0, 0, -5,
        5, -3, -1, 0, 0, -1, 0, -5,
        15, 3, -3, -4, -2, -2, -5,
        8, -1, -2, -2, -1, -5,
        5, -3, -3, -1, -5,
        4, 2, -1, -5,
        4, -1, -5,
        -1, -5,
        1
    },
    {
        5, -2, -1, -2, -1, -1, -1, 0, -2, -1, -2, -1, -1, -3, -1, 1, 0, -3, -2, 0, -2, -1, -1, -5,
        7, -1, -2, -4, 1, 0, -3, 0, -4, -3, 3, -2, -3, -3, -1, -1, -3, -1, -3, -1, 0, -1, -5,
        7, 2, -2, 0, 0, 0, 1, -3, -4, 0, -2, -4, -2, 1, 0, -4, -2, -3, 4, 0, -1, -5,
        8, -4, 0, 2, -1, -1, -4, -4, -1, -4, -5, -1, 0, -1, -5, -3, -4, 5, 1, -1, -5,
        13, -3, -3, -3, -3, -2, -2, -3, -2, -2, -4, -1, -1, -5, -3, -1, -3, -3, -2, -5,
        7, 2, -2, 1, -3, -2, 2, 0, -4, -1, 0, -1, -1, -1, -3, 0, 4, -1, -5,
        6, -3, 0, -4, -3, 1, -2, -3, -1, -1, -1, -3, -2, -3, 1, 5, -1, -5,
        8, -2, -4, -4, -2, -3, -4, -2, 0, -2, -3, -3, -4, -1, -2, -2, -5,
        10, -4, -3, 0, -1, -1, -2, -1, -2, -3, 2, -4, 0, 0, -1, -5,
        5, 2, -3, 2, 0, -3, -3, -1, -3, -1, 4, -4, -3, -1, -5,
        5, -3, 3, 1, -4, -3, -1, -2, -1, 1, -4, -3, -1, -5,
        6, -2, -4, -1, 0, -1, -3, -2, -3, 0, 1, -1, -5,
        7, 0, -3, -2, -1, -1, 0, 1, -3, -1, -1, -5,
        8, -4, -3, -2, 1, 4, -1, -4, -4, -2, -5,
        10, -1, -1, -4, -3, -3, -2, -1, -2, -5,
        5, 2, -4, -2, -2, 0, 0, -1, -5,
        5, -3, -2, 0, 0, -1, 0, -5,
        15, 2, -3, -5, -2, -3, -5,
        8, -1, -3, -2, -1, -5,
        5, -4, -3, -1, -5,
        5, 2, -1, -5,
        5, -1, -5,
        -1, -5,
        1
    },
    {
        4, -1, -2, -2, 0, -1, -1, 0, -2, -1, -1, -1, -1, -2, -1, 1, 0, -3, -2, 0, -2, -1, 0, -4,
        5, 0, -2, -3, 1, 0, -2, 0, -3, -2, 2, -1, -3, -2, -1, -1, -3, -2, -3, -1, 0, -1, -4,
        6, 1, -3, 0, 0, 0, 1, -3, -3, 0, -2, -3, -2, 1, 0, -4, -2, -3, 3, 0, -1, -4,
        6, -3, 0, 2, -1, -1, -3, -4, -1, -3, -3, -1, 0, -1, -4, -3, -3, 4, 1, -1, -4,
        9, -3, -4, -3, -3, -1, -1, -3, -1, -2, -3, -1, -1, -2, -2, -1, -3, -3, -2, -4,
        5, 2, -2, 0, -3, -2, 1, 0, -3, -1, 0, -1, -2, -1, -2, 0, 3, -1, -4,
        5, -2, 0, -3, -3, 1, -2, -3, -1, 0, -1, -3, -2, -2, 1, 4, -1, -4,
        6, -2, -4, -4, -2, -3, -3, -2, 0, -2, -2, -3, -3, -1, -2, -1, -4,
        8, -3, -3, -1, -2, -1, -2, -1, -2, -2, 2, -3, 0, 0, -1, -4,
        4, 2, -3, 1, 0, -3, -2, -1, -3, -1, 3, -3, -3, -1, -4,
        4, -2, 2, 0, -3, -2, -1, -2, -1, 1, -4, -3, -1, -4,
        5, -1, -3, -1, 0, -1, -3, -2, -2, 0, 1, -1, -4,
        5, 0, -2, -1, -1, -1, -1, 1, -3, -1, -1, -4,
        6, -4, -2, -2, 1, 3, -1, -3, -3, -1, -4,
        7, -1, -1, -4, -3, -2, -2, -1, -2, -4,
        4, 1, -3, -2, -2, 0, 0, 0, -4,
        5, -2, -2, 0, -1, -1, 0, -4,
        11, 2, -3, -4, -3, -2, -4,
        7, -1, -3, -2, -1, -4,
        4, -3, -2, -1, -4,
        4, 1, -1, -4,
        4, -1, -4,
        -1, -4,
        1
    },
    {
        7, -3, -3, -3, -1, -2, -2, 0, -3, -3, -3, -1, -2, -4, -1, 2, 0, -5, -4, -1, -3, -2, -1, -8,
        9, -1, -3, -6, 1, -1, -4, 0, -5, -4, 3, -3, -5, -3, -2, -2, -5, -4, -4, -2, 0, -2, -8,
        9, 2, -5, 0, -1, -1, 1, -6, -6, 0, -4, -6, -4, 1, 0, -7, -4, -5, 5, -1, -2, -8,
        10, -7, -1, 2, -3, -2, -7, -7, -2, -6, -6, -3, -1, -2, -8, -6, -6, 6, 1, -3, -8,
        13, -5, -7, -6, -7, -2, -3, -6, -3, -4, -6, -2, -2, -5, -5, -2, -6, -7, -4, -8,
        9, 3, -4, 1, -5, -4, 2, -1, -5, -3, -1, -1, -4, -3, -4, -1, 5, -2, -8,
        8, -4, 0, -6, -6, 1, -4, -6, -2, -1, -2, -6, -5, -4, 1, 6, -2, -8,
        9, -4, -7, -7, -3, -5, -6, -5, -1, -3, -6, -6, -6, -2, -4, -3, -8,
        12, -6, -5, -1, -4, -2, -4, -2, -3, -4, 3, -5, -1, 0, -2, -8,
        7, 2, -5, 2, -1, -5, -4, -2, -5, -3, 4, -6, -6, -2, -8,
        6, -4, 3, 0, -5, -4, -3, -4, -2, 1, -7, -5, -2, -8,
        8, -3, -5, -2, -1, -1, -6, -4, -4, -1, 1, -2, -8,
        9, 0, -4, -3, -1, -3, -3, 1, -5, -3, -2, -8,
        10, -6, -4, -4, 0, 4, -2, -6, -6, -3, -8,
        12, -2, -3, -7, -6, -4, -4, -2, -3, -8,
        7, 2, -6, -3, -3, 0, -1, -1, -8,
        8, -5, -3, 0, -1, -2, -1, -8,
        16, 3, -5, -8, -5, -5, -8,
        11, -3, -5, -4, -3, -8,
        7, -6, -4, -2, -8,
        6, 0, -3, -8,
        6, -1, -8,
        -2, -8,
        1
    },
    {
        5, -2, -2, -3, -1, -1, -1, 0, -2, -2, -2, -1, -2, -3, -1, 1, 0, -4, -3, -1, -2, -1, -1, -6,
        6, -1, -3, -5, 1, -1, -3, 0, -4, -3, 2, -2, -4, -3, -1, -2, -4, -3, -3, -2, 0, -2, -6,
        7, 1, -4, 0, -1, -1, 0, -4, -4, 0, -3, -4, -3, 0, 0, -5, -3, -4, 4, -1, -2, -6,
        7, -5, -1, 1, -2, -2, -5, -5, -1, -4, -5, -3, -1, -2, -6, -4, -5, 4, 0, -2, -6,
        9, -4, -6, -4, -5, -2, -2, -4, -2, -3, -4, -2, -2, -4, -4, -2, -4, -5, -3, -6,
        7, 2, -3, 1, -4, -3, 1, 0, -4, -2, -1, -1, -3, -3, -3, -1, 4, -1, -6,
        6, -3, -1, -4, -4, 0, -3, -5, -2, -1, -1, -5, -4, -3, 0, 4, -2, -6,
        6, -3, -5, -5, -2, -4, -5, -3, -1, -3, -4, -5, -5, -2, -3, -2, -6,
        8, -4, -4, -1, -3, -2, -3, -2, -2, -3, 1, -4, -1, 0, -2, -6,
        5, 1, -4, 1, -1, -4, -3, -1, -4, -2, 3, -5, -4, -2, -6,
        5, -3, 2, 0, -4, -3, -2, -3, -2, 0, -5, -4, -2, -6,
        6, -2, -4, -2, -1, -1, -5, -3, -3, -1, 1, -1, -6,
        7, -1, -3, -2, -1, -2, -2, 0, -4, -2, -1, -6,
        7, -4, -3, -3, 0, 3, -2, -4, -4, -2, -6,
        8, -2, -2, -5, -4, -3, -3, -2, -2, -6,
        5, 1, -4, -3, -2, 0, -1, -1, -6,
        6, -4, -2, -1, -1, -1, -1, -6,
        11, 2, -3, -6, -4, -3, -6,
        8, -3, -4, -3, -2, -6,
        5, -4, -3, -2, -6,
        4, 0, -2, -6,
        4, -1, -6,
        -2, -6,
        1
    },
    {
        6, -7, -4, -3, -6, -4, -2, -2, -7, -5, -6, -7, -5, -8, -2, 0, -1, -13, -8, -2, -3, -3, -3, -17,
        8, -6, -10, -8, -2, -9, -9, -2, -5, -8, 0, -4, -9, -4, -3, -6, -2, -10, -8, -7, -4, -6, -17,
        8, 2, -11, -3, -2, -3, 0, -5, -7, -1, -9, -9, -6, 0, -2, -8, -4, -8, 6, -3, -3, -17,
        8, -14, -2, 2, -3, -4, -7, -12, -4, -11, -15, -8, -4, -5, -15, -11, -8, 6, 1, -5, -17,
        10, -14, -14, -9, -7, -6, -15, -14, -13, -13, -8, -3, -8, -15, -4, -6, -12, -14, -9, -17,
        8, 1, -7, 1, -8, -5, -3, -4, -13, -3, -5, -5, -13, -12, -7, -3, 6, -5, -17,
        8, -4, -5, -5, -9, -4, -7, -14, -5, -4, -6, -17, -8, -6, 1, 6, -5, -17,
        6, -9, -11, -10, -7, -8, -9, -6, -2, -6, -15, -14, -5, -3, -5, -5, -17,
        9, -9, -6, -6, -10, -6, -4, -6, -7, -7, -3, -6, -1, -1, -5, -17,
        8, -1, -6, -1, -2, -8, -7, -2, -14, -6, 2, -6, -6, -5, -17,
        7, -8, 1, -3, -7, -8, -7, -6, -7, -2, -9, -7, -6, -17,
        7, -2, -14, -6, -4, -3, -12, -9, -9, -2, -4, -5, -17,
        11, -4, -8, -5, -4, -13, -11, -1, -10, -5, -5, -17,
        9, -10, -6, -9, -4, 2, -8, -10, -13, -8, -17,
        8, -2, -4, -14, -13, -6, -7, -4, -5, -17,
        6, 0, -5, -7, -6, -1, -5, -3, -17,
        7, -13, -6, -3, -3, -6, -4, -17,
        13, -5, -15, -10, -14, -11, -17,
        10, -7, -6, -9, -7, -17,
        7, -8, -6, -5, -17,
        6, 0, -5, -17,
        6, -5, -17,
        -5, -17,
        1
    },
    {
        5, -4, -2, -1, -4, -2, -1, 0, -4, -2, -4, -4, -3, -6, 0, 1, 1, -9, -5, -1, -1, -1, -2, -11,
        8, -3, -6, -5, 0, -5, -6, 0, -3, -6, 2, -2, -7, -2, -1, -4, 0, -7, -5, -4, -2, -3, -11,
        6, 3, -7, -1, 0, -1, 1, -3, -5, 0, -5, -6, -3, 1, 0, -6, -3, -5, 5, -1, -2, -11,
        6, -9, 0, 3, -1, -1, -5, -8, -2, -7, -10, -4, -1, -2, -10, -7, -5, 5, 2, -3, -11,
        9, -9, -9, -6, -5, -4, -10, -9, -9, -8, -5, -1, -5, -11, -2, -4, -8, -9, -6, -11,
        7, 2, -4, 2, -5, -3, -1, -2, -9, -1, -3, -3, -8, -8, -4, -1, 5, -2, -11,
        6, -2, -2, -4, -6, -2, -4, -9, -3, -2, -3, -11, -6, -4, 2, 5, -3, -11,
        6, -6, -6, -7, -5, -6, -7, -3, 0, -3, -10, -9, -3, -1, -3, -3, -11,
        8, -6, -4, -3, -6, -4, -2, -3, -4, -5, -1, -4, 0, 1, -3, -11,
        7, 1, -4, 1, 0, -5, -4, -1, -9, -4, 3, -4, -4, -3, -11,
        6, -5, 2, -1, -5, -6, -4, -4, -4, 0, -6, -4, -4, -11,
        6, 0, -9, -4, -2, -1, -7, -7, -6, -1, -2, -3, -11,
        10, -2, -5, -3, -2, -8, -7, 0, -6, -3, -3, -11,
        8, -7, -4, -6, -2, 4, -5, -7, -9, -5, -11,
        7, 0, -2, -9, -9, -3, -4, -2, -3, -11,
        5, 2, -3, -5, -3, 0, -2, -1, -11,
        6, -8, -4, -1, -1, -3, -2, -11,
        13, -3, -10, -7, -10, -7, -11,
        9, -5, -4, -7, -5, -11,
        6, -5, -4, -2, -11,
        5, 1, -2, -11,
        5, -3, -11,
        -3, -11,
        1
    },
    {
        2, -2, 0, 0, -2, 0, 0, 1, -1, -1, -2, -1, -1, -3, 1, 1, 1, -6, -3, 0, 0, 0, 0, -8,
        6, 0, -1, -4, 1, -1, -3, 2, -2, -3, 3, 0, -4, 0, 0, -1, 2, -4, -2, -1, 0, -1, -8,
        2, 2, -4, 1, 1, 0, 2, -2, -3, 1, -2, -3, 0, 1, 0, -4, -2, -2, 2, 1, 0, -8,
        4, -5, 2, 3, 1, 1, -2, -4, 0, -3, -6, -1, 0, 0, -7, -4, -2, 3, 3, -1, -8,
        12, -5, -5, -3, -3, -2, -6, -5, -5, -4, -3, 0, -2, -8, 0, -2, -4, -5, -3, -8,
        4, 2, -1, 3, -2, -2, 1, -1, -5, 0, -1, -1, -5, -4, -2, 1, 3, -1, -8,
        4, 0, 1, -2, -3, 0, -2, -5, -1, 0, 0, -7, -4, -2, 3, 3, -1, -8,
        5, -2, -3, -4, -2, -3, -5, 0, 1, 0, -7, -5, -1, 0, 0, -1, -8,
        6, -2, -2, 0, -2, -2, 0, -1, -1, -3, 0, -2, 1, 2, -1, -8,
        5, 2, -2, 2, 1, -2, -1, 0, -5, -1, 4, -2, -2, -1, -8,
        6, -3, 4, 2, -3, -3, -2, -2, -1, 2, -3, -3, -1, -8,
        5, 0, -5, -1, 0, 0, -3, -4, -2, 1, 0, -1, -8,
        6, 0, -2, -2, -1, -4, -2, 2, -2, -2, -1, -8,
        9, -5, -3, -3, 0, 7, -1, -4, -5, -2, -8,
        6, 1, 0, -6, -5, -1, -1, 0, -1, -8,
        2, 1, -2, -3, -1, 0, 0, 0, -8,
        3, -5, -3, 0, 0, -1, 0, -8,
        17, 0, -6, -5, -6, -4, -8,
        10, -2, -3, -4, -2, -8,
        4, -2, -2, -1, -8,
        3, 2, -1, -8,
        3, -1, -8,
        -1, -8,
        1
    }
};

static sBioseq::BlastMatrix default_blast_matrix[sBioseq::ePam250];

const sBioseq::BlastMatrix * sBioseq::getBlastMatrix(sBioseq::EBlastMatrix m)
{
    if( m == eBlosumDefault ) {
        m = eBlosum62;
    }

    idx index = (idx)m - 1;
    if( index < 0 || index >= sDim(default_blast_matrix) ) {
        return 0;
    }
    BlastMatrix * ret = default_blast_matrix + index;
    if( !ret->nletters ) {
        ret->letters = blast_aa_matrix_letters;
        ret->matrix = blast_aa_matrix[index];
        ret->nletters = BLAST_AA_MATRIX_NLETTERS;
    }
    return ret;
}

static void reverseVec(sVec<idx> & out, idx start, idx cnt)
{
    for(idx i = start; i < cnt / 2; i++) {
        sSwap<idx>(out[i], out[start + cnt - 1 - i]);
    }
}

static idx makeLinearizedMapWorker(sVec<idx> & out, idx leaf_start, const sBioseq::BlastMatrix & bm, const sHierarchicalClusteringTree & tree, idx node_index)
{
    if( node_index < 0 || node_index >= tree.dim() ) {
        return 0;
    } else if( node_index < tree.dimLeaves() ) {
        out[leaf_start] = node_index;
        return 1;
    } else {
        idx left_start = leaf_start;
        idx left_cnt = makeLinearizedMapWorker(out, left_start, bm, tree, tree[node_index].out[0]);
        idx right_start = left_start + left_cnt;
        idx right_cnt = makeLinearizedMapWorker(out, right_start, bm, tree, tree[node_index].out[1]);


        idx initial_gap_score = bm.getScore(out[left_start + left_cnt - 1], out[right_start]);
        idx gap_score_l0_r0 = bm.getScore(out[left_start], out[right_start]);
        idx gap_score_l1_r1 = bm.getScore(out[left_start + left_cnt - 1], out[right_start + right_cnt - 1]);
        idx gap_score_l0_r1 = bm.getScore(out[left_start], out[right_start + right_cnt - 1]);


        if( gap_score_l0_r0 > initial_gap_score && gap_score_l0_r0 >= gap_score_l1_r1 && gap_score_l0_r0 >= gap_score_l0_r1 ) {
            reverseVec(out, right_start, right_cnt);
        } else if( gap_score_l1_r1 > initial_gap_score && gap_score_l1_r1 >= gap_score_l0_r0 && gap_score_l1_r1 >= gap_score_l0_r1 ) {
            reverseVec(out, left_start, left_cnt);
            reverseVec(out, right_start, right_cnt);
        } else if( gap_score_l0_r1 > initial_gap_score && gap_score_l0_r1 >= gap_score_l0_r0 && gap_score_l0_r1 >= gap_score_l1_r1 ) {
            reverseVec(out, left_start, left_cnt);
        }

        return left_cnt + right_cnt;
    }
}


bool sBioseq::BlastMatrix::makeLinearizedMap(sDic<idx> & out_map) const
{
    sNeighborJoining nj(true);
    nj.reset(nletters);

    idx min_non_diag_score = sIdxMax;
    for (idx i = 0; i < nletters; i++) {
        for (idx j = i + 1; j < nletters; j++) {
            min_non_diag_score = sMin<idx>(min_non_diag_score, getScore(i, j));
        }
    }

    for (idx i = 0; i < nletters; i++) {
        nj.setDist(i, i, 0);
        for (idx j = i + 1; j < nletters; j++) {
            nj.setDist(i, j, 1 + getScore(i, j) - min_non_diag_score);
        }
    }

    nj.recluster();

    sVec<idx> leaf_indices(sMex::fExactSize | sMex::fSetZero);
    leaf_indices.resize(nj.getTree().dimLeaves());
    makeLinearizedMapWorker(leaf_indices, 0, *this, nj.getTree(), nj.getTree().dim() - 1);
    for(idx i = 0, linval = 0; i < leaf_indices.dim(); i++) {
        char letter = letters[leaf_indices[i]];
        if( i ) {
            linval += 1 + getScore(leaf_indices[i - 1], leaf_indices[i]) - min_non_diag_score;
        }
        *out_map.setString(&letter, 1) = linval;
    }

    return true;
}
