/*******************************************************************************
Copyright (c) 1997 Rensselaer Polytechnic Institute
Copyright (c) 2018 Josh Marshall

Relicensed with permission under the Affero General Public Licence version 3.
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

/*******************************************************************************
@brief This file describes "counter", "distance_counter", and
"interation_counter" classes which serve as suitable wrappers to track various
usage metrics of iterator operations.  It is based off of code originally
developed by David Musser et al. and later updated by Josh Marshall.

It is used by wrapping iterator inputs to a function in the following way:

template <
  typename Container>
typedef distance_counter<
  typename Container::iterator,
  typename Container::difference_type>
distance;

template <
  typename Container>
typedef iteration_counter<
  typename Container::iterator,
  typename Container::value_type,
  typename Container::value_type&,
  distance>
iterator;

...

func(iterator(x.begin()), iterator(x.end()));

*******************************************************************************/

#pragma once

#include <cassert>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <utility>

using std::cout;
using std::endl;
using std::ios;
using std::iterator_traits;
using std::setiosflags;
using std::setw;


static ssize_t iteration_counter_constructions = 0;
static ssize_t iteration_counter_assignments = 0;
static ssize_t iteration_counter_increments = 0;
static ssize_t iteration_counter_dereferences = 0;
static ssize_t iteration_counter_bigjumps = 0;
static ssize_t iteration_counter_comparisons = 0;
static ssize_t iteration_counter_max_generation = 0;

static ssize_t counter_assignments = 0;
static ssize_t counter_comparisons = 0;
static ssize_t counter_accesses = 0;

static ssize_t distance_counter_constructions = 0;
static ssize_t distance_counter_copy_constructions = 0;
static ssize_t distance_counter_conversions = 0;
static ssize_t distance_counter_assignments = 0;
static ssize_t distance_counter_increments = 0;
static ssize_t distance_counter_additions = 0;
static ssize_t distance_counter_subtractions = 0;
static ssize_t distance_counter_multiplications = 0;
static ssize_t distance_counter_divisions = 0;
static ssize_t distance_counter_comparisons = 0;
static ssize_t distance_counter_max_generation = 0;



/*******************************************************************************
Defines class counter<T>, for use in measuring the performance of certain STL
generic algorithms.  Objects of this class behave like those of type T with
respect to assignments and comparison operations, but the class also keeps
counts of those operations, using values of type ssize_t.
*******************************************************************************/

template <class T>
class counter {
protected:
  T value;

public:

  T
  base(
  ) const {
    ++counter_accesses;
    return value;
  }


  counter(
  ) : value(T()) {
    ++counter_assignments;
  }


  explicit
  counter(
    const T& v
  ) : value(v) {
    ++counter_assignments;
  }


  counter(
    const counter<T>& x
  ) : value(x.value) {
    ++counter_assignments;
  }


  friend
  bool
  operator<(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter_comparisons;
    return x.value < y.value;
  }


  friend
  bool
  operator<=(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter_comparisons;
    return x.value <= y.value;
  }


  friend
  bool
  operator>(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter_comparisons;
    return x.value > y.value;
  }


  friend
  bool
  operator>=(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter_comparisons;
    return x.value >= y.value;
  }


  friend
  bool
  operator==(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter_comparisons;
    return x.value == y.value;
  }


  friend
  bool
  operator!=(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter_comparisons;
    return x.value != y.value;
  }


friend
bool
operator<(
  const counter<T>& x,
  const T& y
){
  ++counter_comparisons;
  return x.value < y;
}


friend
bool
operator<=(
  const counter<T>& x,
  const T& y
){
  ++counter_comparisons;
  return x.value <= y;
}


friend
bool
operator>(
  const counter<T>& x,
  const T& y
){
  ++counter_comparisons;
  return x.value > y;
}


friend
bool
operator>=(
  const counter<T>& x,
  const T& y
){
  ++counter_comparisons;
  return x.value >= y;
}


friend
bool
operator==(
  const counter<T>& x,
  const T& y
){
  ++counter_comparisons;
  return x.value == y;
}


friend
bool
operator!=(
  const counter<T>& x,
  const T& y
){
  ++counter_comparisons;
  return x.value != y;
}


friend
bool
operator<(
  const T& x,
  const counter<T>& y
){
  ++counter_comparisons;
  return x < y.value;
}


friend
bool
operator<=(
  const T& x,
  const counter<T>& y
){
  ++counter_comparisons;
  return x <= y.value;
}


friend
bool
operator>(
  const T& x,
  const counter<T>& y
){
  ++counter_comparisons;
  return x > y.value;
}


friend
bool
operator>=(
  const T& x,
  const counter<T>& y
){
  ++counter_comparisons;
  return x >= y.value;
}


friend
bool
operator==(
  const T& x,
  const counter<T>& y
){
  ++counter_comparisons;
  return x == y.value;
}


friend
bool
operator!=(
  const T& x,
  const counter<T>& y
){
  ++counter_comparisons;
  return x != y.value;
}


