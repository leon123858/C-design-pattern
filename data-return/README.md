# data-return

## Return Value

function 把值複製後回傳。

當目前有單一 function 計算多種結果，我們需要 function split 成一個函數回傳一種結果時。

最大的好處是 copy 後的結果，如何操作不影響本體，且函數間沒有互相耦合。

## Out-Parameters

function 把多個值複製後回傳。

當目前有單一 function 計算多種結果，且因為多種結果彼此耦合，所以我們希望直接把所有結果當一組結果複製後回傳。

常用在回傳 snapshot 等場景。

```c
void getData(int *x, int *y) {
  *x = 15;
  *y = 30;
}

int x,y;
getData(&x, &y);
```

## Aggregate Instance

function 把多個值包成 struct 後複製回傳。

當目前有單一 function 計算多種結果，且因為多種結果彼此耦合，所以我們希望直接把所有結果當一組結果複製後回傳。

但注意，使用該方法，struct 建議不要太大，避免複製太多在 stack 上。

```c
struct DataType {
  int x;
  int y;
}

struct DataType getData() {
  struct DataType inst;
  return inst;
}
```

## Immutable Instance

複製後回傳的方法，雖然都可以避免 data caller 修改到原始數據。

但是複製大型結構過於浪費空間，且無法確保資料處於最新狀態。

因此需要一種方法: 可以在不複製資料的前提下，僅依靠提供指標達成回傳大量不可修改資料之目標。

簡單來說就是在讀資料時，利用 const 確保調用者不會修改到原始數據。

```c
// data type header
struct DataType {
  int x;
  int y;
}

// init.c
static struct DataType inst = {0, 0};

// api
const struct DataType* getData() {
  return &inst;
}

// caller
const struct DataType* my_data;
my_data = getData();
```

## Caller-Owned Buffer

如果今天需要回傳的內容是難以確定的型別，又或是超大型的資料，且資料讀出來後需要操作。

可以由 caller 先配置 buffer 後，交給 callee 把資料填充進 buffer

注意: 由 caller 主動配置 buffer，所以 caller 必須清楚 buffer 需要配置的大小。

```c
// API
struct Buffer {
  char data[256];
}
void getData(struct Buffer* buffer) {
  memcpy(buffer->data, some_data, 256);
}

// caller
struct Buffer buffer
getData(&buffer);
```

## Callee Allocates

可以由 callee 配置 buffer/struct[] 後回傳指標，交給 caller 操作與釋放

注意: 由 callee 主動配置 buffer，所以一般發生在 caller 不知道需要配置多大的大小。

```c
// callee
void getData(char** buffer, int* size) {
  *size = 30;
  *buffer = malloc(*size);
}

// caller
char* buffer;
int size;
getData(&buffer, &size);
```


