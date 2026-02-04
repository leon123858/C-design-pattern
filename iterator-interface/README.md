# Iterator Interface

## Index Access

當提供了一個陣列要給 caller 調用，但是希望隱藏陣列 item 的實作細節，可以使用 index access。

caller 透過類似 getItemByIndex 的函數取得 item 的 Handle，接著調用 callee 提供的方法操作 Handle 即可。

但當多個 caller 同時呼叫，會發生 race condition，這方面確實有所限制。

- 長度增減: index 可能變化，需要重新計算 index。
- item 正被更新或刪除時同時被 access 會有 race condition。

```c
// API
void* getElement(int index);
// caller
void* element;
element = getElement(0);
element = getElement(1);
element = getElement(2);
```

```c
// header
#define MAX_USERS 5
char* getLoginName(int index);

// implement
#define MAX_NAME_LENGTH 50
#define MAX_PWD_LENGTH 50

struct ACCOUNT
{
  char loginname[MAX_NAME_LENGTH];
  char password[MAX_PWD_LENGTH];
};

static struct ACCOUNT accountData[MAX_USERS] = {{"A","A"}, {"B","B"}, {"C","C"}, {"X","X"}, {"Y","Y"}};

char* getLoginName(int index)
{
  return accountData[index].loginname;
}

// caller
for(int i=0; i<MAX_USERS; i++)
{
  char* loginName = getLoginName(i);
  if(loginName[0] == 'X')
  {
    return true;
  }
}
```

## cursor Iterator

當底層資料並非一維陣列，而是 tree/map/link-list/...... 等複雜結構，無法輕易用 index 來表達元素。

提供一個游標給 caller 來讀取某一個元素，並且 caller 可以不斷移動游標取得下一個元素，直到 access 了所有元素。

同樣，也可以選擇是否用 handle 包裝 item，用來隱藏底層實作細節。

同時，使用該 pattern 依舊要注意 race condition

- 利用 cursor 不用怕抓錯 index 的 race condition
- access 正在更新或刪除的 item 時依然會出錯。

```c
// API
ITERATOR* createIterator();
void* getNextElement(ITERATOR* iterator);

// caller
void* element;
ITERATOR* cursor = createIterator();
while(element = getNextElement(cursor)) {
	printf("%s\n", (char*)element);
	// ...
};
```

```c
// header
typedef struct ITERATOR* ITERATOR_HANDLE;
ITERATOR_HANDLE createIterator();
char* getNextLoginName(ITERATOR_HANDLE iterator);
void destroyIterator(ITERATOR_HANDLE iterator);

// implement
struct ACCOUNT
{
  char loginname[MAX_NAME_LENGTH];
  char password[MAX_PWD_LENGTH];
  int nextPosition;
};

static struct ACCOUNT accountData[MAX_USERS] = {{"A","A",1}, {"B","B",2}, {"C","C",3}, {"X","X",4}, {"Y","Y",0}};

static struct ACCOUNT* getFirst()
{
    return &accountData[0];
}

static struct ACCOUNT* getNext(struct ACCOUNT* current)
{
  if(current->nextPosition == 0)
  {
    return NULL;
  }
  else
  {
    return &accountData[current->nextPosition];
  }
}


struct ITERATOR
{
  char buffer[MAX_NAME_LENGTH];
  struct ACCOUNT* element;
};

ITERATOR_HANDLE createIterator()
{
  struct ITERATOR* iterator = malloc(sizeof(struct ITERATOR));
  iterator->element = getFirst();
  return iterator;
}

char* getNextLoginName(ITERATOR_HANDLE iterator)
{
  if(iterator->element != NULL)
  {
    strcpy(iterator->buffer, iterator->element->loginname);
    iterator->element = getNext(iterator->element);
    return iterator->buffer;
  }
  else
  {
    return NULL;
  }
}

void destroyIterator(ITERATOR_HANDLE iterator)
{
  free(iterator);
}

// caller
char* loginName;
ITERATOR_HANDLE iterator = createIterator();
while(loginName = getNextLoginName(iterator))
{
  if(loginName[0] == 'X')
  {
    destroyIterator(iterator);
    return true;
  }
}
destroyIterator(iterator);
```

## Callback Iterator

在想要掃描整個 Iterator 時，又希望不會受限於 race condition。

可以直接注入 callback function，讓 callee 在確保順序下完整操作每一個元素。

重要的概念是 callback 執行時，要確保不會因為動態更新陣列而有 race condition，最後出現問題。

```C
// API
typedef void (*FP_CALLBACK)(void* element, void* arg);
void iterate(FP_CALLBACK cb, void* arg);

// caller
void callback(void* element, void* arg) {
	// ...
};
iterate(callback, NULL);
```

```c
// header
typedef void (*FP_CALLBACK)(char* loginName, void* arg);
void iterateLoginNames(FP_CALLBACK callback, void* arg);

// implement
struct ACCOUNT
{
  char loginname[MAX_NAME_LENGTH];
  char password[MAX_PWD_LENGTH];
  struct ACCOUNT* next;
};

static struct ACCOUNT accountData1;
static struct ACCOUNT accountData2;
static struct ACCOUNT accountData3;
static struct ACCOUNT* accountList;

static void initList()
{
  strcpy(accountData1.loginname, "Y");
  strcpy(accountData1.password, "Y");
  accountData1.next = &accountData2;

  strcpy(accountData2.loginname, "X");
  strcpy(accountData2.password, "X");
  accountData2.next = &accountData3;

  strcpy(accountData3.loginname, "Y");
  strcpy(accountData3.password, "Y");
  accountData3.next = NULL;

  accountList = &accountData1;
}

static struct ACCOUNT* getFirst()
{
  return accountList;
}

static struct ACCOUNT* getNext(struct ACCOUNT* current)
{
  return current->next;
}


void iterateLoginNames(FP_CALLBACK callback, void* arg)
{
  initList();

  struct ACCOUNT* element = getFirst();
  while(element != NULL)
  {
    callback(element->loginname, arg);
    element = getNext(element);
  }
}

// caller
void findX(char* loginName, void* arg)
{
  bool* found = (bool*) arg;
  if(loginName[0] == 'X')
  {
    *found = true;
  }
}

bool callback_iterate_anyoneWithX()
{
  bool found=false;
  iterateLoginNames(findX, &found);
  return found;
}
```
