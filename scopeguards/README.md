# Use Boost Scope Guards, say goodbye to resource leaks

In C++, when interfacing with a C library that returns handles to resources that you need to deallocate after use by calling free, fclose, or some other deallocation function -- don't. Avoid the C habit of calling the deallocation function manually because you're guaranteed to forget doing so in one situation or another, be it a forgotten return, an unexpected exception, or a program modification months later by someone who didn't bother to read your code completely. Instead, use a Boost Scope Guard to have the deallocation function be called automatically at the end of the scope.

If you don't use Boost, writing an equivalent class (that takes a lambda as a ctor argument and invokes it in the dtor) is easy enough. If your handle happens to be a pointer, you don't even need that, but you can use `std::unique_ptr` and pass the deallocation function as a second argument ("custom deallocator).

TL;DR:

```cpp
#include <boost/scope_exit.hpp>

const auto handle = allocate_resource();

BOOST_SCOPE_EXIT_ALL(handle) {
    deallocate_resource(handle);
};
```

Details: https://www.boost.org/doc/libs/1_70_0/libs/scope_exit/doc/html/index.html

---
*Wolfram Rösler • wolfram@roesler-ac.de • https://gitlab.com/wolframroesler • https://twitter.com/wolframroesler • https://www.linkedin.com/in/wolframroesler/*
