int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    if (b == 0) return 0;
    return a / b;
}

int main() {
    int x = 10;
    int y = 20;
    int sum = add(x, y);
    printf("Sum: %d\n", sum);
    return 0;
}
