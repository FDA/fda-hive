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
#include "cloneTrees.hpp"

#include <slib/utils.hpp>

#include <ssci/bio.hpp>
#include <violin/violin.hpp>


idx sAlTree::createNJTree(sVec<sAlTree::alNodes> & mappedAl,sBioal * bioal, sStr &buf, idx frameStart, idx frameEnd, idx treeSize, bool fixedTreeSize ){
    idx mappedMax = mappedAl.dim();
    sVec<sAlTree::alNodes> tr;

    sSingleLinkClustering clustNJ;

    if( !fixedTreeSize ) {
        idx nodeCnt=0;
        idx iA = 0;
        for( ; iA < mappedAl.dim() && nodeCnt < treeSize ; ++iA ) {
            nodeCnt += mappedAl[iA].rpt;
        }
        treeSize = iA;
    }
    if(treeSize > mappedMax || !treeSize)
        treeSize = mappedMax;


    idx dist = 0;
#ifdef _DEBUG
    fprintf(stderr, "Calculating distance matrix...\n");
#endif
    sDistMatrix t_Dist_arr;
    udx uniqueNodes = 0 ;
    t_Dist_arr.reset(mappedMax);
    for(idx i = 0; i <mappedMax; ++i) {
        for(idx j = i + 1; j < mappedMax; ++j) {

            if( mappedAl[i].rpt && mappedAl[j].rpt ){
                dist = calculatePairwiseDistance(bioal, &mappedAl[i].Al, &mappedAl[j].Al, frameStart, frameEnd);
                if(dist==0) {
                    mappedAl[i].rpt += mappedAl[j].rpt;
                    mappedAl[j].rpt = 0;
                }
            }
            else{
                dist = 0;
            }
            t_Dist_arr.setDist(i, j, dist);
        }
        if( mappedAl[i].rpt ) {
            ++uniqueNodes;
        }
    }

    if(uniqueNodes < (udx)treeSize) {
        treeSize = uniqueNodes;
    }
    clustNJ.reset( treeSize );
    idx nonzero_i=0, nonzero_j =0;
    sStr nodeLegends;
    for(idx i = 0; i <mappedAl.dim() && nonzero_i < treeSize; ++i) {
        if( !mappedAl[i].rpt ) {
            continue;
        }
        nonzero_j = nonzero_i+1;
        for(idx j = i + 1; j < mappedAl.dim() && nonzero_j < treeSize ; ++j) {
            if( mappedAl[j].rpt ){
                clustNJ.setDist(nonzero_i,nonzero_j++, t_Dist_arr.dist(i,j) );
            }
        }
        nodeLegends.printf("id,%" DEC "&rpt,%" DEC,mappedAl[i].Al,mappedAl[i].rpt);
        nodeLegends.add0();
        ++nonzero_i;
    }
    nodeLegends.add0();


    sStr distMatr;
    clustNJ.printMatrixCSV(distMatr);

#ifdef _DEBUG
    fprintf(stderr,"%s",distMatr.ptr());
    fprintf(stderr, "Calculating tree...\n");
#endif
    if( !clustNJ.recluster() ) {
#ifdef _DEBUG
        fprintf(stderr, "Clustering failed :(\n");
#endif
        return 0;
    }
    clustNJ.printNewick(buf, sHierarchicalClusteringTree::Newick_PRINT_DISTANCE|sHierarchicalClusteringTree::Newick_PRINT_LEAF_NAMES, nodePrintfCallback, &nodeLegends);
    return clustNJ.dim();
}

