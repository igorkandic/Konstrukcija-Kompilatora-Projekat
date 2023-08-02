# Konstrukcija Kompilatora

## Dead Argument Elimination


Build:

    $ cd Konstrukcija-Kompilatora-Projekat
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    $ opt -enable-new-pm=0 -load build/DeadArgElim/libDAEPass.* something.c
