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

################################################################################
# Prepare files ################################################################
################################################################################

for SORT in ${SORTS[@]} ; do
  for CONTAINER in ${CONTAINERS[@]} ; do
    for ORDERING in ${ORDERINGS[@]} ; do
      TESTING_PATH="$CONTAINER/$ORDERING/$SORT"

        if [ $TEST_ITERATOR_METRICS = true ] ; then
          #$ITR_PATH="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          echo "NOT IMPLEMENTED"
        fi
        if [ $TEST_CPU_AND_MEMORY = true ] ; then
          #Create pmem.dat and pcpu.dat, Peak Memory and Percentage CPU resp.
          #pull apart the columns.  Avg. % CPU first, then peak memory
          $RUN_PATH_CPU="tmp_data/rundata/$TESTING_PATH/cpu_points.dat"
          $RUN_PATH_MEM="tmp_data/rundata/$TESTING_PATH/mem_points.dat"
          echo "LENGTH\t$SORT" > "$RUN_PATH_CPU"
          echo "LENGTH\t$SORT" > "$RUN_PATH_MEM"
        fi
        if [ $TEST_TIME = true ] ; then
          $TIME_PATH="tmp_data/timedata/$TESTING_PATH/points.dat"
          echo "LENGTH\t$SORT" > "$TIME_PATH"
        fi
        if [ $TEST_CALLGRIND = true ] ; then
          $CPU_PATH="tmp_data/cpudata/$TESTING_PATH/points.dat"
          echo "LENGTH\t$SORT" > "$CPU_PATH"
        fi
        if [ $TEST_PERF = true ] ; then
          #echo "NOT IMPLEMENTED"
        fi
      done
  done
done

################################################################################
# Add data to files for individual tests #######################################
################################################################################

#Here we take the median for each trial, and combine each length for each
#combination of sort, container, and sequence ordering.  This is done in this
#particular way for later when the sorts are combined.
for SORT in ${SORTS[@]} ; do
  for CONTAINER in ${CONTAINERS[@]} ; do
    for ORDERING in ${ORDERINGS[@]} ; do
      TESTING_PATH="$CONTAINER/$ORDERING/$SORT"
      for LENGTH in ${LENGTHS[@]} ; do
        echo "Selecting data"

        if [ $TEST_ITERATOR_METRICS = true ] ; then
          #$ITR_PATH="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          echo "NOT IMPLEMENTED"
        fi
        if [ $TEST_CPU_AND_MEMORY = true ] ; then
          #Create pmem.dat and pcpu.dat, Peak Memory and Percentage CPU resp.
          #pull apart the columns.  Avg. % CPU first, then peak memory
          $INPUT_PATH="tmp_data/rundata/$TESTING_PATH/$LENGTH.tsv"
          $OUTPUT_CPU="tmp_data/rundata/$TESTING_PATH/cpu_points.dat"
          $OUTPUT_MEM="tmp_data/rundata/$TESTING_PATH/mem_points.dat"
          echo -n "$LENGTH\t" >> "$OUTPUT_CPU"
          echo -n "$LENGTH\t" >> "$OUTPUT_MEM"
          median "$(awk '{print $1}' $RUN_PATH | tr '\n' '\t')" >> "$OUTPUT_CPU"
          median "$(awk '{print $2}' $RUN_PATH | tr '\n' '\t')" >> "$OUTPUT_MEM"
          echo "" >> "$OUTPUT_CPU"
          echo "" >> "$OUTPUT_MEM"
        fi
        if [ $TEST_TIME = true ] ; then
          $RUN_PATH="tmp_data/timedata/$TESTING_PATH/$LENGTH.tsv"
          $OUTPUT_PATH="tmp_data/timedata/$TESTING_PATH/points.dat"
          echo -n "$LENGTH\t" >> "$OUTPUT_PATH"
          median "$(tr '\n' '\t' < $RUN_PATH)" >> "$OUTPUT_PATH"
          echo "" >> "$OUTPUT_PATH"
        fi
        if [ $TEST_CALLGRIND = true ] ; then
          #TODO: NOT IMPLEMENTED
          #$CPU_PATH="tmp_data/cpudata/$TESTING_PATH/$LENGTH.tsv"
          #$OUTPUT_PATH="tmp_data/cpudata/$TESTING_PATH/points.dat"

        fi
        if [ $TEST_PERF = true ] ; then
          #TODO: NOT IMPLEMENTED
        fi
      done
      echo "done!"
    done
  done
done

################################################################################
# Combine points files for sorts to the results are suitable for use ###########
################################################################################

for CONTAINER in ${CONTAINERS[@]} ; do
  for ORDERING in ${ORDERINGS[@]} ; do
    ITR_PATH=()
    RUN_PATH=()
    TIME_PATH=()
    CPU_PATH=()
    MEM_PATH=()
    CACHE_PATH=()

    # Build up paths of files
    for SORT in ${SORTS[@]} ; do
      TESTING_PATH="$CONTAINER/$ORDERING/$SORT"
      if [ $TEST_ITERATOR_METRICS = true ] ; then
        $ITR_PATH+="tmp_data/itrdata/$TESTING_PATH/points.dat"
      fi
      if [ $TEST_CPU_AND_MEMORY = true ] ; then
        $CPU_PATH+="tmp_data/rundata/$TESTING_PATH/cpu_points.dat"
        $MEM_PATH+="tmp_data/rundata/$TESTING_PATH/mem_points.dat"
      fi
      if [ $TEST_TIME = true ] ; then
        $TIME_PATH+="tmp_data/timedata/$TESTING_PATH/points.dat"
      fi
      if [ $TEST_CALLGRIND = true ] ; then
        $CACHE_PATH+="tmp_data/cpudata/$TESTING_PATH/points.dat"
      fi
      if [ $TEST_PERF = true ] ; then
        echo "NOT IMPLEMENTED"
      fi
    done

    # merge files of interest
    COMPILED_TESTS_PATH="$CONTAINER/$ORDERING"
    if [ $TEST_ITERATOR_METRICS = true ] ; then
        recursive_join "$ITR_PATH" > "tmp_data/itrdata/$COMPILED_TESTS_PATH/compiled_points.dat"
    fi
    if [ $TEST_CPU_AND_MEMORY = true ] ; then
        recursive_join "$CPU_PATH" > "tmp_data/rundate/$COMPILED_TESTS_PATH/cpu_compiled_points.dat"
        recursive_join "$MEM_PATH" > "tmp_data/rundata/$COMPILED_TESTS_PATH/mem_compiled_points.dat"
    fi
    if [ $TEST_TIME = true ] ; then
        recursive_join "$TIME_PATH" > "tmp_data/timedata/$COMPILED_TESTS_PATH/compiled_points.dat"
    fi
    if [ $TEST_CALLGRIND = true ] ; then
        recursive_join "$ACHE_PATH" > "tmp_data/cpudata/$COMPILED_TESTS_PATH/compiled_points.dat"
    fi
    if [ $TEST_PERF = true ] ; then
      #TODO: NOT IMPLEMENTED
    fi
  done
done
