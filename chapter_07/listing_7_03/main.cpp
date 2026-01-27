#include <iostream>
#include <vector>
#include <thread>
#include <set>
#include <mutex>
#include <cassert>
#include <atomic>

#include "common.h"
#include "lock_free_stack.h"


void test_stack_multithreaded() {

    LockFreeStack<int> stack;

    const int num_threads = 8;
    const int operations_per_thread = 10'000;
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    std::vector<int> results[ num_threads ];

    sync_cout << "Starting threads..." << std::endl;
    
    for (int i = 0; i < num_threads; ++i) {

        producers.emplace_back( [ &stack, i, operations_per_thread ] () {
            
            for (int j = 0; j < operations_per_thread; ++j) {

                stack.push( i * operations_per_thread + j );
            }
        });
    }


    for (int i = 0; i < num_threads; ++i) {

        consumers.emplace_back( [ &stack, &results, i, num_threads, operations_per_thread ] () {

            int total_to_consume = operations_per_thread ;
            int consumed = 0;
            
            while ( consumed < total_to_consume ) {

                auto val = stack.pop();

                if ( val ) {

                    results[i].push_back( *val );
                    consumed++;
                
                } else {

                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();


    std::set<int> all_found_elements;
    
    for (int i = 0; i < num_threads; ++i) {

        for ( int val : results[i] ) {
        
            all_found_elements.insert( val );
        }
    }

    sync_cout << "Test finished!" << std::endl;
    sync_cout << "Expected elements: " << num_threads * operations_per_thread << std::endl;
    sync_cout << "Actual elements: " << all_found_elements.size() << std::endl;

    assert( all_found_elements.size() == num_threads * operations_per_thread );
}

int main() {

    {
        std::atomic<std::shared_ptr<int>> ptr( std::make_shared<int>(42) ) ;
        sync_cout << ptr.is_lock_free() << " " << ptr.is_always_lock_free << std::endl;

    }

    test_stack_multithreaded();

    return 0;
}