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
#include <ssci/math/geom/geom.hpp>
#include <math.h>

using namespace slib;

// http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/source1.c

    // each edge is defined by its vertix number and one of its 18 possible directions 
    // 6 directions are along +x,-x,+y,-y,+z,-z axes and 
    // 12 directions are along +x+y,+x-y,+x+z,+x-z, -x+y,-x-y,-x+z,-x-z, +y+z,+y-z,-y+z,-y-z, 
    // however only 9 of those are unique since each edge is on a flat surface of the cube
    // and is repeated twice . We take the following nine as unique 
    // +x,+y,+z, +x+y,+x+z,+y+z, +x-y, +x-z, +y-z
    //  0, 1, 2,    3,   4,   5,    6,    7,    8 
struct IVec {idx x,y,z;};
struct EdgeDef {idx dir; IVec cub;};


static  EdgeDef edgeDir[8][8]={
    {
        {0,{0,0,0}},// 0->1, +x in the current cube ix,iy,iz    +
        {1,{0,0,0}},// 0->2, +y in ix,iy,iz                     +
        {3,{0,0,0}},// 0->3, +x+y in ix,iy,iz                   +
        {2,{0,0,0}},// 0->4, +z in ix,iy,iz                     +
        {4,{0,0,0}},// 0->5, +x+z in ix,iy,iz                   +
        {5,{0,0,0}},// 0->6, +y+z in ix,iy,iz                   +
        {-1,{0,0,0}},// 0->7, not possible 
    },{
        {6,{0,1,0}},// 1->2, +x-y in ix,iy+1,iz                 +
        {1,{1,0,0}},// 1->3, +y in ix+1,iy,iz                   +
        {7,{0,0,1}},// 1->4, +x-z in ix,iy,iz+1                 +
        {2,{1,0,0}},// 1->5, +z in ix+1,iy,iz                   +
        {-1,{0,0,0}},// 1->6, not possible 
        {5,{1,0,0}},// 1->7, +y+z in ix+1,iy,iz                 +
    },{
        {0,{0,1,0}},// 2->3, +x in ix,iy+1,iz                   +
        {8,{0,0,1}},// 2->4, +y-z in ix,iy,iz+1                 +
        {-1,{0,0,0}},// 2->5, not possible 
        {2,{0,1,0}},// 2->6, +z in ix,iy+1,iz                   +
        {4,{0,1,0}},// 2->7, +x+z in ix,iy+1,iz                 +
    },{
        {-1,{0,0,0}},// 3->4, not possible 
        {8,{1,0,1}},// 3->5, +y-z in ix+1,iy,iz+1               +
        {7,{0,1,1}},// 3->6, +x-z in ix,iy+1,iz+1               +
        {2,{1,1,0}},// 3->7, +z in ix+1,iy+1,iz                 +
    },{
        {0,{0,0,1}},// 4->5, +x in ix,iy,iz+1                   +
        {1,{0,0,1}},// 4->6, +y in ix,iy,iz+1                   +
        {3,{0,0,1}},// 4->7, +x+y in ix,iy,iz+1                 +
    },{
        {6,{0,1,1}},// 5->6, +x-y in ix,iy+1,iz+1               +
        {1,{1,0,1}},// 5->7, +y in ix+1,iy,iz+1                 +
    },{
        {0,{0,1,1}},// 6->7, +x in ix, iy+1, iz+1               +
    }
};
/*
    Each small cube is numbered as 0,1,2,3,4,5,6,7 and is divided into 5 tetrahedrons with vertices 
    matching those of this cube. Below is the map (idxTetrahedrons) of the vertixes of tetrahedrons 
    defined in this index notation. 

    Z                                             Z
    ^                                             ^
    |   6-------------------7                     |  011-----------------111
    |  /|                  /|                     |  /|                  /| 
    | / |                 / |                     | / |                 / |
    |/  |                /  |                     |/  |                /  |
    4-------------------5   |                    001-----------------101  |
    |   |               |   |                     |   |               |   |
    |   |  y            |   |                     |   |  y            |   |
    |   | /             |   |                     |   | /             |   |
    dz  |/              |   |                     |   |/              |   |
    |   2---------------|---3                     |  010--------------|--110
    |  /                |  /                      |  /                |  /
    | dy                | /                       | /                 | /
    |/                  |/                        |/                  |/
    0----------dx-------1----------->x           000-----------------100--------->x
     If this cube is not at the origin - but shifted by ix, iy, iz then the idxTetraShift index mapping 
     will show the ix,iy,iz of the tetrahedron's vertixes relative to ix,iy,iz of the base point of 
     the marchine cube: 0 means the indexes correspond, 1 means that they are shifted by d[xyz] 
     a*/
