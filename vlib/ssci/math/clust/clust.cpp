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
#include <slib/std/string.hpp>
#include <ssci/math/clust/clust.hpp>
#include <math.h>
#include <ctype.h>

using namespace slib;

#define sClust_USEDROW (-1)
#define sClust_MAXDIST REAL_MAX



#define arr(_v_i,_v_j)  array[ diag(_v_i)+(_v_j)-(_v_i) ]
#define diag(_v_i)  ( ( (_v_i) *( (dim<<1) - (_v_i) + 1 ) ) >>1 )
#define dist( _v_1, _v_2)   ( (_v_1)<(_v_2) ?  arr( (_v_1) , (_v_2) ) : arr ( (_v_2) , (_v_1) ) )

void sClust::debugPrintArr(idx dim, real * array, idx * map)
{
    for(idx i=0; i<dim; ++i){
        if(map[i]==sClust_USEDROW) continue;
        for(idx j=0; j<dim; ++j){
            if(map[j]==sClust_USEDROW) continue;
            printf("%-9.1g ",dist(i,j));
        }
        printf("\n");
    }
    printf("\n");
    printf("\n");
}

void sClust::flattenHierarchy(Queue * queue, idx direction)
{
    Queue tmpq;
    idx inode,q=0, r=0;

    *queue->ptrx(q++)=dim()-1;
    while( q-- ){
        inode=*queue->ptrx(q);

        if(inode>=(idx)baseDim){
            sLst <idx> & children=ptr(inode)->out;
            if(children.dim() ){
                *queue->ptrx(q++)=sClust_DOWN;
                
                for( idx k=0; k<children.dim(); ++k) {
                    if(k)*queue->ptrx(q++)=sClust_NEXT;
                    *queue->ptrx(q++)=children[k];
                }
                *queue->ptrx(q++)=-(inode);
            }
            else *tmpq.ptrx(r++)=inode;
        }
        else {
            *tmpq.ptrx(r++)=inode;
        }
    }

    for(q=0; q<r;++q) {
        if(direction<0)*queue->ptrx(q)=tmpq[r-q-1];
        else *queue->ptrx(q)=tmpq[q];
    }
}


void sClust::printNewick(sStr * out, sClust::NodePrintfCallback func, void * par, idx dodist)
{
    idx inode;
    Queue queue;
    const char * symb="(,)";

    flattenHierarchy(&queue,-1);

    for(idx q=0, c=queue.dim() ; q<c ; ++q){
        inode=queue[q];
        
        if(inode<=-(idx)baseDim) {
            out->printf("%c",symb[ -(sClust_UP+1) ]);
            if(dodist && inode!=-(idx)(dim()-1) )
                out->printf(":%lf",ptr(-inode)->obj);
        }
        else if(inode<0)
            out->printf("%c",symb[ -(inode+1)]);
        else {
            if(func )
                func(out, this, inode, par);
            else 
                out->printf("%" DEC "",inode);
            if(dodist)
                out->printf(":%lf",ptr(inode)->obj);
        }
    }
}

