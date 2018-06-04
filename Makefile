HEADERS = data_preparation.hpp \
          iterator_metrics.hpp \
          parse_arguments.hpp \
          sort_abstracter.hpp

SOURCES = main.cpp

OBJECTS = main.o

CPP_FLAGS = --std=c++17 -I madlib/include

CPP_RELEASE_FLAGS = $(CPP_FLAGS) -O3 -march=native

CPP_DEBUG_FLAGS = $(CPP_FLAGS) -DSCP_DEBUG -O0 -g -Wall -Wextra -Wpedantic

EXEC = SCP

CXX = g++

all: $(OBJECTS) $(HEADERS)
	git submodule init madlib
	git submodule update madlib
	$(CXX) $(CPP_RELEASE_FLAGS) $(OBJECTS) -o $(EXEC)

debug: $(OBJECTS) $(HEADERS)
	git submodule init madlib
	git submodule update madlib
	$(CXX) $(CPP_DEBUG_FLAGS) $(OBJECTS) -o $(EXEC)

clean:
	rm -f a.out gmon.* *.o $(EXEC) $(OBJECTS)

$(OBJECTS):$(SOURCES)
	$(CXX) $(CPP_DEBUG_FLAGS) -c $< -o $@