idx idxTetrahedrons[2][5][4]={
    {
        {0,1,2,4},
        {1,2,4,7},
        {1,4,5,7},
        {2,4,6,7},
        {1,2,3,7},
    },
    {
        {0,4,5,6},
        {0,3,5,6},
        {0,2,3,6},
        {0,1,3,5},
        {3,5,6,7},
    }
};
IVec idxCubeShift[8]={
    {0,0,0}, // the 0-th pont of the cube 
    {1,0,0}, // the 1st pont of the cube 
    {0,1,0}, 
    {1,1,0}, 
    {0,0,1}, 
    {1,0,1}, 
    {0,1,1}, 
    {1,1,1}
};



void sMesh::fix_triangle( Polygon & po , sPnt & ou)
{
    
    Vertix & v1=vlist[po.triangles[0]];
    Vertix & v2=vlist[po.triangles[1]];
    Vertix & v3=vlist[po.triangles[2]];
        
    sPnt vec12,vec23,vec2o;
    vec12.x=v2.crd.x-v1.crd.x;
    vec12.y=v2.crd.y-v1.crd.y;
    vec12.z=v2.crd.z-v1.crd.z;
    vec23.x=v3.crd.x-v2.crd.x;
    vec23.y=v3.crd.y-v2.crd.y;
    vec23.z=v3.crd.z-v2.crd.z;

    vec2o.x=ou.x-v2.crd.x;
    vec2o.y=ou.y-v2.crd.y;
    vec2o.z=ou.z-v2.crd.z;


    // compute the triple product of these two vectors with the arrow to outside 
    real det=  vec12.x*vec23.y*vec2o.z
                +vec12.y*vec23.z*vec2o.x
                +vec12.z*vec23.x*vec2o.y
                -vec12.z*vec23.y*vec2o.x
                -vec12.y*vec23.x*vec2o.z
                -vec12.x*vec23.z*vec2o.y;

    //compute the real perpendicular normal
    if(doNormals){
        po.n.x=vec12.y*vec23.z-vec12.z*vec23.y;
        po.n.y=-vec12.x*vec23.z+vec12.z*vec23.x;
        po.n.z=vec12.x*vec23.y-vec12.y*vec23.x;
        po.srfc=po.n.x*po.n.x+po.n.y*po.n.y+po.n.z*po.n.z;
        real mod=sqrt(po.srfc);
        po.n.x/=mod;po.n.y/=mod;po.n.z/=mod;
    }

    idx itmp;
    if(det<0){ // different signs means rotation is not counterclockwise 
        // swap 
        itmp=po.triangles[1];po.triangles[1]=po.triangles[2];po.triangles[2]=itmp;
        if(doNormals){
            po.n.x*=-1;po.n.y*=-1;po.n.z*=-1;
        }
    }//else tmp=1;
    
    if(doNormals) { 
        // add this triangle normals to vertix normals 
        for(idx ii=0; ii<po.cnt;++ii) {
            vlist[po.triangles[ii]].nrm.x+=po.n.x*po.srfc;
            vlist[po.triangles[ii]].nrm.y+=po.n.y*po.srfc;
            vlist[po.triangles[ii]].nrm.z+=po.n.z*po.srfc;
            ++vlist[po.triangles[ii]].adj;
        }
    }

}


