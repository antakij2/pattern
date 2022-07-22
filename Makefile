INVOKE_COMMON = g++ -g -std=c++11 -Og
INVOKE_COMPILE = $(INVOKE_COMMON) -c
OBJECTS = main.o summarizer.o

.PHONY: clean

pattern: $(OBJECTS)
	$(INVOKE_COMMON) -lunistring -o pattern $(OBJECTS)

main.o: main.cpp summarizer.hpp
	$(INVOKE_COMPILE) main.cpp

summarizer.o: summarizer.cpp summarizer.hpp
	$(INVOKE_COMPILE) summarizer.cpp

clean:
	rm -f $(OBJECTS) pattern