#include <iostream>

int Sum(int a, int b) {
    for (int i = 0; i < 2; ++i) {
        if (a == INT32_MAX and b > 0)
            return INT32_MAX;
        std::swap(a, b);
    }

    for (int i = 0; i < 2; ++i) {
        if (a == INT32_MIN and b < 0)
            return INT32_MIN;
        std::swap(a, b);
    }

    return a + b;
}
