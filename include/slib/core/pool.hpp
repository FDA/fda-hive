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
    /*! \brief Base class for an allocation pool to control the number of expensive object instances that exist simultaneously
     * \tparam T An expensive class whose population of instances needs to be controlled */
    template <class T>
    class sAllocPool
    {
    protected:
        //! Base class for a delayed allocator. See sFilPool::sFilAllocator for implementation example.
        template <class S>
        class sAllocator
        {
        public:
            S* obj; //!< The allocated object
            idx users; //!< Number of users (i.e. number of sAllocPool::request calls for this item)
            idx allocOrder; //!< Of all alloc() calls for sAllocPool items, when was this item's last alloc() call made
            idx unusageOrder; //!< Of all placements into sAllocPool::_unusedAllocated, when was this item last placed there

            /*! \brief Initialize the allocator, but delay really allocating #obj until alloc() is called */
            sAllocator(): obj(NULL), users(0), allocOrder(-1), unusageOrder(-1) {}

            virtual ~sAllocator() { users = 0; dealloc(); }
            /*! \brief This function should allocate #obj */
            virtual bool alloc() = 0;
            /*! \brief Deallocate #obj */
            virtual void dealloc()
            {
                if (obj) {
                    delete obj;
                    obj = NULL;
                }
            }
            /*! \brief Decide whether this allocator is to be dealloc()-ed after rhs */
            virtual bool deallocAfter(sAllocator<S> *rhs)
            {
                return allocOrder > rhs->allocOrder;
            }
#ifdef _DEBUG_POOL
            virtual void dump(sStr *s)
            {
                s->printf("obj = %p, users = %"DEC", allocOrder = %"DEC", unusageOrder = %"DEC, obj, users, allocOrder, unusageOrder);
            }
#endif
        };

        //! binary comparison of sAllocator handles for deallocation queueing purposes
        template <class S>
        struct sAllocatorCmp {
            const sVec<sAllocator<T>*> *_pool;
            sAllocatorCmp(): _pool(NULL) {}
            inline bool operator() (idx x, idx y) const { return !((*_pool)[x]->deallocAfter((*_pool)[y])); }
        };

        typedef sAllocator<T> Talloc;
        typedef sAllocatorCmp<T> TallocCmp;

        sVec<Talloc*> _pool;
        idx _dimPool; //!< number of allocators in _pool
        idx _maxUsable; //!< max number of allocators we can have in use
        idx _dimAllocated; //!< number of alloc()-ed allocators

        TallocCmp _unusedAllocatedCmp;
        sHeap<idx, TallocCmp> _unusedAllocated; //!< queue of allocators that can be dealloc()-ed
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
        /*! \param maxsize Maximum number of pool items allowed to be in use at one time */
        sAllocPool<T>(idx maxUsable = 0): _unusedAllocated(NULL, eHeapFlags_DEFAULT|eHeapFlags_Pushy) { init(maxUsable); }
        virtual ~sAllocPool<T>()
        {
            for (idx i=0; i<_dimPool; i++) {
                delete _pool[i];
            }
        }

        /*! \brief Add a new delayed allocation declaration to the pool. See sFilPool::declare for implementation example.
         * \warning Although declare() does not by itself allocate the item, storing the declaration does
         *          take space, so each declare() call increases the pool's memory usage.
         * \returns The new pool item declaration's handle */
        virtual idx declare() = 0;

        /*! \brief Checks whether the handle points to a valid, declared pool item */
        virtual bool validHandle(idx handle) const { return handle >= 0 && handle < _dimPool; }

        static idx invalidHandle() { return -1; }

        /*! \brief Request for the specified pool item to be allocated if it wasn't allocated already
         * \warning request() increments the pool item's usage counter; ensure each request() call is
         *          eventually followed by a release() to avoid leaks.
         * \param handle Pool item handle
         * \returns pointer to the specified pool item, or NULL on failure */
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

        /*! \brief Inform the pool that an object is not currently needed
         * \note release() decrements the pool item's usage counter; the item will be deallocated only
         *       after the counter reaches zero <em>and</em> the pool reaches its limit of allocated
         *       items allowed at one time.
         * \param handle Pool item handle */
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

            // a is allocated with exactly 1 user; that last user releases it, so we push it to unused-but-allocated queue
            a->users = 0;
            assert(a->unusageOrder < 0);
            _unusedAllocated.push(handle);
            a->unusageOrder = _totalUnusages++;
        }

        /*! \brief Number of pool items that have been declared */
        virtual inline idx dim() const    { return _dimPool; }
        /*! \brief Max number of pool items that can be allocated and in use */
        virtual inline idx maxUsable() const { return _maxUsable; }
        /*! \brief Number of pool items that are allocated and in use */
        virtual inline idx dimUsed() const { return _dimAllocated - _unusedAllocated.dim(); }

