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
#ifndef  sLib_utils_sRangeTree_hpp
#define  sLib_utils_sRangeTree_hpp

#include <slib/core/vec.hpp>

#define NIL -1
#define RED 1
#define BLACK 0

#define RangeMax(a, b) ( (a > b) ? a : b )
#define RangeMin(a, b) ( (a < b) ? a : b )

namespace slib {
template<class Nobj = idx, class Kobj = idx> class sRangenode
{
    public:
        Nobj start;
        Nobj end;
        Nobj max;
        Kobj value;
        idx left, right, parent;
        sRangenode()
        {
            sSet(this, 0);
        }
        sRangenode(Nobj s, Nobj e, Nobj v)
        {
            init(s, e, v);
        }
        void init(Nobj s, Nobj e, Nobj v)
        {
            start = s;
            end = e;
            value = v;
            max = e;
            left = NIL;
            right = NIL;
            parent = NIL;
        }
};

template<class Nobj = idx, class Kobj = idx>
class sRangeTree
{
        typedef sRangenode<Nobj, Kobj> sRangeNode;
    public:
        enum eTreeMode
        {
            eRBmode = 0x01,
            eAVLmode = 0x02
        };

        sRangeNode * root;
        idx mode;
        sRangeNode * _vStart;
        sVec<sRangeNode> _vec;

//    sRangeTree(const char * flnm)  { nil=0;root=0;if(flnm) _vec.init(flnm);}

        void init (){
            _vec.mex()->flags |= sMex::fSetZero;
            _vec.cut(0); // set at the beginning of the buffer
            _Color.mex()->flags |= sMex::fSetZero;
            _Color.cut(0); // set at the beginning of the buffer
            _Color.vadd(1, 0);
            _BFactors.mex()->flags |= sMex::fSetZero;
            _BFactors.cut(0); // set at the beginning of the buffer
            root = 0;
            _vStart = 0;

//        _BFactors.vadd(1,0);

        }

        sRangeTree(idx smode = eRBmode)
        {
            init ();
            setMode(smode);

            _vStart = root;
        }
        sRangeTree(char * rootP, idx smode = 0)
        {
            root = (sRangeNode *) rootP;

            _vStart = (sRangeNode*) root - root->parent;
            setMode(smode);

        }

        void setMode(idx emode = eRBmode)
        {
            if (emode == eRBmode){
                mode = eRBmode;
            }
            else {
                mode = eAVLmode;
            }
        }

        void insert(Nobj start, Nobj end, Kobj value)
        {
            if( mode & eRBmode ) {
                insertRB(start, end, value);
            } else if( mode & eAVLmode ) {
                insertAVL(start, end, value);
            }
        }

        idx search(sVec<Kobj> &results, Nobj start, Nobj end, Kobj * value = 0)
        {
            idx prevResults = results.dim();
            sRangeNode* curr = root ? root : 0;

            sVec<sRangeNode *> stack;
            stack.vadd(1, root);
            while( stack.dim() ) {
                curr = *stack.ptr(stack.dim() - 1);
                stack.cut(stack.dim() - 1);
                if( sOverlap(start,end,curr->start,curr->end) && ((value) ? *value == curr->value : 1) ) {
                    results.vadd(1, curr->value);
                }
                if( curr->left != NIL ) {
                    if( start <= (_vStart + curr->left)->max ) {
                        stack.vadd(1, _vStart + curr->left);
                    }
                }
                if( curr->right != NIL ) {
                    if( start <= (_vStart + curr->right)->max && end >= curr->start ) {
                        stack.vadd(1, _vStart + curr->right);
                    }
                }
            }
            return results.dim() - prevResults;
        }

