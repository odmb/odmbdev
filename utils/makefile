CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Wshadow -O2
LD = g++
LDFLAGS = 
LDLIBS = 

default: parseddu.exe

parseddu.exe: parseddu.o
	$(LD) $(LDFLAGS) -o parseddu.exe parseddu.o $(LDLIBS)

parseddu.o: parseddu.cpp
	$(CXX) $(CXXFLAGS) -c parseddu.cpp

.PHONY: clean

clean:
	-rm -rf *.exe *.o *~ *#