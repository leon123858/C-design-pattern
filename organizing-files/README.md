# organizing-files

## Include Guard

header file 即為某種功能的引用入口，功能可以在同一個專案內的多處被使用。

所以會發生針對相同 header 的多次引用，造成重複定義的問題。

此時使用 Include Guard 在預編譯階段確保只編譯了一份真正的 header file。

法一，用以下方法包住 header

```c
#ifndef XXX_H
#define XXX_H
// ...
#endif
```

法二，直接制止多次編譯

```c
#pragma once

// ...
```

## Software-Module Directories

太多 header file 也會導致 caller 的認知負擔，因此把多個耦合的 header 包成一個 module 是個好方法。

C 語言中包 module 的作法是把多個 header file + 對應實作，放在同一個資料夾中，且該資料夾提供唯一且統一的操作介面，例如 XXX_API.h 供資料夾外部的程式碼引用。

例如以下結構:

```
--- main.c
     |------ FTL資料夾
				|----------ftl_api.h
							  |------------write.h
							  				|------------write.c
							  |------------read.h
							  				|------------read.c
							  |------------WL.h
							  				|------------WL.c
							  |------------GC.h
							  				|------------GC.c
							  |------------err_hdl.h
							  				|------------err_hdl.c
							  |------------XXXXX.h
							  				|------------XXXXX.c
	 |------ Init 資料夾
	 			|----------init_api.h
	 						  |------------init_XXX.h
											|------------init_XXX.c
```

## Global Include Directory

在 software module 中引用另一個 module 的 header file 時，往往需要透過相對路徑來索引。

如果是多層嵌套的目錄結構，這種方式會造成 include path 非常冗長且容易出錯。

又或是某些常用的 software module，會希望在任何地方都能夠直接引用，而不需要知道正確的相對路徑。

此時可以在 compiler 中設定**全域引入路徑**（global include directory），也就是 compiler 會指向某個資料夾，例如: "include"、"common"、"lib"、......等

並且在這個資料夾內放入作為 API 的 header file，就可以直接 include 這個資料夾下的 header file 了。

例如，假設 include 被設定成 global include directory:

- include
  - moduleA_API.h
  - moduleB_API.h
- moduleA
- moduleB
- main.c
  - `#include "moduleA_API.h"`
  - `#include "moduleB_API.h"`

## Self-Contained Component

當 Software-Module 太多或 global include directory 中的 header 太多時，可以採用此方法

增加一個 component 層次，component 包含多個 modules，並且在這個 component 的資料夾內放入 global include directory 作為 API。

舉例來多就像:

- componentA
  - include
    - moduleA_API.h
    - moduleB_API.h
  - moduleA
  - moduleB
- componentB
  - include
    - moduleC_API.h
    - moduleD_API.h
  - moduleC
  - moduleD
- componentC
  - include
    - moduleX_API.h
    - moduleY_API.h
  - moduleX
  - moduleY
- main.c
  - `#include "moduleA_API.h"`
  - `#include "moduleX_API.h"`
  - ...

note: 編譯器作了以下配置，來達成 global include directory

- `/componentA/include`
- `/componentB/include`
- `/componentC/include`

## API Copy

本質上是 component 之間或 module 之間的解耦問題。

當希望公開某一個 component / module 時，最簡單的做法就是直接 copy API 後想辦法給別人調用。

此處的 copy API 描述的概念就是把具備 API 功能的 header 公開出來給別人用，該 header 即被視為解偶必需的介面，最終達成依賴反轉。
