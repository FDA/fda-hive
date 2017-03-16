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
#ifndef sLib_core_atomic_hpp
#define sLib_core_atomic_hpp

#include <slib/core/os.hpp>

#if __STDC_VERSION >= 20112L && !defined(__STDC_NO_ATOMICS__)
#define __C11_ATOMICS
#elif __has_extension(c_atomic) || __has_extension(cxx_atomic)
#define __CLANG_ATOMICS
#elif defined(__c11_atomic_thread_fence) && defined(__ATOMIC_SEQ_CST)
#define __CLANG_ATOMICS
#elif defined(__atomic_thread_fence) && defined(__ATOMIC_SEQ_CST)
#define __GCC_4_7_ATOMICS
#elif defined(__GNUC__)
#define __GCC_OLD_ATOMICS
#else
#error "slib/core/atomic.hpp does not support your compiler"
#endif

#ifdef __C11_ATOMICS

#include <stdatomic.h>
#define sAtomicThreadFence atomic_thread_fence(memory_order_seq_cst)
#define sAtomicSignalFence atomic_signal_fence(memory_order_seq_cst)
#define sAtomicStore(ptr, val) atomic_store(ptr, val)
#define sAtomicLoad(ptr) atomic_load(ptr)
#define sAtomicExchange(ptr, newval) atomic_exchange(ptr, newval)
#define sAtomicCompareExchange(ptr, ptr_expected, newval) atomic_compare_exchange_strong(ptr, ptr_expected, newval)
#define sAtomicFetchAdd(ptr, val) atomic_fetch_add(ptr, val)
#define sAtomicFetchSub(ptr, val) atomic_fetch_sub(ptr, val)
#define sAtomicFetchAnd(ptr, val) atomic_fetch_and(ptr, val)
#define sAtomicFetchOr(ptr, val) atomic_fetch_or(ptr, val)
#define sAtomicFetchXor(ptr, val) atomic_fetch_xor(ptr, val)

#elif defined(__CLANG_ATOMICS)

#define sAtomicThreadFence __c11_atomic_thread_fence(__ATOMIC_SEQ_CST)
#define sAtomicSignalFence __c11_atomic_signal_fence(__ATOMIC_SEQ_CST)
#define sAtomicStore(ptr, val) __c11_atomic_store(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicLoad(ptr) __c11_atomic_load(ptr, __ATOMIC_SEQ_CST)
#define sAtomicExchange(ptr, newval) __c11_atomic_exchange(ptr, newval, __ATOMIC_SEQ_CST)
#define sAtomicCompareExchange(ptr, ptr_expected, newval) __c11_atomic_compare_exchange_strong(ptr, ptr_expected, newval, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define sAtomicFetchAdd(ptr, val) __c11_atomic_fetch_add(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicFetchSub(ptr, val) __c11_atomic_fetch_sub(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicFetchAnd(ptr, val) __c11_atomic_fetch_and(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicFetchOr(ptr, val) __c11_atomic_fetch_or(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicFetchXor(ptr, val) __c11_atomic_fetch_xor(ptr, val, __ATOMIC_SEQ_CST)

#elif defined(__GCC_4_7_ATOMICS)

#define sAtomicThreadFence __atomic_thread_fence(__ATOMIC_SEQ_CST)
#define sAtomicSignalFence __atomic_signal_fence(__ATOMIC_SEQ_CST)
#define sAtomicStore(ptr, val) __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicLoad(ptr) __atomic_load_n(ptr, __ATOMIC_SEQ_CST)
#define sAtomicExchange(ptr, newval) __atomic_exchange_n(ptr, newval, __ATOMIC_SEQ_CST)
#define sAtomicCompareExchange(ptr, ptr_expected, newval) __atomic_compare_exchange_n(ptr, ptr_expected, newval, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define sAtomicFetchAdd(ptr, val) __atomic_fetch_add(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicFetchSub(ptr, val) __atomic_fetch_sub(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicFetchAnd(ptr, val) __atomic_fetch_and(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicFetchOr(ptr, val) __atomic_fetch_or(ptr, val, __ATOMIC_SEQ_CST)
#define sAtomicFetchXor(ptr, val) __atomic_fetch_xor(ptr, val, __ATOMIC_SEQ_CST)

#elif defined(__GCC_OLD_ATOMICS)

#define sAtomicThreadFence __sync_synchronize()
#define sAtomicSignalFence (__asm volatile ("" : : : "memory"))
#define sAtomicStore(ptr, val) (__extension__({ *ptr = val; __sync_synchronize(); }))
#define sAtomicLoad(ptr) (__extension__({ __sync_synchronize(); *ptr; }))
#define sAtomicExchange(ptr, newval) (__extension__({ __sync_synchronize(); __sync_lock_test_and_set(ptr, newval); }))
#define sAtomicCompareExchange(ptr, ptr_expected, newval) \
    (__extension__({ \
        __typeof__(expected) _pe = (ptr_expected); \
        __typeof__(*_pe) _e = *_pe; \
        (*_pe = __sync_bool_compare_and_swap(ptr, _e, newval)) == _e; \
    }))
#define sAtomicFetchAdd(ptr, val) __sync_fetch_and_add(ptr, val)
#define sAtomicFetchSub(ptr, val) __sync_fetch_and_sub(ptr, val)
#define sAtomicFetchAnd(ptr, val) __sync_fetch_and_and(ptr, val)
#define sAtomicFetchOr(ptr, val) __sync_fetch_and_or(ptr, val)
#define sAtomicFetchXor(ptr, val) __sync_fetch_and_xor(ptr, val)

#endif

#endif
