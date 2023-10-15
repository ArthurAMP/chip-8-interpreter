# chip-8-interpreter

A few resources that are amazing and helped me out were:

- Guide to making a CHIP-8 emulator by Tobias V. Langhoff
- Cowgod's Chip-8 Technical Reference v1.0
- How to write an emulator (CHIP-8 interpreter)

### Requirements
[Emscripten](https://emscripten.org/docs/getting_started/Tutorial.html)

### Compiling
```bash
emcc main.c -o main.html --preload-file <path to the game file>
```

I still have to make a better way to change the game than going to the code and switching the name of the file but I'm done with it for now. Might come back later to fix that.
