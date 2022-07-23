#TODO: add "install" phony target
EXECUTABLE = pattern
OBJECTS = main.o summarizer.o
#TODO: add -O3 when done debugging
INVOKE_COMMON = g++
INVOKE_COMPILE = $(INVOKE_COMMON) -g -std=c++11 -c

.PHONY: clean

$(EXECUTABLE): $(OBJECTS)
	$(INVOKE_COMMON) $(OBJECTS) -o $(EXECUTABLE) -lunistring

main.o: main.cpp summarizer.hpp
	$(INVOKE_COMPILE) main.cpp

summarizer.o: summarizer.cpp summarizer.hpp
	$(INVOKE_COMPILE) summarizer.cpp

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)