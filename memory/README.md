# Memory Allocation

## Stack First

一般情況下，優先使用 stack 來配置記憶體

有以下三種方法來配置記憶體

- Global or Static Variable: 在編譯階段就配置，存在於記憶體的最下面，直到程式釋放才消失
- Heap Memory: 存在於 Stack 下方，透過 memory allocation function 如 malloc 動態配置的記憶體，需要手動配置與釋放
- Stack Memory: 放置 function 本身 local 變數的地方，生命週期是函數的開始到結束

最佳記憶體優先配置順序應該是:

1. Stack: 運算快速，不需要主動釋放，不會浪費空間
2. Global: 運算快速，不需釋放，但沒用到的時間太長會很浪費空間
3. Heap: 運算慢速，需要思考記憶體安全釋放，可以動態配置節約空間，Heap 會有記憶體碎片化問題

However Stack 雖好，有其 limitation

- Stack is small, can not put too big data structure
- 無法預知 Stack 總大小，所以如果 Stack 用太大，可能不預期 Overflow
- stack buffer overflow 等資安議題
- data in stack 無法輕易使用 pointer 全域分享

## Eternal Memory

如果存在固定大小的大型資料結構，且該結構存在於整個 program 的生命週期，可以把該結構放置於 global 或 static variable

有以下原因

- 放在 stack 中，結構無法用 ptr 分享，且大型結構在 stack 中傳來傳去，很浪費空間
- 放在 heap 中，需要面對記憶體釋放等管理問題，如果該結構長期存在，不如浪費一點空間，讓其長期存在
- 優點是
  - 更快的速度
  - 更輕鬆地開發
  - 更穩定的記憶體管理 
    - 編譯期就很清楚各個資料結構的記憶體花費
    - 不會有記憶體碎片問題
  - 適用於有穩定需求的嵌入是開發，例如: SSD FW
- 缺點是
  - 如果放在全域變數卻長期沒在用，有點浪費空間
  - 在 init 時需要配置很多記憶體，花費更多時間
  - 另外還需要實作邏輯，在固定空間達成相同於動態記憶體的目標，例如: 當空間達到上限會自動 pop item 的 queue，其適合用來裝 log
  - 另外 global variable 只有一份，multi thread access 時要處理好 race condition

##  Lazy Cleanup

如果存在不固定大小的大型資料結構，且該結構存在於整個 program 的生命週期，並且該 program 可以接受定期重啟或系統崩潰，那可以選擇不斷 malloc heap 後永不釋放，直到 program 關閉，由作業系統主動收掉 heap 中的記憶體

優點很明顯: 簡單方便，不用思考記憶體釋放問題，甚至費時做記憶體釋放
缺點也很明顯: 系統穩定性與持久性受限，浪費空間

只要 program 的使用目的可以接受，那這樣做並無不可，只是需要想清楚再用

## Dedicated Ownership

如果存在

- 不固定大小的大型資料結構
- 該結構的使用不會貫穿完整 program 生命週期
- 並且該結構的每次配置大小都會不同。

此時可以考慮使用 `heap` 來動態配置記憶體，並且在遵循 Dedicated Ownership 的原則下管理記憶體釋放。

這個原則的操作如下

- heap 的配置者(callee)沒有 heap 的所有權
- heap 的使用者(caller)擁有 heap 的所有權
- 當 heap 被配置後，應該隨後由使用者用完後馬上釋放
- 即使 heap 不能直觀的在配置後馬上使用，使用後馬上釋放
  - 也要使用註解在配置後說明 heap 管理權被轉交到誰的手上
  - 並且會在何處被釋放
  - 注意，使用該 pattern 應該確保一筆 memory 只會擁有一位擁有者，避免釋放權責分配不清
- 可能需要文件，明確的說明每一筆記憶體在各個時段的擁有權在誰手上

