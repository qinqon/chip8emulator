CXX=g++
CXXFLAGS=-g -O0 -c -Wall -std=c++11 -Werror -pedantic -I/usr/local/include/
LDFLAGS=-L/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system
SOURCES=$(wildcard src/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=chip8emulator

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

.o:
	$(CXX) $(CXXFLAGS) $< -o $@		

clean:
	    rm src/*o $(EXECUTABLE)
