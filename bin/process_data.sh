#!/bin/bash
#Some code lifted from:
#https://stackoverflow.com/questions/41750008/
#get-median-of-unsorted-array-in-one-line-of-bash
#https://stackoverflow.com/questions/10726471/join-multiple-files
#https://stackoverflow.com/questions/21524585/
#gnuplot-can-i-put-all-the-settings-for-a-plot-in-a-file-and-call-it-before-th

# shellcheck disable=SC1091
# shellcheck disable=SC2153
source functions.sh


function median {
  mapfile -t arr < <(sort <<< "${@}")
  nel=${#arr[@]}
  if (( nel % 2 == 1 )); then     # Odd number of elements
    val="${arr[ $((nel/2)) ]}"
  else                            # Even number of elements
    (( j=nel/2 ))
    (( k=j-1 ))
    (( vall=arr[j] ))
    (( valr=arr[k] ))
    (( val=(vall + valr)/2 ))
  fi
  echo "$val"
}


function recursive_join {
  #print_array "${@}"
  #echo "Joining $1" 2>&1
  if [ $# -eq 2 ]; then
    echo "Joining commencing" 2>&1
    join -j 1 --nocheck-order "$1" "$2"
  else
    f=$1; shift
    recursive_join "$@" | join -j 1 --nocheck-order - "$f"
  fi
}


################################################################################
# Prepare files ################################################################
################################################################################

echo "Preparing files"
for SORT in "${SORTS[@]}" ; do
  for CONTAINER in "${CONTAINERS[@]}" ; do
    for ORDERING in "${ORDERINGS[@]}" ; do
      TESTING_PATH="$CONTAINER/$ORDERING/$SORT"

        if [ "$TEST_ITERATOR_METRICS" == true ] ; then
          #$ITR_PATH="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          echo "NOT IMPLEMENTED"
        fi
        if [ "$TEST_CPU_AND_MEMORY" == true ] ; then
          #Create pmem.dat and pcpu.dat, Peak Memory and Percentage CPU resp.
          #pull apart the columns.  Avg. % CPU first, then peak memory
          RUN_PATH_CPU="tmp_data/rundata/$TESTING_PATH/cpu_points.dat"
          RUN_PATH_MEM="tmp_data/rundata/$TESTING_PATH/mem_points.dat"
          printf 'LENGTH\t%s\n' "$SORT" > "$RUN_PATH_CPU"
          printf 'LENGTH\t%s\n' "$SORT" > "$RUN_PATH_MEM"
        fi
        if [ "$TEST_TIME" == true ] ; then
          TIME_PATH="tmp_data/timedata/$TESTING_PATH/points.dat"
          printf 'LENGTH\t%s\n' "$SORT" > "$TIME_PATH"
        fi
        if [ "$TEST_CALLGRIND" == true ] ; then
          CPU_PATH="tmp_data/cpudata/$TESTING_PATH/points.dat"
          printf 'LENGTH\t%s\n' "$SORT" > "$CPU_PATH"
        fi
        #if [ $TEST_PERF == true ] ; then
          #echo "NOT IMPLEMENTED"
        #fi
      done
  done
done

################################################################################
# Add data to files for individual tests #######################################
################################################################################

#Here we take the median for each trial, and combine each length for each
#combination of sort, container, and sequence ordering.  This is done in this
#particular way for later when the sorts are combined.
echo "Selecting and combinging data from trials"
for SORT in "${SORTS[@]}" ; do
  for CONTAINER in "${CONTAINERS[@]}" ; do
    for ORDERING in "${ORDERINGS[@]}" ; do
      TESTING_PATH="$CONTAINER/$ORDERING/$SORT"
      for LENGTH in "${LENGTHS[@]}" ; do

        if [ "$TEST_ITERATOR_METRICS" == true ] ; then
          #$ITR_PATH="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/$LENGTH.tsv"
          echo "NOT IMPLEMENTED"
        fi
        if [ "$TEST_CPU_AND_MEMORY" == true ] ; then
          #Create pmem.dat and pcpu.dat, Peak Memory and Percentage CPU resp.
          #pull apart the columns.  Avg. % CPU first, then peak memory
          INPUT_PATH="tmp_data/rundata/$TESTING_PATH/$LENGTH.tsv"
          OUTPUT_CPU="tmp_data/rundata/$TESTING_PATH/cpu_points.dat"
          OUTPUT_MEM="tmp_data/rundata/$TESTING_PATH/mem_points.dat"
          CPU_MEDIAN="$(median "$(awk '{print $1}' "$INPUT_PATH" )" )"
          MEM_MEDIAN="$(median "$(awk '{print $2}' "$INPUT_PATH" )" )"
          printf '%d\t%d\n' "$LENGTH" "$CPU_MEDIAN" >> "$OUTPUT_CPU"
          printf '%d\t%d\n' "$LENGTH" "$MEM_MEDIAN" >> "$OUTPUT_MEM"
        fi
        if [ "$TEST_TIME" == true ] ; then
          RUN_PATH="tmp_data/timedata/$TESTING_PATH/$LENGTH.tsv"
          OUTPUT_PATH="tmp_data/timedata/$TESTING_PATH/points.dat"
          MEDIAN="$(median "$(tr '\t' '\n' < "$RUN_PATH" )" )"
          printf '%d\t%d\n' "$LENGTH" "$MEDIAN" >> "$OUTPUT_PATH"
        fi
        #if [ $TEST_CALLGRIND == true ] ; then
          #TODO: NOT IMPLEMENTED
          #$CPU_PATH="tmp_data/cpudata/$TESTING_PATH/$LENGTH.tsv"
          #$OUTPUT_PATH="tmp_data/cpudata/$TESTING_PATH/points.dat"

        #fi
        #if [ $TEST_PERF == true ] ; then
          #TODO: NOT IMPLEMENTED
        #fi
      done
    done
  done
done

################################################################################
# Combine points files for sorts to the results are suitable for use ###########
################################################################################
echo "Merging data from tests"
for CONTAINER in "${CONTAINERS[@]}" ; do
  for ORDERING in "${ORDERINGS[@]}" ; do
    ITR_PATH=()
    RUN_PATH=()
    TIME_PATH=()
    CPU_PATH=()
    MEM_PATH=()
    CACHE_PATH=()

    # Build up paths of files
    for SORT in "${SORTS[@]}" ; do
      TESTING_PATH="$CONTAINER/$ORDERING/$SORT"
      if [ "$TEST_ITERATOR_METRICS" == true ] ; then
        ITR_PATH+=("tmp_data/itrdata/$TESTING_PATH/points.dat")
      fi
      if [ "$TEST_CPU_AND_MEMORY" == true ] ; then
        CPU_PATH+=("tmp_data/rundata/$TESTING_PATH/cpu_points.dat")
        MEM_PATH+=("tmp_data/rundata/$TESTING_PATH/mem_points.dat")
      fi
      if [ "$TEST_TIME" == true ] ; then
        TIME_PATH+=("tmp_data/timedata/$TESTING_PATH/points.dat")
      fi
      if [ "$TEST_CALLGRIND" == true ] ; then
        CACHE_PATH+=("tmp_data/cpudata/$TESTING_PATH/points.dat")
      fi
      if [ "$TEST_PERF" == true ] ; then
        echo "NOT IMPLEMENTED"
      fi
    done

    # merge files of interest
    COMPILED_TESTS_PATH="$CONTAINER/$ORDERING"
    if [ "$TEST_ITERATOR_METRICS" == true ] ; then
      DATA_PATH="tmp_data/itrdata/$COMPILED_TESTS_PATH/compiled_points.dat"
      recursive_join "${ITR_PATH[@]}" > "$DATA_PATH"
      #$1=path to data
      #$2=title
      #$3=x axis label
      #$4=y axis label
      #$5=save file location
      TITLE="Iterator metrics on $ORDERING data in a $CONTAINER"
      XLABEL="Number of Elements"
      YLABEL="Number of Iterator Operations"
      SAVE_PATH="iterator_metrics_$ORDERING""_""$CONTAINER.eps"
      plot_wrapper "$DATA_PATH" "$TITLE" "$XLABEL" "$YLABEL" "$SAVE_PATH"
    fi
    if [ "$TEST_CPU_AND_MEMORY" == true ] ; then
      CPU_DATA="tmp_data/rundate/$COMPILED_TESTS_PATH/cpu_compiled_points.dat"
      MEM_DATA="tmp_data/rundata/$COMPILED_TESTS_PATH/mem_compiled_points.dat"
      recursive_join "${CPU_PATH[@]}" > "$CPU_DATA"
      recursive_join "${MEM_PATH[@]}" > "$MEM_DATA"

      TITLE="Average CPU usage on $ORDERING data in a $CONTAINER"
      XLABEL="Number of Elements"
      YLABEL="CPU utilization percentage"
      SAVE_PATH="cpu_utilization_$ORDERING""_""$CONTAINER.eps"
      plot_wrapper "$CPU_DATA" "$TITLE" "$XLABEL" "$YLABEL" "$SAVE_PATH"

      TITLE="Peak memory usage on $ORDERING data in a $CONTAINER"
      XLABEL="Number of Elements"
      YLABEL="Peak memory usage in KB"
      SAVE_PATH="peak_memory_$ORDERING""_""$CONTAINER.eps"
      plot_wrapper "$MEM_DATA" "$TITLE" "$XLABEL" "$YLABEL" "$SAVE_PATH"
    fi
    if [ "$TEST_TIME" == true ] ; then
      DATA_PATH="tmp_data/timedata/$COMPILED_TESTS_PATH/compiled_points.dat"
      recursive_join "${TIME_PATH[@]}" > "$DATA_PATH"
      TITLE="Sort time on $ORDERING data in a $CONTAINER"
      XLABEL="Number of Elements"
      YLABEL="Nanoseconds"
      SAVE_PATH="time_$ORDERING""_""$CONTAINER.eps"
      plot_wrapper "$DATA_PATH" "$TITLE" "$XLABEL" "$YLABEL" "$SAVE_PATH"
    fi
    if [ "$TEST_CALLGRIND" == true ] ; then
      DATA_PATH="tmp_data/cpudata/$COMPILED_TESTS_PATH/compiled_points.dat"
      recursive_join "${CACHE_PATH[@]}" > "$DATA_PATH"
      TITLE="Sort time on $ORDERING data in a $CONTAINER"
      XLABEL="Number of Elements"
      YLABEL="Total Cache Miss Rate"
      SAVE_PATH="time_$ORDERING""_""$CONTAINER.eps"
      plot_wrapper "$DATA_PATH" "$TITLE" "$XLABEL" "$YLABEL" "$SAVE_PATH"
    fi
    #if [ $TEST_PERF == true ] ; then
      #TODO: NOT IMPLEMENTED
    #fi
  done
done
echo "done"
