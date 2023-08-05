#include<stdio.h>

int zaza(int x, int y){
    int z = x + y;
    return z*z;
}

//TODO pogledati kako u njihovom kodu menjaju call posto tu nesto ne valja
// fale podaci (addrspace(0))
int main(){
    zaza(5, 6);
    return 0;
}