void sClust::positionTree(sClust::NodeLoc * loc,sClust::NodeLoc * limits)
{
    Queue queue;
    idx inode;
    flattenHierarchy(&queue,-1);
    idx q,pos,c=queue.dim(),root=dim()-1;
    

    loc[root].sx=0;
    for( q=c-2; q!=sNotIdx ; --q){
        inode=queue[q];
        if(inode<=-(idx)baseDim)
            inode=-inode;
        else if(inode<0)continue;
        
        sLst < idx> & parents=ptr(inode)->out;
        loc[inode].sx=loc[parents[0u]].sx+ ptr(inode)->obj;
        loc[inode].par=parents[0u];
    }

    for(pos=0, q=0; q<c ; ++q){
        inode=queue[q];
        
        if(inode<=-(idx)baseDim) {
            inode=-inode;
            sLst <idx > & children=ptr(inode)->out;
            idx k;
            loc[inode].sy=loc[inode].fy=loc[children[0u]].sy;
            loc[inode].fx=loc[children[0u]].fx;
            loc[inode].cy=loc[children[0u]].cy;
            for( k=1; k<children.dim(); ++k) {
                loc[inode].sy=sMin ( loc[inode].sy, loc[children[k]].sy ) ;
                loc[inode].fy=sMax ( loc[inode].fy, loc[children[k]].sy ) ;
                loc[inode].fx=sMax ( loc[inode].fx, loc[children[k]].fx);
                loc[inode].cy+=loc[children[k]].cy;
            }
            loc[inode].cy/=k;
        }
        else if(inode<0)
            ;
        else {
            loc[inode].cy=(real)pos;
            loc[inode].sy=(real)pos;
            loc[inode].fy=(real)pos;
            loc[inode].fx=loc[inode].sx;
            ++pos;
        }
    }
 
    limits->sx=0;
    limits->fx=loc[root].fx;
    limits->sy=0;
    limits->fy=(real)(pos-1);
}


void sClust::printListed(sStr * out, sClust::NodePrintfCallback func, void * par, idx dodist)
{
    if(func) {
        for(idx inode=baseDim; inode<dim() ; ++inode) {
            func(out, this, inode, par);
        }
    }
    else {
        for(idx k=0,inode=baseDim; inode<dim() ; ++inode){
            out->printf("%" DEC "=>",inode);
            sLst<idx> & children=ptr(inode)->out;
            for( idx i=0; i<children.dim(); ++ i ) {
                k=children[i];
                if( k<baseDim ) out->printf( " %" DEC "->",  k );
                else out->printf( " %" DEC "=>", k);
            }
            if(dodist)
                out->printf(" %lf",ptr(k)->obj);
            out->printf("\n");
        }
    }
}

void sClust::clusterHierarchically(idx transpose, idx distmethod, idx clustmethod, idx rows, idx cols, real * actmat )
{
    real * distmat=computeDistanceMatrix( transpose, distmethod, rows, cols, actmat , 0);
    clusterLinkage(clustmethod, transpose ? cols : rows, distmat);
    
    sDel((void*)distmat);
}


void sClust::clusterMethod(const char * format, idx * pDstMethod, idx * pClsMethod)
{
    struct clsMethod{ int method;const char * def00;} ;

    static clsMethod mts[]={
        {-1,"////" __},
        {sClust_LINKAGE_WARD,"ward" __},
        {sClust_LINKAGE_MIN,"single" _ "min" _ "sl" __},
        {sClust_LINKAGE_MAX,"complete" _ "max" __},
        {sClust_LINKAGE_AVERAGE_UNWEIGHTED,"average" _ "avr" __},
        {sClust_LINKAGE_AVERAGE_WEIGHTED,"averageW" _ "avrW" __},
        {sClust_LINKAGE_CENTROID,"centroid" __},
        {sClust_LINKAGE_MEDIAN,"median" __},
        {sClust_LINKAGE_NEIGHBORJOINING,"neighborjoining" _ "nj" __}
    };
    
    static clsMethod dts[]={
        {-1,"////" __},
        {sClust_DISTANCE_EUCLIDIAN,"euclidian" _ "metric" __},
        {sClust_DISTANCE_CITIBLOCK,"citiblock" _ "manhattan" __},
        {sClust_DISTANCE_MINIMAX,"chebyshev" _ "minimax" __},
        {sClust_DISTANCE_MINKOVSKI,"minkovski" _ "LP" __},
        {sClust_DISTANCE_CANBERRA,"canberra" __},
        {sClust_DISTANCE_PEARSON,"pearson" _ "linear" __},
        {sClust_DISTANCE_PEARSONUNCENTERED,"pearsonU" _ "linearU" __},
        {sClust_DISTANCE_PEARSONSQUARED,"pearson2" _ "linear2" __},
        {sClust_DISTANCE_SPEARMAN,"spearman" __},
        {sClust_DISTANCE_GIVEN,"given" __}
    };

    const char * clustmethod=format;
    const char * distmethod=strchr(format,':'); 

    idx dmethod=sClust_DISTANCE_EUCLIDIAN, cmethod=sClust_LINKAGE_WARD, rankbased=0;
    
    if(pDstMethod)*pDstMethod=dmethod;
    if(pClsMethod)*pClsMethod=cmethod;

    if( *clustmethod && *clustmethod!=':' ) {
        idx ic;
        for( ic=sDim(mts)-1;  ic && sString::compareChoice(clustmethod,mts[ic].def00,0,1,0)==sNotIdx; --ic )
            ;
        if(!ic) return ;
        cmethod=mts[ic].method;
    }

    if(distmethod && *(distmethod+1)) {
        ++distmethod;
        idx id;
        for( id=sDim(dts)-1;  id && sString::compareChoice(distmethod,dts[id].def00,0,1,0)==sNotIdx; --id )
            ;
        if(!id) return ;
        dmethod=dts[id].method;

        if(dmethod==sClust_DISTANCE_MINKOVSKI){
            idx k;
            for(k=0; isalpha(distmethod[k]); ++k);
            sscanf(distmethod+k,"%" DEC "",&dmethod);
            dmethod*=-1;
        }else if(dmethod==sClust_DISTANCE_SPEARMAN){
            rankbased=1;
        }

    }

    if(pDstMethod)*pDstMethod=dmethod;
    if(pClsMethod)*pClsMethod=cmethod;
    
    UNUSED_VAR(rankbased);
}
    


