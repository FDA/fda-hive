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

using namespace slib;


// global coordinate system 
static sCrdSys _glo_vCrdSys(0,0,0, 1,0,0, 0,1,0, 0,0,1);
sCrdSys * sCrdSys::glo=&_glo_vCrdSys;

///////////////////////////////////////////////////
//
// construction
// 
sSurf::sSurf(sPnt * Pt0,sPnt * Pt1,sPnt * Pt2)
{
    real   module,D,a,b,c,d,e,f; // Ax+By+Cz+D=0 ; A,B,C are N.x N.y Nz 
    a=Pt1->x-Pt0->x;    // surface equation with 3 points 
    b=Pt1->y-Pt0->y;    // | x -x1 y -y1 z -z1 | | x -x1 y -y1 z -z1 |   
    c=Pt1->z-Pt0->z;    // | x2-x1 y2-y1 z2-z1 |=|   a     b     c   | =0
    d=Pt2->x-Pt0->x;    // | x3-x1 y3-y1 z3-z1 | |   d     e     f   |   
    e=Pt2->y-Pt0->y;    // 1=ctr,2=i,3=j 
    f=Pt2->z-Pt0->z;
    N.x=b*f-e*c;N.y=c*d-a*f;N.z=a*e-d*b;
    D=-(Pt0->x*N.x+Pt0->y*N.y+Pt0->z*N.z);
    module=N.normalize();if(module)D/=module;
}

///////////////////////////////////////////////////
//
// transformations
// 

// reflects point on the surface 
real sPnt::reflect(sSurf * pSurf)
{
    real   rho,xproj,yproj,zproj,dist;

    // projection of point in axis  
    dist=pSurf->N.x*x+pSurf->N.y*y+pSurf->N.z*z+pSurf->D;
    if(!dist)return dist;   // if point lies on the axis 

    /* calculation of projection point of i pt on surface */
    rho=dist/sqrt(pSurf->N.modsqr());//(pSurf->N.x*pSurf->N.x+pSurf->N.y*pSurf->N.y+pSurf->N.z*pSurf->N.z);
    xproj=x-pSurf->N.x*rho;
    yproj=y-pSurf->N.y*rho;
    zproj=z-pSurf->N.z*rho;

    // simmetric point coordinates 
    x=2*xproj-x;
    y=2*yproj-y;
    z=2*zproj-z;

    return dist;
}

// rotate a point around the axis 
real sPnt::rotate(sAxis * pAxis,real AngleRad)
{
    real   ppx,ppy,ppz;    //* for coordinates in project point center coordinates 
    real   t,xproj,yproj,zproj,dist;
    real   xx,xy,xz,yx,yy,yz,zx,zy,zz;

    // projection of point in axis calculation 
    t=((x-pAxis->O.x)*pAxis->N.x+(y-pAxis->O.y)*pAxis->N.y+(z-pAxis->O.z)*pAxis->N.z)/(pAxis->N.x*pAxis->N.x+pAxis->N.y*pAxis->N.y+pAxis->N.z*pAxis->N.z);
    xproj=pAxis->O.x+t*pAxis->N.x;
    yproj=pAxis->O.y+t*pAxis->N.y;
    zproj=pAxis->O.z+t*pAxis->N.z;

    // calculation of direction cosinuses 
    zx=pAxis->N.x;   // cosinuses of z_ axis 
    zy=pAxis->N.y;
    zz=pAxis->N.z;
    xx=x-xproj; // cosinuses of x_ axis 
    xy=y-yproj;
    xz=z-zproj;
    dist=sqrt(xx*xx+xy*xy+xz*xz);
    if(!dist)return dist;
    xx/=dist;
    xy/=dist;
    xz/=dist;
            // cosinuses of y_ axis 
    yx=(xy*zz-zy*xz);
    yy=-(xx*zz-zx*xz);
    yz=(xx*zy-zx*xy);       // this we get automatically normalized 


    // coordinates in system with center in projection point
    ppx=dist*cos(AngleRad); // after rotation == Fi 
    ppy=dist*sin(AngleRad); // after rotation == Fi 
    ppz=0;

    // rotation to global system coordinates 
    x=xx*ppx+yx*ppy+zx*ppz;
    y=xy*ppx+yy*ppy+zy*ppz;
    z=xz*ppx+yz*ppy+zz*ppz;

    // translation to global system coordinates 
    x+=xproj;
    y+=yproj;
    z+=zproj;

    return dist;
}

// rotates a system relative to the axis 
void sCrdSys::rotate(sAxis * pAxis,real angl)
{
    X.x+=O.x;X.y+=O.y;X.z+=O.z;
    Y.x+=O.x;Y.y+=O.y;Y.z+=O.z;
    Z.x+=O.x;Z.y+=O.y;Z.z+=O.z;
    
    X.rotate(pAxis,angl);
    Y.rotate(pAxis,angl);
    Z.rotate(pAxis,angl);
    ((sVect* )&O)->rotate(pAxis,angl);

    X.x-=O.x;X.y-=O.y;X.z-=O.z;
    Y.x-=O.x;Y.y-=O.y;Y.z-=O.z;
    Z.x-=O.x;Z.y-=O.y;Z.z-=O.z;

    X.normalize();
    Y.normalize();
    Z.normalize();
}