idx sAlTree::calculatePairwiseDistance(sBioal * bioal, idx * ai1, idx * ai2, idx frameStart, idx frameEnd, bool countNonOverlappingRegions){
    sBioseqAlignment::Al * al1 = bioal->getAl(*ai1), * al2 = bioal->getAl(*ai2);
    idx * m1 = bioal->getMatch(*ai1), * m2 = bioal->getMatch(*ai2);
    idx subStart1 = al1->subStart() + m1[0], subStart2 = al2->subStart() + m2[0];
    if(subStart1 > frameEnd || subStart2 > frameEnd) {
        return -1;
    }

    if(subStart1 > subStart2) {
        sBioseqAlignment::Al * t_al = al1;
        al1 = al2;
        al2 = t_al;
        m1 = bioal->getMatch(*ai2);
        m2 = bioal->getMatch(*ai1);
    }
    subStart1 = al1->subStart();
    subStart2 = al2->subStart();
    const char  * seq1 = bioal->Qry->seq(al1->idQry()), * seq2= bioal->Qry->seq(al2->idQry());



    sVec < idx > uncompressMM1, uncompressMM2;

    if(al1->flags()&sBioseqAlignment::fAlignCompressed){
        uncompressMM1.resize(al1->lenAlign()*2);
        sBioseqAlignment::uncompressAlignment(al1,m1,uncompressMM1.ptr());
        m1=uncompressMM1.ptr();
    }
    if(al2->flags()&sBioseqAlignment::fAlignCompressed){
        uncompressMM2.resize(al2->lenAlign()*2);
        sBioseqAlignment::uncompressAlignment(al2,m2,uncompressMM2.ptr());
        m2=uncompressMM2.ptr();
    }

    idx lenAlign1 = al1->lenAlign(), lenAlign2 = al2->lenAlign();
    idx lenAlign = (lenAlign2 > lenAlign1) ? lenAlign2 : lenAlign1 ;
    idx subEnd1 = subStart1 + m1[2*(lenAlign1-1)], subEnd2 = subStart2 + m2[2*(lenAlign2-1)];
    if(subEnd1 < frameStart || subEnd2 < frameStart) {
        return -1;
#ifdef _DEBUG
    fprintf(stderr, "node out of frame\n");
#endif
    }
    if( subEnd1 < (subStart2 +m2[0]) ) {
        return frameEnd - frameStart +1 ;
    }

    idx flags1 = al1->flags(), flags2 = al2->flags();
    idx qryStart1 = al1->qryStart(), qryStart2 = al2->qryStart();
    idx basecall1, basecall2;
    idx qrybuflen1 = bioal->Qry->len(al1->idQry()), qrybuflen2 = bioal->Qry->len(al2->idQry());
    idx nonCompFlags1=((al1->flags())&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement)));
    idx nonCompFlags2=((al2->flags())&(~(sBioseqAlignment::fAlignBackwardComplement|sBioseqAlignment::fAlignForwardComplement)));
    idx lastSubOK1 = 0 , lastSubOK2 = 0;

    idx dist = 0;

    idx is1, iq1, isx1, iqx1, is2, iq2, isx2, iqx2;
    for( idx i1 = 0 , i2 = 0 ; i1 < 2*lenAlign ; i1+=2 ) {
        idx skpCnt = 0;
        if( i1 > 2*lenAlign1 || i2 > 2*lenAlign2 ) {
            if( countNonOverlappingRegions )
            ++dist;
            continue;
        }

        is1=m1[i1];
        iq1=m1[i1+1];

        if(is1 < 0) {
            is1 = lastSubOK1;
            basecall1 = 4;
        }
        else {
            if(iq1>=0){
                iqx1 = ( flags1&sBioseqAlignment::fAlignBackward ) ? (qrybuflen1-1-(qryStart1+iq1)): (qryStart1+iq1);
                basecall1=(flags1&sBioseqAlignment::fAlignBackward ) ?sBioseq::mapComplementATGC[(idx)sBioseqAlignment::_seqBits(seq1, iqx1, nonCompFlags1)] :(idx)sBioseqAlignment::_seqBits(seq1, iqx1, nonCompFlags1);
            }
            else {
                basecall1 = 5;
            }
            lastSubOK1 = is1 ;
        }
        isx1=(subStart1+is1);

        is2=m2[i2];
        iq2=m2[i2+1];
        if(is2 < 0) {

            is2 = lastSubOK2;
            basecall2 = 4;
        }
        else {
            if(iq2>=0){
                iqx2 = ( flags2&sBioseqAlignment::fAlignBackward ) ? (qrybuflen2-1-(qryStart2+iq2)): (qryStart2+iq2);
                basecall2=(flags2&sBioseqAlignment::fAlignBackward ) ?sBioseq::mapComplementATGC[(idx)sBioseqAlignment::_seqBits(seq2, iqx2, nonCompFlags2)] :(idx)sBioseqAlignment::_seqBits(seq2, iqx2, nonCompFlags2);
            }
            else {
                basecall2 = 5;
            }
            lastSubOK2 = is2 ;
        }
        isx2=(subStart2+is2);



        if( isx1 == isx2 ) {
            i2 += 2;
            if( isx1 < frameStart ) {
                continue;
            }
            if ( isx1+1 >= frameEnd ) {
                break;
            }
            if( basecall1 != basecall2 ) {
                ++dist;
            }
        }
        else {

            if( m2[i2] < 0 || isx1 > isx2) {
                ++skpCnt;
                i1-=2;
                i2+=2;
            }
            if(skpCnt>9) {
#ifdef _DEBUG
    fprintf(stderr, "I shall break here \n");
#endif
                --skpCnt;
            }

            if( isx1 < frameStart) {
                continue;
            }
            else if( countNonOverlappingRegions ){
#ifdef _DEBUG
    fprintf(stderr, "unexpected non overlapping regions (possibly indels) \n");
#endif
                ++dist;
            }
        }

    }

    return dist;
}


