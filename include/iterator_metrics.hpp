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


static ssize_t iteration_counter__constructions = 0;
static ssize_t iteration_counter__assignments = 0;
static ssize_t iteration_counter__increments = 0;
static ssize_t iteration_counter__dereferences = 0;
static ssize_t iteration_counter__bigjumps = 0;
static ssize_t iteration_counter__comparisons = 0;
static ssize_t iteration_counter__max_generation = 0;

static ssize_t counter__assignments = 0;
static ssize_t counter__comparisons = 0;
static ssize_t counter__accesses = 0;

static ssize_t distance_counter__constructions = 0;
static ssize_t distance_counter__copy_constructions = 0;
static ssize_t distance_counter__conversions = 0;
static ssize_t distance_counter__assignments = 0;
static ssize_t distance_counter__increments = 0;
static ssize_t distance_counter__additions = 0;
static ssize_t distance_counter__subtractions = 0;
static ssize_t distance_counter__multiplications = 0;
static ssize_t distance_counter__divisions = 0;
static ssize_t distance_counter__comparisons = 0;
static ssize_t distance_counter__max_generation = 0;



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
    ++counter__accesses;
    return value;
  }


  counter(
  ) : value(T()) {
    ++counter__assignments;
  }


  explicit
  counter(
    const T& v
  ) : value(v) {
    ++counter__assignments;
  }


  counter(
    const counter<T>& x
  ) : value(x.value) {
    ++counter__assignments;
  }


  friend
  bool
  operator<(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter__comparisons;
    return x.value < y.value;
  }


  friend
  bool
  operator<=(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter__comparisons;
    return x.value <= y.value;
  }


  friend
  bool
  operator>(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter__comparisons;
    return x.value > y.value;
  }


  friend
  bool
  operator>=(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter__comparisons;
    return x.value >= y.value;
  }


  friend
  bool
  operator==(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter__comparisons;
    return x.value == y.value;
  }


  friend
  bool
  operator!=(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter__comparisons;
    return x.value != y.value;
  }


friend
bool
operator<(
  const counter<T>& x,
  const T& y
){
  ++counter__comparisons;
  return x.value < y;
}


friend
bool
operator<=(
  const counter<T>& x,
  const T& y
){
  ++counter__comparisons;
  return x.value <= y;
}


friend
bool
operator>(
  const counter<T>& x,
  const T& y
){
  ++counter__comparisons;
  return x.value > y;
}


friend
bool
operator>=(
  const counter<T>& x,
  const T& y
){
  ++counter__comparisons;
  return x.value >= y;
}


friend
bool
operator==(
  const counter<T>& x,
  const T& y
){
  ++counter__comparisons;
  return x.value == y;
}


friend
bool
operator!=(
  const counter<T>& x,
  const T& y
){
  ++counter__comparisons;
  return x.value != y;
}


friend
bool
operator<(
  const T& x,
  const counter<T>& y
){
  ++counter__comparisons;
  return x < y.value;
}


friend
bool
operator<=(
  const T& x,
  const counter<T>& y
){
  ++counter__comparisons;
  return x <= y.value;
}


friend
bool
operator>(
  const T& x,
  const counter<T>& y
){
  ++counter__comparisons;
  return x > y.value;
}


friend
bool
operator>=(
  const T& x,
  const counter<T>& y
){
  ++counter__comparisons;
  return x >= y.value;
}


friend
bool
operator==(
  const T& x,
  const counter<T>& y
){
  ++counter__comparisons;
  return x == y.value;
}


