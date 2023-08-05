#include <stdio.h>

int svi_mrtvi(int x, int y){ // x, y dead
    return 5;
}

int jedan_crko(int x, int y, int z){ // y dead
    return x + z;    
}

void dva_mrtvaka(int x, int y, int z) { // y, z dead
    printf("%d\n",x);
}

void g(int x, int y){ // x dead
    dva_mrtvaka(y,x,y);
}

void f(int x, int y, int z){ // z dead
    int tmp = x;
    g(z,y);
}

int fakt(int n, int nebitan){ // not covered
    if(n == 0)
        return 1;
    return n * fakt(n-1,nebitan);
}

int funkcija(int a, int b) { // b dead
    return a*a;
}
int iplusplus(int i){ // i dead
    return 727;
}

int main() {
    int x = 10, y, z;
    dva_mrtvaka(x,y,z);
    x = jedan_crko(x,y,z);
    int drugi_x = jedan_crko(7,-1,9);
    y = svi_mrtvi(x,y);
    f(x,y,z);
    x = fakt(5,y);
    y = funkcija(x,jedan_crko(5,x,2));
    iplusplus(x++);
}
