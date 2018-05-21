/*******************************************************************************

*******************************************************************************/

/*******************************************************************************
Copyright (c) 1997 Rensselaer Polytechnic Institute

Permission to use, copy, modify, distribute and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright notice
and this permission notice appear in supporting documentation.  Rensselaer
Polytechnic Institute makes no representations about the suitability of this
software for any purpose.  It is provided "as is" without express or implied
warranty.
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

#include <iterator>
#include <utility>
#include <iterator>

using std::iterator_traits;


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
  static ssize_t assignments;
  static ssize_t comparisons;
  static ssize_t accesses;


  T
  base(
  ) const {
    ++accesses;
    return value;
  }


  counter(
  ) : value(T()) {
    ++assignments;
  }


  explicit
  counter(
    const T& v
  ) : value(v) {
    ++assignments;
  }


  counter(
    const counter<T>& x
  ) : value(x.value) {
    ++assignments;
  }


  static
  ssize_t
  total(
  ){
    return assignments + comparisons + accesses;
  }


  static
  void
  reset(
  ){
    assignments = 0;
    comparisons = 0;
    accesses = 0;
  }


  friend
  bool
  operator<(
    const counter<T>& x,
    const counter<T>& y
  ){
    ++counter<T>::comparisons;
    return x.value < y.value;
  }


  counter<T>&
  operator=(
    const counter<T>& x
  ){
    ++assignments;
    value = x.value;
    return *this;
  }


  bool
  operator==(
    const counter<T>& x
  ) const {
    ++comparisons;
    return value == x.value;
  }

  bool
  operator!=(
    const counter<T>& x
  ) const {
    return !(value == x.value);
  }

  T
  operator* (
  ) const {
    return base();
  }
};


template <class T>
ssize_t counter<T>::assignments = 0;

template <class T>
ssize_t counter<T>::comparisons = 0;

template <class T>
ssize_t counter<T>::accesses = 0;


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

  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator==(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator<(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator<(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const _Distance& y);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  bool
  operator<(
    const _Distance& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _RandomAccessIterator
  operator+(
    _RandomAccessIterator i,
    const distance_counter<_RandomAccessIterator, _Distance>& x);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _RandomAccessIterator
  operator+=(
    _RandomAccessIterator& i,
    const distance_counter<_RandomAccessIterator, _Distance>& x);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  RandomAccessIterator
  operator-(
    RandomAccessIterator i,
    const distance_counter<_RandomAccessIterator, _Distance>& x);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  _RandomAccessIterator
  operator-=(
    _RandomAccessIterator &i,
    const distance_counter<_RandomAccessIterator, _Distance>& x);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator-(
    const distance_counter<_RandomAccessIterator, _Distance>& x,
    const distance_counter<_RandomAccessIterator, _Distance>& y);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator+(
    const _Distance& n,
    const distance_counter<_RandomAccessIterator, _Distance>& x);


  template <
    typename _RandomAccessIterator,
    typename _Distance>
  friend
  distance_counter<_RandomAccessIterator, _Distance>
  operator*(
    const _Distance& n,
    const distance_counter<_RandomAccessIterator, _Distance>& x);


protected:
  Distance current;
  ssize_t generation;


public:
  static ssize_t constructions;
  static ssize_t copy_constructions;
  static ssize_t conversions;
  static ssize_t assignments;
  static ssize_t increments;
  static ssize_t additions;
  static ssize_t subtractions;
  static ssize_t multiplications;
  static ssize_t divisions;
  static ssize_t comparisons;
  static ssize_t max_generation;


  static void reset() {
    constructions = 0;
    copy_constructions = 0;
    conversions = 0;
    assignments = 0;
    increments = 0;
    additions = 0;
    subtractions = 0;
    multiplications = 0;
    divisions = 0;
    comparisons = 0;
    max_generation = 0;
  }


  static
  ssize_t
  total(
  ){
    return constructions + copy_constructions + conversions + assignments
         + increments + additions + subtractions + multiplications + divisions
         + comparisons;
  }


  distance_counter(
  ) : generation(0) {
    ++constructions;
  }


  explicit
  distance_counter(
    const Distance& x
  ){
    current = x;
    generation = 0;
    ++conversions;
  }


  operator
  int(
  ) const {
    ++conversions;
    return current;
  }


  distance_counter(
    const distance_counter<RandomAccessIterator, Distance>& c
  ){
    current = c.current;
    generation = c.generation + 1;
    ++copy_constructions;
    if (generation > max_generation) {
      max_generation = generation;
    }
  }


  Distance
  base(
  ) const {
    return current;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator=(
    const distance_counter<RandomAccessIterator, Distance>& x
  ){
    ++assignments;
    current = x.current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator++(
  ){
    ++increments;
    ++current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator++(
    int
  ){
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    ++increments;
    ++current;
    return tmp;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator--(){
    ++increments;
    --current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator--(
    int
  ){
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    ++increments;
    --current;
    return tmp;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator+(
    const distance_counter<RandomAccessIterator, Distance>& n
  ) const {
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    return tmp += n;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator+(
    const Distance& n
  ) const {
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    return tmp += n;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator+=(
    const distance_counter<RandomAccessIterator, Distance>& n
  ){
    ++additions;
    current += n.current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator+=(
    const Distance& n
  ){
    ++additions;
    current += n;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator-(
    const distance_counter<RandomAccessIterator, Distance>& n
  ) const {
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    return tmp -= n;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator-(
    const Distance& n
  ) const {
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    return tmp -= n;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator-=(
    const distance_counter<RandomAccessIterator, Distance>& n
  ){
    ++subtractions;
    current -= n.current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator-=(
    const Distance& n
  ){
    ++subtractions;
    current -= n;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator*(
    const distance_counter<RandomAccessIterator, Distance>& n
  ) const {
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    return tmp *= n;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator*(
    const Distance& n
  ) const {
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    return tmp *= n;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator*=(
    const distance_counter<RandomAccessIterator, Distance>& n
  ){
    ++multiplications;
    current *= n.current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator*=(
    const Distance& n
  ){
    ++multiplications;
    current *= n;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>
  operator/(
    const distance_counter<RandomAccessIterator, Distance>& n
  ) const {
    distance_counter<RandomAccessIterator, Distance> tmp = *this;
    return tmp /= n;
  }


  distance_counter<RandomAccessIterator, Distance>&
  operator/=(
    const distance_counter<RandomAccessIterator, Distance>& n
  ){
    ++divisions;
    current /= n.current;
    return *this;
  }


  distance_counter<RandomAccessIterator, Distance>& operator/=(
    const Distance& n
  ){
    ++divisions;
    current /= n;
    return *this;
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
  ++distance_counter<_RandomAccessIterator, _Distance>::comparisons;
  return x.current == y.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
bool
operator<(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  ++distance_counter<_RandomAccessIterator, _Distance>::comparisons;
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
  ++distance_counter<_RandomAccessIterator, _Distance>::comparisons;
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
  ++distance_counter<_RandomAccessIterator, _Distance>::comparisons;
  return x < y.current;
}

template <
  typename _RandomAccessIterator,
  typename _Distance>
_RandomAccessIterator
operator+(
  _RandomAccessIterator i,
  const distance_counter<_RandomAccessIterator, _Distance>& x
){
  return i + x.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_RandomAccessIterator
operator+=(
  _RandomAccessIterator& i,
  const distance_counter<_RandomAccessIterator, _Distance>& x
){
  return i += x.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_RandomAccessIterator
operator-(
  _RandomAccessIterator i,
  const distance_counter<_RandomAccessIterator, _Distance>& x
){
  return i - x.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
_RandomAccessIterator
operator-=(
  _RandomAccessIterator &i,
  const distance_counter<_RandomAccessIterator, _Distance>& x
){
  return i -= x.current;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator-(
  const distance_counter<_RandomAccessIterator, _Distance>& x,
  const distance_counter<_RandomAccessIterator, _Distance>& y
){
  ++distance_counter<_RandomAccessIterator, _Distance>::subtractions;
  return distance_counter<_RandomAccessIterator, _Distance>(x.current - y.current);
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator+(
  const _Distance& n,
  const distance_counter<_RandomAccessIterator, _Distance>& x
){
  return x + n;
}


template <
  typename _RandomAccessIterator,
  typename _Distance>
distance_counter<_RandomAccessIterator, _Distance>
operator*(
  const _Distance& n,
  const distance_counter<_RandomAccessIterator, _Distance>& x
){
  return x * n;
}


template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::constructions = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::copy_constructions = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::conversions = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::assignments = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::increments = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::additions = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::subtractions = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::multiplications = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::divisions = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::comparisons = 0;
template <
  typename RandomAccessIterator,
  typename Distance>
ssize_t distance_counter<RandomAccessIterator, Distance>::max_generation = 0;



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

  static ssize_t constructions;
  static ssize_t assignments;
  static ssize_t increments;
  static ssize_t dereferences;
  static ssize_t bigjumps;
  static ssize_t comparisons;
  static ssize_t max_generation;


  static
  void
  reset(
  ){
    constructions = 0;
    assignments = 0;
    increments = 0;
    dereferences = 0;
    bigjumps = 0;
    comparisons = 0;
    max_generation = 0;
  }


  static
  ssize_t
  total(
  ){
    return constructions + assignments + increments + dereferences + bigjumps +
           comparisons + max_generation;
  }


  iteration_counter(
  ) : generation(0) {
    ++constructions;
  }


  iteration_counter(
    RandomAccessIterator x
  ){
    current = x;
    generation = 0;
    ++constructions;
  }


  iteration_counter(
    const self& c
  ){
    current = c.current;
    generation = c.generation + 1;
    ++constructions;
    if (generation > max_generation) {
      max_generation = generation;
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
    ++dereferences;
    return *current;
  }


  self&
  operator++(
  ){
    ++increments;
    ++current;
    return *this;
  }


  self&
  operator++(
    int
  ){
    self copy(*this);
    ++increments;
    ++current;
    return copy;
  }


  self&
  operator--(
  ){
    ++increments;
    --current;
    return *this;
  }


  self&
  operator--(
    int
  ){
    self copy = *this;
    ++increments;
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
    ++bigjumps;
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
    ++bigjumps;
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
    dereferences++;
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
  ++iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>::comparisons;
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
   ++iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>::comparisons;
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
   ++iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>::comparisons;
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
   ++iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>::comparisons;
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
   ++iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>::comparisons;
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
  ++iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>::bigjumps;
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
  ++iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>::bigjumps;
  return iteration_counter<_RandomAccessIterator, _T, _Reference, _Distance>(x.current + n.current);
}


template <
  typename RandomAccessIterator,
  typename T,
  typename Reference,
  typename Distance>
ssize_t iteration_counter<RandomAccessIterator, T, Reference, Distance>::constructions = 0;

template <
  typename RandomAccessIterator,
  typename T,
  typename Reference,
  typename Distance>
ssize_t iteration_counter<RandomAccessIterator, T, Reference, Distance>::assignments = 0;

template <
  typename RandomAccessIterator,
  typename T,
  typename Reference,
  typename Distance>
ssize_t iteration_counter<RandomAccessIterator, T, Reference, Distance>::increments = 0;

template <
  typename RandomAccessIterator,
  typename T,
  typename Reference,
  typename Distance>
ssize_t iteration_counter<RandomAccessIterator, T, Reference, Distance>::dereferences = 0;

template <
  typename RandomAccessIterator,
  typename T,
  typename Reference,
  typename Distance>
ssize_t iteration_counter<RandomAccessIterator, T, Reference, Distance>::bigjumps = 0;

template <
  typename RandomAccessIterator,
  typename T,
  typename Reference,
  typename Distance>
ssize_t iteration_counter<RandomAccessIterator, T, Reference, Distance>::comparisons = 0;

template <
  typename RandomAccessIterator,
  typename T,
  typename Reference,
  typename Distance>
ssize_t iteration_counter<RandomAccessIterator, T, Reference, Distance>::max_generation = 0;
