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



## Allocation Wrapper

## Pointer Check

## Memory Pool