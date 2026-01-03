#include <vector>
#include <thread>
#include <algorithm>
#include <functional>
#include <iostream>
#include <syncstream>

void do_work(unsigned id) {

    std::osyncstream(std::cout) << "Thread [" << std::this_thread::get_id() << "]" <<
        " processing value {" << id << "}" << std::endl; 

}

void f() {
    
    std::vector<std::thread> threads;
    
    for (unsigned i = 0; i < 20; ++i) {

        threads.push_back( std::thread(do_work,i) );
    }

    // std::for_each( threads.begin(),threads.end(), std::mem_fn( &std::thread::join ) );

    std::for_each( threads.begin(),threads.end(), [](std::thread& t) { t.join() ; } );
}

int main() {

    f();
}