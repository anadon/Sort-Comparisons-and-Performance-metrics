/*
 * C++ implementation of timsort
 *
 * ported from Python's and OpenJDK's:
 * - http://svn.python.org/projects/python/trunk/Objects/listobject.c
 * - http://cr.openjdk.java.net/~martin/webrevs/openjdk7/timsort/raw_files/new/src/share/classes/java/util/TimSort.java
 *
 * Copyright (c) 2011 Fuji, Goro (gfx) <gfuji@cpan.org>. C++03/move-compliance modifications by Matt Bentley 2017 (mattreecebentley@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <string>
#ifdef __has_include
# if __has_include(<string_view>)
#  include <string_view>
# endif
#endif
#include <type_traits>
#include <utility>
#include <valarray>
#include <vector>


#ifdef ENABLE_TIMSORT_LOG
#include <iostream>
#define GFX_TIMSORT_LOG(expr) (std::clog << "# " << __func__ << ": " << expr << std::endl)
#else
#define GFX_TIMSORT_LOG(expr) ((void)0)
#endif

// If compiler supports both type traits and move semantics - will cover most but not all compilers/std libraries:
#if (defined(_MSC_VER) && _MSC_VER >= 1700) || ((defined(__cplusplus) && __cplusplus >= 201103L && !defined(_LIBCPP_VERSION)) && ((!defined(__GNUC__) || __GNUC__ >= 5)) && (!defined(__GLIBCXX__) ||  __GLIBCXX__ >= 20150422))
  #include <iterator> // iterator_traits
  #include <utility> // std::move

  #define GFX_TIMSORT_MOVE(x) (std::is_move_constructible<value_t>::value && std::is_move_assignable<value_t>::value) ? std::move(x) : (x)
  #define GFX_TIMSORT_MOVE_RANGE(in1, in2, out) \
    if (std::is_move_constructible<value_t>::value && std::is_move_assignable<value_t>::value) \
    { \
      std::move((in1), (in2), (out)); \
    } \
    else \
    { \
      std::copy((in1), (in2), (out)); \
    }
  #define GFX_TIMSORT_MOVE_BACKWARD(in1, in2, out) \
    if (std::is_move_constructible<value_t>::value && std::is_move_assignable<value_t>::value) \
    { \
      std::move_backward((in1), (in2), (out)); \
    } \
    else \
    { \
      std::copy_backward((in1), (in2), (out)); \
    }
#else
  #define GFX_TIMSORT_MOVE(x) (x)
  #define GFX_TIMSORT_MOVE_RANGE(in1, in2, out) std::copy((in1), (in2), (out));
  #define GFX_TIMSORT_MOVE_BACKWARD(in1, in2, out) std::copy_backward((in1), (in2), (out));
#endif


namespace gfx {

// ---------------------------------------
// Declaration
// ---------------------------------------

/**
 * Same as std::stable_sort(first, last).
 */
template <typename RandomAccessIterator>
inline void timsort(RandomAccessIterator const first, RandomAccessIterator const last);

/**
 * Same as std::stable_sort(first, last, c).
 */
template <typename RandomAccessIterator, typename LessFunction>
inline void timsort(RandomAccessIterator const first, RandomAccessIterator const last, LessFunction compare);

// ---------------------------------------
// Implementation
// ---------------------------------------

template <typename Value, typename LessFunction> class Compare {
  public:
    typedef Value value_type;
    typedef LessFunction func_type;

    Compare(LessFunction f) : less_(f) {
    }
    Compare(const Compare<value_type, func_type> &other) : less_(other.less_) {
    }

    bool lt(value_type x, value_type y) {
        return less_(x, y);
    }
    bool le(value_type x, value_type y) {
        return less_(x, y) || !less_(y, x);
    }
    bool gt(value_type x, value_type y) {
        return !less_(x, y) && less_(y, x);
    }
    bool ge(value_type x, value_type y) {
        return !less_(x, y);
    }

    func_type &less_function() {
        return less_;
    }

  private:
    func_type less_;
};

