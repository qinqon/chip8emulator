all: chip8emulator

chip8emulator: src/main.o src/Chip8.o
	    g++ src/main.o src/Chip8.o -o chip8emulator.bin

Chip8.o: src/Chip8.cpp
	    g++ -c src/Chip8.cpp

main.o: src/main.cpp
	    g++ -c src/main.cpp

clean:
	    rm src/*o chip8emulator.bin