real * sClust::computeDistanceMatrix(idx transpose , idx method, idx rowcnt, idx colcnt, real * actmat, real * array )
{
    real d, dist;
    if(transpose){
        idx tmp=rowcnt;rowcnt=colcnt;colcnt=tmp;
    } 


    #define x (transpose ? actmat[col*rowcnt+r1] : actmat[r1*colcnt+col])
    #define y (transpose ? actmat[col*rowcnt+r2] : actmat[r2*colcnt+col])


    if(!array) 
        array=(real *)malloc( sizeof(real)* ((rowcnt*(rowcnt+1))>>1) );
    if(!array)
        return 0;

    idx rowofs=0,col;
    idx order=2;
    if(method<0){
        order=-method;
        method=sClust_DISTANCE_MINKOVSKI;
    }

    for( idx r1=0; r1 < rowcnt; ++r1 )
    {
        array[rowofs+r1]=0; 
        for( idx r2=r1+1; r2 < rowcnt; ++r2 )
        {
            
            dist=0;
            switch ( method) {
                case sClust_DISTANCE_GIVEN:
                    dist=actmat[r1*colcnt+r2];
                    break;
                case sClust_DISTANCE_EUCLIDIAN:
                    for( col=0; col < colcnt; ++col ) {
                        d = (x-y);
                        dist+=d*d;
                    }
                    dist=sqrt(dist);
                    break;
                case sClust_DISTANCE_CITIBLOCK:
                    for( col=0; col < colcnt; ++col ) {
                        d = fabs (x-y); 
                        dist+=d; 
                    }
                    dist=sqrt(dist);
                    break;
                case sClust_DISTANCE_MINIMAX:
                    for( col=0; col < colcnt; ++col ) {
                        d=fabs (x-y);
                        if(dist<d)dist=d;
                    }
                    break;
                case sClust_DISTANCE_MINKOVSKI:
                    for( col=0; col < colcnt; ++col ) {
                        d=pow(fabs(x-y),(real)order);
                        dist+=d;
                    }
                    dist=pow((real)dist,(real)1./order);
                    break;
                case sClust_DISTANCE_CANBERRA:
                    for( col=0; col < colcnt; ++col ) {
                        d = fabs(x-y)/(x+y); 
                        dist+=d;
                    }
                    break;
                case sClust_DISTANCE_SPEARMAN:
                case sClust_DISTANCE_PEARSON:
                case sClust_DISTANCE_PEARSONUNCENTERED:
                case sClust_DISTANCE_PEARSONSQUARED:{
                    real xav=0,yav=0, dx, dy,varxy=0, disx=0, disy=0;
                    if(method!=sClust_DISTANCE_PEARSONUNCENTERED) {
                        for( col=0; col < colcnt; ++col ) {
                            xav+=x;yav+=y;
                        }
                        xav/=colcnt; yav/=colcnt;
                    }
                    for( col=0; col < colcnt; ++col ) {
                        if(method==sClust_DISTANCE_PEARSONUNCENTERED)  
                            {dx=x; dy=y;}
                        else 
                            {dx=x-xav;dy=y-yav;}
                        varxy+=dx*dy;
                        disx+=dx*dx; disy+=dy*dy;
                    }
                    if(disx && disy) {
                        dist=varxy/sqrt(disx*disy);
                        if(method==sClust_DISTANCE_PEARSONSQUARED)
                            dist*=dist;
                        dist=1-dist;
                    }else dist=0;
                    }
                    break;
                default:break;
            }
            
            array[rowofs+r2] = dist;
        }
        rowofs+=rowcnt-r1-1;
    }
    return array;
}






