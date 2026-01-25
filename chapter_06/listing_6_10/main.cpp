#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <cassert>
#include <chrono>
#include <set>
#include <mutex>

#include "threadsafe_queue3.h"
#include "common.h"


void complex_test() {

    threadsafe_queue<int> queue;
    
    const int num_producers = 3;
    const int num_wait_consumers = 2;
    const int num_try_consumers = 2;
    const int items_per_producer = 1000;
    const int total_items = num_producers * items_per_producer;

    std::atomic<int> consumed_count { 0 };
    std::atomic<long long> total_sum { 0 };
    std::atomic<bool> producers_done { false };
    std::atomic<int> failed_try_pop_count { 0 };

    // Для проверки уникальности полученных данных
    std::mutex set_mut;
    std::set<int> received_items;

    sync_cout << "Starting complex test with " << total_items << " items...\n";

    // 1. Потоки-потребители с wait_and_pop
    std::vector<std::jthread> wait_consumers;

    for (int i = 0; i < num_wait_consumers; ++i) {

        wait_consumers.emplace_back( [&] {

            while (true) {

                // Выходим, если всё произведено и очередь пуста
                if ( producers_done && queue.empty() ) break;
                
                // Проверяем версию wait_and_pop() -> shared_ptr
                auto ptr = queue.wait_and_pop();

                if ( ptr ) {

                    int val = *ptr;
                    total_sum += val;
                    consumed_count++;
                    std::lock_guard lock(set_mut);
                    received_items.insert(val);
                }
            }
        });
    }

    // 2. Потоки-потребители с try_pop
    std::vector<std::jthread> try_consumers;

    for (int i = 0; i < num_try_consumers; ++i) {

        try_consumers.emplace_back( [&] {

            while ( !producers_done || !queue.empty() ) {

                int val;

                // Проверяем версию try_pop(T& value)
                if ( queue.try_pop( val ) ) {

                    total_sum += val;
                    consumed_count++;
                    std::lock_guard lock( set_mut );
                    received_items.insert( val );

                } else {

                    ++failed_try_pop_count;
                    std::this_thread::yield(); // Даем поработать другим
                }
            }
        });
    }

    // 3. Потоки-производители
    std::vector<std::jthread> producers;
    for (int i = 0; i < num_producers; ++i) {

        producers.emplace_back( [&](int id) {

            for (int j = 0; j < items_per_producer; ++j) {
                queue.push(id * 10000 + j);
            }
        }, i);
    }

    // Ждем окончания записи
    for (auto& p : producers) p.join();
    producers_done = true;
    sync_cout << "Producers finished work.\n";

    // Ждем окончания чтения
    for (auto& c : wait_consumers) c.join();
    for (auto& c : try_consumers) c.join();

    // --- Анализ результатов ---
    sync_cout << "\n--- Statistics ---\n";
    sync_cout << "Items pushed: " << total_items << "\n";
    sync_cout << "Items popped: " << consumed_count << "\n";
    sync_cout << "Failed try pop count: " << failed_try_pop_count << "\n";
    sync_cout << "Unique items in set: " << received_items.size() << "\n";
    sync_cout << "Queue empty at end: " << ( queue.empty() ? "Yes" : "No") << "\n";

    // Проверки
    assert( consumed_count == total_items );
    assert( received_items.size() == total_items );
    assert( queue.empty() );

    sync_cout << "\n[SUCCESS] All methods (push, try_pop, wait_and_pop, empty) verified!\n";
}

int main() {

    try {

        complex_test();
    } 
    catch (const std::exception& e) {
    
        std::cerr << "Test crashed: " << e.what() << std::endl;
    }

    return 0;
}