idx sAlTree::_generateRandVector(idx iStart, idx iEnd,idx cnt, sVec<idx> & out){
    out.empty();
    idx size = iEnd - iStart;
    if( size <= cnt ) {
        _isRandom = false;
        cnt = size;
        for( idx iAl = iStart ; iAl < iEnd ; ++iAl ) {
            out.vadd(1,iAl);
        }
    }
    else{
        _isRandom = true;
        idx rnd =0 ;
        sVec<idx> idVec; idVec.add( size );
        for( idx i = 0 ; i < idVec.dim() ; ++i ) {
            idVec[i] = iStart + i;
        }
        cnt = idVec.dim();
        for( idx iAl = 0 ; iAl < cnt ; ++iAl ) {
            rnd = (idx)(RAND1*(idVec.dim()));
            if(rnd >= idVec.dim()) rnd = idVec.dim()-1;
            out.vadd(1, idVec[rnd]);
            idVec[rnd]=idVec[idVec.dim()-1];
            idVec.cut(idVec.dim()-1);
        }
    }
    return out.dim();
}
idx sAlTree::_getPosEnd(idx iAl) {
    iAl = _getAl(iAl);
    sBioseqAlignment::Al * hdr = _hiveal->getAl(iAl);
    idx * m=_hiveal->getMatch(iAl);

    sVec < idx > uncompressMM;

    if(hdr->flags()&sBioseqAlignment::fAlignCompressed){
        uncompressMM.resize(hdr->lenAlign()*2);
        sBioseqAlignment::uncompressAlignment(hdr,m,uncompressMM.ptr());
        m=uncompressMM.ptr();
    }
    return hdr->subStart() + m[2*hdr->lenAlign()-2];
}
idx sAlTree::_getPosStart(idx iAl) {
    iAl = _getAl(iAl);
    sBioseqAlignment::Al * hdr = _hiveal->getAl(iAl);
    idx * m=_hiveal->getMatch(iAl);
    return hdr->subStart() + m[0];
}
void sAlTree::_addTree() {
    treeStat * tr = treesList.add();
    tr->id = cur_treeId;
    tr->start = _start;
    tr->end = _end;
    tr->startAl = _startAl;
    tr->endAl = _endAl;
    tr->dim = _endAl - _startAl;
}
void sAlTree::_dic2VecIDs(sVec<alNodes> & qryIds) {
    for(idx i = 0 ; i < readDic.dim() ; ++i ) {
        alNodes * alnode = qryIds.add();
        alnode->Al = readDic[i].iAl;
        alnode->rpt = _hiveal->getRpt(alnode->Al);
    }
}

idx sAlTree::appendToReadsList(cloneNode * node) {
    cloneNode * l_node = readsList.add();

    l_node->iAl = node->iAl;
    l_node->posEnd = node->posEnd;
    l_node->posStart = node->posStart;
    l_node->treeList.copy(&node->treeList);

    return readsList.dim();
}

idx sAlTree::_refreshPoolToPos() {
    idx _forceRefreshCnt = (1 - _forceRefreshPortion) * readDic.dim();
    for( idx i = 0 ; i < readDic.dim() ; ++i ) {
        if( !_isNodeValid( &readDic[i]) || !_forceRefreshCnt ) {
            if( readDic[i].iAl > 0 )
                appendToReadsList(readDic.ptr(i));
            readDic.del(i);
            --i;
            ++_space;
        }
        else{
            _forceRefreshCnt--;
            readDic[i].treeList.vadd(1,cur_treeId);
        }
    }
    return _space;
}
idx sAlTree::_getNextAlInds(bool reInd) {
    idx _iAl = _startAl;

    while ( _getPosStart( _iAl ) < _iStart ) {
        ++_iAl;
        if(_iAl >= _hiveal->dimAl() ) {
            return -1;
        }
    }
    _setStartAl(_iAl);

    while ( _getPosStart( _iAl ) < _iEnd ) {
        ++_iAl;
        if(_iAl >= _hiveal->dimAl() ) {
            break;
        }
    }
    _setEndAl(_iAl);

    if(!reInd) {
        _updateStartsOnAl();
        _updateEndsOnAl();
    }

    return _endAl;
}

