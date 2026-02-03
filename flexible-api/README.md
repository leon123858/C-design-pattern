# Flexible APIs

目標是達成 SOLID

- S: Single Responsibility Principle (SRP)，單一職責
- O: Open/Closed Principle (OCP)，可以在不太需要修改的條件下自由使用
- L: LisKow Substitution Principle (LSP)，相同介面的實作可以變換
- I: Interface Segregation Principle (ISP)，介面應該隔離實作，介面是因應調用者需求設計的
- D: Dependency Inversion Principle (DIP)，依賴反轉，caller 依賴介面，實作也依賴介面

## Header Files

利用 header file 來放供人調用的介面，用 C file 來放實作。

當使用者僅會調用 header file 中定義的 function 時，即可達到隱藏實作的目標。

因此整個步驟如下:

1. 介面設計，使用 header file 定義介面。 (\*.h)
2. 實作介面，使用 C file 寫出 function 的實作。 (\*.c)
3. 如果想封裝多個介面，可以在一個 header file 中引用多個 header files，作為公開入口。
4. 為了避免重複引用，可以使用 Include Guards 來保護。

## Handle

針對相同物件，底層希望提供實作，但是不希望公開物件內部資訊。

概念上是創建一個 Handle，其本質是一個指標，但因為指標指向的 struct 沒有公開，所以調用者無法直接讀取內部資訊。

programer 可以用介面定義的函數來操作 handle。

介面的實作內部包含了指標的結構，因此可以完成目標操作。

```c
// header
typedef struct SORT_STRUCT *SORT_HANDLE;
SORT_HANDLE initSort(int* arr, int size);
void sortDo(SORT_HANDLE context)
void sortFree(SORT_HANDLE context)

// implement
struct SORT_STRUCT {
    int* arr;
    int len;
};

SORT_HANDLE initSort(int* arr, int size) {
	struct SORT_STRUCT* ctx = malloc(sizeof(struct SORT_STRUCT));
	ctx->arr = arr;
	ctx->len = size;
	return ctx;
};

void sortDo(SORT_HANDLE context) {
	// sort logic...
};

void sortFree(SORT_HANDLE context) {
	// free logic...
};

// caller
SORT_HANDLE handle = initSort(intArray, intSize);
sortDo(handle);
sortFree(handle);
```

## Dynamic Interface

希望可以在相同控制流程中引用多種不同底層實作。

可以利用 function pointer 的方法定義函數介面，並且在控制流程中選用不同但符合介面的實作函數。

其中，實作函數的委派可以在 compile time 或 runtime 進行，按照需求選擇。

```c
// API
typedef bool (*compare_func_ptr)(int x, int y);
void sort(compare_func_ptr fp, int* array, size_t size);
bool ascending(int x, int y);
bool descending(int x, int y);

// implement
void sort(compare_func_ptr fp, int* array, size_t size) {
    if (array == NULL || size < 2) return;
    for (size_t i = 0; i < size - 1; i++) {
        for (size_t j = 0; j < size - i - 1; j++) {
            if (fp(array[j], array[j + 1])) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
};
bool ascending(int x, int y) {
    return x > y;
};
bool descending(int x, int y) {
    return x < y;
};

// caller
int nums[] = {42, 7, 13, 1, 99};
size_t n = sizeof(nums) / sizeof(nums[0]);
sort(ascending, nums, n);
for(size_t i=0; i<n; i++) printf("%d ", nums[i]);
sort(descending, nums, n);
for(size_t i=0; i<n; i++) printf("%d ", nums[i]);
```

note:

關於動態調用不同實作函數有一些細節，類似策略模式

由 caller 依據 function pointer 自行實作不同策略後注入到 struct 中。

常見的作法:

- 創建一個含多個函數指標的 struct，
- runtime 按需建構該 struct，裡面放入 caller 依照自己需求實作的函數的指標。
- 調用過程中不使用 global 函數，而是透過 struct 內的函數指標來調用函數。

