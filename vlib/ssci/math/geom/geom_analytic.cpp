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

bool sGeom::isInsidePolygon(idx cnt, sPnt * pnts, sPnt * p)
{
    idx counter=0;
    sPnt * p2 = pnts+0, * p1;
    
    for (idx i=1; i<=cnt; i++) {
        p1=p2;
        p2 = pnts + (i%cnt);
        
        if (p->y <= sMin(p1->y,p2->y) || p->y > sMax(p1->y,p2->y) ) continue; // not intersecting on y axis ... disregard
        if (p->x > sMax(p1->x,p2->x)) continue; // too right from the right point 
        if (p1->y == p2->y) continue ; // the segment lies horizontally
        real xinters = (p->y-p1->y)*(p2->x-p1->x)/(p2->y-p1->y)+p1->x;
        if (p1->x == p2->x || p->x <= xinters)
            counter++;
        p1 = p2;
    }
    
    if (counter % 2 == 0)return false;
    
       
    return true;
}


void sGeom::propInsideAxes(sPnt * pnts, sPnt * p,sPnt * crossProp)
{
    sPnt * p1=pnts;
    sPnt * p2=pnts+1;
    sPnt * p3=pnts+2;
    real d21=1,d31=1;
    real xonX,xonY,yonX,yonY;
    
    if(p3->x!=p1->x) d31=(p3->y-p1->y)/(p3->x-p1->x);
    if(p2->x!=p1->x) d21=(p2->y-p1->y)/(p2->x-p1->x);
        
    if(p2->x==p1->x) { 
        xonX=p->x;
        yonX=p1->y+d31*(xonX-p1->x);
        xonY=p1->x;
        yonY=p1->y+(p->y-yonX);
    }else if(p3->x==p1->x) { 
        xonY=p->x;
        yonY=p1->y+(xonY-p1->x)*d21;
        xonX=p1->x;
        yonX=p1->y+(p->y-yonY);
    } else {
        // equation of the first line is   y=py+(x-p1->x)*d21
        // equation of the second line is  y=py+(x-p1->x)*d31
        // we solve for crossing of those lines with base lines with 
        // y axis y=p1->y+(x-p1->y)*d21    
        // x axis y=p1->y+(x-p1->y)*d31
        xonX=(p->y-p1->y-p->x*d21+p1->x*d31)/(d31-d21);
        yonX=p1->y+(xonX-p1->x)*d31;

        xonY=(p->y-p1->y-p->x*d31+p1->x*d21)/(d21-d31);
        yonY=p1->y+(xonY-p1->x)*d21;
    }

    real dx=xonX-p1->x,DX=p3->x-p1->x,RX=p3->x-xonX;
    real dy=yonX-p1->y,DY=p3->y-p1->y,RY=p3->y-yonX;
    real d2=(dx*dx+dy*dy),D2=(DX*DX+DY*DY),R2=(RX*RX+RY*RY);
    crossProp->x=sqrt(d2/D2);
    if(R2>D2)crossProp->x*=-1;
    
    dx=xonY-p1->x;DX=p2->x-p1->x;RX=p2->x-xonY;
    dy=yonY-p1->y;DY=p2->y-p1->y;RY=p2->y-yonY;
    d2=(dx*dx+dy*dy);D2=(DX*DX+DY*DY);R2=(RX*RX+RY*RY);
    crossProp->y=sqrt(d2/D2);
    if(R2>D2)crossProp->y*=-1;
    return ;
}



void sGeom::xyzMinMax(idx cnt, real * x, real * y, real * z, sPnt * pmin, sPnt * pmax, bool reset)
{
    if(reset) { 
        if(x){pmax->x=-REAL_MAX;pmin->x=REAL_MAX;}else {pmax->x=pmin->x=0;}
        if(y){pmax->y=-REAL_MAX;pmin->y=REAL_MAX;}else {pmax->y=pmin->y=0;}
        if(z){pmax->z=-REAL_MAX;pmin->z=REAL_MAX;}else {pmax->z=pmin->z=0;}
    }

    for(idx iR=0; iR<cnt; ++iR) {
        if(x){
            pmin->x=sMin(pmin->x,x[iR]);
            pmax->x=sMax(pmax->x,x[iR]);
        }
        if(y){
            pmin->y=sMin(pmin->y,y[iR]);
            pmax->y=sMax(pmax->y,y[iR]);
        }
        if(z){ 
            pmin->z=sMin(pmin->z,z[iR]);
            pmax->z=sMax(pmax->z,z[iR]);
        }
    }

}

