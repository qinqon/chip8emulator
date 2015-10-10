# chip8emulator
C++ Chip8 emulator implementation with a Pimpl and a "funcionalish" approach.

The purpose is to have a clear SDL to define the opcodes:

```cpp
,opcodes{{
      nop, 
      jumpTo(nnn), 
      callTo(nnn),
      skipIfEquals(Vx, kk),
      skipIfNotEquals(Vx, kk),
      skipIfEquals(Vx, Vy),
}}
```

Motivation taken from:
http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