///////////////////////////////////////////////////
//
// projective functions
// 


/* perform rotation of coordinates from old coordinate system to the new one */
void sPnt::projectRotate(sCrdSys * Old,sCrdSys * New)
{
    real   lx,ly,lz;
    real   cosxx,cosxy,cosxz;
    real   cosyx,cosyy,cosyz;
    real   coszx,coszy,coszz;

    lx=x;ly=y;lz=z;
    cosxx=(New->X.x*Old->X.x+New->X.y*Old->X.y+New->X.z*Old->X.z);
    cosxy=(New->X.x*Old->Y.x+New->X.y*Old->Y.y+New->X.z*Old->Y.z);
    cosxz=(New->X.x*Old->Z.x+New->X.y*Old->Z.y+New->X.z*Old->Z.z);
    cosyx=(New->Y.x*Old->X.x+New->Y.y*Old->X.y+New->Y.z*Old->X.z);
    cosyy=(New->Y.x*Old->Y.x+New->Y.y*Old->Y.y+New->Y.z*Old->Y.z);
    cosyz=(New->Y.x*Old->Z.x+New->Y.y*Old->Z.y+New->Y.z*Old->Z.z);
    coszx=(New->Z.x*Old->X.x+New->Z.y*Old->X.y+New->Z.z*Old->X.z);
    coszy=(New->Z.x*Old->Y.x+New->Z.y*Old->Y.y+New->Z.z*Old->Y.z);
    coszz=(New->Z.x*Old->Z.x+New->Z.y*Old->Z.y+New->Z.z*Old->Z.z);

    x=lx*cosxx+ly*cosxy+lz*cosxz;
    y=lx*cosyx+ly*cosyy+lz*cosyz;
    z=lx*coszx+ly*coszy+lz*coszz;
}


///////////////////////////////////////////////////
//
// information functions
// 

real sPnt::torsion(sPnt * p1,sPnt * p2,sPnt * p3,sPnt * p4)
{
    real norms,cosval,cosdir;
    //vVEC vec1,vec2,vecmult,vecdir;
    sSurf surf1(p3,p1,p2);
    sSurf surf2(p3,p4,p2);

    norms=sqrt( surf1.N.modsqr()*surf2.N.modsqr() );
    if(norms==0.)return 0.;

    cosval=(surf1.N.x*surf2.N.x+surf1.N.y*surf2.N.y+surf1.N.z*surf2.N.z)/norms;
    //cosval=surf1.N.scalar(&surf2.N)/norms;

    sVect vec1(p1->x-p2->x,p1->y-p2->y,p1->z-p2->z);
    sVect vecdir(p2->x-p3->x,p2->y-p3->y,p2->z-p3->z);
    sVect vec2(p4->x-p3->x,p4->y-p3->y,p4->z-p3->z);
    sVect vecmult(vec1.y*vecdir.z-vecdir.y*vec1.z, vecdir.x*vec1.z-vec1.x*vecdir.z, vec1.x*vecdir.y-vecdir.x*vec1.y);

    cosdir=(vecmult.x*vec2.x+vecmult.y*vec2.y+vecmult.z*vec2.z);
    
    // vector multiplication result of p1-p2 and p4-p3 vectors; 
    return acos( cosval)*(cosdir>=0 ? 1 : -1);
}




///////////////////////////////////////////////////
//
// projective functions
// 
/*
void vgeomCordZMat2XYZ(vPNT * p1,vPNT * p2,vPNT * p3,real dist,real angle,real tors,vPNT * pdst)
{
    vAXIS ax12,ax1Tors;
    vPNT pntTors;
    vSURF surfPerp;

    vgeomCreateSurface_PtPtPt(&surfPerp,p3,p1,p2);
    vgeomCreateAxis_PtPt(&ax12,p1,p2);
    pntTors.x=surfPerp.N.x+p1->x;
    pntTors.y=surfPerp.N.y+p1->y;
    pntTors.z=surfPerp.N.z+p1->z;

    vgeomPointRotateArroundAxis(&pntTors,&ax12,-tors);
    vgeomCreateAxis_PtPt(&ax1Tors,p1,&pntTors);
    *pdst=*p2;
    vgeomPointRotateArroundAxis(pdst,&ax1Tors,-angle);
    
    pdst->x-=p1->x;pdst->y-=p1->y;pdst->z-=p1->z;
    vgeomVectorNormalize((vVEC * )pdst);
    pdst->x=p1->x+pdst->x*dist;
    pdst->y=p1->y+pdst->y*dist;
    pdst->z=p1->z+pdst->z*dist;
}



*/


