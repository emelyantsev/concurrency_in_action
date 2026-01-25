#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <cassert>

#include "common.h"
#include "threadsafe_queue2.h" 


void test_fine_grained_queue() {

    threadsafe_queue<int> queue;
    
    const int num_elements = 10000;
    const int num_consumers = 3;
    const int num_producers = 3;
    
    std::atomic<int> consumed_sum{0};
    std::atomic<int> consumed_count{0};
    std::atomic<bool> production_finished{false};
    std::atomic<int> failed_try_pop_count{0};

    // 1. Потоки-производители (Push в tail_mutex)
    std::vector<std::jthread> producers;

    for (int i = 0; i < num_producers; ++i) {

        producers.emplace_back( [&queue, num_elements, i] {

            std::this_thread::sleep_for(1ms);


            for (int j = 0; j < num_elements; ++j) {

                queue.push(j);
            }
        });
    }

    // 2. Потоки-потребители (Pop из head_mutex)
    std::vector<std::jthread> consumers;

    for (int i = 0; i < num_consumers; ++i) {

        consumers.emplace_back( [&queue, &consumed_sum, &consumed_count, &production_finished, &failed_try_pop_count] {
    
            while ( !production_finished || !queue.empty() ) {

                auto ptr = queue.try_pop();
                
                if ( ptr ) {
                
                    consumed_sum += *ptr; // Суммируем для проверки целостности
                    consumed_count++;
                } 
                else {
                    // Если пусто, даем процессору передохнуть
                    ++failed_try_pop_count;
                    std::this_thread::yield();
                }
            }
        });
    }

    // Ждем завершения производителей
    for (auto& p : producers) p.join();
    production_finished = true; 

    // Ждем завершения потребителей
    for (auto& c : consumers) c.join();

    // 3. Проверка результатов
    sync_cout << "--- Test Results ---\n";
    sync_cout << "Expected elements: " << num_elements * num_producers << "\n";
    sync_cout << "Actually popped:  " << consumed_count.load() << "\n";
    
    if (consumed_count == num_elements * num_producers) {
        sync_cout << "Status: SUCCESS! No data lost.\n";
    } else {
        sync_cout << "Status: FAILURE! Data race detected.\n";
    }

    sync_cout << "consumed sum " << consumed_sum << std::endl;
    sync_cout << "failed try pop count " << failed_try_pop_count << std::endl;
}

int main() {

    test_fine_grained_queue();
    
    return 0;
}