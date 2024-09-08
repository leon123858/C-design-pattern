# Error Handle 常見設計模式

## Function Split

把具有多個職責的大函數，拆分成具有多個單一職責小函數的大函數。

分解原函數取出有用的部分，為這些部分建立新函數。

可以更容易地完成資源與錯誤的處理。

origin

```c
int function_split() {
    char* buffer;
    buffer = malloc(1024);
    if (buffer != NULL) {
        char* tmp = "Hello World!!";
        for (int i=0;i<12;i++) {
            buffer[i] = tmp[i];
        }
        printf("buffer: %s", buffer);
        free(buffer);
        return 0;
    }
    return 1;
}
```

new

```c
char* stringOperation(char* s) {
    // double len s
    char* tmp = "Hello World!!";
    for (int i=0;i<12;i++) {
        s[i] = tmp[i];
    }
    return s;
}

int function_split() {
    char* buffer;
    buffer = malloc(1024);
    if (buffer == NULL) {
        printf("malloc error\n");
        return 1;
    }
    buffer = stringOperation(buffer);
    printf("buffer: %s", buffer);
    free(buffer);
    return 0;
}
```

## Guard Clause

俗稱: 提早回頭

概念: 符合特定條件才會繼續執行函數

更易讀的錯誤處理，條件和邏輯不會混在一起

origin

```c
int stringOperation(char* s) {
    if (s != NULL) {
        char* tmp = "Hello World!!";
        for (int i=0;i<12;i++) {
            s[i] = tmp[i];
        }
        printf("buffer: %s", s);
        return 0;
    }
    printf("malloc error\n");
    return 1;
}
```

new

```c
int stringOperation(char* s) {
    if (s == NULL) {
        printf("malloc error\n");
        return 1;
    }
    char* tmp = "Hello World!!";
    for (int i=0;i<12;i++) {
        s[i] = tmp[i];
    }
    printf("buffer: %s", s);
    return 0;
}
```

## Samurai Principle

武士原則，當軟體無高可用的需求，為了避免後續使用者忽略檢查，帶來更嚴重的錯誤，在特定 case 直接使用 assert 終止程式碼。

sample

```c
int stringOperation(char* s) {
    assert(s != NULL);
    char* tmp = "Hello World!!";
    for (int i=0;i<12;i++) {
        s[i] = tmp[i];
    }
    printf("buffer: %s", s);
    return 0;
}
```

## Goto Error Handle

函數內有多組資源要清除，就在最後一段加入清除資源的邏輯，每當有錯誤發生，就goto到最後一段。

```c
int gotoErr() {
    char* tmp = malloc(1024);
    if (tmp == NULL) {
        goto err2;
    }
    char* tmp2 = malloc(1024);
    if (tmp2 == NULL) {
        goto err1;
    }
    // core login
    free(tmp2);
    err1:
    free(tmp);
    err2:
    return 0;
}
```

## Cleanup Record

Goto Error Handle 的替代品，在最後一段檢查資源狀態並做對處理，更易懂，但不一定好。

只有資源全部建立成功才會執行核心邏輯，離開時，有什麼資源清掉什麼資源。

```c
int cleanupRecord() {
    char* tmp = NULL;
    char* tmp2 = NULL;
    if ((tmp = malloc(1024)) && (tmp2 = malloc(1024))) {
        // core login
    }
    if (tmp != NULL) {
        free(tmp);
    }
    if (tmp2 != NULL) {
        free(tmp2);
    }
    return 0;
}
```

## Object-Based Error Handle

把多個資源當作物件包裝再一起，封裝物件內多個資源的繁瑣錯誤處理。

可以處理最複雜的 case.

sample

```c
struct obj {
    char* tmp;
    char* tmp2;
    FILE* file_ptr;
};

struct obj* newObj(char* file_name) {
    struct obj* o = malloc(sizeof(struct obj));
    if (o == NULL) {
        return o;
    }
    o->file_ptr = fopen(file_name, "r");
    if (o->file_ptr == NULL) {
        free(o);
        return 0;
    }
    return o;
}

void mainLogic(struct obj* o) {
    // do something
}

int cleanupObj(struct obj* o) {
    if (o == NULL) {
        printf("obj is null\n");
        return 1;
    }
    if (o->file_ptr == NULL) {
        printf("file is null\n");
        return 1;
    }
    fclose(o->file_ptr);
    free(o);
    return 0;
}

int objectBase() {
    struct obj* o = newObj("../README.md");
    mainLogic(o);
    return cleanupObj(o);
}
```