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


static sCrdSys _glo_vCrdSys(0,0,0, 1,0,0, 0,1,0, 0,0,1);
sCrdSys * sCrdSys::glo=&_glo_vCrdSys;

sSurf::sSurf(sPnt * Pt0,sPnt * Pt1,sPnt * Pt2)
{
    real   module,D,a,b,c,d,e,f;
    a=Pt1->x-Pt0->x;
    b=Pt1->y-Pt0->y;
    c=Pt1->z-Pt0->z;
    d=Pt2->x-Pt0->x;
    e=Pt2->y-Pt0->y;
    f=Pt2->z-Pt0->z;
    N.x=b*f-e*c;N.y=c*d-a*f;N.z=a*e-d*b;
    D=-(Pt0->x*N.x+Pt0->y*N.y+Pt0->z*N.z);
    module=N.normalize();if(module)D/=module;
}


real sPnt::reflect(sSurf * pSurf)
{
    real   rho,xproj,yproj,zproj,dist;

    dist=pSurf->N.x*x+pSurf->N.y*y+pSurf->N.z*z+pSurf->D;
    if(!dist)return dist;

    rho=dist/sqrt(pSurf->N.modsqr());
    xproj=x-pSurf->N.x*rho;
    yproj=y-pSurf->N.y*rho;
    zproj=z-pSurf->N.z*rho;

    x=2*xproj-x;
    y=2*yproj-y;
    z=2*zproj-z;

    return dist;
}

real sPnt::rotate(sAxis * pAxis,real AngleRad)
{
    real   ppx,ppy,ppz;
    real   t,xproj,yproj,zproj,dist;
    real   xx,xy,xz,yx,yy,yz,zx,zy,zz;

    t=((x-pAxis->O.x)*pAxis->N.x+(y-pAxis->O.y)*pAxis->N.y+(z-pAxis->O.z)*pAxis->N.z)/(pAxis->N.x*pAxis->N.x+pAxis->N.y*pAxis->N.y+pAxis->N.z*pAxis->N.z);
    xproj=pAxis->O.x+t*pAxis->N.x;
    yproj=pAxis->O.y+t*pAxis->N.y;
    zproj=pAxis->O.z+t*pAxis->N.z;

    zx=pAxis->N.x;
    zy=pAxis->N.y;
    zz=pAxis->N.z;
    xx=x-xproj;
    xy=y-yproj;
    xz=z-zproj;
    dist=sqrt(xx*xx+xy*xy+xz*xz);
    if(!dist)return dist;
    xx/=dist;
    xy/=dist;
    xz/=dist;
    yx=(xy*zz-zy*xz);
    yy=-(xx*zz-zx*xz);
    yz=(xx*zy-zx*xy);


    ppx=dist*cos(AngleRad);
    ppy=dist*sin(AngleRad);
    ppz=0;

    x=xx*ppx+yx*ppy+zx*ppz;
    y=xy*ppx+yy*ppy+zy*ppz;
    z=xz*ppx+yz*ppy+zz*ppz;

    x+=xproj;
    y+=yproj;
    z+=zproj;

    return dist;
}

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



real sPnt::torsion(sPnt * p1,sPnt * p2,sPnt * p3,sPnt * p4)
{
    real norms,cosval,cosdir;
    sSurf surf1(p3,p1,p2);
    sSurf surf2(p3,p4,p2);

    norms=sqrt( surf1.N.modsqr()*surf2.N.modsqr() );
    if(norms==0.)return 0.;

    cosval=(surf1.N.x*surf2.N.x+surf1.N.y*surf2.N.y+surf1.N.z*surf2.N.z)/norms;

    sVect vec1(p1->x-p2->x,p1->y-p2->y,p1->z-p2->z);
    sVect vecdir(p2->x-p3->x,p2->y-p3->y,p2->z-p3->z);
    sVect vec2(p4->x-p3->x,p4->y-p3->y,p4->z-p3->z);
    sVect vecmult(vec1.y*vecdir.z-vecdir.y*vec1.z, vecdir.x*vec1.z-vec1.x*vecdir.z, vec1.x*vecdir.y-vecdir.x*vec1.y);

    cosdir=(vecmult.x*vec2.x+vecmult.y*vec2.y+vecmult.z*vec2.z);
    
    return acos( cosval)*(cosdir>=0 ? 1 : -1);
}