template <typename RandomAccessIterator, typename LessFunction> class TimSort {
    typedef RandomAccessIterator iter_t;
    typedef typename std::iterator_traits<iter_t>::value_type value_t;
    typedef typename std::iterator_traits<iter_t>::reference ref_t;
    typedef typename std::iterator_traits<iter_t>::difference_type diff_t;
    typedef Compare<const value_t &, LessFunction> compare_t;

    static const int MIN_MERGE = 32;

    compare_t comp_;

    static const int MIN_GALLOP = 7;

    int minGallop_; // default to MIN_GALLOP

    std::vector<value_t> tmp_; // temp storage for merges
    typedef typename std::vector<value_t>::iterator tmp_iter_t;

    struct run {
        iter_t base;
        diff_t len;

        run(iter_t const b, diff_t const l) : base(b), len(l) {
        }
    };
    std::vector<run> pending_;

    static void sort(iter_t const lo, iter_t const hi, compare_t c) {
        assert(lo <= hi);

        diff_t nRemaining = (hi - lo);
        if (nRemaining < 2) {
            return; // nothing to do
        }

        if (nRemaining < MIN_MERGE) {
            diff_t const initRunLen = countRunAndMakeAscending(lo, hi, c);
            GFX_TIMSORT_LOG("initRunLen: " << initRunLen);
            binarySort(lo, hi, lo + initRunLen, c);
            return;
        }

        TimSort ts(c);
        diff_t const minRun = minRunLength(nRemaining);
        iter_t cur = lo;
        do {
            diff_t runLen = countRunAndMakeAscending(cur, hi, c);

            if (runLen < minRun) {
                diff_t const force = std::min(nRemaining, minRun);
                binarySort(cur, cur + force, cur + runLen, c);
                runLen = force;
            }

            ts.pushRun(cur, runLen);
            ts.mergeCollapse();

            cur += runLen;
            nRemaining -= runLen;
        } while (nRemaining != 0);

        assert(cur == hi);
        ts.mergeForceCollapse();
        assert(ts.pending_.size() == 1);

        GFX_TIMSORT_LOG("size: " << (hi - lo) << " tmp_.size(): " << ts.tmp_.size()
                                 << " pending_.size(): " << ts.pending_.size());
    } // sort()

    static void binarySort(iter_t const lo, iter_t const hi, iter_t start, compare_t compare) {
        assert(lo <= start && start <= hi);
        if (start == lo) {
            ++start;
        }
        for (; start < hi; ++start) {
            assert(lo <= start);
            /*const*/ value_t pivot = GFX_TIMSORT_MOVE(*start);

            iter_t const pos = std::upper_bound(lo, start, pivot, compare.less_function());
            for (iter_t p = start; p > pos; --p) {
                *p = GFX_TIMSORT_MOVE(*(p - 1));
            }
            *pos = GFX_TIMSORT_MOVE(pivot);
        }
    }

    static diff_t countRunAndMakeAscending(iter_t const lo, iter_t const hi, compare_t compare) {
        assert(lo < hi);

        iter_t runHi = lo + 1;
        if (runHi == hi) {
            return 1;
        }

        if (compare.lt(*(runHi++), *lo)) { // descending
            while (runHi < hi && compare.lt(*runHi, *(runHi - 1))) {
                ++runHi;
            }
            std::reverse(lo, runHi);
        } else { // ascending
            while (runHi < hi && compare.ge(*runHi, *(runHi - 1))) {
                ++runHi;
            }
        }

        return runHi - lo;
    }

    static diff_t minRunLength(diff_t n) {
        assert(n >= 0);

        diff_t r = 0;
        while (n >= MIN_MERGE) {
            r |= (n & 1);
            n >>= 1;
        }
        return n + r;
    }

    TimSort(compare_t c) : comp_(c), minGallop_(MIN_GALLOP) {
    }

    void pushRun(iter_t const runBase, diff_t const runLen) {
        pending_.push_back(run(runBase, runLen));
    }

    void mergeCollapse() {
        while (pending_.size() > 1) {
            diff_t n = pending_.size() - 2;

            if ((n > 0 && pending_[n - 1].len <= pending_[n].len + pending_[n + 1].len) ||
                (n > 1 && pending_[n - 2].len <= pending_[n - 1].len + pending_[n].len)) {
                if (pending_[n - 1].len < pending_[n + 1].len) {
                    --n;
                }
                mergeAt(n);
            } else if (pending_[n].len <= pending_[n + 1].len) {
                mergeAt(n);
            } else {
                break;
            }
        }
    }

    void mergeForceCollapse() {
        while (pending_.size() > 1) {
            diff_t n = pending_.size() - 2;

            if (n > 0 && pending_[n - 1].len < pending_[n + 1].len) {
                --n;
            }
            mergeAt(n);
        }
    }

    void mergeAt(diff_t const i) {
        diff_t const stackSize = pending_.size();
        assert(stackSize >= 2);
        assert(i >= 0);
        assert(i == stackSize - 2 || i == stackSize - 3);

        iter_t base1 = pending_[i].base;
        diff_t len1 = pending_[i].len;
        iter_t base2 = pending_[i + 1].base;
        diff_t len2 = pending_[i + 1].len;

        assert(len1 > 0 && len2 > 0);
        assert(base1 + len1 == base2);

        pending_[i].len = len1 + len2;

        if (i == stackSize - 3) {
            pending_[i + 1] = pending_[i + 2];
        }

        pending_.pop_back();

        diff_t const k = gallopRight(*base2, base1, len1, 0);
        assert(k >= 0);

        base1 += k;
        len1 -= k;

        if (len1 == 0) {
            return;
        }

        len2 = gallopLeft(*(base1 + (len1 - 1)), base2, len2, len2 - 1);
        assert(len2 >= 0);
        if (len2 == 0) {
            return;
        }

        if (len1 <= len2) {
            mergeLo(base1, len1, base2, len2);
        } else {
            mergeHi(base1, len1, base2, len2);
        }
    }

    template <typename Iter> diff_t gallopLeft(ref_t key, Iter const base, diff_t const len, diff_t const hint) {
        assert(len > 0 && hint >= 0 && hint < len);

        diff_t lastOfs = 0;
        diff_t ofs = 1;

        if (comp_.gt(key, *(base + hint))) {
            diff_t const maxOfs = len - hint;
            while (ofs < maxOfs && comp_.gt(key, *(base + (hint + ofs)))) {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0) { // int overflow
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs) {
                ofs = maxOfs;
            }

            lastOfs += hint;
            ofs += hint;
        } else {
            diff_t const maxOfs = hint + 1;
            while (ofs < maxOfs && comp_.le(key, *(base + (hint - ofs)))) {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0) {
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs) {
                ofs = maxOfs;
            }

            diff_t const tmp = lastOfs;
            lastOfs = hint - ofs;
            ofs = hint - tmp;
        }
        assert(-1 <= lastOfs && lastOfs < ofs && ofs <= len);

        return std::lower_bound(base + (lastOfs + 1), base + ofs, key, comp_.less_function()) - base;
    }

    template <typename Iter> diff_t gallopRight(ref_t key, Iter const base, diff_t const len, diff_t const hint) {
        assert(len > 0 && hint >= 0 && hint < len);

        diff_t ofs = 1;
        diff_t lastOfs = 0;

        if (comp_.lt(key, *(base + hint))) {
            diff_t const maxOfs = hint + 1;
            while (ofs < maxOfs && comp_.lt(key, *(base + (hint - ofs)))) {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0) {
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs) {
                ofs = maxOfs;
            }

            diff_t const tmp = lastOfs;
            lastOfs = hint - ofs;
            ofs = hint - tmp;
        } else {
            diff_t const maxOfs = len - hint;
            while (ofs < maxOfs && comp_.ge(key, *(base + (hint + ofs)))) {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0) { // int overflow
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs) {
                ofs = maxOfs;
            }

            lastOfs += hint;
            ofs += hint;
        }
        assert(-1 <= lastOfs && lastOfs < ofs && ofs <= len);

        return std::upper_bound(base + (lastOfs + 1), base + ofs, key, comp_.less_function()) - base;
    }

    void mergeLo(iter_t const base1, diff_t len1, iter_t const base2, diff_t len2) {
        assert(len1 > 0 && len2 > 0 && base1 + len1 == base2);

        copy_to_tmp(base1, len1);

        tmp_iter_t cursor1 = tmp_.begin();
        iter_t cursor2 = base2;
        iter_t dest = base1;

        *(dest++) = GFX_TIMSORT_MOVE(*(cursor2++));
        if (--len2 == 0) {
            GFX_TIMSORT_MOVE_RANGE(cursor1, cursor1 + len1, dest);
            return;
        }
        if (len1 == 1) {
            GFX_TIMSORT_MOVE_RANGE(cursor2, cursor2 + len2, dest);
            *(dest + len2) = GFX_TIMSORT_MOVE(*cursor1);
            return;
        }

        int minGallop(minGallop_);

        // outer:
        while (true) {
            int count1 = 0;
            int count2 = 0;

            bool break_outer = false;
            do {
                assert(len1 > 1 && len2 > 0);

                if (comp_.lt(*cursor2, *cursor1)) {
                    *(dest++) = GFX_TIMSORT_MOVE(*(cursor2++));
                    ++count2;
                    count1 = 0;
                    if (--len2 == 0) {
                        break_outer = true;
                        break;
                    }
                } else {
                    *(dest++) = GFX_TIMSORT_MOVE(*(cursor1++));
                    ++count1;
                    count2 = 0;
                    if (--len1 == 1) {
                        break_outer = true;
                        break;
                    }
                }
            } while ((count1 | count2) < minGallop);
            if (break_outer) {
                break;
            }

            do {
                assert(len1 > 1 && len2 > 0);

                count1 = gallopRight(*cursor2, cursor1, len1, 0);
                if (count1 != 0) {
                    GFX_TIMSORT_MOVE_BACKWARD(cursor1, cursor1 + count1, dest + count1);
                    dest += count1;
                    cursor1 += count1;
                    len1 -= count1;

                    if (len1 <= 1) {
                        break_outer = true;
                        break;
                    }
                }
                *(dest++) = GFX_TIMSORT_MOVE(*(cursor2++));
                if (--len2 == 0) {
                    break_outer = true;
                    break;
                }

                count2 = gallopLeft(*cursor1, cursor2, len2, 0);
                if (count2 != 0) {
                    GFX_TIMSORT_MOVE_RANGE(cursor2, cursor2 + count2, dest);
                    dest += count2;
                    cursor2 += count2;
                    len2 -= count2;
                    if (len2 == 0) {
                        break_outer = true;
                        break;
                    }
                }
                *(dest++) = GFX_TIMSORT_MOVE(*(cursor1++));
                if (--len1 == 1) {
                    break_outer = true;
                    break;
                }

                --minGallop;
            } while ((count1 >= MIN_GALLOP) | (count2 >= MIN_GALLOP));
            if (break_outer) {
                break;
            }

            if (minGallop < 0) {
                minGallop = 0;
            }
            minGallop += 2;
        } // end of "outer" loop

        minGallop_ = std::min(minGallop, 1);

        if (len1 == 1) {
            assert(len2 > 0);
            GFX_TIMSORT_MOVE_RANGE(cursor2, cursor2 + len2, dest);
            *(dest + len2) = GFX_TIMSORT_MOVE(*cursor1);
        } else {
            assert(len1 != 0 && "Comparison function violates its general contract");
            assert(len2 == 0);
            assert(len1 > 1);
            GFX_TIMSORT_MOVE_RANGE(cursor1, cursor1 + len1, dest);
        }
    }

    void mergeHi(iter_t const base1, diff_t len1, iter_t const base2, diff_t len2) {
        assert(len1 > 0 && len2 > 0 && base1 + len1 == base2);

        copy_to_tmp(base2, len2);

        iter_t cursor1 = base1 + (len1 - 1);
        tmp_iter_t cursor2 = tmp_.begin() + (len2 - 1);
        iter_t dest = base2 + (len2 - 1);

        *(dest--) = GFX_TIMSORT_MOVE(*(cursor1--));
        if (--len1 == 0) {
            GFX_TIMSORT_MOVE_RANGE(tmp_.begin(), tmp_.begin() + len2, dest - (len2 - 1));
            return;
        }
        if (len2 == 1) {
            dest -= len1;
            cursor1 -= len1;
            GFX_TIMSORT_MOVE_BACKWARD(cursor1 + 1, cursor1 + (1 + len1), dest + (1 + len1));
            *dest = GFX_TIMSORT_MOVE(*cursor2);
            return;
        }

        int minGallop(minGallop_);

        // outer:
        while (true) {
            int count1 = 0;
            int count2 = 0;

            bool break_outer = false;
            do {
                assert(len1 > 0 && len2 > 1);

                if (comp_.lt(*cursor2, *cursor1)) {
                    *(dest--) = GFX_TIMSORT_MOVE(*(cursor1--));
                    ++count1;
                    count2 = 0;
                    if (--len1 == 0) {
                        break_outer = true;
                        break;
                    }
                } else {
                    *(dest--) = GFX_TIMSORT_MOVE(*(cursor2--));
                    ++count2;
                    count1 = 0;
                    if (--len2 == 1) {
                        break_outer = true;
                        break;
                    }
                }
            } while ((count1 | count2) < minGallop);
            if (break_outer) {
                break;
            }

            do {
                assert(len1 > 0 && len2 > 1);

                count1 = len1 - gallopRight(*cursor2, base1, len1, len1 - 1);
                if (count1 != 0) {
                    dest -= count1;
                    cursor1 -= count1;
                    len1 -= count1;
                    GFX_TIMSORT_MOVE_BACKWARD(cursor1 + 1, cursor1 + (1 + count1), dest + (1 + count1));

                    if (len1 == 0) {
                        break_outer = true;
                        break;
                    }
                }
                *(dest--) = GFX_TIMSORT_MOVE(*(cursor2--));
                if (--len2 == 1) {
                    break_outer = true;
                    break;
                }

                count2 = len2 - gallopLeft(*cursor1, tmp_.begin(), len2, len2 - 1);
                if (count2 != 0) {
                    dest -= count2;
                    cursor2 -= count2;
                    len2 -= count2;
                    GFX_TIMSORT_MOVE_RANGE(cursor2 + 1, cursor2 + (1 + count2), dest + 1);
                    if (len2 <= 1) {
                        break_outer = true;
                        break;
                    }
                }
                *(dest--) = GFX_TIMSORT_MOVE(*(cursor1--));
                if (--len1 == 0) {
                    break_outer = true;
                    break;
                }

                minGallop--;
            } while ((count1 >= MIN_GALLOP) | (count2 >= MIN_GALLOP));
            if (break_outer) {
                break;
            }

            if (minGallop < 0) {
                minGallop = 0;
            }
            minGallop += 2;
        } // end of "outer" loop

        minGallop_ = std::min(minGallop, 1);

        if (len2 == 1) {
            assert(len1 > 0);
            dest -= len1;
            GFX_TIMSORT_MOVE_BACKWARD(cursor1 + (1 - len1), cursor1 + 1, dest + (1 + len1));
            *dest = GFX_TIMSORT_MOVE(*cursor2);
        } else {
            assert(len2 != 0 && "Comparison function violates its general contract");
            assert(len1 == 0);
            assert(len2 > 1);
            GFX_TIMSORT_MOVE_RANGE(tmp_.begin(), tmp_.begin() + len2, dest - (len2 - 1));
        }
    }

    void copy_to_tmp(iter_t const begin, diff_t const len) {
        tmp_.clear();
        tmp_.reserve(len);
        GFX_TIMSORT_MOVE_RANGE(begin, begin + len, std::back_inserter(tmp_));
    }

    // the only interface is the friend timsort() function
    template <typename IterT, typename LessT> friend void timsort(IterT first, IterT last, LessT c);
};

