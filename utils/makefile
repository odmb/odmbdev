CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Wshadow -O2
LD = g++
LDFLAGS =
LDLIBS = 

default: parseddu.exe

parseddu.exe: parseddu.o data_packet.o
	$(LD) $(LDFLAGS) -o parseddu.exe parseddu.o data_packet.o $(LDLIBS)

parseddu.o: parseddu.hpp parseddu.cpp data_packet.hpp
	$(CXX) $(CXXFLAGS) -c parseddu.cpp

data_packet.o: data_packet.hpp data_packet.cpp
	$(CXX) $(CXXFLAGS) -c data_packet.cpp

.PHONY: clean

clean:
	-rm -rf *.exe *.o *~ *#