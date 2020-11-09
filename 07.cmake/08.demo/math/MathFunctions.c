/*************************************************************************
	> File Name: MathFunctions.c
	> Author: 
	> Mail: 
	> Created Time: 一 11/ 9 21:40:51 2020
 ************************************************************************/

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