        bool searchOneNode(sVec<sRangeNode *> &stack, Nobj start, Nobj end, Kobj * value = 0)
        {
            sRangeNode* curr = 0;
            if (stack.dim()){
                curr = *stack.ptr(stack.dim() - 1);
                stack.cut(stack.dim() - 1);
                if( curr->left != NIL ) {
                    if( start <= (_vStart + curr->left)->max ) {
                        stack.vadd(1, _vStart + curr->left);
                    }
                }
                if( curr->right != NIL ) {
                    if( start <= (_vStart + curr->right)->max && end >= curr->start ) {
                        stack.vadd(1, _vStart + curr->right);
                    }
                }
                if( sOverlap(start,end,curr->start,curr->end) && ((value) ? *value == curr->value : 1) ) {
                    return true;
                }
            }
            return false;
        }

        idx search(Nobj start, Nobj end, Kobj * value = 0)
        {
            sVec<Kobj> t_res;
            return search(t_res, start, end, value);
        }

        idx exactSearch(sVec<Kobj> &results, Nobj start, Nobj end, Kobj * value = 0)
        {
            idx prevResults = results.dim();
            sRangeNode* curr = root ? root : 0;

            if (root == 0){
                return 0;
            }
            sVec<sRangeNode *> stack;
            stack.vadd(1, root);
            while( stack.dim() ) {
                curr = *stack.ptr(stack.dim() - 1);
                stack.cut(stack.dim() - 1);
                if( start == curr->start && end == curr->end && ((value) ? *value == curr->value : 1) ) {
                    results.vadd(1, curr->value);
                }
                if( curr->left != NIL ) {
                    if( start <= (_vStart + curr->left)->max ) {
                        stack.vadd(1, _vStart + curr->left);
                    }
                }
                if( curr->right != NIL ) {
                    if( start <= (_vStart + curr->right)->max && end >= curr->start ) {
                        stack.vadd(1, _vStart + curr->right);
                    }
                }
            }
            return results.dim() - prevResults;
        }

//                   *sub->set(t,0,&subIDtemp)=(idx)(ptr0-fileContent);

//    idx search(sVec<Kobj> &results, Nobj position, Kobj * value =0) {
//        return search(results,position,position,value);
//    }
//
//    idx search(Nobj position, Kobj * value =0) {
//        sVec<Kobj> t_res;
//        return search(t_res,position,value);
//    }
        sRangeNode * getRoot()
              {
                  return root;
              }
    protected:
        sVec<udx> _Color;
        sVec<udx> _BFactors;


        bool isLeft (sRangeNode * xParent, sRangeNode * child)
        {
            return xParent->left == (child - _vStart);
        }

        bool isLeft (sRangeNode * xParent, Nobj start)
        {
            return (xParent->start < start);
        }

        bool isRelativeLeft (sRangeNode * xParent, sRangeNode * child)
        {
            sRangeNode * t_node = 0 ;
            sRangeNode * t_child = child;

            if( xParent == child )
                return false;

            while( t_child->parent && t_child->parent + _vStart != xParent ){
                t_node = t_child;
                t_child = t_child->parent + _vStart;
            }
            return isLeft (t_child, t_node);
        }

        void insertAVL(Nobj start, Nobj end, Kobj value)
        {
            idx lastOffRoot = root - _vStart;

            sRangeNode * x = _vec.add();
            if( _vec.dim() == 1 ) {
                _vStart = x;
            }

            _vStart = _vec.ptr();
            root = lastOffRoot + _vStart;
            root->parent = NIL;
            x->init(start, end, value);

            sRangeNode * xParent = PositionInTree(x);

            while( xParent ) {

                // Get Parent's BF
                idx iParent = xParent - _vStart;
                idx parentBF = getBFactors (iParent);

                // Adjusting BF's
                if( isLeft(xParent, x) ) {
                    --parentBF;
                } else {
                    ++parentBF;
                }

                // Check if we go outbounds -2 or +2 in the Balance Factors
                if( parentBF < -1 ) {
                    //LEFT LEFT
                    idx leftBF = getBFactors (x - _vStart);
                    if( leftBF == -1 ) {
                        SimpleRightRotate(xParent);
                    }
                    //LEFT RIGHT
                    else {
                        LeftRightRotate(x, xParent);
                    }
                }
                else if( parentBF > 1 ) {
                    //RIGHT RIGHT
                    idx rightBF = getBFactors (x - _vStart);
                    if( rightBF == 1 ) {
                        SimpleLeftRotate(xParent);
                    }
                    //RIGHT LEFT
                    else { // equal to double left
                        RightLeftRotate(x, xParent);
                    }
                }
                else if (parentBF == 0){
                    setBFactors (iParent, parentBF);
                    break;
                }
                else {
                    setBFactors (iParent, parentBF);
                }
                // Update the value of the node for the next check
                if (x->parent == xParent-_vStart){
                    x = xParent;
                }
                else if (x->parent == xParent->parent){
                    x = x->parent + _vStart;
                }
                // Update the parent
                xParent = x->parent != NIL ? x->parent + _vStart : 0;
                // If the current balance factor is 0, we must finish
                if (getBFactors(x-_vStart) == 0){
                    break;
                }

            }
            root->parent = root - _vStart;  // might not be necessary
        }

