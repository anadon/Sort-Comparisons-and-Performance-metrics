#!/usr/bin/make

SOURCES = main.cpp

all: $(EXEC)
	@echo "Building: $(EXEC)"

$(EXEC): $(SOURCES)
	$(CXX) $(CPP_FLAGS) $(SOURCES) -o $(EXEC)

clean:
	rm -f a.out gmon.* *.o $(EXEC) $(OBJECTS)
