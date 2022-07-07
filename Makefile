OBJECTS = main.o summarizer.o
COMPILE_LINK = g++ -g -std=c++11 -Og
COMPILE = $(COMPILE_LINK) -c

pattern: $(OBJECTS)
	$(COMPILE_LINK) -o pattern $(OBJECTS)

main.o: main.cpp summarizer.hpp
	$(COMPILE) main.cpp

summarizer.o: summarizer.cpp summarizer.hpp
	$(COMPILE) summarizer.cpp

.PHONY: clean

clean: rm $(OBJECTS) pattern
