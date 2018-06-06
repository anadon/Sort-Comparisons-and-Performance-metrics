/*******************************************************************************
Copyright Josh Marshall, 2018

The following segments were copied from core files of the GNU ISO C++ Library.

The change to the Affero General Public License version 3 is explicitly allowed.
*******************************************************************************/

/*******************************************************************************
This file is part of "Sort Performance Comparison".

"Sort Performance Comparison" is free software: you can redistribute it and/or
modify it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

"Sort Performance Comparison" is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public License
version 3 for more details.

You should have received a copy of the GNU Affero General Public License along
with "Sort Performance Comparison".  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

// Copyright (C) 2001-2017 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */



#pragma once


//#include <cstdlib>       // for rand
#include <bits/algorithmfwd.h>
#include <bits/stl_heap.h>
#include <bits/stl_tempbuf.h>  // for _Temporary_buffer
#include <bits/predefined_ops.h>

//#if __cplusplus >= 201103L
//#include <bits/uniform_int_dist.h>
//#endif


namespace SCP{
/**
 *  @doctodo
 *  This controls some aspect of the sort routines.
*/
enum { _S_threshold = 16 };

/// This is a helper function for the sort routine.
template<
  typename _RandomAccessIterator,
  typename _Compare>
void
final_insertion_sort(
  _RandomAccessIterator __first,
  _RandomAccessIterator __last,
  _Compare __comp
){
  if (__last - __first > int(_S_threshold)){
    std::__insertion_sort(__first, __first + int(_S_threshold), __comp);
    std::__unguarded_insertion_sort(__first + int(_S_threshold), __last, __comp);
  }else{
  std::__insertion_sort(__first, __last, __comp);
  }
}

/// This is a helper function...
template<
  typename _RandomAccessIterator,
  typename _Compare>
_RandomAccessIterator
unguarded_partition(
  _RandomAccessIterator __first,
  _RandomAccessIterator __last,
  _RandomAccessIterator __pivot, _Compare __comp
){
  while (true){
    while (__comp(__first, __pivot))
      ++__first;
    --__last;
    while (__comp(__pivot, __last))
      --__last;
    if (!(__first < __last))
      return __first;
    std::iter_swap(__first, __last);
    ++__first;
  }
}


/// This is a helper function...
template<
  typename _RandomAccessIterator,
  typename _Compare>
inline
_RandomAccessIterator
unguarded_partition_pivot(
  _RandomAccessIterator __first,
  _RandomAccessIterator __last,
  _Compare __comp
){
  _RandomAccessIterator __mid = __first + (__last - __first) / 2;
  std::__move_median_to_first(__first, __first + 1, __mid, __last - 1, __comp);
  return std::__unguarded_partition(__first + 1, __last, __first, __comp);
}


/// This is a helper function for the sort routines.
template<
  typename _RandomAccessIterator,
  typename _Compare>
void
heap_select(
  _RandomAccessIterator __first,
  _RandomAccessIterator __middle,
  _RandomAccessIterator __last,
  _Compare __comp
){
  std::__make_heap(__first, __middle, __comp);
  for (_RandomAccessIterator __i = __middle; __i < __last; ++__i)
    if (__comp(__i, __first))
      std::__pop_heap(__first, __middle, __i, __comp);
}


template<
  typename _RandomAccessIterator,
  typename _Compare>
inline
void
SCP_partial_sort(
  _RandomAccessIterator __first,
  _RandomAccessIterator __middle,
  _RandomAccessIterator __last,
  _Compare __comp
){
  std::__heap_select(__first, __middle, __last, __comp);
  std::__sort_heap(__first, __middle, __comp);
}


/// This is a helper function for the sort routine.
template<
  typename _RandomAccessIterator,
  typename _Size,
  typename _Compare>
void
introsort_loop(
  _RandomAccessIterator __first,
  _RandomAccessIterator __last,
  _Size __depth_limit,
  _Compare __comp
){
  while (__last - __first > int(_S_threshold)){
    if (__depth_limit == 0){
      SCP_partial_sort(__first, __last, __last, __comp);
      return;
    }
    --__depth_limit;
    _RandomAccessIterator __cut = unguarded_partition_pivot(__first, __last, __comp);
    introsort_loop(__cut, __last, __depth_limit, __comp);
    __last = __cut;
  }
}


template<
  typename _RandomAccessIterator,
  typename _Compare>
inline
void
sort_impl(
  _RandomAccessIterator __first,
  _RandomAccessIterator __last,
  _Compare __comp
){
  if (__first != __last){
    introsort_loop(__first, __last, std::__lg(__last - __first) * 2, __comp);
    final_insertion_sort(__first, __last, __comp);
  }
}


/**
*  @brief Sort the elements of a sequence.
*  @ingroup sorting_algorithms
*  @param  __first   An iterator.
*  @param  __last    Another iterator.
*  @return  Nothing.
*
*  Sorts the elements in the range @p [__first,__last) in ascending order,
*  such that for each iterator @e i in the range @p [__first,__last-1),
*  *(i+1)<*i is false.
*
*  The relative ordering of equivalent elements is not preserved, use
*  @p stable_sort() if this is needed.
*/
template<
  typename _RandomAccessIterator>
inline
void
introsort(
  _RandomAccessIterator __first,
  _RandomAccessIterator __last
){
  sort_impl(__first, __last, __gnu_cxx::__ops::__iter_less_iter());
}


/**
*  @brief Sort the elements of a sequence using a predicate for comparison.
*  @ingroup sorting_algorithms
*  @param  __first   An iterator.
*  @param  __last    Another iterator.
*  @param  __comp    A comparison functor.
*  @return  Nothing.
*
*  Sorts the elements in the range @p [__first,__last) in ascending order,
*  such that @p __comp(*(i+1),*i) is false for every iterator @e i in the
*  range @p [__first,__last-1).
*
*  The relative ordering of equivalent elements is not preserved, use
*  @p stable_sort() if this is needed.
*/
template<
  typename _RandomAccessIterator,
  typename _Compare>
inline
void
introsort(
  _RandomAccessIterator __first,
  _RandomAccessIterator __last,
  _Compare __comp
){
  sort_impl(__first, __last, __gnu_cxx::__ops::__iter_comp_iter(__comp));
}

};
