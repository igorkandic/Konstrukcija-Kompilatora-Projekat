 clang -S -emit-llvm -Xclang -disable-O0-optnone examples/test.c;
 opt -enable-new-pm=0 -load build/DeadArgElim/libDAEPass.so -deadae -S test.ll -o test_opt.ll;
