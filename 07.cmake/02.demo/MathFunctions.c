/*************************************************************************
	> File Name: MathFunctions.c
	> Author: 
	> Mail: 
	> Created Time: 一 11/ 9 19:49:21 2020
 ************************************************************************/

#include <stdio.h>

double power(double base, int exponent) {
    int result = base;
    int i;

    if (exponent == 0) {
        return 1;
    }

    for (i = 1; i < exponent; ++i) {
        result = result * base;
    }

    return result;
}

