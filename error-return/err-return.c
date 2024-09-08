//
// Created by Leon Lin on 2024/9/7.
//
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

#define logAssert(x) \
if (!(x)) {            \
printf("file:%s, line: %i\n", __FILE__, __LINE__); \
assert(0);       \
}

int apiCall(int parameter) {
    logAssert(parameter >= 0);
    if (parameter > 0) {
        return 1;
    }
    return 0;
}

int main() {
    int ret = apiCall(0);
    if (ret) {
        printf("error\n");
        return 1;
    }
    return 0;
}