  counter<T>&
  operator=(
    const counter<T>& x
  ){
    ++counter_assignments;
    value = x.value;
    return *this;
  }

  T
  operator* (
  ) const {
    return base();
  }
};




/*******************************************************************************
Defines class distance_counter<RandomAccessIterator, Distance>, for use in
measuring the performance of certain STL generic algorithms.  Objects of this
class behave like those of type Distance (which should be a type capable of
representing the difference between iterators of type RandomAccessIterator) but
the class also keeps counts of all Distance operations, using values of type
ssize_t.  Type RandomAccessIterator is used as the result type of certain
addition and subtraction operators, with the assumption that Distance is its
distance type.
*******************************************************************************/

template <class RandomAccessIterator, class Distance>
class distance_counter {
public:

  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  bool
  operator==(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  bool
  operator==(
    const Distance_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  bool
  operator==(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  bool
  operator<(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  bool
  operator<(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  bool
  operator<(
    const Distance_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  RandomAccessIterator_
  operator+(
    const RandomAccessIterator_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>
  operator+(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>
  operator+(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  Distance_
  operator+(
    const Distance_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  RandomAccessIterator_&
  operator+=(
    RandomAccessIterator_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator+=(
    distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator+=(
    distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>
  operator-(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>
  operator-(
    const distance_counter<RandomAccessIterator_, Distance_>& s,
    const Distance_& n
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator-(
    const RandomAccessIterator_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  RandomAccessIterator_&
  operator-=(
    RandomAccessIterator_& i,
    const distance_counter<RandomAccessIterator_, Distance_>& x
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator-=(
    distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator-=(
    distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>
  operator*(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>
  operator*(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  Distance_
  operator*(
    const Distance_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator*=(
    distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator*=(
    distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  Distance_&
  operator*=(
    Distance_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>
  operator/(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>
  operator/(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_ &y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  Distance_
  operator/(
    const Distance_ &x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator/=(
    distance_counter<RandomAccessIterator_, Distance_>& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  Distance_&
  operator/=(
    Distance_& x,
    const distance_counter<RandomAccessIterator_, Distance_>& y
  );


  template<
    typename RandomAccessIterator_,
    typename Distance_>
  friend
  distance_counter<RandomAccessIterator_, Distance_>&
  operator/=(
    const distance_counter<RandomAccessIterator_, Distance_>& x,
    const Distance_& y
  );


public:
  Distance current;
  ssize_t generation;


public:


  distance_counter<RandomAccessIterator, Distance>&
  operator++(
  ){
    ++distance_counter_increments;
    ++current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator++(
    int
  ){
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    ++distance_counter_increments;
    ++current;
    return tmp;
  }

//*
  operator
  int(
  ) const {
    ++distance_counter_conversions;
    return current;
  }
//*/

  distance_counter<RandomAccessIterator, Distance>&
  operator--(){
    ++distance_counter_increments;
    --current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator--(
    int
  ){
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    ++distance_counter_increments;
    --current;
    return tmp;
  }

  distance_counter(
  ) : generation(0) {
    ++distance_counter_constructions;
  }


  explicit
  distance_counter(
    const Distance& x
  ){
    current = x;
    generation = 0;
    ++distance_counter_conversions;
  }


  distance_counter(
    const distance_counter<RandomAccessIterator, Distance>& c
  ){
    current = c.current;
    generation = c.generation + 1;
    ++distance_counter_copy_constructions;
    if (generation > distance_counter_max_generation) {
      distance_counter_max_generation = generation;
    }
  }

  distance_counter<RandomAccessIterator, Distance>&
  operator=(
    const distance_counter<RandomAccessIterator, Distance>& x
  ){
    current = x.current;
    generation = x.generation + 1;
    ++distance_counter_assignments;
    if (generation > distance_counter_max_generation) {
      distance_counter_max_generation = generation;
    }
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator=(
    const Distance& x
  ){
    current = x;
    ++distance_counter_assignments;
    return *this;
  }


  Distance
  base(
  ) const {
    return current;
  }

};




template <
  typename RandomAccessIterator_,
  typename Distance_>
bool
operator==(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  ++distance_counter_comparisons;
  return x.current == y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
bool
operator==(
  const Distance_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  ++distance_counter_comparisons;
  return x == y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
bool
operator==(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_& y
){
  ++distance_counter_comparisons;
  return x.current == y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
bool
operator<(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  ++distance_counter_comparisons;
  return x.current < y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
bool
operator<(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_& y
){
  ++distance_counter_comparisons;
    return x.current < y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
bool
operator<(
  const Distance_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  ++distance_counter_comparisons;
  return x < y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
RandomAccessIterator_
operator+(
  const RandomAccessIterator_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  RandomAccessIterator_ tmp(x);
  return tmp += y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>
operator+(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  distance_counter<RandomAccessIterator_, Distance_> tmp(x);
  return tmp += y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>
operator+(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_& y
){
  distance_counter<RandomAccessIterator_, Distance_> tmp(x);
  return tmp += y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
Distance_
operator+(
  const Distance_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  Distance_ tmp(x);
  return tmp += y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
RandomAccessIterator_&
operator+=(
  RandomAccessIterator_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  return x += y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator+=(
  distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  return x += y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator+=(
  distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_& y
){
  ++distance_counter_additions;
  x.current += y;
  return x;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>
operator-(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  distance_counter<RandomAccessIterator_, Distance_> tmp(x);
  return tmp -= y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>
operator-(
  const distance_counter<RandomAccessIterator_, Distance_>& s,
  const Distance_& n
){
  distance_counter<RandomAccessIterator_, Distance_> tmp(s);
  return tmp -= n;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator-(
  const RandomAccessIterator_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  RandomAccessIterator_ tmp(x);
  return tmp -= y;
}


//NOTE: Counts as iterator, so pass it along
template <
  typename RandomAccessIterator_,
  typename Distance_>
RandomAccessIterator_&
operator-=(
  RandomAccessIterator_& i,
  const distance_counter<RandomAccessIterator_, Distance_>& x
){
  return i -= x.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator-=(
  distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  return x -= y.current;
}


//NOTE: Key function
template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator-=(
  distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_& y
){
  ++distance_counter_subtractions;
  x.current -= y;
  return x;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>
operator*(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  distance_counter<RandomAccessIterator_, Distance_> tmp(x);
  return x *= y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>
operator*(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_& y
){
  distance_counter<RandomAccessIterator_, Distance_> tmp(x);
  return tmp *= y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
Distance_
operator*(
  const Distance_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  Distance_ tmp(x);
  return tmp *= y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator*=(
  distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  return x *= y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator*=(
  distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_& y
){
  ++distance_counter_multiplications;
  x.current *= y;
  return x;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
Distance_&
operator*=(
  Distance_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  ++distance_counter_multiplications;
  return x *= y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>
operator/(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  distance_counter<RandomAccessIterator_, Distance_> tmp(x);
  return tmp /= y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>
operator/(
  const distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_ &y
){
  distance_counter<RandomAccessIterator_, Distance_> tmp(x);
  return tmp /= y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
Distance_
operator/(
  const Distance_ &x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  Distance_ tmp(x);
  return tmp /= y;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator/=(
  distance_counter<RandomAccessIterator_, Distance_>& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  return x /= y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
Distance_&
operator/=(
  Distance_& x,
  const distance_counter<RandomAccessIterator_, Distance_>& y
){
  ++distance_counter_divisions;
  return x /= y.current;
}


template <
  typename RandomAccessIterator_,
  typename Distance_>
distance_counter<RandomAccessIterator_, Distance_>&
operator/=(
  distance_counter<RandomAccessIterator_, Distance_>& x,
  const Distance_& y
){
  ++distance_counter_divisions;
  x.current /= y;
  return x;
}





/*******************************************************************************
Defines class iteration_counter<RandomAccessIterator, T, Reference, Distance>,
for use in measuring the performance of certain STL generic algorithms.  Objects
of this class behave like those of type RandomAccessIterator, but the class also
keeps counts of all iterator operations, using values of type Counting.  Type T
should be the value type of RandomAccessIterator, and Reference should be the
reference type of T.  Type Distance should be a distance type for
RandomAccessIterator, and Counting is the type used for the counts.
*******************************************************************************/

template <
  typename RandomAccessIterator,
  typename T,
  typename Reference,
  typename Distance>
class iteration_counter : public iterator_traits<RandomAccessIterator> {

  typedef iteration_counter<RandomAccessIterator, T, Reference, Distance> self;


  template <
    typename RandomAccessIterator_,
    typename T_,
    typename Reference_,
    typename Distance_>
  friend
  bool
  operator==(
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y);


  template <
    typename RandomAccessIterator_,
    typename T_,
    typename Reference_,
    typename Distance_>
  friend
  bool
  operator!=(
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y);


  template <
    typename RandomAccessIterator_,
    typename T_,
    typename Reference_,
    typename Distance_>
  friend
  bool
  operator<(
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y);


  template <
    typename RandomAccessIterator_,
    typename T_,
    typename Reference_,
    typename Distance_>
  friend
  self
  operator+(
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y);


  template <
    typename RandomAccessIterator_,
    typename T_,
    typename Reference_,
    typename Distance_>
  friend
  Distance_
  operator-(
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
    const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y);


protected:
  RandomAccessIterator current;
  ssize_t generation;
public:

  iteration_counter(
  ) : generation(0) {
    ++iteration_counter_constructions;
  }


  iteration_counter(
    RandomAccessIterator x
  ){
    current = x;
    generation = 0;
    ++iteration_counter_constructions;
  }


  iteration_counter(
    const self& c
  ){
    current = c.current;
    generation = c.generation + 1;
    ++iteration_counter_constructions;
    if (generation > iteration_counter_max_generation) {
      iteration_counter_max_generation = generation;
    }
  }


  RandomAccessIterator
  base(
  ){
    return current;
  }


  Reference
  operator*(
  ) const {
    ++iteration_counter_dereferences;
    return *current;
  }


  self&
  operator++(
  ){
    ++iteration_counter_increments;
    ++current;
    return *this;
  }


  self&
  operator++(
    int
  ){
    self copy(*this);
    ++iteration_counter_increments;
    ++current;
    return copy;
  }


  self&
  operator--(
  ){
    ++iteration_counter_increments;
    --current;
    return *this;
  }


  self&
  operator--(
    int
  ){
    self copy = *this;
    ++iteration_counter_increments;
    --current;
    return copy;
  }


  self
  operator+(
    const Distance& n
  ) const {
    self self_copy(*this);
    return self_copy += n;
  }


  self
  operator+(
    const ssize_t n
  ) const {
    self self_copy(*this);
    return self_copy += Distance(n);
  }


  self&
  operator+=(
    const ssize_t& n
  ){
    return (*this) += Distance(n);
  }


  self&
  operator+=(
    const Distance& n
  ){
    ++iteration_counter_bigjumps;
    current += n;
    return *this;
  }


  self
  operator-(
    const Distance& n
  ) const {
    self self_copy(*this);
    return self_copy -= n;
  }


  self
  operator-(
    const ssize_t n
  ) const {
    self self_copy(*this);
    return self_copy -= Distance(n);
  }


  self&
  operator-=(
    const ssize_t n
  ) {
    return (*this) -= Distance(n);
  }


  self&
  operator-=(
    const Distance& n
  ) {
    ++iteration_counter_bigjumps;
    current -= n;
    return *this;
  }


  Reference
  operator[](
    const ssize_t n
  ){
    return (*this)[Distance(n)];
  }


  Reference
  operator[](
    const Distance& n
  ){
    iteration_counter_dereferences++;
    return *(*this + n);
  }

};


template <
  typename RandomAccessIterator_,
  typename T_,
  typename Reference_,
  typename Distance_>
bool
operator==(
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y
){
  ++iteration_counter_comparisons;
  return x.current == y.current;
}


template <
  typename RandomAccessIterator_,
  typename T_,
  typename Reference_,
  typename Distance_>
bool
operator!=(
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y
){
  return !(x == y);
}


template <
  typename RandomAccessIterator_,
  typename T_,
  typename Reference_,
  typename Distance_>
bool
operator<(
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y
){
   ++iteration_counter_comparisons;
   return x.current < y.current;
}


template <
  typename RandomAccessIterator_,
  typename T_,
  typename Reference_,
  typename Distance_>
bool
operator<=(
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y
){
   ++iteration_counter_comparisons;
   return x.current <= y.current;
}


template <
  typename RandomAccessIterator_,
  typename T_,
  typename Reference_,
  typename Distance_>
bool
operator>=(
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y
){
   ++iteration_counter_comparisons;
   return x.current >= y.current;
}


template <
  typename RandomAccessIterator_,
  typename T_,
  typename Reference_,
  typename Distance_>
bool
operator>(
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y
){
   ++iteration_counter_comparisons;
   return x.current > y.current;
}


template <
  typename RandomAccessIterator_,
  typename T_,
  typename Reference_,
  typename Distance_>
Distance_
operator-(
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x,
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& y
){
  ++iteration_counter_bigjumps;
  return Distance_(x.current - y.current);
}


template <
  typename RandomAccessIterator_,
  typename T_,
  typename Reference_,
  typename Distance_>
iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>
operator+(
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& n,
  const iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>& x
){
  ++iteration_counter_bigjumps;
  return iteration_counter<RandomAccessIterator_, T_, Reference_, Distance_>(x.current + n.current);
}


void print_iterator_stats(
  const struct config args
){
  const int width = 26;

  std::vector<std::string> headers = {
//    "Algorithm",
    "data assignments",
    "data comparisons",
    "data accesses",
    "distance constructions",
    "distance copy constructions",
    "distance conversions",
    "distance assignments",
    "distance increments",
    "distance additions",
    "distance subtractions",
    "distance multiplications",
    "distance divisions",
    "distance comparisons",
    "distance max generation",
    "iterator constructions",
    "iterator assignments",
    "iterator increments",
    "iterator dereferences",
    "iterator bigjumps",
    "iterator comparisons",
    "iterator max generation",
    "total",
  };


  std::vector<ssize_t> op_counts;

  op_counts.push_back(counter_assignments);
  op_counts.push_back(counter_comparisons);
  op_counts.push_back(counter_accesses);

  op_counts.push_back(distance_counter_constructions);
  op_counts.push_back(distance_counter_copy_constructions);
  op_counts.push_back(distance_counter_conversions);
  op_counts.push_back(distance_counter_assignments);
  op_counts.push_back(distance_counter_increments);
  op_counts.push_back(distance_counter_additions);
  op_counts.push_back(distance_counter_subtractions);
  op_counts.push_back(distance_counter_multiplications);
  op_counts.push_back(distance_counter_divisions);
  op_counts.push_back(distance_counter_comparisons);
  op_counts.push_back(distance_counter_max_generation);

  op_counts.push_back(iteration_counter_constructions);
  op_counts.push_back(iteration_counter_assignments);
  op_counts.push_back(iteration_counter_increments);
  op_counts.push_back(iteration_counter_dereferences);
  op_counts.push_back(iteration_counter_bigjumps);
  op_counts.push_back(iteration_counter_comparisons);
  op_counts.push_back(iteration_counter_max_generation);

  //int_least64_t total = std::accumulate(op_counts.begin(), op_counts.end(), 0);
  ssize_t total =
      counter_assignments
    + counter_comparisons
    + counter_accesses
    + distance_counter_constructions
    + distance_counter_copy_constructions
    + distance_counter_conversions
    + distance_counter_assignments
    + distance_counter_increments
    + distance_counter_additions
    + distance_counter_subtractions
    + distance_counter_multiplications
    + distance_counter_divisions
    + distance_counter_comparisons
    + distance_counter_max_generation
    + iteration_counter_constructions
    + iteration_counter_assignments
    + iteration_counter_increments
    + iteration_counter_dereferences
    + iteration_counter_bigjumps
    + iteration_counter_comparisons
    + iteration_counter_max_generation;
  op_counts.push_back(total);

  //cout << setiosflags(ios::fixed);

  cout << "algorithm";
  for(size_t i = 0; i < op_counts.size(); i++){
    if(0 != op_counts[i])
      cout << setw(width) << headers[i];
  }
  cout << endl;

  switch(args.chosen_sort){
    case introsort:
      {
        cout << "introsort";
      }
      break;
    case timsort:
      {
        cout << "timsort";
      }
      break;
    default:
      //Don't worry about this because it should never be possible to reach here
      exit(1);
  };
  for(auto i = op_counts.begin(); i != op_counts.end(); i++){
    if(0 != *i)
      cout << setw(width) << *i;
  }
  cout << endl;
}
