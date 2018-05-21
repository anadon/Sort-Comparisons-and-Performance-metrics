HEADERS = data_preparation.hpp \
          iterator_metrics.hpp \
          parse_arguments.hpp \
          sort_abstracter.hpp

SOURCES = main.cpp

EXEC = sortperf

all:
	g++ -O3 -march=native --std=c++17 main.cpp -o $(EXEC)

debug:
	$(CXX) -ggdb -Wall -Wextra -Wpedantic -O0 --std=c++17 main.cpp -o $(EXEC)

clean:
	rm -f a.out gmon.* *.o $(EXEC)
