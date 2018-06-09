#!/bin/bash


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

if [ $TEST_ITERATOR_METRICS = true ] ; then
  echo "Testing using built in iterator metrics"
fi
if [ $TEST_TIME_AND_MEMORY = true ] ; then
  echo "Testing using time"
fi
if [ $TEST_CALLGRIND = true ] ; then
  echo "Testing using callgrind"
fi
if [ $TEST_PERF = true ] ; then
  echo "Testing using perf"
fi

echo "Each of the above possible combinations will run for $NUM_TRIALS trials"

echo "Starting tests"

rm -rf tmp_data

for SORT in ${SORTS[@]} ; do
  for CONTAINER in ${CONTAINERS[@]} ; do
    for ORDERING in ${ORDERINGS[@]} ; do
      for LENGTH in ${LENGTHS[@]} ; do
        echo "Running test for sort '$SORT' on data type '$CONTAINER' with $ORDERING elements of length $LENGTH "
        echo -n "Trial: "
        for REP in $(seq 1 $NUM_TRIALS) ; do
          echo -n "$REP."

          INSTANCE_ARGS="--container=$CONTAINER --length=$LENGTH --sort-type=$SORT --test=$ORDERING"

          if [ $TEST_ITERATOR_METRICS = true ] ; then
            echo -n '.'
            ITR_PATH="tmp_data/itrdata/$SORT/$CONTAINER/$ORDERING/"
            mkdir -p "$ITR_PATH"
            ./SCP --enable-interator-metrics=true --container="$CONTAINER" --length="$LENGTH" --sort-type="$SORT" --test="$ORDERING" >> "$ITR_PATH/$LENGTH.tsv"
          fi
          if [ $TEST_TIME_AND_MEMORY = true ] ; then
            echo -n '.'
            RUN_PATH="tmp_data/rundata/$SORT/$CONTAINER/$ORDERING/"
            mkdir -p "$RUN_PATH"
            /usr/bin/time -v -o "$RUN_PATH/$LENGTH.tsv" -a ./SCP --enable-interator-metrics=false --container="$CONTAINER" --length="$LENGTH" --sort-type="$SORT" --test="$ORDERING"
          fi
          if [ $TEST_CALLGRIND = true ] ; then
            echo -n '.'
            CPU_PATH="tmp_data/cpudata/$SORT/$CONTAINER/$ORDERING/"
            mkdir -p "$CPU_PATH"
            valgrind --tool=cachegrind --cachegrind-out-file=/dev/null ./SCP --enable-interator-metrics=false --container="$CONTAINER" --length="$LENGTH" --sort-type="$SORT" --test="$ORDERING" 2> "$CPU_PATH/$LENGTH.tsv"
          fi
          if [ $TEST_PERF = true ] ; then
            echo -n '.'
            echo "NOT IMPLEMENTED"
          fi
        done
        echo "done!"
      done
    done
  done
done

mv tmp_data/ data/
