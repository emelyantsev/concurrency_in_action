#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include <string>
#include <chrono>

#include "common.h"
#include "threadsafe_lookup_table.h"


void test_concurrent_insert( threadsafe_lookup_table<int, int>& table, int num_threads, int ops_per_thread ) {

    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {

        threads.emplace_back( [ &table, i, ops_per_thread ] () {
        
            for (int j = 0; j < ops_per_thread; ++j) {
                // Каждый поток пишет в свой диапазон ключей
                table.add_or_update_mapping(i * ops_per_thread + j, j);
            }
        }
        );
    }

    for (auto& t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();
    sync_cout << "[OK] Insert Test: " << num_threads * ops_per_thread << " elements added.\n";
}

void test_concurrent_read_write( threadsafe_lookup_table<int, std::string>& table) {

    std::thread writer( [&] () {
        
        for (int i = 0; i < 100; ++i) {
        
            table.add_or_update_mapping( i, "value_" + std::to_string(i));
            std::this_thread::sleep_for( 1ms );
        }
    });

    std::thread reader( [&] () {
        
        for (int i = 0; i < 100; ++i) {
            // Читаем значение, если его нет — получим пустую строку
            table.value_for(i);
            std::this_thread::sleep_for( 1ms );
        }
    });

    writer.join();
    reader.join();
    sync_cout << "[OK] Read/Write Consistency Test passed.\n";
}

void test_remove(threadsafe_lookup_table<int, int>& table) {

    table.add_or_update_mapping(42, 100);
    assert( table.value_for(42) == 100 );
    
    table.remove_mapping(42);
    assert(table.value_for(42, -1) == -1); 
    
    sync_cout << "[OK] Remove Test passed.\n";
}

// int main() {

//     // Создаем таблицу с 23 корзинами (простое число)
//     threadsafe_lookup_table<int, int> table(23);

//     sync_cout << "Starting tests...\n";

//     test_remove(table);
//     test_concurrent_insert(table, 8, 1000); // 8 потоков по 1000 операций

//     threadsafe_lookup_table<int, std::string> string_table(11);
//     test_concurrent_read_write(string_table);

//     sync_cout << "All basic thread-safety tests passed!\n";
//     return 0;
// }

int main() {

    threadsafe_lookup_table<int, int> table(23);

    auto start = std::chrono::high_resolution_clock::now();

    // Запускаем твои многопоточные тесты здесь
    test_concurrent_insert(table, 8, 10000); 

    auto end = std::chrono::high_resolution_clock::now();
    
    // Вычисляем разницу
    std::chrono::duration<double, std::milli> elapsed = end - start;
    sync_cout << "Test took: " << elapsed.count() << " ms" << std::endl;

    return 0;
}