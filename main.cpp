#include "include/pyish.hpp"

using namespace python;

int main() {
    var x = 1;
    x = "abc";
    x = 42.0;
    std::cout << x << '\n';

    var arr = list(1, 2.3, "abc");
    var y = 20;
    for (var x : arr) {
        std::cout << x << '\n';
    }
    std::cout << arr[3];

    var obj = new List();
    

    return 0;
}