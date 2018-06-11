#!/bin/bash
#Some code lifted from:
#https://stackoverflow.com/questions/41750008/get-median-of-unsorted-array-in-one-line-of-bash

source functions.sh

setup_variables


function median() {
  arr=($(printf '%d\n' "${@}" | sort -n))
  nel=${#arr[@]}
  if (( $nel % 2 == 1 )); then     # Odd number of elements
    val="${arr[ $(($nel/2)) ]}"
  else                            # Even number of elements
    (( j=nel/2 ))
    (( k=j-1 ))
    (( val=(${arr[j]} + ${arr[k]})/2 ))
  fi
  echo $val
}


function recursive_join() {

}


for SORT in ${SORTS[@]} ; do
  for CONTAINER in ${CONTAINERS[@]} ; do
    for ORDERING in ${ORDERINGS[@]} ; do
        echo "Selecting data"

        if [ $TEST_ITERATOR_METRICS = true ] ; then
          $ITR_PATH="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          echo "NOT IMPLEMENTED"
        fi
        if [ $TEST_CPU_AND_MEMORY = true ] ; then
          echo -n '.'
          $RUN_PATH="tmp_data/rundata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          #Create pmem.dat and pcpu.dat, Peak Memory and Percentage CPU resp.
          #pull apart the columns.  Avg. % CPU first, then peak memory

        fi
        if [ $TEST_TIME = true ] ; then
          $TIME_PATH="tmp_data/timedata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          echo "LENGTH\t$SORT" > points.dat
        fi
        if [ $TEST_CALLGRIND = true ] ; then
          $CPU_PATH="tmp_data/cpudata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"

        fi
        if [ $TEST_PERF = true ] ; then
          echo "NOT IMPLEMENTED"
        fi
      done
  done
done


for SORT in ${SORTS[@]} ; do
  for CONTAINER in ${CONTAINERS[@]} ; do
    for ORDERING in ${ORDERINGS[@]} ; do
      for LENGTH in ${LENGTHS[@]} ; do
        echo "Selecting data"

        if [ $TEST_ITERATOR_METRICS = true ] ; then
          $ITR_PATH="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          echo "NOT IMPLEMENTED"
        fi
        if [ $TEST_CPU_AND_MEMORY = true ] ; then
          echo -n '.'
          $RUN_PATH="tmp_data/rundata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          #Create pmem.dat and pcpu.dat, Peak Memory and Percentage CPU resp.
          #pull apart the columns.  Avg. % CPU first, then peak memory

        fi
        if [ $TEST_TIME = true ] ; then
          $RUN_PATH="tmp_data/timedata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          echo -n "$LENGTH\t" >> points.dat
          median "$(tr '\n' '\t' < $RUN_PATH)" >> points.dat
          echo ""  >> points.dat
        fi
        if [ $TEST_CALLGRIND = true ] ; then
          $CPU_PATH="tmp_data/cpudata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"

        fi
        if [ $TEST_PERF = true ] ; then
          echo "NOT IMPLEMENTED"
        fi
      done
      echo "done!"
    done
  done
done


for CONTAINER in ${CONTAINERS[@]} ; do
  for ORDERING in ${ORDERINGS[@]} ; do
    echo "Combining data"

    if [ $TEST_ITERATOR_METRICS = true ] ; then
      $ITR_PATH="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
      echo "NOT IMPLEMENTED"
    fi
    if [ $TEST_CPU_AND_MEMORY = true ] ; then
      echo -n '.'
      $RUN_PATH="tmp_data/rundata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
      #Create pmem.dat and pcpu.dat, Peak Memory and Percentage CPU resp.
      #pull apart the columns.  Avg. % CPU first, then peak memory

    fi
    if [ $TEST_TIME = true ] ; then
      $TIME_PATH="tmp_data/timedata/$SORT/$CONTAINER/$ORDERING/points.dat"

    fi
    if [ $TEST_CALLGRIND = true ] ; then
      $CPU_PATH="tmp_data/cpudata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"

    fi
    if [ $TEST_PERF = true ] ; then
      echo "NOT IMPLEMENTED"
    fi
  done
done


for CONTAINER in ${CONTAINERS[@]} ; do
  for ORDERING in ${ORDERINGS[@]} ; do
    ITR_PATH=()
    RUN_PATH=()
    TIME_PATH=()
    CPU_PATH=()

    for SORT in ${SORTS[@]} ; do
      if [ $TEST_ITERATOR_METRICS = true ] ; then
        $ITR_PATH+="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
      fi
      if [ $TEST_CPU_AND_MEMORY = true ] ; then
        $RUN_PATH+="tmp_data/rundata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
      fi
      if [ $TEST_TIME = true ] ; then
        $TIME_PATH+="tmp_data/timedata/$SORT/$CONTAINER/$ORDERING/points.dat"
      fi
      if [ $TEST_CALLGRIND = true ] ; then
        $CPU_PATH+="tmp_data/cpudata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
      fi
      if [ $TEST_PERF = true ] ; then
        echo "NOT IMPLEMENTED"
      fi
    done


    if [ $TEST_ITERATOR_METRICS = true ] ; then
        recursive_join "$ITR_PATH"
    fi
    if [ $TEST_CPU_AND_MEMORY = true ] ; then
        recursive_join "$RUN_PATH"
    fi
    if [ $TEST_TIME = true ] ; then
        recursive_join "$TIME_PATH"
    fi
    if [ $TEST_CALLGRIND = true ] ; then
        recursive_join "$CPU_PATH"
    fi
    if [ $TEST_PERF = true ] ; then
      echo "NOT IMPLEMENTED"
    fi
  done
done
