# Konstrukcija Kompilatora

## Dead Argument Elimination


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

    $ opt -enable-new-pm=0 -load build/DeadArgElim/libDAEPass.* something.c
