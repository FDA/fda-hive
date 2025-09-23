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
#pragma once
#ifndef sMath_geom_h
#define sMath_geom_h

#include <slib/core/vec.hpp>
#include <math.h>

namespace slib
{
    class sPnt;
    class sVect;
    class sAxis;
    class sSurf;
    class sCrdSys;

    class sPnt
    {
    public:
        real x,y,z;
        sPnt(){}
        sPnt(real lx, real ly, real lz){x=lx;y=ly;z=lz;}
        void set(real lx, real ly, real lz){x=lx;y=ly;z=lz;}
        real modsqr(void) const
            {return x*x+y*y+z*z;}
        real normalize(real scale=1.){
            real module=sqrt(modsqr());
            if(module){x*=scale/module;y*=scale/module;z*=scale/module;}
            return module;
        }
        static real torsion(sPnt * p1,sPnt * p2,sPnt * p3,sPnt * p4);
        real reflect(sSurf * pSurf);
        real rotate(sAxis * pAxis,real AngleRad);
        void projectRotate(sCrdSys * Old,sCrdSys * New);
    };

    class sVect  : public sPnt
    {
    public:
        sVect(){}
        sVect(real lx, real ly, real lz):sPnt(lx,ly,lz){}
    };




    class sSurf{
    public:
        sVect N;
        real D;
        sSurf(sPnt * Pt0,sPnt * Pt1,sPnt * Pt2);
    };

    class sAxis{
    public:
        sVect N;
        sPnt O;
        sAxis(){}
        sAxis(sPnt * Pt0,sPnt * Pt)
        {
            O=*Pt0;
            N.x=Pt->x-Pt0->x;
            N.y=Pt->y-Pt0->y;
            N.z=Pt->z-Pt0->z;
            N.normalize();
        }
        sAxis(sPnt * Pt0,sSurf * Sf)
        {
            O=*Pt0;
            N=Sf->N;
        }

    };

        
    class sCrdSys
    {
    public:
        sPnt O;
        sVect X,Y,Z;
        static sCrdSys * glo;

        sCrdSys(real ox, real oy, real oz,
                real xx, real xy, real xz,
                real yx, real yy, real yz,
                real zx, real zy, real zz)
                : O(ox,oy,oz),X(xx,xy,xz),Y(yx,yy,yz),Z(zx,zy,zz){}
        sCrdSys(){}
        void rotate(sAxis * pAxis,real angl);
        real scaleGamma(real  z,int perspmode)
            {if(!perspmode)return 1.;real foc=Z.modsqr();return (z+foc)/foc;}

    };

    struct vScale{
        double sx1,sy1,sz1,fx1,fy1,fz1;
        double sx2,sy2,sz2,fx2,fy2,fz2;
        double sclX,sclY,sclZ;
        vScale (double lsx1, double lsy1,double lsz1,double lfx1,double lfy1, double lfz1, double lsx2, double lsz2, double lsy2,double lfx2,double lfy2,double lfz2)
            {set(lsx1, lsy1,lsz1,lfx1,lfy1, lfz1, lsx2, lsy2, lsz2, lfx2,lfy2, lfz2 );}
        void set (double lsx1, double lsy1,double lsz1,double lfx1,double lfy1, double lfz1, double lsx2, double lsz2, double lsy2,double lfx2,double lfy2,double lfz2)
        {
            sx1=lsx1,sy1=lsy1,sz1=lsz1,fx1=lfx1,fy1=lfy1,fz1=lfz1;
            sx2=lsx2,sy2=lsy2,sz2=lsz2,fx2=lfx2,fy2=lfy2,fz2=lfz2;

            if(fx1!=sx1)sclX=(fx2-sx2)/(fx1-sx1);else sclX=0;
            if(fy1!=sy1)sclY=(fy2-sy2)/(fy1-sy1);else sclY=0;
            if(fz1!=sz1)sclZ=(fz2-sz2)/(fz1-sz1);else sclZ=0;
        }
        sPnt * pnt(sPnt * p){
            p->x=sx2+(p->x-sx1)*sclX;
            p->y=sy2+(p->y-sy1)*sclY;
            p->z=sz2+(p->z-sz1)*sclZ;
            return p;
        }
        double x(double x){return sx2+(x-sx1)*sclX;}
        double y(double y){return sy2+(y-sy1)*sclY;}
        double z(double z){return sz2+(z-sz1)*sclZ;}
    };





    class sMesh {
    public:

        struct Polygon {
            idx cnt;
            sPnt n;
            idx feature;
            idx triangles[4];
            real srfc;
            idx clr;
        };
        sVec < Polygon > polys;

        struct Vertix{
            idx cube;
            idx edge;
            sPnt crd;
            sPnt nrm;
            idx adj;
            idx clr;
        }; 
        sVec < Vertix > vlist;

        
        bool isComputed;
        bool doNormals;
        
        typedef real (* callbackFieldValue)(void * param,real x,real y,real z);
        
        public:    
            sMesh (callbackFieldValue func=0, void * param=0, real cutoff=0.) {set(func, param,cutoff);}
            sMesh * set (callbackFieldValue func=0, void * param=0, real cutoff=0.) {
                computeFunc=func;
                computeParam=param;
                computeCutoff=cutoff;
                isComputed=false;
                doNormals=true;
                return this;
            }
            virtual ~sMesh() {}
            virtual real computeField(real x,real y,real z){
                return (x*x+y*y+z*z) ; 
            }

            idx build( const sPnt & bmin, const sPnt & bmax, idx lstpx, idx lstpy, idx lstpz, idx cntseed=1, const real * seeds=0);

    private:
        real computeCutoff;
        void * computeParam;
        callbackFieldValue computeFunc;

        real compute(real x,real y,real z) {
            if(computeFunc)return computeFunc(computeParam, x,y,z);
            else return computeField(x,y,z);
        }

        void fix_triangle( Polygon & po, sPnt & ou );

        sPnt d;
        sPnt bmin;
        idx stpx,stpy,stpz,stpyz;

        struct Cube{
            idx vrt[9];
            real val[8];
            idx situation;
        };
        sVec < Cube > cubes; 
        idx funCalls,funCallCubes;
        idx cubeSplits;
        idx tetraHedrons;
        idx tetraCross;

        idx crossTetrahedron(idx tau, sVec <idx > * queue=0);
        void adjustNormals(void);

    };

    class sGeom {
        public:
        static bool isInsidePolygon(idx cnt, sPnt * pnts, sPnt * p);
        static void propInsideAxes(sPnt * pnts, sPnt * p, sPnt * crossProp);
        
        class Primitive {
            public:
            class Cube {
                public:
                static idx sideIndices[6][4][3];

            };
        };

        static void xyzMinMax(idx cnt, real * x, real * y, real * z, sPnt * min, sPnt * max, bool reset=true);

    };
}
#endif






