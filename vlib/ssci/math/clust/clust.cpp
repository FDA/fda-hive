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

/////////////////////////////////////////////////////////////
// 
//  computation functions
//


//#define diag(_v_i)  ( ( (_v_i) *( (dim<<1) - (_v_i) - 1 ) ) >>1 ) // for upper triangular diagonalless matrix
#define arr(_v_i,_v_j)  array[ diag(_v_i)+(_v_j)-(_v_i) ]
#define diag(_v_i)  ( ( (_v_i) *( (dim<<1) - (_v_i) + 1 ) ) >>1 ) // for upper triangular matrix with diagonal
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
    Queue tmpq; // what to process next queue
    idx inode,q=0, r=0;

    *queue->ptrx(q++)=dim()-1; // the root 
    while( q-- ){
        inode=*queue->ptrx(q);

        if(inode>=(idx)baseDim){ // an intermediate node 
            //sClust_Net children(sub(inode));
            sLst <idx> & children=ptr(inode)->out;
            if(children.dim() ){
                *queue->ptrx(q++)=sClust_DOWN;
                
                for( idx k=0; k<children.dim(); ++k) {
                    if(k)*queue->ptrx(q++)=sClust_NEXT;
                    //idx a=children[k];
                    *queue->ptrx(q++)=children[k];
                }
                *queue->ptrx(q++)=-(inode);//sClust_UP;
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
        
        if(inode<=-(idx)baseDim) { //closing parenthesis
            out->printf("%c",symb[ -(sClust_UP+1) ]);
            //if(dodist && inode!=(idx)((baseDim<<1)-2))
            if(dodist && inode!=-(idx)(dim()-1) )
                out->printf(":%lf",ptr(-inode)->obj);
        }
        else if(inode<0) // opening parenthesis
            out->printf("%c",symb[ -(inode+1)]);
        /*else if(inode>=(idx)baseDim) { 
            if(dodist)
                out->printf(":"realFormat,*ptr(inode));
            ;//out->printf("%" DEC "",inode);
        }*/
        else { // normal nodes 
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
    

    // determine the x coordinates by scannig in reverse order and summing up the distances 
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

    // determine y coordinates ... the branch positions
    for(pos=0, q=0; q<c ; ++q){
        inode=queue[q];
        
        if(inode<=-(idx)baseDim) { //closing parenthesis
            inode=-inode; // parent node 
            sLst <idx > & children=ptr(inode)->out;
            idx k;
//            idx accum=0;
            loc[inode].sy=loc[inode].fy=loc[children[0u]].sy;//+loc[children[1u]].sy)/2;
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
        else if(inode<0) // opening parenthesis
            ;
        else { // normal nodes 
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

/*
vClr * sClust::bitmap( idx direction, idx cx, idx cy ) 
{
    real * rbmp=(real*)vMem::New( cx * cy );
    real dmin=vFloat_MAX, dmax=vFloat_MIN;
    vClr * bmp=(vClr *)rbmp;
    idx icell;

    // compute the average distances 
    for(icell=0; icell<celldim; ++icell) {
        idx in, ndim=getneigbohrs( icell, 1 ); 
        real dave=0; 

        for(dave=0, in=1; in<ndim; ++in ) { // we start on 1 to skip self 
            idx ingb=nList[in] ;
            real dd=0;
            for(idx ic=0; ic<vdim; ++ic) { //adjust the vector of neighborhood
                real diff=(map[icell*vdim+ic]-map[ingb*vdim+ic]);
                dd+=diff*diff;
            }
            dave+=sqrt(dd); // add up the distance for this pair of neigborhs
        }
        dave/=ndim; // compute average distance 
        if(dave<dmin)dmin=dave;
        if(dave>dmax)dmax=dave;
        rbmp[icell]=dave;
    }
    
    // compute the colors 
    for(icell=0; icell<celldim; ++icell) {
        idx val=(idx)(255*(1.-(dmin+rbmp[icell]/(dmax-dmin))));
        (bmp+icell)->set(val,val,val); 
    }

    return bmp;

}

 */

void sClust::printListed(sStr * out, sClust::NodePrintfCallback func, void * par, idx dodist)
{
    if(func) {
        for(idx inode=baseDim; inode<dim() ; ++inode) {
            func(out, this, inode, par);
        }
    }
    else {
        for(idx k=0,inode=baseDim; inode<dim() ; ++inode){
            out->printf("%" DEC "=>",inode);//out->printf("branch%" DEC "",inode+1-baseDim);
            sLst<idx> & children=ptr(inode)->out;
            for( idx i=0; i<children.dim(); ++ i ) {
                k=children[i];
                if( k<baseDim ) out->printf( " %" DEC "->",  k );//out->printf( " leave%" DEC "",  k+1 );
                else out->printf( " %" DEC "=>", k);//out->printf( " branch%" DEC "", k+1-baseDim );
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
/*    
    FILE * fp=fopen ("c.mtx","r") ;
    fscanf(fp,"%" DEC "",&rows);
    real * distmat=(real*)malloc(rows*(rows+1)/2*sizeof(real));
    for(idx i=0; i<rows*(rows+1)/2; ++ i) {
        fscanf(fp,"%f",&distmat[i]);
    }
    fclose(fp);
*/  
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
        //{sClust_DISTANCE_QUADRATIC,"quadratic" __},
        {sClust_DISTANCE_CANBERRA,"canberra" __},
        {sClust_DISTANCE_PEARSON,"pearson" _ "linear" __},
        {sClust_DISTANCE_PEARSONUNCENTERED,"pearsonU" _ "linearU" __},
        {sClust_DISTANCE_PEARSONSQUARED,"pearson2" _ "linear2" __},
        {sClust_DISTANCE_SPEARMAN,"spearman" __},
        //{sClust_DISTANCE_COSINE,"cosine" __},
        {sClust_DISTANCE_GIVEN,"given" __}
    };

    const char * clustmethod=format;
    const char * distmethod=strchr(format,':'); 

    idx dmethod=sClust_DISTANCE_EUCLIDIAN, cmethod=sClust_LINKAGE_WARD, rankbased=0;
    
    if(pDstMethod)*pDstMethod=dmethod;
    if(pClsMethod)*pClsMethod=cmethod;

    // search for the method 
    if( *clustmethod && *clustmethod!=':' ) {
        idx ic;
        for( ic=sDim(mts)-1;  ic && sString::compareChoice(clustmethod,mts[ic].def00,0,1,0)==sNotIdx; --ic )
            ;
        if(!ic) return ;
        cmethod=mts[ic].method;
    }

    // search for the method 
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
}
    


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ The Distance MAtrix computation algorithms 
_/
_/
*/

real * sClust::computeDistanceMatrix(idx transpose , idx method, idx rowcnt, idx colcnt, real * actmat, real * array )
{
    real d, dist;
    //#define x (actmat[r1*colcnt+col])
    //#define y (actmat[r2*colcnt+col])
    if(transpose){ // ecxhcange columns and rows 
        idx tmp=rowcnt;rowcnt=colcnt;colcnt=tmp;
    } 


    #define x (transpose ? actmat[col*rowcnt+r1] : actmat[r1*colcnt+col])
    #define y (transpose ? actmat[col*rowcnt+r2] : actmat[r2*colcnt+col])


    // allocate array if needed
    if(!array) 
        array=(real *)malloc( sizeof(real)* ((rowcnt*(rowcnt+1))>>1) ); // for upper triangular matrix with diagonal
    if(!array)
        return 0;

    idx rowofs=0,col; // for upper triangular matrix with diagonal
    idx order=2;
    if(method<0){
        order=-method;
        method=sClust_DISTANCE_MINKOVSKI;
    }

    for( idx r1=0; r1 < rowcnt; ++r1 )
    {
        array[rowofs+r1]=0; 
        for( idx r2=r1+1; r2 < rowcnt; ++r2 ) // for upper triangular matrix with diagonal
        {
            
            //http://genome.tugraz.at/Theses/Sturn2001.pdf
            //http://www.ucl.ac.uk/oncology/MicroCore/HTML_resource/distances_popup.htm
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
                //case sClust_DISTANCE_QUADRATIC:
                    //break;
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
                        for( col=0; col < colcnt; ++col ) { // compute the averages 
                            xav+=x;yav+=y;
                        }
                        xav/=colcnt; yav/=colcnt;
                    }
                    for( col=0; col < colcnt; ++col ) { // compute the averages 
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
                //case sClust_DISTANCE_COSINE:
                //    break;
                default:break;
            }
            
            array[rowofs+r2] = dist;//sqrt( dist );
        }
        rowofs+=rowcnt-r1-1; // for upper triangular matrix with diagonal
    }
    return array;
}




/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ The real linkage algorithms 
_/
_/
*/

// http://www.math.tau.ac.il/~rshamir/algmb/00/scribe00/html/lec08/node22.html
// http://mbe.oxfordjournals.org/cgi/reprint/4/4/406

void sClust::clusterLinkage(idx method, idx dim , real * array )
{
    idx i,j, isel, jsel, icell, imsel, jmsel, inode , iscond, irow;
    idx aldim=(dim<<1)-1;
    real dsel,rsum;
    
    
    idx * map=(idx * )sNew(sizeof(idx ) * (dim+aldim) + sizeof(real) * (aldim) +((method==sClust_LINKAGE_NEIGHBORJOINING) ? dim*sizeof(real) : 0 ) ) ; 
    idx * wei=map+dim; // the size of clusters
    real * subsize=(real*)(wei+aldim);
    real * sums=subsize+aldim;

    //idx * idx=map+dim;

    add( aldim );
    for(i=0; i<dim; ++i){
        map[i]=i; 
        wei[i]=1;
    }
    for(i=0; i<aldim; ++i){
        subsize[i]=0;
    }

    //printf("input\n");
    //debugPrintArr(dim,array,map);

    for(inode=dim; inode<(dim<<1)-1; ++inode ) {
        
        // compute sums 
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

        // select the minimal (or maximal, depending on a method) element 
        dsel=(real)sClust_MAXDIST;
        
        isel=0; jsel=0;
        for(i=0; i<dim ; ++i)  {
            
            if(map[i]==sClust_USEDROW) 
                continue;

            irow=diag(i);
            for(j=i+1; j<dim ; ++j)  {
            
                if(map[j]==sClust_USEDROW) 
                    continue;

                icell = irow + j - i  ; // for upper triangular matrix with diagonal
                
                // check if this cell is to be picked up
                iscond=0;
                rsum = (method==sClust_LINKAGE_NEIGHBORJOINING) ? (sums[i] + sums[j]): 0 ;
                real r = array[icell] - rsum ;
                if( r < dsel ) 
                    iscond = 1;
                else if(r>dsel) 
                    iscond=0;
                else if( wei[map[i]]+wei[map[j]]<wei[map[isel]]+wei[map[jsel]] ) // we chose the one with lower weight if the distances are equal 
                    iscond=1;


                // remember its location
                if( iscond ) {
                    dsel = r;//array[ icell ] - rsum;
                    isel=i;
                    jsel=j;

                }
            }
        }

        imsel=map[isel];
        jmsel=map[jsel];

        //remember the children we join
        sLst <idx> & children = ptr(inode)->out;
        children.vadd(2,imsel,jmsel);

        dsel=(real)0.5*dist(isel,jsel);

        // keep their distance to the new parent 
        ptr(imsel)->obj=dsel-subsize[imsel];//-array[diag(isel)];
        ptr(jmsel)->obj=dsel-subsize[jmsel];//-array[diag(jsel)];
        
        if(method==sClust_LINKAGE_NEIGHBORJOINING) {
            rsum=(real)((sums[isel]-sums[jsel])*0.5);
            ptr(imsel)->obj+=rsum;
            ptr(jmsel)->obj-=rsum;

            if(ptr(imsel)->obj<0){ //transfering negative branch length into the adjacent
                ptr(jmsel)->obj-=ptr(imsel)->obj;
                ptr(imsel)->obj=0;
            }else if(ptr(jmsel)->obj<0){ //transfering negative branch length into the adjacent
                ptr(imsel)->obj-=ptr(jmsel)->obj;
                ptr(jmsel)->obj=0;
            }

        }
        
        // and remember the distance correction measure        
        //array[diag(isel)]=dsel;

        dsel*=2.;

        // remember the parent 
        //sClust_Net parents( sub(inode)+1 );
        //parents[0u]=inode;
        sLst <idx> & par1=ptr(imsel)->in, & par2=ptr( jmsel)->in;
        par1.vadd(1,inode);
        par2.vadd(1,inode);

        // remember the mapping 
        map[isel]=inode;
        map[jsel]=sClust_USEDROW;

        // and the size of the new subcluster
        wei[inode]=wei[imsel]+wei[jmsel];
        subsize[inode]+=dsel/2;

        // now we join rows : measure the distance to the new cluster from all other nodes 
        for (i=0; i<dim; ++i) {
            if(i==isel)
                continue;
            if (map[i] == sClust_USEDROW)
                continue;

            real di=dist(isel,i);
            real dj=dist(jsel,i);
            idx wr;
                        
            switch ( method) {
                // http://www.quantlet.com/mdstat/scripts/mva/htmlbook/mvahtmlnode80.html
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
        
        //printf("%" DEC "=%" DEC " %" DEC " (%" DEC " %" DEC ")\n", inode, imsel, jmsel, isel, jsel);
        //debugPrintArr(dim,array,map);

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
        sizeof(real)*cols*cntclust+  // centers of clusters
        sizeof(real)*cols*cntclust+  // accumulators of clusters
        //sizeof(real)*cols*2+  // minimums and maximums per column
        sizeof(idx)*cntclust+  // counters per cluster : how many points in this cluster 
        sizeof(idx)*rows+  // the serial of the cluster this points belongs to 
        sizeof(real)*(rows)+  // distance to corresponding cluster centroids for rows 
        sizeof(real)*(cntclust)+  // distance from cluster centroids to complete centroi
        sizeof(real)*cols; // centorid of the whole 
        
    real * kCent=(real * ) sNew( sizemem );
    real * kAccum = kCent+cntclust*cols;
    //real * fMin = kAccum+cntclust*cols;
    //real * fMax = fMin+cols;
    idx *  kCounters= (idx *)(kAccum+cntclust*cols); 
    idx *  rowCls=kCounters+cntclust; 
    real * rowDist=(real * )(rowCls+rows); 
    real * kDist=rowDist+rows; 
    real * centr=kDist+cntclust; 
    idx ir,ic,ik,kTotZero=0,kTotOnes=0;
    real conv=convMax+1,diff=0;

    // find minimax
/*    
    for(ic=0; ic<cols ; ++ic) { // initalize minimax to the 0-th row for all columns
        fMax[ic]=fMin[ic]=act(0,ic);
    }
    for(ir=0; ir<rows ; ++ir) { // scan the rest of the table 
        for(ic=0; ic<cols ; ++ic) { // starting from 1-th
            if( fMin[ic] > act(ir,ic) ) fMin[ic] = act(ir,ic);
            if( fMax[ic] < act(ir,ic) ) fMax[ic] = act(ir,ic);
        }
    }

    // initialize cluster centers and setting acumulators

    for(ik=0; ik<cntclust ; ++ik) {
        for(ic=0; ic<cols ; ++ic) { 
            real rnum = (real) ( fMin[ic]+ (fMax[ic] - fMin[ic]) * vMath::random1() ) ;
            kct(ik,ic) = rnum;//(real) ( fMin[ic]+ (fMax[ic] - fMin[ic]) * vMath::random1() ) ;
        }
    }
*/

    // perform iterations until convergence 
    for(idx it=0; it<iter && conv>convMax ; ++it) {
        kTotZero=0;
        // set the cluster center-accumulators into zero 
        for(ik=0; ik<cntclust ; ++ik) {
            for(ic=0; ic<cols ; ++ic) { 
                kac(ik,ic) = 0;
            }
            kCounters[ik]=0;
        }

        // compute and compare the distances, and accumulate the contribution of all rows 

        idx bestcluster=0;
        for (ir=0; ir<rows; ++ir) {
            real mindist=REAL_MAX; 

            // determine which cluster this gets attached to
            if(it==0){  // the first iteration randomly decides the cluster 
                bestcluster= ((idx)(sMath::random1()*(cntclust+1)))%cntclust;
            } else {
                // measure the distance from a given row to a given cluster 
                // and remember the cluster closest to this row 
                for(ik=0; ik<cntclust ; ++ik) {
                    real sumsquare = 0;
                    for(ic=0; ic<cols ; ++ic) { 
                        real dif = act(ir,ic) - kct(ik,ic);
                        sumsquare += dif*dif;
                    }
                    rowDist[ir]=(real)sqrt(sumsquare);
                    // compare if this is the closest as of yet and remember it
                    if (sumsquare < mindist) {
                        mindist = sumsquare; 
                        bestcluster = ik;
                    }
                }
            }
            //accumulating this points continution into closest cluster's accumulator
            for(ic=0; ic<cols ; ++ic) { 
                kac(bestcluster, ic) += act(ir, ic);
            }
            rowCls[ir]=bestcluster;
            kCounters[bestcluster]++;
        }

        // recomute the centers from accumulators
        if(it)conv=0;
        for(ik=0; ik<cntclust ; ++ik) {
            if( kCounters[ik] != 0 ) {
                for(ic=0; ic<cols ; ++ic) { 
                //store old position before updating
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

    // compute the distance from a cluster center to complete center 
    // get the complete centroid
    for(ic=0; ic<cols ; ++ic) { 
        centr[ic]=0;
        for(ik=0; ik<cntclust ; ++ik) {
            if( kCounters[ik] == 0 )continue; 
            centr[ic]+=kct(ik, ic);
        }
        centr[ic]/=cntclust-kTotZero;
    }
    // get the distances 
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



    idx imnode=rows + cntclust ; // intermediate nodes start after cluster nodes ;
    if(doBinary) add( rows+cntclust + (rows+kTotZero-1) );
    else add( rows + cntclust+1 );
    
    // add all nodes to their clusters
    
    for (ir=0; ir<rows+cntclust ; ++ir) { // imnode<dim()-1
        if (ir>=rows && kCounters[ir-rows]==1) // single leave clusters do not participate in the tree 
            continue; // their children are directly connected to a root node 

        // determine the parent node 
        // cluster nodes (from rows to rows+cntclst) have a parent = root 
        // single leave nodes also have a parent = root 
        // other nodes have their conventional cluster parents
        idx par=(ir<rows && kCounters[rowCls[ir]]!=1) ? rows+rowCls[ir] : dim()-1;  
        real dst= (real) (ir<rows ? rowDist[ir] : ( kCounters[ir-rows] ?  2*kDist[ir-rows] : 10. ) ) ;

        sClust_Net parents( sub(ir) );
        parents[0u]=par;

        // set the distance 
        ptr(ir)->obj = dst;// rowDist[ir];

        //remember the children we join
        sClust_Net children( sub( par ) ); // rows+rowCls[ir]
        //sClust_Net cc( sub( 72 ) ); // rows+rowCls[ir]
        if(doBinary && children.dim()==2) {
            sClust_Net nchdr( sub(imnode) );
            idx n=(doBinary==sNotIdx) ? 1 : (vMath::random1()>0.5 ? 1 : 0) ;
            
            // reparent the children of the parent node  to imnode 
            unsigned c0=children[0u],c1=children[1u];
            nchdr[0u]=c0;nchdr[1u]=c1;
//            sClust_Net par1( sub(c0) ), par2( sub(c1) );
//            par1[0u]=imnode;par2[0u]=imnode;
            // set the parent of the imnode and newly attaching node ir
            //sClust_Net parIR( sub(ir) ), parIM( sub(imnode) );
            children[n]=imnode;children[1-n]=ir;
            //parIR[0u]=par;parIM[0u]=par;

            ptr(imnode)->obj=(real)0.05;
            ++imnode;
        }
        else *children.add()=ir;
    }
    baseDim=rows;
    sDel((void*)kCent);
}

#endif 