friend
bool
operator!=(
  const T& x,
  const counter<T>& y
){
  ++counter__comparisons;
  return x != y.value;
}


  counter<T>&
  operator=(
    const counter<T>& x
  ){
    ++counter__assignments;
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
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator==(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator==(
    const _Distance& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator==(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator<(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator<(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator<(
    const _Distance& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _RandomAccessIterator
  operator+(
    const _RandomAccessIterator& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator+(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator+(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _Distance
  operator+(
    const _Distance& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _RandomAccessIterator&
  operator+=(
    _RandomAccessIterator& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator+=(
    distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator+=(
    distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator-(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator-(
    const distance_counter<_RandomAccessIterator, _Distance>& s,
    const _Distance& n
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator-(
    const _RandomAccessIterator& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _RandomAccessIterator&
  operator-=(
    _RandomAccessIterator& i,
    const distance_counter<_RandomAccessIterator, _Distance>& x
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator-=(
    distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator-=(
    distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator*(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator*(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _Distance
  operator*(
    const _Distance& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator*=(
    distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator*=(
    distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _Distance&
  operator*=(
    _Distance& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator/(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator/(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance &y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _Distance
  operator/(
    const _Distance &x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator/=(
    distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _Distance&
  operator/=(
    _Distance& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y
  );


  template<
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>&
  operator/=(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y
  );


public:
  Distance current;
  ssize_t generation;


public:


  distance_counter<RandomAccessIterator, Distance>&
  operator++(
  ){
    ++distance_counter__increments;
    ++current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator++(
    int
  ){
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    ++distance_counter__increments;
    ++current;
    return tmp;
  }


  operator
  int(
  ) const {
    ++distance_counter__conversions;
    return current;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator--(){
    ++distance_counter__increments;
    --current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator--(
    int
  ){
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    ++distance_counter__increments;
    --current;
    return tmp;
  }

  distance_counter(
  ) : generation(0) {
    ++distance_counter__constructions;
  }


  explicit
  distance_counter(
    const Distance& x
  ){
    current = x;
    generation = 0;
    ++distance_counter__conversions;
  }


  distance_counter(
    const distance_counter<RandomAccessIterator, Distance>& c
  ){
    current = c.current;
    generation = c.generation + 1;
    ++distance_counter__copy_constructions;
    if (generation > distance_counter__max_generation) {
      distance_counter__max_generation = generation;
    }
  }

  distance_counter<RandomAccessIterator, Distance>&
  operator=(
    const distance_counter<RandomAccessIterator, Distance>& x
  ){
    current = x.current;
    generation = x.generation + 1;
    ++distance_counter__assignments;
    if (generation > distance_counter__max_generation) {
      distance_counter__max_generation = generation;
    }
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator=(
    const Distance& x
  ){
    current = x;
    ++distance_counter__assignments;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator=(
    const int& x
  ){
    current = Distance(x);
    ++distance_counter__assignments;
    return *this;
  }


  Distance
  base(
  ) const {
    return current;
  }

};




template <
  typename _RandomAccessIterator,
  typename _Distance>
bool
operator==(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  ++distance_counter__comparisons;
  return x.current == y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
bool
operator==(
  const _Distance& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  ++distance_counter__comparisons;
  return x == y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
bool
operator==(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance& y
){
  ++distance_counter__comparisons;
  return x.current == y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
bool
operator<(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  ++distance_counter__comparisons;
  return x.current < y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
bool
operator<(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance& y
){
  ++distance_counter__comparisons;
    return x.current < y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
bool
operator<(
  const _Distance& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  ++distance_counter__comparisons;
  return x < y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_RandomAccessIterator
operator+(
  const _RandomAccessIterator& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  _RandomAccessIterator tmp(x);
  return tmp += y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator+(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  distance_counter<_RandomAccessIterator, _Distance> tmp(x);
  return tmp += y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator+(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance& y
){
  distance_counter<_RandomAccessIterator, _Distance> tmp(x);
  return tmp += y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_Distance
operator+(
  const _Distance& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  _Distance tmp(x);
  return tmp += y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_RandomAccessIterator&
operator+=(
  _RandomAccessIterator& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  return x += y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator+=(
  distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  return x += y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator+=(
  distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance& y
){
  ++distance_counter__additions;
  x.current += y;
  return x;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator-(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  distance_counter<_RandomAccessIterator, _Distance> tmp(x);
  return tmp -= y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator-(
  const distance_counter<_RandomAccessIterator, _Distance>& s,
  const _Distance& n
){
  distance_counter<_RandomAccessIterator, _Distance> tmp(s);
  return tmp -= n;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator-(
  const _RandomAccessIterator& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  _RandomAccessIterator tmp(x);
  return tmp -= y;
}


//NOTE: Counts as iterator, so pass it along
template <
  typename _RandomAccessIterator,
  typename _Distance>
_RandomAccessIterator&
operator-=(
  _RandomAccessIterator& i,
  const distance_counter<_RandomAccessIterator, _Distance>& x
){
  return i -= x.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator-=(
  distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  return x -= y.current;
}


//NOTE: Key function
template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator-=(
  distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance& y
){
  ++distance_counter__subtractions;
  x.current -= y;
  return x;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator*(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  distance_counter<_RandomAccessIterator, _Distance> tmp(x);
  return x *= y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator*(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance& y
){
  distance_counter<_RandomAccessIterator, _Distance> tmp(x);
  return tmp *= y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_Distance
operator*(
  const _Distance& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  _Distance tmp(x);
  return tmp *= y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator*=(
  distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  return x *= y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator*=(
  distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance& y
){
  ++distance_counter__multiplications;
  x.current *= y;
  return x;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_Distance&
operator*=(
  _Distance& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  ++distance_counter__multiplications;
  return x *= y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator/(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  distance_counter<_RandomAccessIterator, _Distance> tmp(x);
  return tmp /= y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator/(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance &y
){
  distance_counter<_RandomAccessIterator, _Distance> tmp(x);
  return tmp /= y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_Distance
operator/(
  const _Distance &x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  _Distance tmp(x);
  return tmp /= y;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator/=(
  distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  return x /= y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_Distance&
operator/=(
  _Distance& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  ++distance_counter__divisions;
  return x /= y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>&
operator/=(
  distance_counter<_RandomAccessIterator, _Distance>& x,
  const _Distance& y
){
  ++distance_counter__divisions;
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
    typename _RandomAccessIterator,
    typename _T,
    typename _Reference,
    typename _Distance>
  friend
  bool
  operator==(
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y);


  template <
    typename _RandomAccessIterator,
    typename _T,
    typename _Reference,
    typename _Distance>
  friend
  bool
  operator!=(
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y);


  template <
    typename _RandomAccessIterator,
    typename _T,
    typename _Reference,
    typename _Distance>
  friend
  bool
  operator<(
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y);


  template <
    typename _RandomAccessIterator,
    typename _T,
    typename _Reference,
    typename _Distance>
  friend
  self
  operator+(
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y);


  template <
    typename _RandomAccessIterator,
    typename _T,
    typename _Reference,
    typename _Distance>
  friend
  _Distance
  operator-(
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
    const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y);


protected:
  RandomAccessIterator current;
  ssize_t generation;
public:

  iteration_counter(
  ) : generation(0) {
    ++iteration_counter__constructions;
  }


  iteration_counter(
    RandomAccessIterator x
  ){
    current = x;
    generation = 0;
    ++iteration_counter__constructions;
  }


  iteration_counter(
    const self& c
  ){
    current = c.current;
    generation = c.generation + 1;
    ++iteration_counter__constructions;
    if (generation > iteration_counter__max_generation) {
      iteration_counter__max_generation = generation;
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
    ++iteration_counter__dereferences;
    return *current;
  }


  self&
  operator++(
  ){
    ++iteration_counter__increments;
    ++current;
    return *this;
  }


  self&
  operator++(
    int
  ){
    self copy(*this);
    ++iteration_counter__increments;
    ++current;
    return copy;
  }


  self&
  operator--(
  ){
    ++iteration_counter__increments;
    --current;
    return *this;
  }


  self&
  operator--(
    int
  ){
    self copy = *this;
    ++iteration_counter__increments;
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
    ++iteration_counter__bigjumps;
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
    ++iteration_counter__bigjumps;
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
    iteration_counter__dereferences++;
    return *(*this + n);
  }

};


template <
  typename _RandomAccessIterator,
  typename _T,
  typename _Reference,
  typename _Distance>
bool
operator==(
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y
){
  ++iteration_counter__comparisons;
  return x.current == y.current;
}


template <
  typename _RandomAccessIterator,
  typename _T,
  typename _Reference,
  typename _Distance>
bool
operator!=(
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y
){
  return !(x == y);
}


template <
  typename _RandomAccessIterator,
  typename _T,
  typename _Reference,
  typename _Distance>
bool
operator<(
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y
){
   ++iteration_counter__comparisons;
   return x.current < y.current;
}


template <
  typename _RandomAccessIterator,
  typename _T,
  typename _Reference,
  typename _Distance>
bool
operator<=(
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y
){
   ++iteration_counter__comparisons;
   return x.current <= y.current;
}


template <
  typename _RandomAccessIterator,
  typename _T,
  typename _Reference,
  typename _Distance>
bool
operator>=(
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y
){
   ++iteration_counter__comparisons;
   return x.current >= y.current;
}


template <
  typename _RandomAccessIterator,
  typename _T,
  typename _Reference,
  typename _Distance>
bool
operator>(
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y
){
   ++iteration_counter__comparisons;
   return x.current > y.current;
}


template <
  typename _RandomAccessIterator,
  typename _T,
  typename _Reference,
  typename _Distance>
_Distance
operator-(
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x,
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& y
){
  ++iteration_counter__bigjumps;
  return _Distance(x.current - y.current);
}


template <
  typename _RandomAccessIterator,
  typename _T,
  typename _Reference,
  typename _Distance>
iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>
operator+(
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& n,
  const iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>& x
){
  ++iteration_counter__bigjumps;
  return iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>(x.current + n.current);
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

  op_counts.push_back(counter__assignments);
  op_counts.push_back(counter__comparisons);
  op_counts.push_back(counter__accesses);

  op_counts.push_back(distance_counter__constructions);
  op_counts.push_back(distance_counter__copy_constructions);
  op_counts.push_back(distance_counter__conversions);
  op_counts.push_back(distance_counter__assignments);
  op_counts.push_back(distance_counter__increments);
  op_counts.push_back(distance_counter__additions);
  op_counts.push_back(distance_counter__subtractions);
  op_counts.push_back(distance_counter__multiplications);
  op_counts.push_back(distance_counter__divisions);
  op_counts.push_back(distance_counter__comparisons);
  op_counts.push_back(distance_counter__max_generation);

  op_counts.push_back(iteration_counter__constructions);
  op_counts.push_back(iteration_counter__assignments);
  op_counts.push_back(iteration_counter__increments);
  op_counts.push_back(iteration_counter__dereferences);
  op_counts.push_back(iteration_counter__bigjumps);
  op_counts.push_back(iteration_counter__comparisons);
  op_counts.push_back(iteration_counter__max_generation);

  //int_least64_t total = std::accumulate(op_counts.begin(), op_counts.end(), 0);
  ssize_t total =
      counter__assignments
    + counter__comparisons
    + counter__accesses
    + distance_counter__constructions
    + distance_counter__copy_constructions
    + distance_counter__conversions
    + distance_counter__assignments
    + distance_counter__increments
    + distance_counter__additions
    + distance_counter__subtractions
    + distance_counter__multiplications
    + distance_counter__divisions
    + distance_counter__comparisons
    + distance_counter__max_generation
    + iteration_counter__constructions
    + iteration_counter__assignments
    + iteration_counter__increments
    + iteration_counter__dereferences
    + iteration_counter__bigjumps
    + iteration_counter__comparisons
    + iteration_counter__max_generation;
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
