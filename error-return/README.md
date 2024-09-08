# Error Return 常見設計模式

## Return Status Codes

相比起 C 語言標準的`errno`機制有以下好處

1. 不需要額外讀取錯誤原因，減少冗余的程式碼
2. 不使用 global 存儲資料，平行化不會導致錯誤難以解讀

做法，明確宣告錯誤代碼，調用者查看回傳代碼。

但當有其他資訊想回傳，但回傳被錯誤代碼佔用時，可以使用以下方法。

1. Out-Parameters：用指標傳參，函數內直接更改參數。
2. Aggregate-Instance：回傳 `struct` 等型別，內含 Error Code 與其他資訊

```c
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
```

## Return Relevant Error

只回傳和調用者相關的錯誤

1. 降低開發者負擔(文件與code)
2. 降低調用者負擔(認知與QA)

可以選擇以下兩種方法來減少錯誤案例。

- 整合錯誤
- 直接中斷應用

```c
typedef enum{
    OK,
    Error
}ErrorCode;

ErrorCode apiCall(int parameter) {
    assert(parameter >= 0);
    if (parameter > 0) {
        return Error;
    }
    return OK;
}

int main() {
    ErrorCode c = apiCall(0);
    if (c == Error) {
        printf("error\n");
        return 1;
    }
    return 0;
}
```

## Special Return Value

把特殊值當作特定資訊，用以傳達錯誤發生，是簡單情況下的最優做法。

常見的回傳 `-1` `0` `1` `NULL` 都屬於此類。

```c
int apiCall(int parameter) {
    assert(parameter >= 0);
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
```

## Log Errors

確保事後可以輕易的找出錯誤，解決問題。

不需要直接影響到調用者。

不會漏掉有價值的資訊，使用者想看也可以看。

```c
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
```