template <typename RandomAccessIterator>
inline void timsort(RandomAccessIterator const first, RandomAccessIterator const last) {
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;
    timsort(first, last, std::less<value_type>());
}

template <typename RandomAccessIterator, typename LessFunction>
inline void timsort(RandomAccessIterator const first, RandomAccessIterator const last, LessFunction compare) {
    TimSort<RandomAccessIterator, LessFunction>::sort(first, last, compare);
}

} // namespace gfx

#undef GFX_TIMSORT_LOG
#undef GFX_TIMSORT_MOVE
#undef GFX_TIMSORT_MOVE_RANGE
#undef GFX_TIMSORT_MOVE_BACKWARD

\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


# ifndef TIMSORT_NO_USE_COMPILER_INTRINSICS

#  if defined _MSC_VER
#   include "intrin.h"
#  endif

#  if defined  _MSC_VER
#   define COMPILER_UNREACHABLE_ __assume(0);
#  elif defined __GNUC__ || defined __clang__
#   define COMPILER_UNREACHABLE_ __builtin_unreachable();
#  else
#   define COMPILER_UNREACHABLE_
#  endif

#  if defined __GNUC__
#   define GCC_ASSUME__(x) do{ if(not (x)) {__builtin_unreachable();} } while(false);
#  endif

#  if defined  _MSC_VER || defined __INTEL_COMPILER
#   define COMPILER_ASSUME_(x) __assume(x);
#  elif defined __clang__
#   if __has_builtin(__builtin_assume)
#    define COMPILER_ASSUME_(x) __builtin_assume(x)
#   else
#    define COMPILER_ASSUME_(x) GCC_ASSUME__(x)
#   endif
#  elif defined __GNUC__
#   define COMPILER_ASSUME_(x) GCC_ASSUME__(x)
#  else
#   define COMPILER_ASSUME_(x)
#  endif

#  if defined __GNUC__
#   define COMPILER_LIKELY_(x) __builtin_expect(x, true)
#   define COMPILER_UNLIKELY_(x) __builtin_expect(x, false)
#  else
#   define COMPILER_LIKELY_(x)   (x)
#   define COMPILER_UNLIKELY_(x) (x)
#  endif

# else
#  define COMPILER_LIKELY_(x)   (x)
#  define COMPILER_UNLIKELY_(x) (x)
#  define COMPILER_ASSUME_(x)   (x)
#  define COMPILER_UNREACHABLE_

# endif /* TIMSORT_NO_USE_COMPILER_INTRINSICS */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


namespace tim {
namespace internal {

template <class It>
using iterator_difference_type_t = typename std::iterator_traits<It>::difference_type;

template <class It>
using iterator_value_type_t = typename std::iterator_traits<It>::value_type;

} /* namespace internal */
} /* namespace tim */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


namespace tim {
namespace internal {

template <class It>
struct is_valarray_iterator
{
private:
  using _value_type = iterator_value_type_t<It>;
  using _va = std::valarray<_value_type>;
  using _begin_result = std::decay_t<decltype(std::begin(std::declval<_va>()))>;
  using _begin_cresult = std::decay_t<decltype(std::begin(std::declval<const _va>()))>;
  using _end_result = std::decay_t<decltype(std::begin(std::declval<_va>()))>;
  using _end_cresult = std::decay_t<decltype(std::begin(std::declval<const _va>()))>;
public:
  static constexpr const bool value =    std::is_same_v<It, _begin_result>
              or std::is_same_v<It, _begin_cresult>
              or std::is_same_v<It, _end_result>
              or std::is_same_v<It, _end_cresult>;

};

template <class It, bool = std::is_pod_v<iterator_value_type_t<It>>>
struct is_string_iterator;

template <class It>
struct is_string_iterator<It, true>
{
  using _value_type = iterator_value_type_t<It>;
  static constexpr const bool value =
       std::is_same_v<It, typename std::basic_string<_value_type>::iterator>
          or std::is_same_v<It, typename std::basic_string<_value_type>::const_iterator>;
};

template <class It>
struct is_string_iterator<It, false> : std::false_type {};

template <class It, bool = std::is_pod_v<iterator_value_type_t<It>>>
struct is_string_view_iterator;

template <class It>
struct is_string_view_iterator<It, true>
{
  using _value_type = iterator_value_type_t<It>;
  static constexpr const bool value =
       std::is_same_v<It, typename std::basic_string<_value_type>::iterator>
          or std::is_same_v<It, typename std::basic_string<_value_type>::const_iterator>;
};

template <class It>
struct is_string_view_iterator<It, false> : std::false_type {};

template <class It>
struct is_stringview_iterator
{
  using _value_type = iterator_value_type_t<It>;
  static constexpr const bool value =
#ifdef __has_include
# if __has_include(<string_view>)
       std::is_same_v<It, typename std::basic_string_view<_value_type>::iterator>
    or std::is_same_v<It, typename std::basic_string_view<_value_type>::const_iterator>;
# else
    false;
# endif
#endif
};

template <class It>
struct is_vector_iterator
{
  using _value_type = iterator_value_type_t<It>;
  static constexpr const bool value =
       std::is_same_v<It, typename std::vector<_value_type>::iterator>
    or std::is_same_v<It, typename std::vector<_value_type>::const_iterator>;
};

template <class It>
struct is_contiguous_iterator
{
  using _value_type = iterator_value_type_t<It>;
  static constexpr const bool value =
      (is_vector_iterator<It>::value and not std::is_same_v<_value_type, bool>)
      or is_string_iterator<It>::value
      or is_string_view_iterator<It>::value
      or is_valarray_iterator<It>::value;
};

template <class T>
struct is_contiguous_iterator<T*>: std::true_type {};

template <class T>
struct is_contiguous_iterator<const T*>: std::true_type {};

template <class It>
struct is_contiguous_iterator<const It>: is_contiguous_iterator<It> {};

template <class It>
struct is_contiguous_iterator<std::reverse_iterator<std::reverse_iterator<It>>>: is_contiguous_iterator<It>{};

template <class It>
inline constexpr const bool is_contiguous_iterator_v = is_contiguous_iterator<It>::value;

template <class It>
struct is_reverse_contiguous_iterator: std::false_type{};

template <class It>
struct is_reverse_contiguous_iterator<std::reverse_iterator<It>>: is_contiguous_iterator<It>{};

template <class It>
inline constexpr const bool is_reverse_contiguous_iterator_v = is_reverse_contiguous_iterator<It>::value;


} /* namespace internal */
} /* namespace tim */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


