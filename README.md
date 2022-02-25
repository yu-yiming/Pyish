# Pyish

A toy library of C++ that can be used like Python.

## Introduction

**Python** is famous for that it's easy to write and read, while **C++** is blamed for its complexity. With the help of templates and ranges library in modern **C++**, it's possible to invent a python-like **C++** library that is as concise as **Python**, yet still fast.

Here is a snippet of code as an example:

```cpp
int main() {
    var x = 12;
    x = list(1, 2.0, 3, "abc");
    for (var x : x.slice(1, 3)) {
        print(x);
    }
    x = tuple(1, 2.0, "abc");
    auto [a, _, c] = x
    print(a)
}
```