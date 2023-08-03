# Konstrukcija Kompilatora

## Dead Argument Elimination

This pass deletes dead arguments from internal functions. Dead argument elimination removes arguments which are directly dead, as well as arguments only passed into function calls as dead arguments of other functions. This pass also deletes dead arguments in a similar way.

This pass is often useful as a cleanup pass to run after aggressive interprocedural passes, which add possibly-dead arguments.

### Dependencies
Ubuntu:

    $ sudo apt install llvm clang cmake 
Build:

    $ cd Konstrukcija-Kompilatora-Projekat
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Ako nece build treba postaviti `LLVM_DIR`:

    $ LLVM_DIR=/home/igor/Kompilatori/llvm-project/llvm

Run:

    $ clang -S -emit-llvm -Xclang -disable-O0-optnone something.c
    $ opt -enable-new-pm=0 -load build/DeadArgElim/libDAEPass.* something.ll
