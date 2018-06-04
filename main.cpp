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

/*
Josh Marshall
TODO: add license header
TODO: add copyright
TODO: add brief explanation
TODO: Add rebust argument handling
*/

#include <deque>
//#include <forward_list>
//#include <list>
#include <vector>

#include "data_preparation.hpp"
#include "iterator_metrics.hpp"
#include "parse_arguments.hpp"
#include "sort_abstracter.hpp"


using std::vector;
using std::deque;


template<
  typename T,
  template<typename, typename...> class container>
void
run_test_on_container(
  const struct config args,
  container<T>
){

  typedef iteration_counter<
    typename container<T>::iterator,
    typename container<T>::value_type,
    typename container<T>::value_type&,
    distance_counter<
      typename container<T>::iterator,
      typename container<T>::difference_type> >
  iterator;

  container<T> data;
  populate_container(args, data);

  if(args.enable_iterator_metrics){
    auto begin = iterator(data.begin());
    auto end   = iterator(data.end());
    auto sorter = get_sort_func_ptr(args, begin);
    sorter(begin, end);
    print_iterator_stats(args);
  }else{
    auto begin = data.begin();
    auto end   = data.end();
    auto sorter = get_sort_func_ptr(args, begin);
    sorter(begin, end);
  }
}


void
test_bootstrap(
  const struct config args
){
  if(args.enable_iterator_metrics){
    switch(args.chosen_container){
      case deque_:
        run_test_on_container(args, std::deque<counter<long int> >());
        break;
      case vector_:
        run_test_on_container(args, std::vector<counter<long int> >());
        break;
      default:
        exit(-2);
    };
  }else{
      switch(args.chosen_container){
        case deque_:
          run_test_on_container(args, std::deque<long int>());
          break;
        case vector_:
          run_test_on_container(args, std::vector<long int>());
          break;
        default:
          exit(-2);
      };

  }

}


int main(int argc, char **argv){

  #ifdef SCP_DEBUG
  cout << "Starting" << endl;
  #endif

  config run_config = {undefined_sort, undefined_test, undefined_container, 0};

  argp_parse(&interpreter, argc, argv, 0, 0, &run_config);

  #ifdef SCP_DEBUG
  cout << "Got configuration, running analysis" << endl;
  #endif

  test_bootstrap(run_config);

  #ifdef SCP_DEBUG
  cout << "Complete" << endl;
  #endif

  return 0;
}