namespace tim {
namespace internal {


/*
 * Modified variant of the compute_minrun() function used in CPython's
 * list_sort().
 *
 * The CPython version of this function chooses a value in [32, 65) for
 * minrun.  Unlike in CPython, C++ objects aren't guaranteed to be the
 * size of a pointer.  A heuristic is used here under the assumption
 * that std::move(some_arbitrary_cpp_object) is basically a bit-blit.
 * If the type is larger that 4 pointers then minrun maxes out at 32
 * instead of 64.  Similarly, if the type is larger than 8 pointers,
 * it maxes out at 16.  This is a major win for large objects
 * (think tuple-of-strings).
 * Four pointers is used as the cut-off because libstdc++'s std::string
 * implementation was slightly, but measurably worse in the benchmarks
 * when the max minrun was 32 instead of 64 (and their std::string
 * is 4 pointers large).
 */
template <class T>
static constexpr std::size_t max_minrun() noexcept
{
  if constexpr(sizeof(T) > (sizeof(void*) * 8))
    return 16;
  else if constexpr(sizeof(T) > (sizeof(void*) * 4))
    return 32;
  else
    return 64;
}

template <class T>
static constexpr std::size_t compute_minrun(std::size_t n) noexcept
{
  constexpr std::size_t minrun_max = max_minrun<T>();
  std::size_t r = 0;
  while (n >= minrun_max)
  {
    r |= (n & 1);
    n >>= 1;
  }
  return (n + r);
}


} /* namespace internal */
} /* namespace tim */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


namespace tim {
namespace internal {

template <class It>
inline constexpr const bool has_memcpy_safe_value_type_v =
    std::is_trivially_copyable_v<iterator_value_type_t<It>>;

template <class It>
static constexpr const bool can_forward_memcpy_v = is_contiguous_iterator_v<It> and has_memcpy_safe_value_type_v<It>;

template <class It>
static constexpr const bool can_reverse_memcpy_v = is_reverse_contiguous_iterator_v<It> and has_memcpy_safe_value_type_v<It>;


template <class It>
struct GetMemcpyIterator
{
  static iterator_value_type_t<It>* get(It iter) noexcept
  {
    return &(*iter);
  }
};

template <class T>
struct GetMemcpyIterator<T*>
{
  static T* get(T* iter) noexcept
  {
    return iter;
  }
};

template <class T>
struct GetMemcpyIterator<const T*>
{
  static const T* get(const T* iter) noexcept
  {
    return iter;
  }
};

template <class It>
auto get_memcpy_iterator(It iter) noexcept
{
  return GetMemcpyIterator<It>::get(iter);
}

/**
 * Try to memcpy to the destination range, otherwise use std::move.
 */
template <class SrcIt, class DestIt>
auto move_or_memcpy(SrcIt begin, SrcIt end, DestIt dest)
{
  using value_type = iterator_value_type_t<SrcIt>;
  if constexpr(can_forward_memcpy_v<SrcIt> and can_forward_memcpy_v<DestIt>)
  {
    std::memcpy(get_memcpy_iterator(dest), get_memcpy_iterator(begin), (end - begin) * sizeof(value_type));
    return dest + (end - begin);
  }
  else if constexpr(can_reverse_memcpy_v<SrcIt> and can_reverse_memcpy_v<DestIt>)
  {
    std::memcpy(get_memcpy_iterator(dest + (end - begin) - 1), get_memcpy_iterator(end - 1), (end - begin) * sizeof(value_type));
    return dest + (end - begin);
  }
  else
  {
    return std::move(begin, end, dest);
  }
}
} /* namespace internal */
} /* namespace tim */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


namespace tim {
namespace internal {


static constexpr std::size_t div_by_log2phi(std::size_t num) noexcept
{
  constexpr const double log2phi = 0.69424191363061737991557720306445844471454620361328125;
  return std::size_t(num / log2phi) + 1;
}

template <class IntType>
static constexpr std::size_t timsort_max_stack_size() noexcept
{
  return div_by_log2phi(std::numeric_limits<IntType>::digits) + 1;
}


template <class IntType, class ValueType>
struct timsort_stack_buffer
{
  using buffer_pointer_t = ValueType*;
  template <class It>
  using buffer_iter_t = std::conditional_t<(not can_forward_memcpy_v<It>) and can_reverse_memcpy_v<It>,
                   std::reverse_iterator<buffer_pointer_t>,
                   buffer_pointer_t>;
  using self_t = timsort_stack_buffer<IntType, ValueType>;
  static constexpr const bool trivial_destructor = std::is_trivially_destructible_v<ValueType>;
  static constexpr const bool nothrow_destructor = std::is_nothrow_destructible_v<ValueType>;
  static constexpr const bool nothrow_move = std::is_nothrow_move_constructible_v<ValueType>;
  static constexpr const bool trivial = std::is_trivial_v<ValueType>;

  timsort_stack_buffer() noexcept:
    buffer{},
    top{buffer + (buffer_size - 1)},
    num_in_merge_buffer{0}
  {
    push(0);
  }

  ~timsort_stack_buffer() noexcept(nothrow_destructor)
  {
    destroy_merge_buffer();
  }
  timsort_stack_buffer(const self_t&) = delete;
  timsort_stack_buffer(self_t&&) = delete;
  timsort_stack_buffer& operator=(self_t&&) = delete;
  timsort_stack_buffer& operator=(const self_t&) = delete;

  /** Returns the number of indices stored in the run stack. */
  inline std::size_t offset_count() const noexcept
  {
    return (buffer + (buffer_size - 1)) - top;
  }

  /**
   * Returns the number of runs in the run stack.
   * This is always one less than offset_count() because the first
   * item in the run stack is always SizeType(0).
   */
  inline std::size_t run_count() const noexcept
  {
    return offset_count() - 1;
  }

  /**
   * Returns the number of bytes used by the run stack.
   */
  inline std::size_t bytes_consumed_by_run_stack() const noexcept
  {
    return offset_count() * sizeof(IntType);
  }

  /**
   * Returns the number of bytes that would be used by the run stack
   * if an additional run were to be added.
   *
   * Used to determine whether some of the 'ValueType' objects in the
   * stack need to be destroyed before pushing an additional run onto
   * the run stack.
   */
  inline std::size_t bytes_consumed_by_one_more_run() const noexcept
  {
    return (offset_count() + 1) * sizeof(IntType);
  }

  /**
   * Returns the number of bytes in the stack buffer that are not
   * currently used to keep track of pending runs.
   *
   * Used to determine whether the free space in the stack buffer can be
   * used as a temporary buffer for the merge algorithm.
   */
  inline std::size_t bytes_available_for_merge_buffer() const noexcept
  {
    return buffer_size * sizeof(IntType) - bytes_consumed_by_run_stack();
  }

  /**
   * Returns the number of 'ValueType' objects that can fit in the
   * stack buffer without bleeding in to the space used to keep track
   * of pending runs.
   *
   * Used to determine whether the free space in the stack buffer can be
   * used as a temporary buffer for the merge algorithm.
   */
  inline std::size_t count_available_for_merge_buffer() const noexcept
  {
    return bytes_available_for_merge_buffer() / sizeof(ValueType);
  }

  /**
   * Returns the number of 'ValueType' objects that currently live in
   * the stack buffer and have yet to be destroyed.
   *
   * Used to determine whether some of the 'ValueType' objects in the
   * stack need to be destroyed before pushing an additional run onto
   * the run stack.
   */
  inline std::size_t bytes_consumed_by_merge_buffer() const noexcept
  {
    return num_in_merge_buffer * sizeof(ValueType);
  }
  /** Return the total number of bytes in the stack buffer. */
  static inline constexpr std::size_t total_bytes_in_buffer() noexcept
  {
    return buffer_size * sizeof(IntType);
  }
  /**
   * Returns a bool indicating whether some of the 'ValueType' objects
   * in the stack need to be destroyed before pushing an additional run
   * onto the run stack.
   */
  inline bool need_to_trim_merge_buffer() const noexcept
  {
    return (num_in_merge_buffer > 0) and
      (total_bytes_in_buffer() - bytes_consumed_by_merge_buffer()) < bytes_consumed_by_one_more_run();
  }

