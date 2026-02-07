# ifdef hell

C 語言中會大量使用條件編譯來同時處理多樣的硬體平台，但這種做法會導致 code 的可讀性變差。

## avoid variants

在不需要使用條件編譯的情況下，應該避免使用 `ifdef`，因為任意新增的編譯變體，都會提高複雜度。

如果存在通用的操作介面，應該盡可能的使用。

例如:

- 要操作文件
- 不應該用 windows API / Linux API / Mac API 在各個平台直接操作
- 而應該使用通用的 C stander library 封裝調平台差異，例如 `fopen` / `fclose` / `fprintf` 等等。

## Isolated Primitive

隔絕原始邏輯 (隔絕平台相關邏輯):

當存在平台差異，不得不使用特定平台的 API 時，應該將這些操作封裝成個別函數。

並且該函數要足夠單純，不包含主要邏輯。

換句話說，主要邏輯應該和平台相關函數產生隔離。

以下舉例:

```c
void handle_something() {
#if (WIN32)
	// Windows specific code
#elif (LINUX)
	// Linux specific code
#endif
};

int main() {
	// main logic
	handle_something();
	return 0;
};
```

## Atomic Primitives

單一函數應該只封裝了單一類型變體，舉例來說，盡可能避免在同一個函數內用條件編譯同時處理 "OS" 和 "GPU" 兩種差異。

最好拆成兩個 Atomic 的函數，一個處理 "OS" 變體，一個處理 "GPU" 變體。

進一步舉例，以下提供錯誤示範

```c
void calculate_hash() {
#if (_ENABLE_GPU_)
#if (_UNIX_SYSTEM_)
	// linux gpu acc code
#elif (_WINDOWS_SYSTEM_)
	// windows gpu acc code
#endif
#else
#if (_UNIX_SYSTEM_)
	// linux general code
#elif (_WINDOWS_SYSTEM_)
	// windows general code
#endif
#endif
};

int main() {
	// main logic
	calculate_hash();
	return 0;
};
```

很明顯，巢狀 `ifdef` 可以說得上是非常難以理解。

改成這樣會更好:

```c
void gpu_calculate_hash() {
#if (_UNIX_SYSTEM_)
	// linux gpu acc code
#elif (_WINDOWS_SYSTEM_)
	// windows gpu acc code
#endif
}

void cpu_calculate_hash() {
#if (_UNIX_SYSTEM_)
	// linux general code
#elif (_WINDOWS_SYSTEM_)
	// windows general code
#endif
}


void calculate_hash() {
#if (_ENABLE_GPU_)
	gpu_calculate_hash();
#else
	cpu_calculate_hash();
#endif
};

int main() {
	// main logic
	calculate_hash();
	return 0;
};
```

## Abstraction Layer

當需要大量針對不同平台的函數時，

為了讓 callee 負責處理多種平台的細節，caller 負責處理核心邏輯。

可以透過 header file 定義抽象層 (Abstraction Layer)，

caller 直接調用抽象層，compile 階段自動根據平台匹配對應實作。

例如 IC design 中總是有一個 hal (Hardware Abstraction Layer) 資料夾，裡面放著針對不同版本 IP 的底層實作。

以下提供範例:

```c
// hal header
void* alloc_buf(uint size);

// implement
#if (_BMU_V0001_)
void* alloc_buf(uint size) {
	// allocate buffer by buffer management unit v0001
}
#endif
#if (_BMU_V0002_)
void* alloc_buf(uint size) {
	// allocate buffer by buffer management unit v0002
}
#endif
#if (_BMU_V0003_)
void* alloc_buf(uint size) {
	// allocate buffer by buffer management unit v0003
}
#endif

// caller
ptr = alloc_buf(3);
```

note: 實務上 hal 內的實作可以選擇不同平台不同檔案，或是直接在函數內切分。

## Split Implementation Variants

Abstraction Layer 下透過檔案來切分不同平台的實作就是 Split Implementation Variants

舉例來說可以這樣設計

- hal
  - fileOperation.h
    - fileOperation_windows.c
    - fileOperation_linux.c

更進一步，挑選實作檔案可以透過

1. `ifdef` 的條件編譯達成
2. `compiler` 設定目標檔案達成

此處的選擇，依團隊需求來定，個人傾向透過條件編譯來完成。
