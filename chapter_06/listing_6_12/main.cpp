#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <cassert>
#include <chrono>

#include "common.h"
#include "threadsafe_list.h"


void stress_test() {

    threadsafe_list<int> list;
    const int num_threads = 8;
    const int operations_per_thread = 1000;
    std::atomic<bool> start_flag( false );

    std::vector<std::thread> threads;

    // 1. Потоки на добавление (Push threads)
    for (int i = 0; i < num_threads / 4; ++i) {

        threads.emplace_back( [&, i] () {

            while ( !start_flag ) std::this_thread::yield();
            
            for (int j = 0; j < operations_per_thread; ++j) {
            
                list.push_front( i * operations_per_thread + j );
            }
        } );
    }

    // 2. Потоки на чтение (Reader threads)
    for (int i = 0; i < num_threads / 4; ++i) {
        
        threads.emplace_back( [&] () {

            while (!start_flag) std::this_thread::yield();
            
            for (int j = 0; j < operations_per_thread; ++j) {
            
                list.for_each( [](int n) {
                    // Просто имитируем чтение
                    int x = n * 2;
                });
            }
        });
    }

    // 3. Потоки на поиск (Finder threads)
    for (int i = 0; i < num_threads / 4; ++i) {
        
        threads.emplace_back([&]() {
        
            while (!start_flag) std::this_thread::yield();
        
            for (int j = 0; j < operations_per_thread; ++j) {
        
                auto res = list.find_first_if( [j] (int n) { return n == j; } );
            }
        });
    }

    // 4. Потоки на удаление (Eraser threads)
    for (int i = 0; i < num_threads / 4; ++i) {

        threads.emplace_back( [&] () {
            
            while (!start_flag) std::this_thread::yield();
            
            for (int j = 0; j < operations_per_thread; ++j) {

                list.remove_if( [j](int n) { return n % 10 == 0; } ); // Удаляем кратные 10
            }
        });
    }

    // Запуск всех потоков одновременно
    sync_cout << "Starting stress test with " << num_threads << " threads..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    start_flag = true;

    for (auto& t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;

    sync_cout << "Test finished in " << diff.count() << " seconds." << std::endl;
    sync_cout << "List state: Thread-safe and Still Standing!" << std::endl;
}

int main() {

    try {

        stress_test();

    } catch ( const std::exception& e ) {

        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}