void sClust::clusterLinkage(idx method, idx dim , real * array )
{
    idx i,j, isel, jsel, icell, imsel, jmsel, inode , iscond, irow;
    idx aldim=(dim<<1)-1;
    real dsel,rsum;
    
    
    idx * map=(idx * )sNew(sizeof(idx ) * (dim+aldim) + sizeof(real) * (aldim) +((method==sClust_LINKAGE_NEIGHBORJOINING) ? dim*sizeof(real) : 0 ) ) ; 
    idx * wei=map+dim;
    real * subsize=(real*)(wei+aldim);
    real * sums=subsize+aldim;


    add( aldim );
    for(i=0; i<dim; ++i){
        map[i]=i; 
        wei[i]=1;
    }
    for(i=0; i<aldim; ++i){
        subsize[i]=0;
    }


    for(inode=dim; inode<(dim<<1)-1; ++inode ) {
        
        if (method==sClust_LINKAGE_NEIGHBORJOINING) {
            for(i=0; i<dim; ++i){
                if(map[i]==sClust_USEDROW) 
                    continue;
                sums[i]=0;
                if( ( dim - (inode-dim)  - 2) ) {
                    for (j=0; j<dim; ++j) {
                        if(map[j]==sClust_USEDROW) 
                            continue;
                        if(i!=j)
                            sums[i]+= dist(i,j);
                    }
                    sums[i]/=( dim - (inode-dim)  - 2);
                }
            }
        }

        dsel=(real)sClust_MAXDIST;
        
        isel=0; jsel=0;
        for(i=0; i<dim ; ++i)  {
            
            if(map[i]==sClust_USEDROW) 
                continue;

            irow=diag(i);
            for(j=i+1; j<dim ; ++j)  {
            
                if(map[j]==sClust_USEDROW) 
                    continue;

                icell = irow + j - i  ;
                
                iscond=0;
                rsum = (method==sClust_LINKAGE_NEIGHBORJOINING) ? (sums[i] + sums[j]): 0 ;
                real r = array[icell] - rsum ;
                if( r < dsel ) 
                    iscond = 1;
                else if(r>dsel) 
                    iscond=0;
                else if( wei[map[i]]+wei[map[j]]<wei[map[isel]]+wei[map[jsel]] )
                    iscond=1;


                if( iscond ) {
                    dsel = r;
                    isel=i;
                    jsel=j;

                }
            }
        }

        imsel=map[isel];
        jmsel=map[jsel];

        sLst <idx> & children = ptr(inode)->out;
        children.vadd(2,imsel,jmsel);

        dsel=(real)0.5*dist(isel,jsel);

        ptr(imsel)->obj=dsel-subsize[imsel];
        ptr(jmsel)->obj=dsel-subsize[jmsel];
        
        if(method==sClust_LINKAGE_NEIGHBORJOINING) {
            rsum=(real)((sums[isel]-sums[jsel])*0.5);
            ptr(imsel)->obj+=rsum;
            ptr(jmsel)->obj-=rsum;

            if(ptr(imsel)->obj<0){
                ptr(jmsel)->obj-=ptr(imsel)->obj;
                ptr(imsel)->obj=0;
            }else if(ptr(jmsel)->obj<0){
                ptr(imsel)->obj-=ptr(jmsel)->obj;
                ptr(jmsel)->obj=0;
            }

        }
        

        dsel*=2.;

        sLst <idx> & par1=ptr(imsel)->in, & par2=ptr( jmsel)->in;
        par1.vadd(1,inode);
        par2.vadd(1,inode);

        map[isel]=inode;
        map[jsel]=sClust_USEDROW;

        wei[inode]=wei[imsel]+wei[jmsel];
        subsize[inode]+=dsel/2;

        for (i=0; i<dim; ++i) {
            if(i==isel)
                continue;
            if (map[i] == sClust_USEDROW)
                continue;

            real di=dist(isel,i);
            real dj=dist(jsel,i);
            idx wr;
                        
            switch ( method) {
                case sClust_LINKAGE_MIN:
                    if( dj>=di )
                        continue;
                    di=dj;
                    break;
                case sClust_LINKAGE_MAX:
                    if( dj<=di )
                        continue;
                    di=dj;
                    break;
                case sClust_LINKAGE_AVERAGE_UNWEIGHTED:
                    di=(real)((di+dj)*0.5);
                    break;
                case sClust_LINKAGE_AVERAGE_WEIGHTED:
                    di=(di*wei[imsel]+dj*wei[jmsel])/wei[inode];
                    break;
                case sClust_LINKAGE_CENTROID:
                    di=(di*wei[imsel]+dj*wei[jmsel] - (dsel*wei[imsel]*wei[jmsel]/wei[inode]) )/wei[inode];
                    break;
                case sClust_LINKAGE_MEDIAN:
                    di=real(0.5*(di+dj)-0.25*dsel);
                    break;
                case sClust_LINKAGE_WARD:
                    wr=wei[map[i]];
                    di=( (wei[imsel]+wr)*di+(wei[jmsel]+wr)*dj - wr*dsel ) / (wr+wei[inode]);
                    break;
                case sClust_LINKAGE_NEIGHBORJOINING:
                    di=(real)((di+dj-dsel)*0.5);
                    break;
                default: 
                    break;

            }
            
            dist( isel, i ) = di ;
        }
        

    }

    baseDim=dim;

    sDel((void*)map);
}


