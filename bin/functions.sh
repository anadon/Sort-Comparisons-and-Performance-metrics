#!/bin/bash


if [ ALREADY_SETUP != true ]
then

  function print_array {
    arr=("$@")
    for i in "${arr[@]}" ; do
      echo -n "$i "
    done
    echo ""
  }


  #introsort ignored at this time because it is implemented as std_sort
  SORTS=( null std_sort std_stable_sort sequential_timsort gfx_timsort tvs_timsort )
  #deque omitted because it is slower and seems to be a little unstable
  CONTAINERS=( vector )
  ORDERINGS=( sorted reverse_sorted random_order median_of_three_killer )
  LENGTHS=( )

  #Calculate out sizes to test from 1 entry to ~1,000,000,000 entries.
  #for i in $(seq 1 28) ;
  #do
  #  LENGTHS+=( $(( 2**$i )) )
  #done
  #But for development, only worry about 512
  LENGTHS+=( 512 )

  #While developing use 2, else 7
  NUM_TRIALS=2

  #These are the particular things to run, like gathering iterator metrics,
  #timing and memory information from gnu time, cache profiling from valgrind
  # --tool=callgrind, perf, or anything else that gets added.
  TEST_ITERATOR_METRICS=false
  TEST_TIME_AND_MEMORY=true
  TEST_CALLGRIND=false
  TEST_PERF=false


  ALREADY_SETUP=true

fi