        void insertRB(Nobj start, Nobj end, Kobj value)
        {
            sRangeNode * y;
            idx lastOffRoot = root - _vStart;

            sRangeNode * x = _vec.add();
            if( _vec.dim() == 1 ) {
                _vStart = x;
            }

            _vStart = _vec.ptr();
            root = lastOffRoot + _vStart;
            root->parent = NIL;
            x->init(start, end, value);

            PositionInTree(x);
            while( isRed(x->parent) ) {
                if( x->parent == ((_vStart + x->parent)->parent + _vStart)->left ) {
                    y = _vStart + ((x->parent + _vStart)->parent + _vStart)->right;
                    if( isRed(y - _vStart) ) {
                        setColor(x->parent, BLACK);
                        setColor(y - _vStart, BLACK);
                        setColor(y->parent, RED);
                        x = y->parent + _vStart;
                    } else {
                        if( x == _vStart + (_vStart + x->parent)->right ) {
                            x = _vStart + x->parent;
                            LeftRotate(x);
                        }
                        setColor(x->parent, BLACK);
                        if( x->parent != NIL )
                            setColor((_vStart + x->parent)->parent, RED);
                        if( x->parent != NIL )
                            RightRotate((_vStart + x->parent)->parent + _vStart);
                    }

                } else {
                    y = _vStart + ((_vStart + x->parent)->parent + _vStart)->left;
                    if( isRed(y - _vStart) ) {
                        setColor(x->parent, BLACK);
                        setColor(y - _vStart, BLACK);
                        setColor(y->parent, RED);
                        x = y->parent + _vStart;
                    } else {
                        if( x == _vStart + (_vStart + x->parent)->left ) {
                            x = _vStart + x->parent;
                            RightRotate(x);
                        }
                        setColor(x->parent, BLACK);
                        if( x->parent != NIL )
                            setColor((_vStart + x->parent)->parent, RED);
                        if( x->parent != NIL )
                            LeftRotate((_vStart + x->parent)->parent + _vStart);
                    }
                }
            }
            FixUpMax (x);
            root->parent = root - _vStart;
            setColor(root - _vStart, BLACK);
        }

    private:

        idx SimpleRightRotate(sRangeNode * y)
        {
            idx yLeft = y->left;
            idx res = RightRotate(y);
            if( res ){
                idx yLeftBFactor = getBFactors(yLeft);
                idx yBFactor = -2;//getBFactors(y - _vStart);
                yBFactor = yBFactor + 1 + RangeMax (0, -yLeftBFactor);
                yLeftBFactor = yLeftBFactor + 1 + RangeMax (0, yBFactor);
                setBFactors(yLeft, yLeftBFactor);
                setBFactors(y-_vStart, yBFactor);
            }
            return res;
        }

        idx SimpleLeftRotate(sRangeNode * x)
        {
            idx xRight = x->right;
            idx res = LeftRotate(x);
            if( res ){
                idx xBFactor = 2;//getBFactors(x - _vStart);
                idx xRightBFactor = getBFactors(xRight);
                xBFactor = xBFactor - 1 + RangeMin (0, -xRightBFactor);
                xRightBFactor = xRightBFactor - 1 + RangeMin (0, xBFactor);
                setBFactors(x-_vStart, xBFactor);
                setBFactors(xRight, xRightBFactor);
            }
            return res;
        }