```c
char* functionA() {
  char* memory = malloc(1024);
  return memory; // heap 的配置者沒有 heap 的所有
}

char* functionB() {
  // function B owns buf, and allocate it on the end of functionB
  char* buf = functionA();
  do_something_with(buf);
  // release buf
  free(buf);
}
```

## Allocation Wrapper

記憶體的配置與釋放是嚴謹與複雜的，為了簡化這個過程，我們可以使用 Wrapper Interface 來包裝相關操作，讓所以記憶體的配置與釋放都經由相同入口。缺點是缺乏彈性，可能無法處理複雜情境。

以下是用函數創建的範例

```c
void* checkedMalloc(size_t size) {
  void* pointer = malloc(size);
  assert(pointer);
  return pointer;
}

void checkedFree(void* pointer) {
  free(pointer);
}
```

以下是用 macro 創建的範例

```c
#define NEW(object, type)         \
do {                              \
  object = malloc(sizeof(type));  \
  assert(object);                 \
} while(0)

#define DELETE(object)    \
do {                      \
  free(object);           \
  object = NULL;          \
} while(0)
```

## Pointer Check

記憶體配置，大多數的錯誤是源於針對已經失效的指針(pointer)進行操作。

所以，一個合理的做法是顯式的(explicitly)在所有指針操作前檢查，指針釋放後顯式的無效化指針。

以下舉例:

```c
void functionA(char* ptr) {
  // 明確的檢查 ptr 是否合法，不合法就不做事
  if (ptr != NULL) {
    do_some_thing_with(ptr);
    free(ptr);
    // 釋放 ptr 後，要明確讓其他函數得知該 ptr 非法
    ptr = NULL;
  }
}
```

## Memory Pool

如果存在會頻繁配置與釋放的大型資料結構，期望:

- 提高配置與釋放速度
- 限制最大配置總量
- 避免記憶體碎片化

並且可以接受

- 在特定的最小單位下配置記憶體

那就可以使用 memory pool 作為記憶體操作解決方案

常見的設計是:

- 直接在 static 中宣告一個固定大小的大型空間
- 該大型空間切分成 N 個固定大小的 Unit
- function 可以透過 memoryPool.alloc 取得 Unit 的指標
- function 可以直接透過 Unit 的指標來操作記憶體
- function 用完記憶體後，可以透過 memoryPool.free 來交還 unit 的指標

此外 mem pool 還可以添加許多功能，例如筆者遇過最強的 mem pool 還具備以下功能

- 配置多個 unit 的連續記憶體，且邏輯指標連續，但物理空間不連續。
- 提供各式各樣 unit 使用者的 meta data 作為額外管理機制(讀取、寫入、搜索......)。
- 提供 event trigger 機制，thread 可以監聽某個 unit 遇到的事件。
- 配置與釋放提供順序性保證，以作為分布式鎖。(buff addr 4 的 function 先於 buffer addr 5 的 function)
- 各個使用目標可以設定 quota、upper limit、lower limit，所以相同目標的 buffer 使用數量可以被控管。(在申請 buffer unit 時要設定申請目的)
- ......許許多多

總之，memory pool 是實用且複雜的設計架構，能夠依據實際業務做獨特的優化

note: 以下為最簡單的實作

```c
#define ELEMENT_SIZE 255
#define MAX_ELEMENTS 10

typedef struct
{
  bool occupied;
  char memory[ELEMENT_SIZE];
}PoolElement;

static PoolElement memory_pool[MAX_ELEMENTS];

void* poolTake(size_t size)
{
  if(size <= ELEMENT_SIZE)
  {
    for(int i=0; i<MAX_ELEMENTS; i++)
    {
      if(memory_pool[i].occupied == false)
      {
        memory_pool[i].occupied = true;
        return &(memory_pool[i].memory);
      }
    }
  }
  return NULL;
}

void poolRelease(void* pointer)
{
  for(int i=0; i<MAX_ELEMENTS; i++)
  {
    if(&(memory_pool[i].memory) == pointer)
    {
      memory_pool[i].occupied = false;
      return;
    }
  }
}
```
