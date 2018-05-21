/*******************************************************************************
Copyright <AMBIGUOUS>, 2018
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

#include <iostream>
#include <random>

#include "parse_arguments.hpp"


template<typename container>
void fill_with_sorted(
  container &data,
  ssize_t length)
{
  for(ssize_t i = 0; i < length; i++)
    data.push_back(LONG_MIN + i);
}


template<typename container>
void fill_with_reverse_sorted(
  container &data,
  ssize_t length)
{
  for(ssize_t i = 0; i < length; i++)
    data.push_back(LONG_MAX - i);
}


template<typename container>
void fill_with_random(
  container &data,
  ssize_t length)
{
  std::mt19937_64 rng(0);
  for(ssize_t i = 0; i < length; i++)
    data.push_back(rng());
}


template<typename container>
void fill_with_median_of_three_killer(
  container &data,
  ssize_t length)
{
  //TODO
  if(length%2 != 0){
    cout << "The 'median_of_three' requires an even length." << endl;
    exit(EINVAL);
  }
  for(ssize_t i = 0; i < length; i++){
    if(i < length/2){
      if(i % 2 == 0){
        data.push_back(i);
      }else{
        data.push_back((length/2)+(i-1));
      }
    }else{
      data.push_back((i-length) * 2);
    }
  }
}


template<typename container>
void fill_with_stdin(
  container data,
  ssize_t length)
{
  //TODO: don't ignore length and add it as a way to limit read size.  This will
  //allow for things like reading from /dev/urandom.
  cout << "ERROR: unimplemented" << endl;
  exit(1);
  //std::istream_iterator<char> begin(std::cin), end;
  //std::copy(begin, end, std::back_inserter(data));
}


template<typename container>
void fill_with_sorted_forward(
  container &data,
  ssize_t length)
{
  for(ssize_t i = 0; i < length; i++)
    data.push_front(LONG_MAX - i);
}


template<typename container>
void fill_with_reverse_sorted_forward(
  container &data,
  ssize_t length)
{
  for(ssize_t i = 0; i < length; i++)
    data.push_front(LONG_MIN + i);
}


template<typename container>
void fill_with_random_forward(
  container &data,
  ssize_t length)
{
  std::mt19937_64 rng(0);
  for(ssize_t i = 0; i < length; i++)
    data.push_front(rng());
}


template<typename container>
void fill_with_median_of_three_killer_forward(
  container &data,
  ssize_t length)
{
  //TODO
  if(length%2 != 0){
    cout << "The 'median_of_three' requires an even length." << endl;
    exit(EINVAL);
  }
  for(ssize_t i = length-1; i >= 0; i--){
    if(i < length/2){
      if(i % 2 == 0){
        data.push_front(i);
      }else{
        data.push_front((length/2)+(i-1));
      }
    }else{
      data.push_front((i-length) * 2);
    }
  }
}


template<typename container>
void fill_with_stdin_forward(
  container data,
  ssize_t length)
{
  //TODO: don't ignore length and add it as a way to limit read size.  This will
  //allow for things like reading from /dev/urandom.
  cout << "ERROR: unimplemented" << endl;
  exit(1);
  //std::istream_iterator<char> begin(std::cin), end;
  //std::copy(begin, end, std::front_inserter(data));
}


//NOTE: In future iterations of C++, this code will be able to become massively
//simpler.  But not at the time of writing.

template<
  typename T,
  template<typename, typename...> class container>
void
populate_container(
  const struct config &args,
  container<T> &data
){
  switch(args.chosen_test){
    case sorted:
      {
        fill_with_sorted(data, args.test_length);
      }
      break;
    case reverse_sorted:
      {
        fill_with_reverse_sorted(data, args.test_length);
      }
      break;
    case random_order:
      {
        fill_with_random(data, args.test_length);
      }
      break;
    case median_of_three_killer:
      {
        fill_with_median_of_three_killer(data, args.test_length);
      }
      break;
    case stdin_:
      {
        fill_with_stdin_forward(std::front_inserter(data), args.test_length);
      }
      break;
    default:
      exit(-3);
  };
}