  /**
   * Destroy some of the 'ValueType' objects in the stack buffer to
   * make room for an additional run to be pushed onto the stack.
   */
  inline void destroy_enough_to_fit_one_more_run() noexcept
  {
    if constexpr(sizeof(ValueType) >= sizeof(IntType))
    {
      --num_in_merge_buffer;
      std::destroy_at(merge_buffer_begin() + num_in_merge_buffer);
    }
    else
    {
      std::size_t overlap =
        (total_bytes_in_buffer() - bytes_consumed_by_one_more_run())
        - bytes_consumed_by_merge_buffer();
      overlap = overlap / sizeof(ValueType) + ((overlap % sizeof(ValueType)) > 0);
      std::destroy(merge_buffer_end() - overlap, merge_buffer_end());
      num_in_merge_buffer -= overlap;
    }
  }

  /**
   * Destroy all of the 'ValueType' objects in the stack buffer.
   */
  void destroy_merge_buffer() noexcept(nothrow_destructor)
  {
    if constexpr(not trivial_destructor)
    {
      std::destroy(merge_buffer_begin(), merge_buffer_end());
      num_in_merge_buffer = 0;
    }
  }


  /*
  Move elements from [begin, end) onto the unused memory in the run stack.
  It is assumed that the caller has ensured that the stack buffer has sufficient
  unused space to hold the provided range without overwriting any of the run counts.

  This member function tries suuuuper hard to safely use a memcpy().  std::copy() and
  friends cannot
  Using compile-time type information, this copy
  */

  template <class It>
  buffer_iter_t<It> move_to_merge_buffer(It begin, It end)
    noexcept(trivial or nothrow_move)
  {

    static_assert(std::is_same_v<std::decay_t<ValueType>,
               std::decay_t<iterator_value_type_t<It>>>,
            "If you see this, timsort() is broken.");
    if constexpr (can_forward_memcpy_v<It>)
    {
      // both contiguous iterators to trivially copyable type.  do a memcpy
      std::memcpy(buffer, get_memcpy_iterator(begin), (end - begin) * sizeof(ValueType));
      return merge_buffer_begin();
    }
    else if constexpr (can_reverse_memcpy_v<It>)
    {
      // It is a reverse contiguous iterators to trivially copyable type.  do a memcpy
      // and return a reverse iterator
      std::memcpy(buffer, get_memcpy_iterator(end - 1), (end - begin) * sizeof(ValueType));
      return std::make_reverse_iterator(merge_buffer_begin() + (end - begin));
    }
    else
    {
      // not trivially copyable.  provide at least basic exception guarantee
      fill_merge_buffer(begin, end);
      return merge_buffer_begin();
    }
  }

  /**
   * Move the range [begin, end) on to the stack buffer.
   * If an exception is thrown while moving one of the objects to the
   * stack buffer, some of the objects in [begin, end) will be left in
   * a valid, but unspecified state.  No resources are leaked.
   */
  template <class It>
  void fill_merge_buffer(It begin, It end) noexcept(nothrow_move)
  {
    auto dest = merge_buffer_begin();
    if constexpr (not trivial_destructor)
    {
      if(num_in_merge_buffer < std::size_t(end - begin))
      {
        dest = std::move(begin, begin + num_in_merge_buffer, dest);
        begin += num_in_merge_buffer;
      }
      else
      {
        dest = std::move(begin, end, dest);
        begin = end;
        return;
      }
    }
    // if the destructor is trivial we don't need to increment 'num_in_merge_buffer'
    for(; begin < end; (void)++dest, (void)++begin)
    {
      ::new(static_cast<ValueType*>(std::addressof(*dest))) ValueType(std::move(*begin));
      if constexpr(not trivial_destructor)
        ++num_in_merge_buffer;
    }
  }

  template <class It>
  inline bool can_acquire_merge_buffer([[maybe_unused]] It begin, [[maybe_unused]] It end) const noexcept
  {
    if constexpr(sizeof(ValueType) > (buffer_size * sizeof(IntType)))
      return false;
    else
      return (end - begin) <= std::ptrdiff_t(count_available_for_merge_buffer());
  }
  /**
   * @brief          Push a run onto the run stack.
   * @param run_end_pos  The index of the run being pushed onto the stack.
   */
  inline void push(IntType run_end_pos) noexcept(nothrow_destructor)
  {
    // TODO: assume(i > *(top + 1))
    if constexpr(not trivial_destructor)
    {
      if(need_to_trim_merge_buffer())
        destroy_enough_to_fit_one_more_run();
    }
    *top = run_end_pos;
    --top;
  }

  /**
   * @brief     Erase the index at the top of the stack.
   * @param runlen  The index of the run being pushed onto the stack.
   */
  inline void pop() noexcept
  {
    ++top;
  }


  template <std::size_t I>
  inline const IntType& get_offset() const noexcept
  {
    static_assert(I < 5);
    return top[I + 1];
  }

  template <std::size_t I>
  inline IntType& get_offset() noexcept
  {
    static_assert(I < 5);
    return top[I + 1];
  }

  inline const IntType& operator[](std::ptrdiff_t i) const noexcept
  {
    return top[i + 1];
  }

  /**
   * Return a bool indicating whether a merge needs to occur between
   * two of the three runs at the top of the stack.
   */
  inline bool merge_ABC_case_1() const noexcept
  {
    return get_offset<2>() - get_offset<3>() <= get_offset<0>() - get_offset<2>();
  }

  /**
   * Return a bool indicating whether a merge needs to occur between
   * two of the three runs at the top of the stack.
   */
  inline bool merge_ABC_case_2() const noexcept
  {
    return get_offset<3>() - get_offset<4>() <= get_offset<1>() - get_offset<3>();
  }

  /**
   * Return a bool indicating whether a merge needs to occur between
   * two of the three runs at the top of the stack.
   */
  inline bool merge_ABC() const noexcept
  {
    return merge_ABC_case_1() or merge_ABC_case_2();
  }

  /**
   * Given that a merge needs to occur between two of the three runs
   * at the top of the stack, returns true if the merge should occur
   * between the third-from-the-top and second-from-the-top runs
   * on the stack.  If false is returned, the two topmost runs should
   * be merged instead.
   */
  inline bool merge_AB() const noexcept
  {
    return get_offset<2>() - get_offset<3>() < get_offset<0>() - get_offset<1>();
  }

  /**
   * Return a bool indicating whether a merge needs to occur between
   * the two runs at the top of the stack.
   */
  inline bool merge_BC() const noexcept
  {
    return get_offset<1>() - get_offset<2>() <= get_offset<0>() - get_offset<1>();
  }

  /**
   * Modifies the run stack to reflect the fact that the run ending at
   * index 'I' was merged with the run beginning at index 'I'.
   */
  template <std::size_t I>
  inline void remove_run() noexcept
  {
    static_assert(I < 4);
    if constexpr(I == 0)
    {
      pop();
    }
    else
    {
      get_offset<I>() = get_offset<I - 1>();
      remove_run<I - 1>();
    }
  }

  /**
   * Obtain an iterator to the beginning of the merge buffer.
   */
  inline const ValueType* merge_buffer_begin() const noexcept
  {
    return reinterpret_cast<const ValueType*>(buffer);
  }

  /**
   * Obtain an iterator to the beginning of the merge buffer.
   */
  inline ValueType* merge_buffer_begin() noexcept
  {
    return reinterpret_cast<ValueType*>(buffer);
  }

  /**
   * Obtain an iterator to the end of the merge buffer.
   */
  inline const ValueType* merge_buffer_end() const noexcept
  {
    return reinterpret_cast<const ValueType*>(buffer) + num_in_merge_buffer;
  }

  /**
   * Obtain an iterator to the end of the merge buffer.
   */
  inline ValueType* merge_buffer_end() noexcept
  {
    return reinterpret_cast<ValueType*>(buffer) + num_in_merge_buffer;
  }

  static constexpr std::size_t extra_stack_alloc() noexcept
  {
    constexpr std::size_t minrun_bytes = max_minrun<ValueType>() * sizeof(ValueType);
    constexpr std::size_t max_bytes = 128 * sizeof(void*);
    // a merge only happens after at least two runs are on the
    // stack so subtract off 2 from the stack size to simulate
    constexpr std::size_t stack_max_bytes = (timsort_max_stack_size<IntType>() - 2) * sizeof(IntType);

    if constexpr(minrun_bytes < max_bytes)
    {
      std::size_t nbytes = minrun_bytes;
      // see how many '(2**k) * minrun's we can fit on the stack
      while(nbytes < max_bytes)
        nbytes += nbytes;
      return (nbytes / sizeof(IntType)) / 2;
    }
    else if constexpr(max_bytes + stack_max_bytes >= minrun_bytes and not (stack_max_bytes >= minrun_bytes))
      // if adding 'max_bytes' bytes to the run stack allows fitting
      // a run of length 'minrun' on the stack, then go ahead and allocate
      return max_bytes / sizeof(IntType);
    else
      // otherwise don't bother allocating extra, we'll probably be on the
      // heap the whole time anyway
      return 0;

  }


