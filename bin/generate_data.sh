#!/bin/bash

#Some code pulled from:
#https://stackoverflow.com/questions/16959337/usr-bin-time-format-output-elapsed-time-in-milliseconds

if [ -d "$DIRECTORY" ]; then
  exit 0
fi

echo "preparing variables"

source functions.sh


#While this sets up the environemnt, I also used it as practice to show a few
#different ways to handle arrays and function stuff.

echo -n "Sorts: "
for SORT in "${SORTS[@]}" ; do
  echo -n "$SORT "
done
echo ""
#printf "Containers: %s\n" "${CONTAINERS[@]}"
echo -n "Containers: "
for CONTAINER in "${CONTAINERS[@]}" ; do
  echo -n "$CONTAINER "
done
echo ""
echo -n "Orderings: "
print_array "${ORDERINGS[@]}"
echo -n "Lengths: "
print_array "${LENGTHS[@]}"

if [ $TEST_ITERATOR_METRICS == true ] ; then
  echo "Testing using built in iterator metrics"
fi
if [ $TEST_TIME_AND_MEMORY == true ] ; then
  echo "Testing using time"
fi
if [ $TEST_CALLGRIND == true ] ; then
  echo "Testing using callgrind"
fi
if [ $TEST_PERF == true ] ; then
  echo "Testing using perf"
fi

echo "Each of the above possible combinations will run for $NUM_TRIALS trials"

echo "Starting tests"

################################################################################
#
# Now we conduct the actual tests, and obtain output.  We will later use this to
# compare results of each algorithm against each other in the same category.  In
# order to make this each, each test condition (container, ordering, length)
# will be at the higher levels so as to keep unrelated tests as far seperate
# from each other as possible.  Later, this makes it easier to work with because
# the combined tests can just be at the root directory of a particular
# combination of conditions.  Each of these later on are easier to annotate and
# safely comingle in a single results directory.
#
################################################################################

#rm -rf tmp_data

################################################################################
# Prepare the directories ######################################################
################################################################################

for SORT in ${SORTS[@]} ; do
  for CONTAINER in ${CONTAINERS[@]} ; do
    for ORDERING in ${ORDERINGS[@]} ; do
      TESTING_PATH="$CONTAINER/$ORDERING/$SORT"

      if [ $TEST_ITERATOR_METRICS = true ] ; then
        ITR_PATH="tmp_data/itrdata/$TESTING_PATH"
        mkdir -p "$ITR_PATH"
      fi
      if [ $TEST_CPU_AND_MEMORY = true ] ; then
        CPU_AND_MEM_PATH="tmp_data/rundata/$TESTING_PATH"
        mkdir -p "$CPU_AND_MEM_PATH"
      fi
      if [ "$TEST_TIME" = true ] ; then
        TIME_PATH="tmp_data/timedata/$TESTING_PATH"
        mkdir -p "$TIME_PATH"
      fi
      if [ "$TEST_CALLGRIND" = true ] ; then
        CACHE_PATH="tmp_data/cpudata/$TESTING_PATH"
        mkdir -p "$CACHE_PATH"
      fi
      if [ "$TEST_PERF" = true ] ; then
        PERF_PATH="tmp_data/cpudata/$TESTING_PATH"
        mkdir -p "$PERF_PATH"
      fi
    done
  done
done

################################################################################
# Conduct Tests ################################################################
################################################################################

for SORT in ${SORTS[@]} ; do
  for CONTAINER in ${CONTAINERS[@]} ; do
    for ORDERING in ${ORDERINGS[@]} ; do
      for LENGTH in ${LENGTHS[@]} ; do
        echo "Running test for sort '$SORT' on data type '$CONTAINER' with $ORDERING elements of length $LENGTH "
        echo -n "Trial: "
        TESTING_PATH="$CONTAINER/$ORDERING/$SORT"
        for REP in $(seq 1 $NUM_TRIALS) ; do
          echo -n "$REP."

          if [ $TEST_ITERATOR_METRICS = true ] ; then
            echo -n '.'
            ITR_PATH="tmp_data/itrdata/$TESTING_PATH"
            ./SCP --enable-interator-metrics=true --container="$CONTAINER" --length="$LENGTH" --sort-type="$SORT" --test="$ORDERING" >> "$ITR_PATH/$LENGTH.tsv"
          fi
          if [ $TEST_CPU_AND_MEMORY = true ] ; then
            #TODO: this one can actually be more appropriately split up more.
            echo -n '.'
            CPU_AND_MEM_PATH="tmp_data/rundata/$TESTING_PATH"
            /usr/bin/time -f '%P\t%M' -o "$CPU_AND_MEM_PATH/$LENGTH.tsv" -a ./SCP --enable-interator-metrics=false --container="$CONTAINER" --length="$LENGTH" --sort-type="$SORT" --test="$ORDERING"
          fi
          if [ $TEST_TIME = true ] ; then
            echo -n '.'
            TIME_PATH="tmp_data/timedata/$TESTING_PATH"
            ts=$(date +%s%N)
            ./SCP --enable-interator-metrics=false --container="$CONTAINER" --length="$LENGTH" --sort-type="$SORT" --test="$ORDERING"
            echo $(($(date +%s%N) - $ts)) >> "$TIME_PATH/$LENGTH.tsv"
          fi
          if [ $TEST_PERF = true ] ; then
            echo -n '.'
            #TODO: NOT IMPLEMENTED
          fi
        done
        #NOTE: CALLGRIND is entirely a simulation, and so multiple runs return
        #identical results.  However, this is an excellent opportubity to
        #compare against perf's results.
        if [ $TEST_CALLGRIND = true ] ; then
          echo -n '.'
          CPU_PATH="tmp_data/cpudata/$TESTING_PATH"
          valgrind --tool=cachegrind --cachegrind-out-file=/dev/null ./SCP --enable-interator-metrics=false --container="$CONTAINER" --length="$LENGTH" --sort-type="$SORT" --test="$ORDERING" 2>> "$CPU_PATH/$LENGTH.tsv"
        fi
        echo "done!"
      done
    done
  done
done

cp -R tmp_data/* data/*
rm -r tmp_data
