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
#include <slib/core.hpp>
#include <ssci/chem/spectr/spectr.hpp>
#include <ssci/chem/spectr/spectraMolecule.hpp>

using namespace slib;

sSpectrMS::isoDistributionList sSpctrMolecule::IsoD;

bool sSpctrMolecule::inputMolecules(sStr & rst, sStr & inpMolList)
{
    idx st = sNotIdx, ed = sNotIdx;
    char * sect = sString::searchStruc(rst.ptr(), rst.length(), "[Molecules]" __, "[" __, &st, &ed);
    if( !sect || !IsoD.readIsoDistributions(sect, ed - st, 1e-6, 1e-3, 0) ) {
        return false;
    }
    inpMolList.printf("%s", "%n=0^none^");
    prepareMolList(&inpMolList);
    inpMolList.printf(";");
    return true;
}

const char * sSpctrMolecule::prepareMolList(sStr * mollist,const char * separ /* ="^" */)
{
    for(idx imol = 0; imol < IsoD.isoDistr.dim(); ++imol) {
        if( imol ) {
            if( separ )
            mollist->printf("%s", separ);
            else
            mollist->add0(1);
        }
        mollist->printf("%" DEC " %.3lf %s", imol + 1, IsoD.isoDistr[imol].getMass(), (const char *) IsoD.isoDistr.id(imol));
    }
    return mollist->ptr();
}
