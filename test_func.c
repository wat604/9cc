#include <stdio.h>

int foo() { printf("function call OK\n"); }

int myadd(int x, int y) {
    printf("%d\n", x + y);
    return x + y; 
    }