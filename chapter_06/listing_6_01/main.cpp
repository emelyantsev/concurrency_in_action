#include <iostream>
#include <vector>
#include <thread> 
#include <cassert>
#include <atomic>

#include "threadsafe_stack.h"
#include "common.h"


void stress_test() {

    threadsafe_stack<int> original_stack;
    std::atomic<int> popped_count{0};
    const int ops = 500; 
    
    {
        std::vector<std::jthread> producers;

        for (int i = 0; i < 4; ++i) {
        
            producers.emplace_back( [&original_stack, ops] (int id) {
         
                for (int j = 0; j < ops; ++j) {

                    original_stack.push(id * 1000 + j);
                }              
            }, i);
        }
    }

    sync_cout << "1. Push completed. Stack empty? " << ( original_stack.empty() ? "Yes" : "No") << "\n";

    threadsafe_stack<int> copied_stack( original_stack );
    sync_cout << "2. Stack copied successfully.\n";

    {
        std::vector<std::jthread> consumers;
        
        for (int i = 0; i < 2; ++i) {

            consumers.emplace_back( [ &copied_stack, &popped_count, ops ] {
            
                for (int j = 0; j < ops; ++j) {
                    
                    try {
                        int v;
                        copied_stack.pop(v);
                        popped_count++;
                    } 
                    catch (const empty_stack&) {}
                }
            } );
        }

        for (int i = 0; i < 2; ++i) {

            consumers.emplace_back( [ &copied_stack, &popped_count, ops ] {
            
                for (int j = 0; j < ops; ++j) {
            
                    try {
            
                        auto ptr = copied_stack.pop();
                        if (ptr) popped_count++;
                    } catch (const empty_stack&) {}
                }
            });
        }
    }

    sync_cout << "3. Pop test finished. Elements popped from copy: " << popped_count << "\n";
    sync_cout << "4. Is copied stack empty now? " << (copied_stack.empty() ? "Yes" : "No") << "\n";
}

int main() {

    {
        std::jthread test_thread(stress_test) ;
    }    

    sync_cout << "\nAll tests passed with jthread!" << std::endl;

    return 0;
}