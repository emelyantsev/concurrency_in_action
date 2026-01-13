#include <mutex>
#include <iostream>

#include "hierarchical_mutex.h"


int main() {

    hierarchical_mutex m1( 2042 );
    hierarchical_mutex m2( 2000 );

    // m1.lock();
    // m2.lock();

    try {

        std::lock(m1, m2) ;
    }
    catch (const std::exception& e) {

        std::cout << e.what() << std::endl;
    }
}