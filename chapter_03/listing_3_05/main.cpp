#include <iostream>

#include "threadsafe_stack.h"


int main() {

    threadsafe_stack<int> si;
    si.push(5);
    
    try {

        auto top_val = si.pop();
        std::cout << *top_val << std::endl;
    }
    catch (const empty_stack& e) {
    
        std::cout << e.what() << std::endl;
    }


    if ( !si.empty() ) {

        int x;
        si.pop(x);
    }
    
}