#include <stdio.h>

int foo() { printf("function call OK \n"); }

int myadd(int x, int y) {
    // printf("%d\n", x + y);
    return x + y; 
    }


int myadd3(int x, int y, int z){
    return x + y + z;
}

int call_myadd() {
    myadd (1, 2);
    return myadd3(1, 2, 3);
}
