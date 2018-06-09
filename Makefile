#!/usr/bin/make

HEADERS = include/data_preparation.hpp \
          include/iterator_metrics.hpp \
          include/parse_arguments.hpp \
          include/sort_abstracter.hpp \
          include/introsort.hpp

DEPENDENCIES = madlib/include

SOURCES = src/main.cpp

BASE_PATH = $(shell pwd)

CPP_COMMON_FLAGS = --std=c++17 -I$(BASE_PATH)/include/ \
                   -I$(BASE_PATH)/madlib/include/

CPP_RELEASE_FLAGS = $(CPP_COMMON_FLAGS) -O3 -march=native

CPP_DEBUG_FLAGS = $(CPP_COMMON_FLAGS) -DSCP_DEBUG -O0 -ggdb -Wall -Wextra -Wpedantic

EXEC = $(BASE_PATH)/bin/SCP

CXX = g++

export

all: release

release: CPP_FLAGS=$(CPP_RELEASE_FLAGS)
release: $(EXEC)

debug: CPP_FLAGS=$(CPP_DEBUG_FLAGS)
debug: $(EXEC)

$(EXEC): $(DEPENDENCIES) $(SOURCES) $(HEADERS)
	cd src ; make

clean:
	rm -f a.out gmon.* *.o $(EXEC)
	cd src ; make clean

$(DEPENDENCIES):
	git submodule init madlib
	git submodule update madlib
