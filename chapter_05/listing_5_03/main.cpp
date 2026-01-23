#include <iostream>

#include "common.h"


void foo(int a, int b) {

    sync_cout << a << "," << b << std::endl;
}

int get_num() {

    static int i = 0;
    return ++i;
}

int main() {

    foo( get_num(), get_num() );
}