void sMesh::adjustNormals(void)
{
    for ( idx ii=0 ; ii<vlist.dim(); ++ii ) {
        vlist[ii].nrm.x/=vlist[ii].adj;
        vlist[ii].nrm.z/=vlist[ii].adj;
        vlist[ii].nrm.y/=vlist[ii].adj;
        real mod=sqrt(vlist[ii].nrm.x*vlist[ii].nrm.x+vlist[ii].nrm.y*vlist[ii].nrm.y+vlist[ii].nrm.z*vlist[ii].nrm.z);
        vlist[ii].nrm.x/=mod;
        vlist[ii].nrm.z/=mod;
        vlist[ii].nrm.y/=mod;
    }

}
idx sMesh::crossTetrahedron(idx tau, sVec <idx > * queue)
{
    idx cntFound=0;

    Polygon po;
    po.feature=0;
    po.clr=0;
    po.cnt=0;
    idx iCube=(tau&0xFFFFFF);
    idx it=(tau>>24)&0xFF;

    // TOO make degrees of two to make this operation much faster 
    idx rest=iCube;
    idx ix=rest/stpyz;rest-=ix*stpyz;
    idx iy=rest/stpz;rest-=iy*stpz;
    idx iz=rest;
    idx iParity=(ix+iy+iz)&0x01;

    Cube & cub=cubes[ iCube ];
    
    real x=bmin.x+ix*d.x;
    real y=bmin.y+iy*d.y;
    real z=bmin.z+iz*d.z;
    idx iout=-1; // remember one of the outside vertixes
    
    sPnt ou;
    EdgeDef * edd, * fdd;
    Vertix v={0,0,sPnt(0,0,0),sPnt(0,0,0),0,0};
              

    if (!(cub.situation&0x00000100)) { // this 8th bit is set means it has been computed 
        cub.situation=0;

        for( idx ip=0; ip<8; ++ip) {
            cub.val[ ip ]=compute(x+idxCubeShift[ip].x*d.x,y+idxCubeShift[ip].y*d.y,z+idxCubeShift[ip].z*d.z)-computeCutoff;
            ++funCalls;

            if(cub.val[ip] <= 0)  
                cub.situation|=(((idx)1)<<ip); // now we collect the in|out-ness of tetrahedron vertixes as bits in this number 
            if(cub.val[ip]==0) 
                cub.val[ip]=d.x/(10*stpx);
        }
        cub.situation|=0x00000100; // set the 8th bit means it has been computed 
        ++cubeSplits;
    }
        
    // all points are in one side: all bits are set or none is set ?
    if(cub.situation==0x1FF || cub.situation==0x100) 
        return 0; 
    
    ++tetraHedrons;
    // scan through the 4 points of the tetrahedron 
    for( idx ip1=0; ip1<(idx) (sizeof(idxTetrahedrons[0][0])/sizeof(idxTetrahedrons[0][0][0])); ++ip1) {
        // get the cube vertix matching tetrahedrons vertix
        idx ic1=idxTetrahedrons[iParity][it][ip1];
        IVec & shift1= idxCubeShift[ ic1 ];

        if(iout==-1 && !((cub.situation>>ic1)&0x01)){
            iout=ic1;
            ou.x=x+d.x*shift1.x;
            ou.y=y+d.y*shift1.y;
            ou.z=z+d.z*shift1.z; // this contributes to the position of the outside points center 
        }
        
        for( idx ip2=ip1+1; ip2<(idx) (sizeof(idxTetrahedrons[0][0])/sizeof(idxTetrahedrons[0][0][0])); ++ip2) {
            // get the cube vertix matching tetrahedrons vertix
            idx ic2=idxTetrahedrons[iParity][it][ip2];
            IVec & shift2= idxCubeShift[ ic2 ];

            // do nothing if they are at the same side of the curve 
            if(  ((cub.situation>>ic1)&0x01) == ((cub.situation>>ic2)&0x01) ) 
                continue;

            
            // now lets see if this pair has alread been considered 
            // first find the real cube number which this pair belongs to
            idx ie1,ie2;
            if(ic1<ic2){ie1=ic1;ie2=ic2;}
            else {ie1=ic2;ie2=ic1;}
            edd=&(edgeDir[ie1][ie2-ie1-1]);
            

            idx iVert;
            if( (ix+edd->cub.x)>=stpx || (iy+edd->cub.y )>=stpy || (iz+edd->cub.z)>=stpz ) {
                v.cube=iCube;
                v.edge=-1;
                iVert=0;
            } else {
                v.cube=iCube+edd->cub.x*stpy*stpz+edd->cub.y*stpz+edd->cub.z;
                v.edge=edd->dir;
                iVert=cubes[v.cube].vrt[v.edge];
            }
            if(iVert==0) { 
                real weight1=fabs(cub.val[ic1])/(fabs(cub.val[ic1])+fabs(cub.val[ic2]));

                v.crd.x=x+d.x*( shift1.x + (shift2.x - shift1.x)*weight1 );
                v.crd.y=y+d.y*( shift1.y + (shift2.y - shift1.y)*weight1 );
                v.crd.z=z+d.z*( shift1.z + (shift2.z - shift1.z)*weight1 );

                iVert=vlist.dim();
                cubes[v.cube].vrt[v.edge]=iVert;// remember this cube 
                v.nrm.x=0;v.nrm.y=0;v.nrm.z=0;v.adj=0;
                *vlist.add(1)=v;// remember this vertix


                if(queue){ 
                    // now lets determine what other tetrahedrons are to be affected.
                    // first find the difference between the indexes of vertexes involved
                    for(idx if1=0; if1<(idx)(sizeof(edgeDir)/sizeof(edgeDir[0])); ++if1) {
                        for(idx if2=if1+1; if2<8; ++if2) {
                            if(ie1==if1 && ie2==if2) // ignore self : we are looking for other edges of the same direction
                                continue;
                            fdd=&(edgeDir[if1][if2-if1-1]);
                            if(edd->dir==fdd->dir) {
                                //now look at the differences in cube indexes 
                                idx cx=ix+edd->cub.x-fdd->cub.x;
                                idx cy=iy+edd->cub.y-fdd->cub.y;
                                idx cz=iz+edd->cub.z-fdd->cub.z;
                                idx rParity=(cx+cy+cz)&0x01;

                                if( cx>=stpx || cy>=stpy || cz>=stpz || cx<0 || cy<0 || cz<0)  // out of boundaries
                                    continue;

                                idx rcube=cx*stpyz+cy*stpz+cz;
                                for( idx tt=0 ; tt<(idx) (sizeof(idxTetrahedrons[0])/sizeof(idxTetrahedrons[0][0])); ++tt) {
                                    idx * pts=idxTetrahedrons[rParity][tt];
                                    bool f1exists=false,f2exists=false;
                                    for(idx k=0; k<4; ++k) {
                                        if(pts[k]==if1)f1exists=true;
                                        else if(pts[k]==if2)f2exists=true;
                                    }
                                    if(f1exists && f2exists)  {
                                        if( !( cubes[rcube].situation&(((idx)1)<<(16+tt)) ) ){ // if this bit is set .. .we already have pushed this to the queue 
                                            queue->vadd(1,rcube|(tt<<24));  
                                            cubes[rcube].situation|=(((idx)1)<<(16+tt));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }




            }
            po.triangles[po.cnt]=iVert;
            ++po.cnt;
        }        
    }

    if(po.cnt==0)
        return 0;

    if(po.cnt==4){
        po.cnt=3;
        fix_triangle( po, ou);
        *polys.add()=po;
        ++cntFound;
        
        po.triangles[0]=po.triangles[3];
        fix_triangle( po, ou);
        *polys.add()=po;
        ++cntFound;
    }
    else {
        fix_triangle( po , ou);
        *polys.add()=po;
        ++cntFound;
    }
    ++tetraCross;
    return cntFound;
}




idx sMesh::build( const sPnt & lbmin, const sPnt & lbmax, idx lstpx, idx lstpy, idx lstpz, idx cntseeds, const real * seeds)
{
    vlist.empty();
    polys.empty();
    funCalls=0;
    funCallCubes=0;
    cubeSplits=0;
    tetraHedrons=0;
    tetraCross=0;

    isComputed=false;


    //bmin.x=lxmin;ymin=lymin,zmin=lzmin;
    bmin=lbmin;
    stpx=lstpx;stpy=lstpy;stpz=lstpz;
    stpyz=stpy*stpz;

    idx cntFound=0;
    d.x=(lbmax.x-bmin.x)/stpx;
    d.y=(lbmax.y-bmin.y)/stpy;
    d.z=(lbmax.z-bmin.z)/stpz;
    

    //Vertix v={0,0,0,0,0};
    Vertix v={0,0,sPnt(0,0,0),sPnt(0,0,0),0};
    *vlist.add()=v;
    cubes.resize(stpx*stpyz);memset(&cubes[0],0,sizeof(Cube)*cubes.dim());


    
    //vector < idx > cuberef;cuberef.resize(stpx*stpyz);memset(&cuberef[0],0,sizeof(idx)*cuberef.dim());
    sVec <idx > queue;
    bool run=true;

    // if seeds are defined then we use neighboring technique 

    if(cntseeds) { // first we try from seeds walking down to bmin.x,ymin,zmin
        if(seeds) {
            for( idx ic=0; ic<cntseeds; ++ic) {
                idx ix=(idx)((seeds[3*ic+0]-bmin.x)/d.x);if(ix>=stpx || ix<0)continue;
                idx iy=(idx)((seeds[3*ic+1]-bmin.y)/d.y);if(iy>=stpy || iy<0 )continue;
                idx iz=(idx)((seeds[3*ic+2]-bmin.z)/d.z);if(iz>=stpz || iz<0 )continue;
         
                bool runthis=true;
                while(runthis && ix+iy+iz>0 ) { // now start walking down towards minimum 
                    idx iCube=ix*stpyz+iy*stpz+iz;

                    for( idx it=0; it<(idx) (sizeof(idxTetrahedrons[0])/sizeof(idxTetrahedrons[0][0])); ++it )  {
                        idx cc=crossTetrahedron(iCube|(it<<24), &queue);
                        if( cc ){// for each seed its enough to find a single triangle
                            runthis=false;
                            run=false;
                        }
                        cntFound+=cc;
                    }
                    if(ix)--ix;
                    if(iy)--iy;
                    if(iz)--iz;
                }
            }
        }
    }

    if(run){ // and now plainly try all the cubes if no other way we were able to find  at least one 
        for ( idx ix=0 , iCube=0; ix<stpx && run; ++ix ) {
            for ( idx iy=0; iy<stpy && run; ++iy ) {
                for ( idx iz=0; iz<stpz && run; ++iz , ++iCube) {
                    for( idx it=0; it<(idx) (sizeof(idxTetrahedrons[0])/sizeof(idxTetrahedrons[0][0])); ++it )  {
                        cntFound+=crossTetrahedron(iCube|(it<<24), &queue);
                        if( cntseeds && cntFound)
                            run=false;
                    }
                }
            }
        }
    }
    
    if(cntseeds) { 
        for(idx iq=0; iq<queue.dim(); ++iq ) {
            cntFound+=crossTetrahedron(queue[iq],&queue);
        }
    }

    if(doNormals)adjustNormals();
    isComputed=true;

    cubes.empty();

    return cntFound;
}