void sAlTree::_updateStartsOnAl() {
    idx a_start = _getPosStart( _startAl );
    idx t_iStart = _oStart;
    idx dif = a_start - t_iStart;
    if( dif > 0 ) {
        _start += dif ;
        if( _start >= _end ) {
            _end = _start + 1;
        }
        _curWidth = _end - _start;
        _setOverlapCord ();
        _setInputOffset();
        _getNextAlInds(true);
    }
}

void sAlTree::_updateEndsOnAl() {
    idx a_end = _getPosStart( _endAl - 1 ) + 1 + _readSize;
    idx t_iEnd = _oEnd;
    idx dif = t_iEnd - a_end;
    if( dif > 0) {
        _end -= dif;
        if( _start >= _end ) {
            _end = _start + 1;
        }
        _curWidth = _end - _start;
        _setOverlapCord ();
        _setInputOffset();
        _getNextAlInds(true);
    }
}

void sAlTree::_moveFrameInds() {
    if( _isFixedMode ) {
        _curWidth = _width;
        _setCord (_fStart + _step );
    }

    _setOverlapCord ();
    _setInputOffset();
}

idx sAlTree::fillRange(sVec<alNodes> & qryIds, idx * startPoint, idx * endPoint ) {
    if( (startPoint || endPoint) && !_isFixedMode ) {
        if(startPoint && endPoint) {
            if ( !_overlap ) {
                idx startPos = _getPosStart(*startPoint), endPos = _getPosStart(*endPoint-1)+1;
                if( startPos <= _fStart || startPos < 0 || endPos < 0 || endPos <= startPos || endPos <= _fEnd){
                    return 0;
                }
                _fStart = startPos;
                _fEnd = endPos;
                _curWidth = _fEnd - _fStart;
                _setEndAl(*endPoint);
                _setStartAl(*startPoint);
                _moveFrameInds();
            }
            else {
                _fStart = *startPoint;
                _fEnd = *endPoint;
                _curWidth = _fEnd - _fStart;
                _moveFrameInds();
                if ( _getNextAlInds() < 0 )
                    return 0;
            }
        }
        else {
            return 0;
        }
    }
    else if( _isFixedMode ){
        _moveFrameInds();
        if ( _getNextAlInds() < 0 )
            return 0;
    }
    else {
        return 0;
    }
    _refreshPoolToPos();


    ++cur_treeId;


    _addTree();

    sVec<idx> alInds;
#ifdef _DEBUG
    fprintf(stderr, "Constructing random pool...\n");
#endif

    if(!_generateRandVector(_startAl, _endAl, _space, alInds)){
        return false;
    }
    idx cnt = _space;

    for( idx i = 0 ; i < cnt && i < alInds.dim() && _space; ++i ) {
        idx t_iAl = alInds[i];
        idx t_posStart = _getPosStart(t_iAl);
        idx t_posEnd = _getPosEnd(t_iAl);

        if( _isValid(t_posStart, t_posEnd) ) {

            cloneNode * node = readDic.add();

            node->iAl = _getAl(t_iAl);
            node->posStart = t_posStart;
            node->posEnd = t_posEnd;
            node->treeList.vadd(1,cur_treeId);

            --_space;

            if( _endAl >= _hiveal->dimAl() ) {
                appendToReadsList(node);
            }
        }
        else {
            cnt++;
        }
    }

    _dic2VecIDs(qryIds);

    return true;
}

void sAlTree::printTreeListJSON( sStr & buf ) {
    buf.cut(0);
    buf.printf("[");
    sStr t_buf;
    for( idx i = 0 ; i < treesList.dim() ; ++i) {
        treeStat * trSt = treesList.ptr(i);
        t_buf.printf(",{\"id\":%" DEC ",\"start\":%" DEC ",\"end\":%" DEC ",\"dim\":%" DEC "}",trSt->id,trSt->start,trSt->end,trSt->dim);
    }
    buf.printf("%s]",t_buf.ptr(1));
}
