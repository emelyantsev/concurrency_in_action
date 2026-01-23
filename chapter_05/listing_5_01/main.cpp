#include <vector>
#include <thread>
#include <mutex>

#include "common.h"
#include "spin_lock_mutex.h"


int counter = 0; 
//spinlock_mutex spin;
smart_mutex spin;


void increment(int iterations) {

    for ( int i = 0; i < iterations; ++i ) {
        
        std::lock_guard<smart_mutex> lock( spin ) ;     
        counter++; 
    }
}


int main() {

    const int num_threads = 4;
    const int iterations = 1'000'000;
    std::vector<std::jthread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment, iterations);
    }

    for (auto& t : threads) {
        t.join();
    }

    sync_cout << "Result: " << counter << std::endl;
    sync_cout << "Expected: " << num_threads * iterations << std::endl;

    return 0;
}