#ifdef _DEBUG_POOL
        virtual void dump(sStr *s)
        {
            s->printf("Pool items (%"DEC" total):\n", dim());
            for (idx i=0; i<dim(); i++) {
                s->printf("\t%"DEC" = {", i);
                _pool[i]->dump(s);
                s->printf("}\n");
            }
            s->printf("Unused allocated queue : %"DEC" total", _unusedAllocated.dim());
            if (_unusedAllocated.dim()) {
                s->printf("; top = %"DEC", other = {", _unusedAllocated.peekValue());
                bool first_other = true;
                for (idx i=0; i<dim(); i++) {
                    if (i == _unusedAllocated.peekValue())
                        continue;
                    if (_unusedAllocated.validIndex(_pool[i]->unusageOrder)) {
                        if (first_other)
                            first_other = false;
                        else
                            s->printf(", ");
                        s->printf("%"DEC, i);
                    }
                }
                s->printf("}");
            }
            s->printf("\n");
        }
#endif
    };

    //! Allocation pool for sFil objects
    /*! Useful when no more than a certain fixed number of files must be open at any one time. */
    class sFilPool : public sAllocPool<sFil>
    {
    protected:
        //! Allocates sFil objects
        class sFilAllocator : public sAllocPool<sFil>::sAllocator<sFil>
        {
        public:
            sStr _flnm;
            idx _flags;
            /*! \brief Initialize the allocator, but delay really allocating the sFil #obj */
            sFilAllocator(const char * flnm=0, idx flags=sMex::fBlockDoubling):
                sAllocPool<sFil>::sAllocator<sFil>(), _flnm("%s",flnm), _flags(flags) {}
            /*! \brief Really allocate the sFil #obj */
            virtual bool alloc()
            {
                this->obj = new sFil(_flnm.ptr(), _flags);
                return (this->obj != NULL);
            }

#ifdef _DEBUG_POOL
            virtual void dump(sStr *s)
            {
                s->printf("flnm = %s, flags = %"DEC", ", _flnm.ptr(), _flags);
                sAllocPool<sFil>::sAllocator<sFil>::dump(s);
            }
#endif
        };

    public:
        /*! \param maxUsable Maximum number of files allowed to be open at any time */
        sFilPool(idx maxUsable = 0): sAllocPool<sFil>(maxUsable) {}
        virtual ~sFilPool() {}

        /*! \brief Add a new sFil delayed allocation declaration to the pool
         * \param flnm Filename
         * \param flags Flags parameter for sFil constructor
         * \returns The new sFil declaration's handle */
        virtual idx declare(const char * flnm, idx flags=sMex::fBlockDoubling)
        {
            this->_pool.add();
            this->_pool[_dimPool] = new sFilAllocator(flnm, flags);
            return this->_dimPool++;
        }
        //! Add a trivial sFil delayed allocation declaration to the pool; needed to override pure virtual base class method
        virtual inline idx declare() { return declare(0); }
    };
}

#endif
