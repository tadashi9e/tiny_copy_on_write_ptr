Tiny copy-on-write pointer

## Usage

```c++
#include "cow_ptr.h"

void test() {
  // Create copy-on-write pointer
  CowPtr<std::string> p1(std::make_shared<std::string>());
  // Assign value.
  *(p1.put()) = "this is p1";
  std::cout << *(p1.get()) << std::endl;

  // Create copy-on-write pointer which shares managed object.
  CowPtr<X> p2(p1);
  // p1 and p2 shares same managed object.
  std::cout << *(p2.get()) << std::endl;

  // Put value to p2. Copy-on-write occurs.
  *(p2.put()) = "this is p2";

  // Now you can see "this is p1" on p1,
  // and you can see "this is p2" on p2.
  std::cout << *(p1.get()) << std::endl;
  std::cout << *(p2.get()) << std::endl;
}
```

If you define WITH_COW_OPERATOR_ACCESS,
you can use 'operator*' and 'operator->' like a ordinary pointer.

```c++
#define WITH_COW_OPERATOR_ACCESS
#include "cow_ptr.h"

  CowPtr<std::string> p1(std::make_shared<std::string>());
  // write-mode access
  *p1 = "THIS IS p1";
  // read-mode access
  std::cout << *(const CowPtr<std::string>)p1 << std::endl;
``

The 'operator*' and 'operator->' are write-mode access. It can cause copy.
If you want to avoid copy-on-write, you have to use read-mode access.
You can force read-mode access by const-casting as above.

I think get()/put() fassion is more clear than const-caasting.

## Requirement

C++11 compiler