        idx RightLeftRotate(sRangeNode * y, sRangeNode * x)
        {
            idx yLeft = y->left;
            idx res = RightRotate(y);
            if( res ) {
                idx yLeftBFactor = getBFactors(yLeft);
                idx yBFactor = getBFactors(y - _vStart);
                yBFactor = yBFactor + 1 + RangeMax (0, -yLeftBFactor);
                yLeftBFactor = yLeftBFactor + 1 + RangeMax (0, yBFactor);
                setBFactors(y-_vStart, yBFactor);

                idx xRight = x->right;
                res = LeftRotate(x);
                if( res ) {
                    idx xBFactor = 2;//getBFactors(x - _vStart);
                    idx xRightBFactor = yLeftBFactor;
                    xBFactor = xBFactor - 1 + RangeMin (0, -xRightBFactor);
                    xRightBFactor = xRightBFactor - 1 + RangeMin (0, xBFactor);
                    setBFactors(x-_vStart, xBFactor);
                    setBFactors(xRight, xRightBFactor);
                    return res;
                }
            }
            return res;
        }

        idx LeftRightRotate(sRangeNode * x, sRangeNode * y)
        {
            idx xRight = x->right;
            idx res = LeftRotate(x);
            if( res ) {
                idx xBFactor = getBFactors(x - _vStart);
                idx xRightBFactor = getBFactors(xRight);
                xBFactor = xBFactor - 1 + RangeMin (0, -xRightBFactor);
                xRightBFactor = xRightBFactor - 1 + RangeMin (0, xBFactor);
                setBFactors(x-_vStart, xBFactor);

                idx yLeft = y->left;
                res = RightRotate(y);
                if( res ) {
                    idx yLeftBFactor = xRightBFactor;
                    idx yBFactor = -2;//getBFactors(y - _vStart);
                    yBFactor = yBFactor + 1 + RangeMax (0, -yLeftBFactor);
                    yLeftBFactor = yLeftBFactor + 1 + RangeMax (0, yBFactor);
                    setBFactors(yLeft, yLeftBFactor);
                    setBFactors(y-_vStart, yBFactor);
                    return res;
                }
            }
            return res;
        }

        idx LeftRotate(sRangeNode * x)
        {
            if( x->right == NIL )
                return 0;
            sRangeNode * y = _vStart + x->right;

            idx xParent = x->parent, yLeft = y->left;
            x->parent = x->right;
            x->right = y->left;

            y->left = y->parent;
            y->parent = xParent;

            if( xParent != NIL ) {
                sRangeNode * parent = _vStart + xParent;
                if( (_vStart + parent->right) == x ) {
                    parent->right = y - _vStart;
                } else {
                    parent->left = y - _vStart;
                }
            }
            if( yLeft != NIL ) {
                sRangeNode * leftleaf = _vStart + yLeft;
                leftleaf->parent = x - _vStart;
            }

            FixUpMax(x, true);
            FixUpMax(y, true);

            if( x == root )
                root = y;
            return 1;
        }

        idx RightRotate(sRangeNode * y)
        {
            if( y->left == NIL )
                return 0;
            sRangeNode * x = _vStart + y->left;

            idx yParent = y->parent, xRight = x->right;
            y->parent = y->left;
            y->left = x->right;

            x->right = x->parent;
            x->parent = yParent;

            if( yParent != NIL ) {
                sRangeNode * parent = _vStart + yParent;
                if( (_vStart + parent->right) == y ) {
                    parent->right = x - _vStart;
                } else {
                    parent->left = x - _vStart;
                }
            }
            if( xRight != NIL ) {
                sRangeNode * leftleaf = _vStart + xRight;
                leftleaf->parent = y - _vStart;
            }

            FixUpMax(y, true);
            FixUpMax(x, true);
            if( y == root )
                root = x;
            return 1;
        }

