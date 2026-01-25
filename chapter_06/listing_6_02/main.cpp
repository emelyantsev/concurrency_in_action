#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include <chrono>
#include <string>

#include "threadsafe_queue.h" 
#include "common.h"

void test_queue() {

    threadsafe_queue<std::string> queue;

    std::string val;
    
    assert( queue.try_pop(val) == false );
    assert( queue.try_pop() == nullptr );
    
    sync_cout << "1. Non-blocking try_pop on empty queue: OK\n";

    {
        std::jthread consumer( [&queue] {

            sync_cout << "   [Consumer] Waiting for data...\n";
            
            std::string item1;
            queue.wait_and_pop(item1);
            sync_cout << "   [Consumer] Got via wait_and_pop(T&): " << item1 << "\n";

            // Проверка wait_and_pop() -> shared_ptr
            auto item2 = queue.wait_and_pop();
            sync_cout << "   [Consumer] Got via wait_and_pop(): " << *item2 << "\n";
        });

        std::jthread producer([&queue] {

            std::this_thread::sleep_for(1s); 
            sync_cout << "   [Producer] Pushing 'Hello'...\n";
            queue.push("Hello");

            std::this_thread::sleep_for(500ms);
            sync_cout << "   [Producer] Pushing 'World'...\n";
            queue.push("World");
        });
        
        
    }

    sync_cout << "2. Blocking wait_and_pop test: OK\n";

    const int num_items = 1000;
    std::atomic<int> count{0};

    {
        std::vector<std::jthread> workers;
        
        for (int i = 0; i < 5; ++i) {

            workers.emplace_back( [&queue, &count ] {
            
                for (int j = 0; j < 200; ++j) {
                    if ( queue.try_pop() ) {
                        count++;
                    }
                }
            });
        }

        
        for (int i = 0; i < num_items; ++i) {
            queue.push("data");
        }
    }

    

    sync_cout << "3. Parallel stress test finished. Elements in queue: " 
              << (queue.empty() ? "0" : "Some left") << "\n";
    sync_cout << "count is " << count << std::endl;
}

int main() {

    try {
        
        sync_cout << "Starting Threadsafe Queue Test...\n";
        
        test_queue();
        
        sync_cout << "\nAll tests completed successfully!\n";

    } catch (const std::exception& e) {

        std::cerr << "Test failed with exception: " << e.what() << "\n";
    }
    return 0;
}