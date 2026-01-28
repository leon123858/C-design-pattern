# Data Lifetime and Ownership

## Stateless Software-Modules

當今天 function 調用之間

- 不需要共享資訊 (no share data between functions)
- 每一次 function 調用都是純粹(pure)的輸入計算後產生輸出(logically related functionality)
- 且無須提前準備任何計算所需的資源 (without preparation of any resources needed for computation)

可以有以下實作方式：

- 把近似的函數包在同一個 header file 中
- 每個函數的使用不需要 init 或事後 cleanup
- 每次調用必然結果獨立 (因為沒有內部狀態)
- 確保每個函數內部沒有狀態變化，例如使用到會變化的全域變數、會調用外部APIs、或會修改外部狀態等。
- 如果需要使用某些資源，例如 Buffer、外部服務、全域變數等，必須當作函數的輸入傳進去。
- 在數學上需要達到: 相同輸入必取得相同輸出，但實務上可以自己拿捏是否要做到這麼 pure

note: 此類 function 不需要思考 ownership，因為沒有使用任何資源

## Software-Module with Global State

當今天 function 調用之間

- 同一函數調用間，或多個函數調用間需要共享資訊 (share state)
- 每一次 function 調用，有可能需要 access 動態變化的外部資源
- 須提前準備資源，或事後清理資源

可以有以下實作方式：

- 把近似的函數包在同一個 header file 中
- 可以在 global 創建變數來 maintain shared state
- global 變數添加 static 可以做為 private state 使用 (限制同一個檔案內的函數 access)
- 多個函數同時 access 一個 global variable，可能需要注意 race condition 問題
- 每個函數的使用可能需要 init 或事後 cleanup 所需的資源

注意: 重點是這種做法的函數，由函數自身管理自己的狀態，而不是由呼叫者管理。換而言之，所有權在自己身上。

```c
// header
int addNext(int value);

// implement
static int sum = 0;
int addNext(int value) {
  sum += value;
  return sum;
}

// caller
addNext(5);   // sum = 5
addNext(10);  // sum = 15
```

## Caller-Owned Instance

然而，由 function 自身管理狀態的方法在 multi caller 環境有其上限。

唯一的狀態可能被同時操作，即使追加 lock 來處理 race condition，也可能導致複雜性的問題。

更複雜的是，multi caller 可能會有多組狀態的需求，例如:

- N 個 function 共用一個狀態
- 另外 M 個 function 使用另一個狀態

所以最後改成由 caller 管理狀態，

- 在 caller 呼叫函數時把狀態的管理權暫時移交到函數中
- 函數運作完後，再歸還狀態的管理權給 caller
- caller 也可以把狀態的管理權給另外一個 caller，只要最後一個 state owner 有記得清理資源即可

```c
// header
struct INSTANCE {
  int x;
  int y;
}

struct INSTANCE *create_instance(void);
void operation(struct INSTANCE *instance);
void destroy_instance(struct INSTANCE *instance);

// implement
struct INSTANCE *create_instance(void) {
  struct INSTANCE* ptr;
  ptr = malloc(sizeof(struct INSTANCE));
  return ptr;
}

void operation(struct INSTANCE *instance) {
  // do something
}

void destroy_instance(struct INSTANCE *instance) {
  free(instance);
}

// caller
struct INSTANCE *ptr = create_instance();
operation(ptr);
destroy_instance(ptr);
```

## Shared Instance

在 Caller-Owned Instance 中，

透過讓 caller 掌握狀態所有權來達到避免 multi-caller race condition 的目標。

因為，只有當下持有狀態所有權的函數，才可以操作該狀態。

但是有一類特殊情況，我們需要允許多個 caller 可以同時操作某個狀態，且狀態又要可以同時存在多組。

這種情況下，我們需要讓狀態有 shared ownership 的特性。

概念上是:

- 在 global 創建一個狀態的管理中心
- 狀態的管理中心內部可以動態或靜態的創建多組狀態，且每組狀態都有其識別 ID
- 每個 function 都可以透過識別 ID 取得該狀態的 shared ownership
- 亦即，多個 function 可以透過 ID 來操作同一組狀態
- 需要注意相應產生的 race condition
- 狀態的管理中心要 init / cleanup 各種資源

```c
// header
struct STATE {
  int caller_cnt;
  // some state
}

struct STATE *get_state_by_id(int id);
void operate_state(struct STATE* state);
void release_state(int id);

// implement
struct STATE states[MAX_INSTANCE];

struct STATE *get_state_by_id(int id) {
  if (id >= MAX_INSTANCE || id < 0) {
    return NULL;
  }
  states[id].caller_cnt++;
  return &states[id];
}
void operate_state(struct STATE* state) {
  // do something
}
void release_state(int id) {
  states[id].caller_cnt--;
}

// caller1
struct STATE *ptr = get_state_by_id(3);
operate_state(ptr);
release_state(3)
// caller2
struct STATE *ptr = get_state_by_id(3);
operate_state(ptr);
release_state(3)
```