        sRangeNode * PositionInTree(sRangeNode * z)
        {
            sRangeNode * zParent = 0;
            if( root != z ) {
                sRangeNode* x = root;
                sRangeNode* y = 0;

                while( x ) {
                    y = x;
                    if( x->start > z->start ) {
                        x = x->left < 0 ? 0 : (_vStart + x->left);
                    } else {
                        x = x->right < 0 ? 0 : (_vStart + x->right);
                    }
                }
                z->parent = y - _vStart;
                if( y->start > z->start ) {
                    y->left = z - _vStart;
                } else {
                    y->right = z - _vStart;
                }
                zParent = z->parent + _vStart;
            }
            if( mode == eRBmode )
                setColor(z - _vStart, RED);
            else {
                setBFactors (z - _vStart, 0);
            }
            return zParent;
        }

        void FixUpMax(sRangeNode * x, idx oneStep = 0)
        {
            idx xMax = 0;
            idx leftMax = x->left < 0 ? -sIdxMax : (_vStart + x->left)->max;
            idx rightMax = x->right < 0 ? -sIdxMax : (_vStart + x->right)->max;
            x->max = RangeMax(x->end, RangeMax(leftMax ,rightMax));
            x = (x->parent == NIL || oneStep) ? 0 : _vStart + x->parent;
            while( x ) {
                leftMax = x->left < 0 ? -sIdxMax : (_vStart + x->left)->max;
                rightMax = x->right < 0 ? -sIdxMax : (_vStart + x->right)->max;
                xMax = RangeMax(x->end, RangeMax(leftMax ,rightMax));
                if (x->max == xMax) {
                    break;
                }
                x->max = xMax;
                x = (x->parent == NIL || oneStep) ? 0 : _vStart + x->parent;
            }
        }

        void setColor(idx pos, bool color)
        {
            idx bitsizeUDX = (sizeof(udx) * 8);
            if( ((pos / bitsizeUDX) + 1) > _Color.dim() ) {
                _Color.vadd(1, 0);
            }
            idx cPtr = pos / bitsizeUDX;
            idx cBit = pos % bitsizeUDX;
            *_Color.ptr(cPtr) &= ~((udx)1 << cBit);
            *_Color.ptr(cPtr) |= (udx)color << cBit;
        }
        idx isRed(idx pos)
        {
            if( pos < 0 )
                return BLACK;
            idx bitsizeUDX = (sizeof(udx) * 8);
            if( ((pos / bitsizeUDX) + 1) > _Color.dim() ) {
                return 0;
            }
            idx cPtr = pos / bitsizeUDX;
            idx cBit = pos % bitsizeUDX;
            return (*_Color.ptr(cPtr)) & ( (udx)1 << cBit);
        }

        void setBFactors(idx pos, idx value)
        {
            idx halfByteSizeUDX = (sizeof(udx) * 4);
            if( ((pos / halfByteSizeUDX) + 1) > _BFactors.dim() ) {
                //udx initB = 0x2222222222222222ull;
                udx initB = 0xAAAAAAAAAAAAAAAAull;
                _BFactors.vadd(1, initB);
            }
            idx cPtr = pos / halfByteSizeUDX;
            idx cBit = 2 * (pos % halfByteSizeUDX);
            *_BFactors.ptr(cPtr) &= ~((udx)0x3 << cBit);
            *_BFactors.ptr(cPtr) |= (udx)(2 + value) << cBit;
        }

        idx getBFactors(idx pos)
        {
            if( pos < 0 )
                return 0;
            idx halfByteSizeUDX = (sizeof(udx) * 4);
            if( ((pos / halfByteSizeUDX) + 1) > _BFactors.dim() ) {
                return 0;
            }
            idx cPtr = pos / halfByteSizeUDX;
            idx cBit = 2 * (pos % halfByteSizeUDX);
            return ((*_BFactors.ptr(cPtr) >> cBit) & 0x3) - 2;
        }
};

};
#endif
