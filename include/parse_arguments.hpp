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

#include <algorithm>
#include <argp.h>
#include <iostream>
#include <cstring>


using std::cout;
using std::endl;


enum sort_type{
  undefined_sort,
  std_sort,
  std_stable_sort,
  introsort,
  timsort,
  null
};


enum sort_test_type{
  undefined_test,
  sorted,
  reverse_sorted,
  random_order,
  median_of_three_killer,
  stdin_
};


enum container_type{
  undefined_container,
  deque_,
//  forward_list_,
//  list_,
  vector_
};


struct config{
  sort_type chosen_sort = undefined_sort;
  sort_test_type chosen_test = undefined_test;
  container_type chosen_container = undefined_container;
  ssize_t test_length = 0;
  bool enable_iterator_metrics = false;
};



const char *argp_program_version = "1.0.0";


const char *argp_program_bug_address = "jrmarsha@mtu.edu";


argp_parser_t parser;


const char *doc = "This is the 'sort-performance-comparison' tool, developed to "
"evaluate the differences between the established introsort and a new proposed "
" timsort.";


static struct argp_option options[] = {
  {"test", 't', "STRING", 0, "Perform one of the following specified tests: sorted, reverse_sorted, random_order, median_of_three_killer, stdin.  This must be specified once.", 0},
  {"length", 'l', "INT", 0, "Specify the size of the data to test with a sort.  This argument is required for 'sorted', 'reverse-sorted', 'random_order', and 'median_of_three_killer'.  It is optional for 'stdin'.  This may only be specified once, and must be a positive integer value.", 0},
  {"sort-type", 's', "STRING", 0, "Specify the sort to use on the input data.  Currently supported sorts are 'std_sort', 'std_stable_sort', 'introsort', 'timsort' and 'null'.  The 'null' option is intended to be an option to allow measurement of the overhead of setting up incurred by the program in order to allow more accurate evaluation and comparison of the other sort functions.", 0},
  {"container", 'c', "STRING", 0, "Specify the underlying container type from the Standard Template Library to use.  Be aware that not every sort can use every container type, so you must be aware of the different underlying differences.  For most cases, this should be set to 'vector'.", 0},
  {"enable-interator-metrics", 'i', "STRING", OPTION_ARG_OPTIONAL, "Default: disabled.  Track various metrics related to iterator operations to better understand what kind of operations a sort is doing, and allow direct comparison of the performance of various operations between sorts.  This can be turned on with 'enable' or 'true', and explicitly disabled with 'disable' or 'false'.", 0},
  { 0 , 0, 0, 0, 0, 0}
};


static error_t parse_opt(int key, char *arg, struct argp_state *state){
  struct config *args;
  char *sanityCheck;
  std::string notPositiveMsg;
  args = (struct config*) state->input;
  switch(key){
    case 't':
      {
        if(nullptr == arg){
          cout << "No argument given for 'test type' parameter" << endl;
          exit(EINVAL);
        }

        if(args->chosen_test != undefined_test){
          cout << "Cannot test multiple data in the same run." << endl;
          exit(EINVAL);
        }

        if(!strcmp("sorted", arg)){
          args->chosen_test = sorted;
        }else if(!strcmp("reverse_sorted", arg)){
          args->chosen_test = reverse_sorted;
        }else if(!strcmp("random_order", arg)){
          args->chosen_test = random_order;
        }else if(!strcmp("median_of_three_killer", arg)){
          args->chosen_test = median_of_three_killer;
        }else if(!strcmp("stdin", arg)){
          args->chosen_test = stdin_;
        }else{
          cout << "Specified test is not supported." << endl;
          exit(EINVAL);
        }
      }
      break;
    case 'l':
      {
        if(nullptr == arg){
          cout << "No argument given for 'test length' parameter" << endl;
          exit(EINVAL);
        }
        if(0 != args->test_length){
          cout << "Can't set length multiple times" << endl;
          exit(EINVAL);
        }
        errno = 0;
        args->test_length = strtol(arg, &sanityCheck, 10);
        notPositiveMsg = std::string("Specified length is not a positive integer.");
        if(arg+strlen(arg) != sanityCheck){
          //User may be trying to abuse this.  We'll catch this, but not let them
          //know it was detected seperately.
          cout << notPositiveMsg << endl;
          exit(EINVAL);
        }
        if(args->test_length <= 0 ){
          cout << notPositiveMsg << endl;
          exit(EINVAL);
        }
        if(ERANGE == errno){
          cout << "Specified length is too large for this hardware." << endl;
          exit(EINVAL);
        }
      }
      break;
    case 's':
      {
        if(nullptr == arg){
          cout << "No argument given for 'chosen' parameter" << endl;
          exit(EINVAL);
        }
        if(args->chosen_sort != undefined_sort){
          cout << "Cannot test multiple sorts in the same run." << endl;
          if(args->chosen_sort == undefined_sort){
            cout << "HE AWAKENS" << endl;
          }else if(args->chosen_sort == std_sort){
            cout << "Previously set to std_sort" << endl;
          }else if(args->chosen_sort == std_stable_sort){
            cout << "Previously set to std_stable_sort" << endl;
          }else if(args->chosen_sort == introsort){
            cout << "Previously set to introsort" << endl;
          }else if(args->chosen_sort == timsort){
            cout << "Previously set to timsort" << endl;
          }else if(args->chosen_sort == null){
            cout << "Previously set to null" << endl;
          }else{
            cout << "Totally undefined state" << endl;
          }

          exit(EINVAL);
        }

        if(!strcmp("std_sort", arg)){
          args->chosen_sort = std_sort;
        }else if(!strcmp("std_stable_sort", arg)){
          args->chosen_sort = std_stable_sort;
        }else if(!strcmp("introsort", arg)){
          args->chosen_sort = introsort;
        }else if(!strcmp("timsort", arg)){
          args->chosen_sort = timsort;
        }else if(!strcmp("null", arg)){
          args->chosen_sort = null;
        }else{
          cout << "Specified sort is not supported." << endl;
          exit(EINVAL);
        }
      }
      break;
    case 'c':
      {
        if(nullptr == arg){
          cout << "No argument given for 'container' parameter" << endl;
          exit(EINVAL);
        }
        if(args->chosen_container != undefined_container){
          cout << "Can't set what container to use multiple times" << endl;
          exit(EINVAL);
        }else if(!strcmp("deque", arg)){
          args->chosen_container = deque_;
        }else if(!strcmp("vector", arg)){
          args->chosen_container = vector_;
  //      }else if(!strcmp("forward_list", arg)){
  //        args->chosen_container = forward_list_;
  //      }else if(!strcmp("list", arg)){
  //        args->chosen_container = list_;
        }else{
          cout << "Unrecognized container type \"" << arg << "\"" << endl;
          exit(EINVAL);
        }
      }
      break;
    case 'i':
      {
        if(nullptr == arg){
          cout << "No argument given for 'iterator metrics' parameter" << endl;
          exit(EINVAL);
        }
        std::string lc_arg(arg);
        std::transform(lc_arg.begin(), lc_arg.end(), lc_arg.begin(), ::tolower);
        if(lc_arg == "true" || lc_arg == "enable"){
          args->enable_iterator_metrics = true;
        }else if(lc_arg == "false" || lc_arg == "disable"){
          args->enable_iterator_metrics = false;
        }else{
          cout << "Unrecognized argument for iterator metrics." << endl;
          exit(EINVAL);
        }
      }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}


static struct argp interpreter = {options, parse_opt, 0, doc, 0, 0, 0};
