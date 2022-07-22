INVOKE_COMMON = g++ -Og
INVOKE_COMPILE = $(INVOKE_COMMON) -g -std=c++11 -c
OBJECTS = main.o summarizer.o
EXECUTABLE = pattern

.PHONY: clean

$(EXECUTABLE): $(OBJECTS)
	$(INVOKE_COMMON) $(OBJECTS) -o $(EXECUTABLE) -lunistring

main.o: main.cpp summarizer.hpp
	$(INVOKE_COMPILE) main.cpp

summarizer.o: summarizer.cpp summarizer.hpp
	$(INVOKE_COMPILE) summarizer.cpp

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)