# liir

## Overview

**liir** is a simple [REPL (Read, eval, print and loop)](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop) system developed in C. LIIR stands for `Leia, interprete, imprima e repita` which is the brazilian portuguese translation for the english acronym `Read, eval, print and loop - REPL`. 
Key features of this project include:

- Developed with C11;
- Cross-platform compatibility: Tested on Linux and Windows, and can be compiled with GNU GCC, clang, or MinGW (the compilation process was always run on a linux system);
- It is capable of saving and loading variables from a file;
- It can export the abstract syntax tree generated from expressions to a [graphviz](https://graphviz.org/) representation;
- All the operations are performed with double-precision floating-point numbers (64 bits);
- Run `./release/liir --help` to view available command-line options;

## Usage

Download this project and compile it by typing the command `make` in its folder. Next, just run the executable `./release/liir`:

```console
$ make
$ ./release/liir
```

To use it, just type the expression you want to evaluate and then press enter. Here is an example of usage:

![usage](./usage.png)