  /** Size of the buffer measured in 'IntType' objects. */
  static constexpr const std::size_t buffer_size = timsort_max_stack_size<IntType>();// + extra_stack_alloc();
  /** Alignment of the buffer. */
  static constexpr const std::size_t required_alignment = alignof(std::aligned_union_t<sizeof(IntType), IntType, ValueType>);
  alignas(required_alignment) IntType buffer[buffer_size];
  IntType* top;
  /**
   * Number of 'ValueType' objects in the merge buffer.
   * If 'ValueType' is trivially destructible, this is always zero.
   */
  std::conditional_t<trivial_destructor, const std::size_t, std::size_t> num_in_merge_buffer = 0;
};


} /* namespace internal */
} /* namespace tim */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


namespace tim {
namespace internal {


/**
 * Number of consecutive elements for which galloping would be a win over
 * either linear or binary search.
 */
inline constexpr const std::size_t gallop_win_dist = 7;

/**
 * Function object type implementing a generic operator<.
 * This is used instead of std::less<> to stay consistent with std::sort() and
 * std::stable_sort() when std::less<>::operator() is specialized.  This isn't
 * necessary in general, because specializing member functions of types defined
 * in namespace std is undefined behavior, but it doesn't hurt either.
 */
struct DefaultComparator
{
  template <class Left, class Right>
  inline bool operator()(Left&& left, Right&& right) const
  {
    return std::forward<Left>(left) < std::forward<Right>(right);
  }
};

/**
 * @brief    Semantically equivalent to std::upper_bound(), except requires
 *           random access iterators.
 * @param begin  Iterator to the first element in the range to search.
 * @param end    Past-the-end iterator to the range to search.
 * @param value  The value to search for.
 * @param comp   The comparison function to use.
 *
 * Similar to std::upper_bound, but first finds the k such that
 * comp(value, begin[2**(k - 1) - 1]) and not comp(begin[2**k - 1], value)
 * and then returns the equivalent of
 *    std::upper_bound(begin + 2**(k - 1), begin + 2**k - 1, value, comp)
 *
 */
template <class It, class T, class Comp>
inline It gallop_upper_bound(It begin, It end, const T& value, Comp comp)
{
  std::size_t i = 0;
  std::size_t len = end - begin;
  for(; i < len and not comp(value, begin[i]); i = 2 * i + 1) { /* LOOP */ }

  if(len > i)
    len = i;
  len -= (i / 2);
  begin += (i / 2);
  // hand-rolled std::upper_bound.
  for(; len > 0;)
  {
    i = len / 2;
    if (comp(value, begin[i]))
      len = i;
    else
    {
      begin += (i + 1);
      len -= (i + 1);
    }
  }
  return begin;
}


/**
 * @brief        Performs a leftwards rotation by one.
 * @param begin  Random access iterator to the first element in the range
 *         to be rotated.
 * @param end    Past-the-end random access iterator to the range to be
 *     rotated.
 *
 * Semantically equivalent to std::rotate(begin, begin + 1, end) except
 * random access iterators are required.
 */
template <class It>
inline void rotate_left(It begin, It end)
{
  // heuristic for deciding whether to use a temporary + range-move
  // if a type is smaller than a 'pointer-size-capacity' type like std::vector or std::string,
  // then implement the rotate as:
  //   move-to-temporary -> std::move_backward -> move-from-temporary
  // otherwise implement as a series of swaps.
  //
  // this heuristic is, of course, evaluated at compile-time
  //
  // benchmarking across a number of different cases shows that this usually wins over
  // a call to std::rotate()
  using value_type = iterator_value_type_t<It>;
  constexpr std::size_t use_temporary_upper_limit = 3 * sizeof(void*);
  constexpr bool use_temporary = sizeof(value_type) < use_temporary_upper_limit;
  if constexpr(use_temporary)
  {
    // for small types, implement using a temporary.
    if(end - begin > 1)
    {
      value_type temp = std::move(end[-1]);
      std::move_backward(begin, end - 1, end);
      *begin = std::move(temp);
    }
  }
  else
  {
    // for large types, implement as a series of adjacent swaps
    for(end -= (end > begin); end > begin; --end)
      std::swap(end[-1], *end);
  }
}


/**
 * @brief        Insertion sort the range [begin, end), where [begin, mid) is
 *      already sorted.
 * @param begin  Random access iterator to the first element in the range.
 * @param begin  Random access iterator to the first out-of-order element
 *      in the range.
 * @param end    Past-the-end random access iterator to the range.
 * @param comp   Comparator to use.
 *
 */
template <class It, class Comp>
void finish_insertion_sort(It begin, It mid, It end, Comp comp)
{
  using value_type = iterator_value_type_t<It>;
  if constexpr(std::is_scalar_v<value_type>
         and (   std::is_same_v<Comp, std::less<>>
              or std::is_same_v<Comp, std::less<value_type>>
              or std::is_same_v<Comp, std::greater<>>
              or std::is_same_v<Comp, std::greater<value_type>>
              or std::is_same_v<Comp, DefaultComparator>)
  )
  {
    // if types are cheap to compare and cheap to copy, do a linear search
    // instead of a binary search
    while(mid < end)
    {
      for(auto pos = mid; pos > begin and comp(*pos, pos[-1]); --pos)
        std::swap(pos[-1], *pos);
      ++mid;
    }
  }
  else
  {
    // do the canonical 'swap it with the one before it' insertion sort up
    // to max_minrun<value_type>() / 4 elements
    for(const auto stop = begin + std::min(max_minrun<value_type>() / 4, std::size_t(end - begin)); mid < stop; ++mid)
      for(auto pos = mid; pos > begin and comp(*pos, pos[-1]); --pos)
        std::swap(pos[-1], *pos);
    // finish it off with a binary insertion sort
    for(; mid < end; ++mid)
      rotate_left(std::upper_bound(begin, mid, *mid, comp), mid + 1);
  }

}

} /* namespace internal */
} /* namespace tim */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


namespace tim {

namespace internal {

template <class It,
    class Comp>
struct TimSort
{
  /**
   * @brief Perform a timsort on the range [begin_it, end_it).
   * @param begin_it   Random access iterator to the first element in the range.
   * @param end_it     Past-the-end random access iterator.
   * @param comp_func  Comparator to use.
   */
  using value_type = iterator_value_type_t<It>;
  TimSort(It begin_it, It end_it, Comp comp_func):
    stack_buffer{},
    heap_buffer{},
    start(begin_it),
    stop(end_it),
    position(begin_it),
    comp(comp_func),
    minrun(compute_minrun<value_type>(end_it - begin_it)),
    min_gallop(default_min_gallop)
  {
    // try_get_cached_heap_buffer(heap_buffer);
    fill_run_stack();
    collapse_run_stack();
    // try_cache_heap_buffer(heap_buffer);
  }


  /*
   * Continually push runs onto the run stack, merging adjacent runs
   * to resolve invariants on the size of the runs in the stack along
   * the way.
   */
  void fill_run_stack()
  {
    // push the first two runs on to the run stack, unless there's
    // only one run.
    push_next_run();
    if(not (position < stop))
      return;
    push_next_run();
    while(position < stop)
    {
      // resolve invariants before pushing the next run
      resolve_invariants();
      push_next_run();
    }
  }

  /*
   * Grand finale.  Keep merging the top 2 runs on the stack until there
   * is only one left.
   */
  inline void collapse_run_stack()
  {
    for(auto count = stack_buffer.run_count() - 1; count > 0; --count)
      merge_BC();
  }

  /*
   * Get the next run of already-sorted elements.  If the length of the
   * natural run is less than minrun, force it to size with an insertion sort.
   */

