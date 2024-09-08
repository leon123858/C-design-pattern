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