```c
// header
typedef bool (*compare_func_ptr)(int x, int y);
typedef struct {
    compare_func_ptr compare;
    const char* mode_name;
} Sorter;

void sort(Sorter* sorter, int* array, size_t size);
Sorter* create_custom_sorter(compare_func_ptr custom_logic, const char* name);

// implement
void sort(Sorter* sorter, int* array, size_t size) {
    if (!sorter || !sorter->compare || !array || size < 2) return;
    for (size_t i = 1; i < size; i++) {
        int key = array[i];
        int j = (int)i - 1;
        while (j >= 0 && sorter->compare(array[j], key)) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
};
Sorter* create_custom_sorter(compare_func_ptr custom_logic, const char* name) {
    if (!custom_logic) return NULL;

    Sorter* s = (Sorter*)malloc(sizeof(Sorter));
    if (!s) return NULL;

    s->compare = custom_logic;
    s->mode_name = name ? name : "Undefined Mode";
    return s;
};

// caller
bool my_ascending(int x, int y) { return x > y; }
bool my_descending(int x, int y) { return x < y; }
bool my_absolute_cmp(int x, int y) { return abs(x) > abs(y); }
void print_array(int* arr, size_t size) { ... }
int FUNC() {
    int data[] = {25, -10, 55, -70, 5};
    size_t n = sizeof(data) / sizeof(data[0]);
    // 調用者主動注入「升冪」邏輯
    Sorter* asc_sorter = create_custom_sorter(my_ascending, "User Ascending");
    sort(asc_sorter, data, n);
    printf("%s: ", asc_sorter->mode_name);
    print_array(data, n);
    // 調用者主動注入「絕對值」邏輯
    Sorter* abs_sorter = create_custom_sorter(my_absolute_cmp, "Absolute Value Mode");
    sort(abs_sorter, data, n);
    printf("%s: ", abs_sorter->mode_name);
    print_array(data, n);
	...
}
```

## Function Control

和 Dynamic Interface 最大的差異是，

- 不是由 caller 提供 function pointer 的實作
- 而是由 function module 內部提供 function pointer 的實作

原因可以有很多，例如: 內部耦合太高、實作太難、簡化調用、......等等。

其本質類似工廠模式，範例如下

```c
// header
typedef bool (*compare_func_ptr)(int x, int y);

typedef struct {
    compare_func_ptr compare;
    const char* mode_name;
} Sorter;

typedef enum {
	// 透過 enum 讓工廠在一開始就使用內部實作達成目標 function pointer 的嵌入
    MODE_ASCENDING,
    MODE_DESCENDING
} SortMode;

Sorter* create_sorter(SortMode mode);
void sort(Sorter* sorter, int* array, size_t size);

// implement
static bool ascending_logic(int x, int y) {
    return x > y; // 左大於右則交換，達成升冪
};
static bool descending_logic(int x, int y) {
    return x < y; // 左小於右則交換，達成降冪
};

Sorter* create_sorter(SortMode mode) {
    Sorter* s = (Sorter*)malloc(sizeof(Sorter));
    if (!s) return NULL;

    if (mode == MODE_ASCENDING) {
        s->compare = ascending_logic;
        s->mode_name = "升冪 (Ascending)";
    } else if (mode == MODE_DESCENDING) {
        s->compare = descending_logic;
        s->mode_name = "降冪 (Descending)";
    }
    return s;
};

void sort(Sorter* sorter, int* array, size_t size) {
    if (!sorter || !sorter->compare || !array || size < 2) return;
    for (size_t i = 1; i < size; i++) {
        int key = array[i];
        int j = (int)i - 1;
        // 這裡的 sorter->compare 會根據工廠產出的物件不同而有不同行為
        while (j >= 0 && sorter->compare(array[j], key)) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
};

// caller
int data[] = {25, 10, 55, 30, 5};
size_t n = sizeof(data) / sizeof(data[0]);
Sorter* asc_sorter = create_sorter(MODE_ASCENDING);
sort(asc_sorter, data, n);
print_array(data, n);
Sorter* desc_sorter = create_sorter(MODE_DESCENDING);
sort(desc_sorter, data, n);
print_array(data, n);
```

note:

除了用 enum 實作工廠，也可以用 define 完成實作。

關鍵是由 callee 完成不同的實作方案，caller 透過傳入不同參數創建包含不同 function pointer 的 handle。

最後該 handle 可以在相同流程中，基於不同底層實作來處理。