#ifdef DONOTCOMPILE
#define act(_my_rownum, _my_colnum) actmat[ (_my_rownum) * cols + (_my_colnum)]
#define kct(_my_clsnum, _my_colnum) kCent[ (_my_clsnum) * cols + (_my_colnum)]
#define kac(_my_clsnum, _my_colnum) kAccum[ (_my_clsnum) * cols + (_my_colnum)]
void sClust::clusterKMeans(idx doBinary, idx iter, real convMax, idx cntclust, idx rows, idx cols, real * actmat )
{

    idx sizemem=
        sizeof(real)*cols*cntclust+
        sizeof(real)*cols*cntclust+
        sizeof(idx)*cntclust+
        sizeof(idx)*rows+
        sizeof(real)*(rows)+
        sizeof(real)*(cntclust)+
        sizeof(real)*cols;
        
    real * kCent=(real * ) sNew( sizemem );
    real * kAccum = kCent+cntclust*cols;
    idx *  kCounters= (idx *)(kAccum+cntclust*cols); 
    idx *  rowCls=kCounters+cntclust; 
    real * rowDist=(real * )(rowCls+rows); 
    real * kDist=rowDist+rows; 
    real * centr=kDist+cntclust; 
    idx ir,ic,ik,kTotZero=0,kTotOnes=0;
    real conv=convMax+1,diff=0;


    for(idx it=0; it<iter && conv>convMax ; ++it) {
        kTotZero=0;
        for(ik=0; ik<cntclust ; ++ik) {
            for(ic=0; ic<cols ; ++ic) { 
                kac(ik,ic) = 0;
            }
            kCounters[ik]=0;
        }


        idx bestcluster=0;
        for (ir=0; ir<rows; ++ir) {
            real mindist=REAL_MAX; 

            if(it==0){
                bestcluster= ((idx)(sMath::random1()*(cntclust+1)))%cntclust;
            } else {
                for(ik=0; ik<cntclust ; ++ik) {
                    real sumsquare = 0;
                    for(ic=0; ic<cols ; ++ic) { 
                        real dif = act(ir,ic) - kct(ik,ic);
                        sumsquare += dif*dif;
                    }
                    rowDist[ir]=(real)sqrt(sumsquare);
                    if (sumsquare < mindist) {
                        mindist = sumsquare; 
                        bestcluster = ik;
                    }
                }
            }
            for(ic=0; ic<cols ; ++ic) { 
                kac(bestcluster, ic) += act(ir, ic);
            }
            rowCls[ir]=bestcluster;
            kCounters[bestcluster]++;
        }

        if(it)conv=0;
        for(ik=0; ik<cntclust ; ++ik) {
            if( kCounters[ik] != 0 ) {
                for(ic=0; ic<cols ; ++ic) { 
                    real newval = kac(ik, ic)/kCounters[ik];
                    if(it) {
                        diff= newval - kct(ik, ic) ; 
                        diff*=diff;
                    }
                    kct(ik, ic) = newval;
                    if(it)conv+=diff;
                }
            }
            else {
                kTotZero++;
            }
        }
            
    }

    for(ic=0; ic<cols ; ++ic) { 
        centr[ic]=0;
        for(ik=0; ik<cntclust ; ++ik) {
            if( kCounters[ik] == 0 )continue; 
            centr[ic]+=kct(ik, ic);
        }
        centr[ic]/=cntclust-kTotZero;
    }
    for(ik=0; ik<cntclust ; ++ik) {
        kDist[ik]=0;
        if( kCounters[ik] == 1 )++kTotOnes;
    }
    for(ik=0; ik<cntclust ; ++ik) {
        if( kCounters[ik] == 0 )continue; 
        for(ic=0; ic<cols ; ++ic) { 
            real dif=kct(ik, ic)-centr[ic];
            kDist[ik]+=(real)(dif*dif);
        }
        kDist[ik]=sqrt(kDist[ik]);
    }



    idx imnode=rows + cntclust ;
    if(doBinary) add( rows+cntclust + (rows+kTotZero-1) );
    else add( rows + cntclust+1 );
    
    
    for (ir=0; ir<rows+cntclust ; ++ir) {
        if (ir>=rows && kCounters[ir-rows]==1)
            continue;

        idx par=(ir<rows && kCounters[rowCls[ir]]!=1) ? rows+rowCls[ir] : dim()-1;  
        real dst= (real) (ir<rows ? rowDist[ir] : ( kCounters[ir-rows] ?  2*kDist[ir-rows] : 10. ) ) ;

        sClust_Net parents( sub(ir) );
        parents[0u]=par;

        ptr(ir)->obj = dst;

        sClust_Net children( sub( par ) );
        if(doBinary && children.dim()==2) {
            sClust_Net nchdr( sub(imnode) );
            idx n=(doBinary==sNotIdx) ? 1 : (vMath::random1()>0.5 ? 1 : 0) ;
            
            unsigned c0=children[0u],c1=children[1u];
            nchdr[0u]=c0;nchdr[1u]=c1;
            children[n]=imnode;children[1-n]=ir;

            ptr(imnode)->obj=(real)0.05;
            ++imnode;
        }
        else *children.add()=ir;
    }
    baseDim=rows;
    sDel((void*)kCent);
}

#endif 
