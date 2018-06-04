/*******************************************************************************
Copyright Josh Marshall, 2018
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

#pragma once

#include <algorithm>
#include "timsort.hpp"

#include "introsort.hpp"


/*******************************************************************************
This is to evaluate the constant program overhead, which can then be subtracted
from the results of running actual sort to obtain a more accurate performance
comparison.
*******************************************************************************/
template<
  typename RandomAccessIterator>
void
null_sort(
  RandomAccessIterator begin,
  RandomAccessIterator end
){
  return;
}


/*******************************************************************************
Here we obtain a function pointer to a templated sort function in a dynamic way
at runtime.  It isn't possible to make it elegant, so here some of the ugly bits
are handled as elegantly as possible.
*******************************************************************************/
template<
  typename RandomAccessItertor>
void (*get_sort_func_ptr(
  const struct config args,
  RandomAccessItertor
))(
  RandomAccessItertor,
  RandomAccessItertor
){
    switch(args.chosen_sort){
      case std_sort:        return std::sort;
      case std_stable_sort: return std::stable_sort;
      case introsort:       return introsort;
      case timsort:         return madlib::timsort;
      case null:            return null_sort;
      default: exit(-3);
    };
}