  void push_next_run()
  {
    // check if the next run is at least two elements long
    if(const std::size_t remain = stop - position;
       COMPILER_LIKELY_(remain > 1))
    {
      std::size_t idx = 2;
      // descending?
      if(comp(position[1], position[0]))
      {
        // see how long it is descending for and then reverse it
        while(idx < remain and comp(position[idx], position[idx - 1]))
          ++idx;
        std::reverse(position, position + idx);
      }
      // ascending
      // even if the run was initially descending, after reversing it the
      // following elements may form an ascending continuation of the
      // now-reversed run.
      // unconditionally attempt to continue the ascending run
      while(idx < remain and not comp(position[idx], position[idx - 1]))
        ++idx;
      // if needed, force the run to 'minrun' elements, or until all elements
      // in the range are exhausted (whichever comes first) with an insertion
      // sort.
      if(idx < remain and idx < minrun)
      {
        auto extend_to = std::min(minrun, remain);
        finish_insertion_sort(position, position + idx, position + extend_to, comp);
        idx = extend_to;
      }
      // advance 'position' by the length of the run we just found
      position += idx;
    }
    else
    {
      // only one element
      position = stop;
    }
    // push the run on to the run stack.
    stack_buffer.push(position - start);
  }

  /*
   * MERGE PATTERN STUFF
   */

  /*
   * Assume the run stack has the following form:
   *   [ ..., W, X, Y, Z]
         * Where Z is the length of the run at the top of the run stack.
   *
   * This function continually merges with Y with Z or X with Y until
   * the following invariants are satisfied:
   *   (1) X > Y + Z
   *     (1.1) W > X + Y
   *      (2) Y > Z
   *
   * If (1) or (1.1) are not satisfied, Y is merged with the smaller of
   * X and Z.  Otherwise if (2) is not satisfied, Y and Z are merged.
   *
   * This gives a reasonable upper bound on the size of the run stack.
   *
   *   NOTE:
   *   invariant (1.1) implements a fix for a bug in the original
   *   implementation described here:
   *   http://envisage-project.eu/wp-content/uploads/2015/02/sorting.pdf
   *
   *   The original description of these invariants written by Tim Peters
   *   accounts for only the top three runs and refers to them as: A, B,
   *   and C.  This implementation uses Tim's labelling scheme in some
   *   function names, but implements the corrected invariants as
   *   described above.
   *
   * ALSO NOTE:
   *
   * For more details see:
   * https://github.com/python/cpython/blob/master/Objects/listsort.txt
   */
  void resolve_invariants()
  {
    // Check all of the invariants in a loop while there are at least
    // two runs.
    auto run_count = stack_buffer.run_count();
    do{
      if(((run_count > 2) and stack_buffer.merge_ABC_case_1())
         or ((run_count > 3) and stack_buffer.merge_ABC_case_2()))
        if(stack_buffer.merge_AB())
          merge_AB();
        else
          merge_BC();
      else if(stack_buffer.merge_BC())
        merge_BC();
      else
        break;
      --run_count;
    } while(run_count > 1);
  }

  /* RUN STACK STUFF */

  /*
   * @brief Merge the second-to-last run with the last run.
   */
  void merge_BC()
  {
    merge_runs(start + get_offset<2>(),
         start + get_offset<1>(),
         start + get_offset<0>());
    stack_buffer.template remove_run<1>();
  }

  /*
   * @brief Merge the third-to-last run with the second-to-last run.
   */
  void merge_AB()
  {
    merge_runs(start + get_offset<3>(),
         start + get_offset<2>(),
         start + get_offset<1>());
    stack_buffer.template remove_run<2>();
  }

  /*
   * @brief Fetches the offset at the Ith position down from the top of
   * the run stack.
   *
   * Each offset on the run stack is the end position of each run.  So
   * if we wanted two iterators defining the run at the top of the stack
   * we would do something like this:
   *
   *   auto begin = this->start + this->get_offset<1>();
   *   auto end   = this->start + this->get_offset<0>();
   */
  template <std::size_t I>
  inline auto get_offset() const noexcept
  {
    return stack_buffer.template get_offset<I>();
  }

  /*
   * MERGE/SORT IMPL STUFF
   */

  /*
   * @brief Merges the range [begin, mid) with the range [mid, end).
   * @param begin  Iterator to the first item in the left range.
   * @param mid    Iterator to the last/first item in the left/right range.
   * @param end    Iterator to the last item in the right range.
   *
   * Requires:
   *     begin < mid and mid < end.
   *     std::is_sorted(begin, mid, this->comp)
   *     std::is_sorted(mid, end, this->comp)
   */
  void merge_runs(It begin, It mid, It end)
  {
    // We're going to need to copy the smaller of these ranges
    // into a temporary buffer (which may end up having to be on
    // the heap).  Before we do any copying, try to reduce the
    // effective size of each range by looking for position 'p1'
    // in [begin, mid) that mid[0] belongs in. Similarly look
    // for mid[-1] in [mid, end) and call that 'p2'.
    // We then only need to merge [p1, mid) with [mid, p2).
    //
    // This also may reduce the number of items we need to copy
    // into the temporary buffer, and if we're lucky, it may even
    // make it possible to use just the space we have on the stack.

    begin = gallop_upper_bound(begin, mid, *mid, comp);
    end = gallop_upper_bound(std::make_reverse_iterator(end),
           std::make_reverse_iterator(mid),
           mid[-1],
           [comp=this->comp](auto&& a, auto&& b){
              return comp(std::forward<decltype(b)>(b), std::forward<decltype(a)>(a));
           }).base();

    if(COMPILER_LIKELY_(begin < mid or mid < end))
    {
      if((end - mid) > (mid - begin))
        // merge from the left
        do_merge(begin, mid, end, comp);
      else
        // merge from the right
        do_merge(std::make_reverse_iterator(end),
           std::make_reverse_iterator(mid),
           std::make_reverse_iterator(begin),
           [comp=this->comp](auto&& a, auto&& b){
            // reverse the comparator
            return comp(std::forward<decltype(b)>(b),
                  std::forward<decltype(a)>(a));
           }
        );
    }
  }

  /*
   * @brief Merges the range [begin, mid) with the range [mid, end).
   * @param begin  Iterator to the first item in the left range.
   * @param mid    Iterator to the last/first item in the left/right range.
   * @param end    Iterator to the last item in the right range.
   * @param cmp    Comparator to merge with respect to.
   *
   * Requires:
   *     begin < mid and mid < end.
   *     std::is_sorted(begin, mid, cmp)
   *     std::is_sorted(mid, end, cmp)
   */
  template <class Iter, class Cmp>
  void do_merge(Iter begin, Iter mid, Iter end, Cmp cmp)
  {
    // check to see if we can use the run stack as a temporary buffer
    if(stack_buffer.can_acquire_merge_buffer(begin, mid))
    {
      // allocate the merge buffer on the stack
      auto stack_mem = stack_buffer.move_to_merge_buffer(begin, mid);
      gallop_merge(stack_mem, stack_mem + (mid - begin),
                 mid, end,
                 begin, cmp);
    }
    else
    {
      // TODO: clean this up
      // fall back to a std::vector<> for the merge buffer
      // try to use memcpy if possible
      //
      // bypass speculative overallocation of std::vector.
      // this measurably improves benchmarks for all cases.
      heap_buffer.reserve(mid - begin);
      if constexpr(can_forward_memcpy_v<Iter> or not can_reverse_memcpy_v<Iter>)
      {
        // memcpy() it if we can
        if constexpr (can_forward_memcpy_v<Iter>)
        {
          heap_buffer.resize(mid - begin);
          std::memcpy(heap_buffer.data(), get_memcpy_iterator(begin), (mid - begin) * sizeof(value_type));
        }
        else
        {
          heap_buffer.assign(std::make_move_iterator(begin), std::make_move_iterator(mid));
        }
        gallop_merge(heap_buffer.begin(), heap_buffer.end(),
                   mid, end,
                   begin, cmp);
      }
      else
      {
        heap_buffer.resize(mid - begin);
        std::memcpy(heap_buffer.data(), get_memcpy_iterator(mid - 1), (mid - begin) * sizeof(value_type));
        gallop_merge(heap_buffer.rbegin(), heap_buffer.rend(),
                   mid, end,
                   begin, cmp);
      }
      // clear the vector so that the next call to reserve()
      // doesn't attempt to copy anything if it reallocates
      // it also helps to call the destructors while the
      // heap buffer is still hot (possibly cached)
      heap_buffer.clear();
    }
  }

