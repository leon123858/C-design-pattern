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

C 語言中包 module 的作法是把多個 header file + 對應實作，放在同一個資料夾中，且該資料夾提供唯一且統一的操作介面，例如 XXX_API.h

## Global Include Directory

## Self-Contained Component

## API Copy
