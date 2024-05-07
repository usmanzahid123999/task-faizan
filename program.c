#include <stdio.h>

int main() {
    int num1 = 10;
    int num2 = 20;
    int sum = num1 + num2;

    printf("Address of num1: %p\n", (void *)&num1);
    printf("Address of num2: %p\n", (void *)&num2);
    printf("Address of sum: %p\n", (void *)&sum);

    printf("The sum of %d and %d is %d\n", num1, num2, sum);

    return 0;
}

