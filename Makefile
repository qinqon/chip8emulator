all: chip8emulator

chip8emulator: src/main.o
	    g++ src/main.o -o chip8emulator.bin

main.o: src/main.cpp
	    g++ -c src/main.cpp

clean:
	    rm src/*o chip8emulator
