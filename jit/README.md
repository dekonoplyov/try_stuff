# try jit
https://solarianprogrammer.com/2018/01/10/writing-minimal-x86-64-jit-compiler-cpp/

Compile
```
clang++ -std=c++17 -Wall -pedantic -g hello.cpp -o hello
```

Assemble and disassemble
```
as chunk.s -o chunk.o
objdump -M intel -D chunk.o
```
