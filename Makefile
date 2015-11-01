CC=g++
CFLAGS=-g -O0 -c -Wall -std=c++11 -Werror -pedantic -I/usr/local/include/
LDFLAGS=-L/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system
SOURCES=$(wildcard src/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=chip8emulator

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@		

clean:
	    rm src/*o $(EXECUTABLE)
