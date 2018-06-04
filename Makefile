HEADERS = data_preparation.hpp \
          iterator_metrics.hpp \
          parse_arguments.hpp \
          sort_abstracter.hpp

DEPENDENCIES = madlib/include

SOURCES = main.cpp

CPP_FLAGS = --std=c++17 -Imadlib/include/

CPP_RELEASE_FLAGS = $(CPP_FLAGS) -O3 -march=native

CPP_DEBUG_FLAGS = $(CPP_FLAGS) -DSCP_DEBUG -O0 -ggdb -Wall -Wextra -Wpedantic

EXEC = SCP

CXX = g++

all: $(DEPENDENCIES) $(SOURCES) $(HEADERS)
	$(CXX) $(CPP_RELEASE_FLAGS) $(SOURCES) -o $(EXEC)

debug: $(DEPENDENCIES) $(SOURCES) $(HEADERS)
	$(CXX) $(CPP_DEBUG_FLAGS) $(SOURCES) -o $(EXEC)

clean:
	rm -f a.out gmon.* *.o $(EXEC) $(OBJECTS)

$(DEPENDENCIES):
	git submodule init madlib
	git submodule update madlib
