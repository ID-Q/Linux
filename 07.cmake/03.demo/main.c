/*************************************************************************
	> File Name: main.c
	> Author: 
	> Mail: 
	> Created Time: 一 11/ 9 19:58:40 2020
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "math/MathFunctions.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s base exponent \n", argv[0]);
        return 1;
    }
    double base = atof(argv[1]);
    int exponent = atoi(argv[2]);
    double result = power(base, exponent);
    printf("%g ^ %d is %g\n", base, exponent, result);
    return 0;
}
