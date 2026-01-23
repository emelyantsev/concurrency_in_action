#include <vector>
#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>

#include "common.h"

std::vector<int> data;
std::atomic_bool data_ready(false);

void reader_thread() {

    while( !data_ready.load() ) {

        std::this_thread::sleep_for( 100ms );
        sync_cout << "The data is not ready" << std::endl;
    }

    sync_cout << "The answer = " << data[0] << std::endl;
}


void writer_thread() {

    std::this_thread::sleep_for( 1s );

    data.push_back(42);
    data_ready = true;
}

int main() {

    std::jthread t1( reader_thread ) ;
    std::jthread t2( writer_thread ) ;

    return 0;
}