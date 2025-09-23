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
#ifndef sLib_core_pool_hpp
#define sLib_core_pool_hpp

#include <slib/core/heap.hpp>
#include <slib/core/str.hpp>
#include <slib/core/vec.hpp>
#include <assert.h>

namespace slib {
    template <class T>
    class sAllocPool
    {
    protected:
        template <class S>
        class sAllocator
        {
        public:
            S* obj;
            idx users;
            idx allocOrder;
            idx unusageOrder;

            sAllocator(): obj(NULL), users(0), allocOrder(-1), unusageOrder(-1) {}

            virtual ~sAllocator() { users = 0; dealloc(); }
            virtual bool alloc() = 0;
            virtual void dealloc()
            {
                if (obj) {
                    delete obj;
                    obj = NULL;
                }
            }
            virtual bool deallocAfter(sAllocator<S> *rhs)
            {
                return allocOrder > rhs->allocOrder;
            }
#ifdef _DEBUG_POOL
            virtual void dump(sStr *s)
            {
                s->printf("obj = %p, users = %" DEC ", allocOrder = %" DEC ", unusageOrder = %" DEC, obj, users, allocOrder, unusageOrder);
            }
#endif
        };

        template <class S>
        struct sAllocatorCmp {
            const sVec<sAllocator<T>*> *_pool;
            sAllocatorCmp(): _pool(NULL) {}
            inline bool operator() (idx x, idx y) const { return !((*_pool)[x]->deallocAfter((*_pool)[y])); }
        };

        typedef sAllocator<T> Talloc;
        typedef sAllocatorCmp<T> TallocCmp;

        sVec<Talloc*> _pool;
        idx _dimPool;
        idx _maxUsable;
        idx _dimAllocated;

        TallocCmp _unusedAllocatedCmp;
        sHeap<idx, TallocCmp> _unusedAllocated;
        idx _totalAllocs;
        idx _totalUnusages;

        virtual void init(idx maxUsable)
        {
            _dimPool = _dimAllocated = _totalAllocs = _totalUnusages = 0;
            _maxUsable = maxUsable;
            _unusedAllocatedCmp._pool = &_pool;
            _unusedAllocated.setCmp(&_unusedAllocatedCmp);
        }

        virtual bool deallocUnused()
        {
            if (_unusedAllocated.dim() <= 0)
                return false;

            Talloc *a = _pool[_unusedAllocated.pop()];
            assert (a->obj);
            assert (!a->users);

            a->unusageOrder = -1;
            a->allocOrder = -1;
            a->dealloc();
            _dimAllocated--;

            return true;
        }

        virtual bool startUsing(idx handle)
        {
            Talloc *a = _pool[handle];

            if (!a->obj && a->alloc()) {
                _dimAllocated++;
            }

            if (a->obj) {
                a->users++;
                if (_unusedAllocated.validIndex(a->unusageOrder))
                    _unusedAllocated.remove(a->unusageOrder);
                a->allocOrder = _totalAllocs++;
                a->unusageOrder = -1;
                return true;
            }
            return false;
        }

    public:
        sAllocPool<T>(idx maxUsable = 0): _unusedAllocated(NULL, eHeapFlags_DEFAULT|eHeapFlags_Pushy) { init(maxUsable); }
        virtual ~sAllocPool<T>()
        {
            for (idx i=0; i<_dimPool; i++) {
                delete _pool[i];
            }
        }

        virtual idx declare() = 0;

        virtual bool validHandle(idx handle) const { return handle >= 0 && handle < _dimPool; }

        static idx invalidHandle() { return -1; }

        virtual T* request(idx handle)
        {
            Talloc *a = _pool[handle];

            if (a->users > 0) {
                a->users++;
            } else {
                if (_dimAllocated >= _maxUsable && !deallocUnused())
                    return NULL;
                assert(_dimAllocated < _maxUsable);
                startUsing(handle);
            }
            return a->obj;
        }

        virtual void release(idx handle)
        {
            Talloc *a = _pool[handle];

            if (!a->obj || a->users <= 0) {
                assert (a->users == 0);
                return;
            }

            if (a->users > 1) {
                a->users--;
                return;
            }

            a->users = 0;
            assert(a->unusageOrder < 0);
            _unusedAllocated.push(handle);
            a->unusageOrder = _totalUnusages++;
        }

        virtual inline idx dim() const    { return _dimPool; }
        virtual inline idx maxUsable() const { return _maxUsable; }
        virtual inline idx dimUsed() const { return _dimAllocated - _unusedAllocated.dim(); }

#ifdef _DEBUG_POOL
        virtual void dump(sStr *s)
        {
            s->printf("Pool items (%" DEC " total):\n", dim());
            for (idx i=0; i<dim(); i++) {
                s->printf("\t%" DEC " = {", i);
                _pool[i]->dump(s);
                s->printf("}\n");
            }
            s->printf("Unused allocated queue : %" DEC " total", _unusedAllocated.dim());
            if (_unusedAllocated.dim()) {
                s->printf("; top = %" DEC ", other = {", _unusedAllocated.peekValue());
                bool first_other = true;
                for (idx i=0; i<dim(); i++) {
                    if (i == _unusedAllocated.peekValue())
                        continue;
                    if (_unusedAllocated.validIndex(_pool[i]->unusageOrder)) {
                        if (first_other)
                            first_other = false;
                        else
                            s->printf(", ");
                        s->printf("%" DEC, i);
                    }
                }
                s->printf("}");
            }
            s->printf("\n");
        }
#endif
    };

    class sFilPool : public sAllocPool<sFil>
    {
    protected:
        class sFilAllocator : public sAllocPool<sFil>::sAllocator<sFil>
        {
        public:
            sStr _flnm;
            idx _flags;
            sFilAllocator(const char * flnm=0, idx flags=sMex::fBlockDoubling):
                sAllocPool<sFil>::sAllocator<sFil>(), _flnm("%s",flnm), _flags(flags) {}
            virtual bool alloc()
            {
                this->obj = new sFil(_flnm.ptr(), _flags);
                return (this->obj != NULL);
            }

#ifdef _DEBUG_POOL
            virtual void dump(sStr *s)
            {
                s->printf("flnm = %s, flags = %" DEC ", ", _flnm.ptr(), _flags);
                sAllocPool<sFil>::sAllocator<sFil>::dump(s);
            }
#endif
        };

    public:
        sFilPool(idx maxUsable = 0): sAllocPool<sFil>(maxUsable) {}
        virtual ~sFilPool() {}

        virtual idx declare(const char * flnm, idx flags=sMex::fBlockDoubling)
        {
            this->_pool.add();
            this->_pool[_dimPool] = new sFilAllocator(flnm, flags);
            return this->_dimPool++;
        }
        virtual inline idx declare() { return declare(0); }
    };
}

#endif
