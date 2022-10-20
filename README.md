# liir

## Overview

**liir** is a simple [REPL (Read, eval, print and loop)](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop) system developed in C. LIIR stands for `Leia, interprete, imprima e repita` which is the brazilian portuguese translation for the english acronym `Read, eval, print and loop - REPL`. 
This project posseses the following characteristics:

- Developed with C11;
- This code was tested only on Linux (compiled with GNU GCC);
- This code uses a custom meomry allocator based on (slab allocation)[https://en.wikipedia.org/wiki/Slab_allocation];
- All the operations are performed with `double` precision floating point numbers (64 bits);

## Usage

Download this project and compile it by typing the command `make` in its folder. Next, just run the executable `./release/liir`:

```console
$ make
$ ./release/liir
```

To use it, just type the expression you want to evaluate and then press enter. Here is an example of usage:

![usage](./usage.png)