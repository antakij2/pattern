PREFIX = /usr/local/bin/

EXECUTABLE = pattern
OBJECTS = main.o summarizer.o

OPTIMIZATION_FLAG = -O3
DEBUG_FLAG =
CXXFLAGS = $(OPTIMIZATION_FLAG) $(DEBUG_FLAG)

.SUFFIXES: .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) -std=c++11 -c $<

.PHONY: all install uninstall clean

all: $(EXECUTABLE)

install:
	cp -i $(EXECUTABLE) $(PREFIX)

uninstall:
	rm $(PREFIX)$(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE) -l unistring

main.o: main.cpp summarizer.hpp
summarizer.o: summarizer.cpp summarizer.hpp