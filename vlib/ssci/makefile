#/*
# *  ::718604!
# * 
# * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
# * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
# * Affiliation: Food and Drug Administration (1), George Washington University (2)
# * 
# * All rights Reserved.
# * 
# * The MIT License (MIT)
# * 
# * Permission is hereby granted, free of charge, to any person obtaining
# * a copy of this software and associated documentation files (the "Software"),
# * to deal in the Software without restriction, including without limitation
# * the rights to use, copy, modify, merge, publish, distribute, sublicense,
# * and/or sell copies of the Software, and to permit persons to whom the
# * Software is furnished to do so, subject to the following conditions:
# * 
# * The above copyright notice and this permission notice shall be included
# * in all copies or substantial portions of the Software.
# * 
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# * DEALINGS IN THE SOFTWARE.
# */

LIBRARIES += ssci

ssci_SRCS = phys/simul/simul.cpp \
	math/stat/MixtureFactorAnalyzers.cpp \
	math/stat/student.cpp \
	math/stat/statistics.cpp \
	math/rand/rand.cpp \
	math/objects/matrix.cpp \
	math/nr/nrv_filter.cpp \
	math/nr/nr_brent.cpp \
	math/nr/nr_convlv.cpp \
	math/nr/nr_daub4.cpp \
	math/nr/nr_f1dim.cpp \
	math/nr/nr_four1.cpp \
	math/nr/nr_gaussj.cpp \
	math/nr/nr_jacoby.cpp \
	math/nr/nr_linmin.cpp \
	math/nr/nr_lubksb.cpp \
	math/nr/nr_ludcmp.cpp \
	math/nr/nr_mnbrack.cpp \
	math/nr/nr_powell.cpp \
	math/nr/nr_pwt.cpp \
	math/nr/nr_realft.cpp \
	math/nr/nr_savgol.cpp \
	math/nr/nr_select.cpp \
	math/nr/nr_twofft.cpp \
	math/nr/nr_util.cpp \
	math/nr/nr_wt1.cpp \
	math/func/spcfunc.cpp \
	math/func/spline.cpp \
	math/clust/textclust.cpp \
	math/clust/clust.cpp \
	math/clust/rlda.cpp \
	math/algebra/intgrl.cpp \
	math/algebra/matrixDiagGivens.cpp \
	math/algebra/matrixDiagJacoby.cpp \
	math/algebra/matrixDiagNonSymNewC.cpp \
	math/algebra/matrixDiagSort.cpp \
	math/algebra/matrixGeneral.cpp \
	math/algebra/matrixPCA.cpp \
	math/algebra/matrixStat.cpp \
	math/constants.cpp \
	math/geom/geom.cpp \
	math/geom/geom_primitives.cpp \
	math/geom/geom_analytic.cpp \
	math/geom/mesh.cpp \
	chem/spectr/spectrMS.cpp \
	chem/spectr/spectraMolecule.cpp \
	chem/spectr/spectraFile.cpp \
	chem/spectr/spectraPeaks.cpp \
	chem/elements/elements.cpp \
	chem/elements/formula.cpp \
	bio/bioseq.cpp \
	bio/bioseqset.cpp \
	bio/bioal.cpp \
	bio/bioalset.cpp \
	bio/biogencode.cpp \
	bio/bioseqhash.cpp \
	bio/bioseqalign.cpp \
	bio/bioseqpopul.cpp \
	bio/bioseqtree.cpp \
	bio/biosnp.cpp \
	bio/viosam.cpp \
	bio/vioseq.cpp \
	bio/vioseq2.cpp \
	bio/sraseq.cpp \
	bio/ncbitax.cpp \
	bio/idmap.cpp \
	bio/bioproject.cpp \
	bio/vioal.cpp \
	bio/viopop.cpp \
	bio/blast-parser.cpp \
	bio/blastmatrix.cpp \
	bio/bioAnnot.cpp \
	bio/sVioAnnot.cpp \
	bio/bumper.cpp \
	bio/filterseq.cpp \
	bio/tax-ion.cpp \
	bio/ion-biolib.cpp \
	bio/vax-bio.cpp \
	bio/biofingerprint.cpp \
	bio/randseq.cpp \
	c2cpp.cpp \
	bio/ionseq/suffixTree.cpp \
	bio/ionseq/ionseqid.cpp \
	bio/bioseqqc.cpp

ifeq ($(devdir),)
    devdir = $(abspath ../..)
endif
include $(devdir)/build/makefile.meta