  /**
   * @brief Implementation of the merge routine.
   * @param lbegin  Iterator to the begining of the left range.
   * @param lend    Iterator to the end of the left range.
   * @param rbegin  Iterator to the begining of the right range.
   * @param rend    Iterator to the end of the right range.
   * @param cmp     Comparator.  Either this->comp or a lambda reversing
   *        the direction of this->comp.
   *
   * This merge rountine has been fine-tuned to take advantage of the
   * the preconditions imposed by the other components of this timsort
   * implementation.  This is not a general-purpose merge routine and
   * it should not be used outside of this implementation.
   *
   * Most of the logic of this merge routine is implemented inline, and
   * questionable constructs (such as gotos) are used where benchmarking
   * has shown that they speed up the routine. The code isn't pretty,
   * but it's damn fast.
   *
   * requires:
   *   [lbegin, lend) does not overlap with [rbegin, rend)
   *  lend - lbegin > 0 and rend - rbegin > 0
   *      cmp(*rbegin, *lbegin)
   *      cmp(rend[-1], lend[-1])
   */
  template <class LeftIt, class RightIt, class DestIt, class Cmp>
  void gallop_merge(LeftIt lbegin, LeftIt lend, RightIt rbegin, RightIt rend, DestIt dest, Cmp cmp)
  {
    // God bless you if you're reading this.  I'll try to explain
    // what I'm doing here to the best of my ability.  Much like the
    // merge routine in CPython's list_sort(), there are two modes
    // of operation, linear and galloping.  Linear mode is simply
    // the classic merge routine with the caveat that after one of
    // the two ranges wins 'this->min_gallop' times in a row, we
    // jump into the gallop loop.
    //
    // To understand the gallop loop, see the required reading here:
    // https://github.com/python/cpython/blob/master/Objects/listsort.txt
    //
    // Here are few refinements we make over CPython's list_sort().
    // Each of these were benchmarked on a variety of test cases
    // to ensure that they actually sped up the merge routine.
    //   - When switching to galloping mode from linear mode,
    //     jump straight to the left range if we were already
    //     merging the left range in linear mode, and jump
    //     do the opposite if we were merging the right range.
    //      - If we're already in galloping mode and we're changing
    //        to the other range (e.g. just finished the left range
    //     and are now going to gallop in the right range) set
    //     start galloping at index 1 instead of 0.  This saves
    //     a redundant comparison.
    //     This is achieved by setting num_galloped to 1 just
    //    before the entry points for the gotos in the linear
    //     mode loop.  Every time we enter the linear mode loop,
    //      num_galloped is set to zero again.
    //   - Since, before each call to gallop_merge() we first
    //     make sure to cut off all elements at the end of the
    //    right range that are already in the correct place, we
    //     know that we will exhaust the right range before the
    //     left.  This means we can skip any "lbegin < lend"
    //     checks and instead just unconditionally copy the
    //     remainder of the left range into the destination
    //     range once we've used up the right range.
    //   - Another small optimization that speeds up the merge
    //     if we're sorting trivially copyable types, is using
    //     memcpy() when copying from the left range.  Since
    //     the left range is always the one that got copied into
    //     the merge buffer, we know that the left range doesn't
    //    overlap with the destination range.  Some additional
    //     compile-time checks are done to ensure that memcpy()
    //     can be done without invoking UB. (e.g. checking that
    //     'dest' is a contiguous iterator or that both 'DestIt'
    //     and 'LeftIt' are reverse contiguous iterators and
    //     ensuring that values being sorted are trivially
    //     copyable.
    for(std::size_t num_galloped=0, lcount=0, rcount=0 ; ;)
    {
      // LINEAR SEARCH MODE
      // do a naive merge until evidence shows that galloping may be faster.

      // set lcount to 1 if num_galloped > 0. if we got here from exiting the
      // gallop loop, then we already copied one extra element from the
      // left range and we should count that towards lcount here.
      for(lcount=(num_galloped > 0), rcount=0, num_galloped=0;;)
      {
        if(cmp(*rbegin, *lbegin))
        {
          // move from the right-hand-side
          *dest = std::move(*rbegin);
          ++dest;
          ++rbegin;
          ++rcount;
          // merge is done.  copy the remaining elements from the left range and return
          if(not (rbegin < rend))
          {
            move_or_memcpy(lbegin, lend, dest);
            return;
          }
          else if(rcount >= min_gallop)
            goto gallop_right; // continue this run in galloping mode
          lcount = 0;
        }
        else
        {
          // move from the left-hand side
          *dest = std::move(*lbegin);
          ++dest;
          ++lbegin;
          ++lcount;
          // don't need to check if we reached the end.  that will happen on the right-hand-side
          if(lcount >= min_gallop)
            goto gallop_left; // continue this run in galloping mode
          rcount = 0;
        }
      }
      COMPILER_UNREACHABLE_;
      // GALLOP SEARCH MODE
      while(lcount >= gallop_win_dist or rcount >= gallop_win_dist) {
        // decrement min_gallop every time we continue the gallop loop
        if(min_gallop > 1)
          --min_gallop;

        // we already know the result of the first comparison, so set num_galloped to 1 and skip it
        num_galloped = 1;
          gallop_left: // when jumping here from the linear merge loop, num_galloped is set to zero
        lcount = lend - lbegin;
        // gallop through the left range
        while((num_galloped < lcount) and not cmp(*rbegin, lbegin[num_galloped]))
          num_galloped = 2 * num_galloped + 1;
        if(lcount > num_galloped)
          lcount = num_galloped;
        // do a binary search in the narrowed-down region
        lcount = std::upper_bound(lbegin + (num_galloped / 2), lbegin + lcount, *rbegin, cmp) - lbegin;
        dest = move_or_memcpy(lbegin, lbegin + lcount, dest);
        lbegin += lcount;

        // don't need to check if we reached the end.  that will happen on the right-hand-side
        // we already know the result of the first comparison, so set num_galloped to 1 and skip it
        num_galloped = 1;
          gallop_right: // when jumping here from the linear merge loop, num_galloped is set to zero
        rcount = rend - rbegin;
        // gallop through the right range
        while((num_galloped < rcount) and cmp(rbegin[num_galloped], *lbegin))
          num_galloped = 2 * num_galloped + 1;
        if(rcount > num_galloped)
          rcount = num_galloped;
        // do a binary search in the narrowed-down region
        rcount = std::lower_bound(rbegin + (num_galloped / 2), rbegin + rcount, *lbegin, cmp) - rbegin;
        dest = std::move(rbegin, rbegin + rcount, dest);
        rbegin += rcount;

        // merge is done.  copy the remaining elements from the left range and return
        if(not (rbegin < rend))
        {
          move_or_memcpy(lbegin, lend, dest);
          return;
        }
      }
      // exiting the loop means we just finished galloping
      // through the right-hand side.  We know for a fact
      // that 'not cmp(*rbegin, *lbegin)', so do one copy for free.
      ++min_gallop;
      *dest = std::move(*lbegin);
      ++dest;
      ++lbegin;
    }
    COMPILER_UNREACHABLE_;
  }

  /**
   * Stack-allocated stack data structure that holds location (end position)
   * of each run.  Bottom of the stack always holds 0.
   * Empty stack space is used for merge buffer when possible.
   */
  timsort_stack_buffer<std::size_t, value_type> stack_buffer;
  /** Fallback heap-allocated array used for merge buffer. */
  std::vector<value_type> heap_buffer;
  /** 'begin' iterator to the range being sorted. */
  const It start;
  /** 'end' iterator to the range being sorted. */
  const It stop;
  /**
   * Iterator to keep track of how far we've scanned into the range to be
   * sorted.  [start, position) contains already-found runs while
   * [position, stop) is still untouched.  When position == end, the run stack
   * is collapsed.
   */
  It position;
  /** Comparator used to sort the range. */
  Comp comp;
  /** Minimum length of a run */
  const std::size_t minrun;
  /**
   * Minimum number of consecutive elements for which one side of the
   * merge must "win" in a row before switching from galloping mode to
   * linear mode.
   */
  std::size_t min_gallop = default_min_gallop;

  static constexpr const std::size_t default_min_gallop = gallop_win_dist;
};


template <class It, class Comp>
static void _timsort(It begin, It end, Comp comp)
{
  using value_type = iterator_value_type_t<It>;
  std::size_t len = end - begin;
  if(len > max_minrun<value_type>())
    TimSort<It, Comp>(begin, end, comp);
  else
    finish_insertion_sort(begin, begin + (end > begin), end, comp);
}

} /* namespace internal */


template <class It, class Comp>
void timsort(It begin, It end, Comp comp)
{
  internal::_timsort(begin, end, comp);
}


template <class It>
void timsort(It begin, It end)
{
  timsort(begin, end, tim::internal::DefaultComparator{});
}

} /* namespace tim */


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#ifdef   COMPILER_LIKELY_
# undef COMPILER_LIKELY_
#endif

#ifdef   COMPILER_UNLIKELY_
# undef COMPILER_UNLIKELY_
#endif

#ifdef   COMPILER_ASSUME_
# undef COMPILER_ASSUME_
#endif

#ifdef   COMPILER_UNREACHABLE_
# undef COMPILER_UNREACHABLE_
#endif
