/*************************************************************************
	> File Name: main.c
	> Author: 
	> Mail: 
	> Created Time: 一 11/ 9 21:05:58 2020
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#ifdef USE_MYMATH
    #include <MathFunctions.h>
#else
    #include <math.h>
#endif


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s base exponent \n", argv[0]);
        return 1;
    }

    double base = atof(argv[1]);
    int exponent = atoi(argv[2]);

#ifdef USE_MYMATH
    printf("Now we use our own Math library. \n");
    double result = power(base, exponent);
#else
    printf("Now we use the standard library. \n");
    double result = pow(base, exponent);
#endif

    printf("%g ^ %d is %g\n", base, exponent, result);
    return 0;
}
