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
        sPnt(){} // default construction 
        sPnt(real lx, real ly, real lz){x=lx;y=ly;z=lz;} // construction 
        void set(real lx, real ly, real lz){x=lx;y=ly;z=lz;} // construction 
        real modsqr(void) const // square of the module 
            {return x*x+y*y+z*z;}
        real normalize(real scale=1.){ // normalize the vectore (module = 1 ) 
            real module=sqrt(modsqr());
            if(module){x*=scale/module;y*=scale/module;z*=scale/module;}
            return module;
        }
        static real torsion(sPnt * p1,sPnt * p2,sPnt * p3,sPnt * p4); // compute the torsion angle between 4 points
        real reflect(sSurf * pSurf); // reflect the point on a given surface
        real rotate(sAxis * pAxis,real AngleRad); // rotate around given axis so many radians
        void projectRotate(sCrdSys * Old,sCrdSys * New); // perform rotational transformations between coordinate systems
    };

    class sVect  : public sPnt
    {
    public:
        // construction
        sVect(){}
        sVect(real lx, real ly, real lz):sPnt(lx,ly,lz){}
    };




    class sSurf{
    public:
        sVect N; // the normal 
        real D; // the free coefficient
        sSurf(sPnt * Pt0,sPnt * Pt1,sPnt * Pt2);
    };

    class sAxis{
    public:
        sVect N; // the direction vector
        sPnt O; // the origin 
        sAxis(){} // default constructor
        sAxis(sPnt * Pt0,sPnt * Pt) // constructor by two points
        {
            O=*Pt0;
            N.x=Pt->x-Pt0->x;
            N.y=Pt->y-Pt0->y;
            N.z=Pt->z-Pt0->z;
            N.normalize();
        }
        sAxis(sPnt * Pt0,sSurf * Sf) // constructor by a point and surface normal
        {
            O=*Pt0;
            N=Sf->N;
        }

    };

        
    class sCrdSys
    {
    public:
        sPnt O;         // origin
        sVect X,Y,Z; // basis vectors
        //static sCrdSys * gloSys; // global orthonormal system of coordinates 
        static sCrdSys * glo; // global single instance 

        sCrdSys(real ox, real oy, real oz, // constructor
                real xx, real xy, real xz,
                real yx, real yy, real yz,
                real zx, real zy, real zz)
                : O(ox,oy,oz),X(xx,xy,xz),Y(yx,yy,yz),Z(zx,zy,zz){}
        sCrdSys(){}
        void rotate(sAxis * pAxis,real angl); // rotate around an axis
        //void rotate(sCrdSys * Rel,char axs, real angl); // rotate around another system of coordinates
        real scaleGamma(real  z,int perspmode) // returns the scaling factor the distant object at <z> by focal distance at the this->Z
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
        
        typedef real (* callbackFieldValue)(void * param,real x,real y,real z); // param is a user defined value for callback
        
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
            // default zero center sphere implementation of the callback
            virtual real computeField(real x,real y,real z){
                return (x*x+y*y+z*z) ; 
            }

            // the marching algorithm
            idx build( const sPnt & bmin, const sPnt & bmax, idx lstpx, idx lstpy, idx lstpz, idx cntseed=1, const real * seeds=0);

    private:
        real computeCutoff; // the values smaller than cutoff returned by callbackFunc are considered inside points, larger - outside, if equal - on the surface
        void * computeParam;
        callbackFieldValue computeFunc;

        real compute(real x,real y,real z) {
            if(computeFunc)return computeFunc(computeParam, x,y,z); // this way we use callback 
            else return computeField(x,y,z); // this way we use overloaded computeFunc
        }

        void fix_triangle( Polygon & po, sPnt & ou );

        sPnt d;//real dx,dy,dz;
        sPnt bmin;//real xmin, ymin, zmin;
        idx stpx,stpy,stpz,stpyz;

        struct Cube{
            idx vrt[9]; // indexes of th verticies in vlist array for each one of the nine edges
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



/*
real sVect::rotate(sCrdSys * Rel,char axs, real angl)
{
    sAxis Axis;Axis.O=Rel->O;
    Axis.N = (axs=='X') ? (Rel->X) : ( (axs=='Y') ? (Rel->Y) : (Rel->Z) );
    return rotate(&Axis,angl);
}

void sVect::intersectSphereAxis(sPnt * pctr,real rad,sAxis * pAxis)
{
    x=pctr->x+rad*pAxis->N.x;
    y=pctr->y+rad*pAxis->N.y;
    z=pctr->z+rad*pAxis->N.z;
}

void sVect::projectMove(sCrdSys * Old,sCrdSys * New)
{
    x+=Old->O.x-New->O.x;
    y+=Old->O.y-New->O.y;
    z+=Old->O.z-New->O.z;
}

        void sCrdSys::move(sVect * vec,real step=1.)
        {
            O.x+=vec->x*step;
            O.y+=vec->y*step;
            O.z+=vec->z*step;
        }
        void sCrdSys::move(sCrdSys * Rel,char axs, real step)
        {
            move( (axs=='X') ? &(Rel->X) : ( (axs=='Y') ? &(Rel->Y) : &(Rel->Z) )  ,step);
        }
*/



