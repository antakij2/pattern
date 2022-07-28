EXECUTABLE = pattern
OBJECTS = main.o summarizer.o

OPTIMIZATION_FLAG = -O3
DEBUG_FLAG =

debug: OPTIMIZATION_FLAG =
debug: DEBUG_FLAG = -g

INVOKE_COMMON = g++ $(OPTIMIZATION_FLAG)
INVOKE_COMPILE = $(INVOKE_COMMON) $(DEBUG_FLAG) -std=c++11 -c
INVOKE_LINK = $(INVOKE_COMMON) $(OBJECTS) -o $(EXECUTABLE) -l unistring

.PHONY: all debug install clean

all: $(OBJECTS)
	$(INVOKE_LINK)

debug: $(OBJECTS)
	$(INVOKE_LINK)

install:
	cp -i $(EXECUTABLE) /usr/local/bin

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)



main.o: main.cpp summarizer.hpp
	$(INVOKE_COMPILE) main.cpp

summarizer.o: summarizer.cpp summarizer.hpp
	$(INVOKE_COMPILE) summarizer.cpp