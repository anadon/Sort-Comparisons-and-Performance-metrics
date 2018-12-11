#!/bin/bash
# shellcheck disable=SC2034

if [ "$ALREADY_SETUP" != true ]
then

  function print_array {
    arr=("$@")
    for i in "${arr[@]}" ; do
      echo -n "$i "
    done
    echo ""
  }


  #These are the particular things to run, like gathering iterator metrics,
  #timing and memory information from gnu time, cache profiling from valgrind
  # --tool=callgrind, perf, or anything else that gets added.
  TEST_ITERATOR_METRICS=false
  TEST_TIME=true
  TEST_CPU_AND_MEMORY=true
  TEST_CALLGRIND=false
  TEST_PERF=true

  DEV_SETTINGS=false

  if [ "$DEV_SETTINGS" == false ] ; then

    #introsort ignored at this time because it is implemented as std_sort
    SORTS=( sequential_timsort null std_sort std_stable_sort  tvs_timsort )
    #deque omitted because it is slower and seems to be a little unstable
    CONTAINERS=( vector )
    ORDERINGS=( random_order median_of_three_killer sorted )
    LENGTHS=( )

    #Calculate out sizes to test from 1 entry to ~1,000,000,000 entries.
    for i in $(seq 1 20) ;
    do
      LENGTHS+=( $(( 2**i )) )
    done

    #While developing use 2, else 7
    NUM_TRIALS=7

    #These are the particular things to run, like gathering iterator metrics,
    #timing and memory information from gnu time, cache profiling from valgrind
    # --tool=callgrind, perf, or anything else that gets added.
    #TEST_ITERATOR_METRICS=false
    #TEST_CPU_AND_MEMORY=true
    #TEST_TIME=true
    #TEST_CALLGRIND=false
    #TEST_PERF=false
  else

    #introsort ignored at this time because it is implemented as std_sort
    SORTS=( null std_sort std_stable_sort sequential_timsort gfx_timsort tvs_timsort )
    #deque omitted because it is slower and seems to be a little unstable
    CONTAINERS=( vector )
    ORDERINGS=( sorted random_order median_of_three_killer )

    LENGTHS=( 512 )

    NUM_TRIALS=2

    #TEST_ITERATOR_METRICS=false
    TEST_CPU_AND_MEMORY=false
    TEST_TIME=true
    TEST_CALLGRIND=false
    #TEST_PERF=false
  fi


  ALREADY_SETUP=true

fi

#$1=path to data
#$2=title
#$3=x axis label
#$4=y axis label
#$5=save file location
function plot_wrapper {
  NUM_COLS="$(awk '{print NF}' $1 | sort -nu | tail -n 1)"
  PLOT_CMD=""
#  PLOT_CMD+="set term wxt enhanced\n"
  PLOT_CMD+="set term postscript eps enhanced color\n"
  PLOT_CMD+="set key autotitle columnheader\n"
  PLOT_CMD+="set title \"$2\"\n"
  PLOT_CMD+="set xlabel \"$3\"\n"
  PLOT_CMD+="set ylabel \"$4\"\n"
  PLOT_CMD+="set output \"$5\"\n"
  PLOT_CMD+="show grid \n"
  PLOT_CMD+="plot for [i=2:$NUM_COLS] '$1' using 1:i with lines \n"
  PLOT_CMD="$(echo -e $PLOT_CMD)"
  echo "$PLOT_CMD" | gnuplot -

}
