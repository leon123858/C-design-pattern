//
// Created by Leon Lin on 2024/9/7.
//
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

typedef enum{
    OK,
    NotFound,
    NoPerm,
    Forbid
}ErrorCode;

ErrorCode apiCall(int parameter) {
    if (parameter > 2) {
        return NoPerm;
    }
    if (parameter > 1) {
        return  NotFound;
    }
    if (parameter == 0) {
        return OK;
    }
    return Forbid;
}

int main() {
    ErrorCode c = apiCall(0);
    if (c == NotFound) {
        printf("not found!\n");
        return 1;
    }
    if (c == NoPerm) {
        printf("no prem!\n");
        return 1;
    }
    if (c == Forbid) {
        printf("forbid!\n");
        return 1;
    }
    return 0;
}
