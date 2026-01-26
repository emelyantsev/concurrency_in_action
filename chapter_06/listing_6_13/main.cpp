#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <cassert>
#include <set>
#include <mutex>

#include "common.h"
#include "threadsafe_list.h"


void functional_correctness_test() {

    threadsafe_list<int> list;
    const int num_push_threads = 4;
    const int items_per_thread = 1000;
    const int total_items = num_push_threads * items_per_thread;

    sync_cout << "--- Запуск теста корректности ---" << std::endl;

    // 1. Параллельная вставка
    std::vector<std::thread> push_threads;

    for (int i = 0; i < num_push_threads; ++i) {

        push_threads.emplace_back( [&list, i, items_per_thread] () {

            for (int j = 0; j < items_per_thread; ++j) {

                list.push_front( i * items_per_thread + j );
            }
        });
    }

    for (auto& t : push_threads) t.join();

    // Промежуточная проверка: считаем элементы через for_each
    int count = 0;
    list.for_each( [&count] (int) { count++; } ) ;
    sync_cout << "После вставки элементов: " << count << " (ожидалось " << total_items << ")" << std::endl;
    assert( count == total_items );


    // 2. Параллельное удаление: каждый поток удаляет свой диапазон чисел
    // Например, удалим все четные числа
    std::vector<std::thread> remove_threads;

    for (int i = 0; i < num_push_threads; ++i) {

        remove_threads.emplace_back( [&list] () {

            list.remove_if( [](int n) {
                return n % 2 == 0; // Удаляем только четные
            });
        } );
    }

    for (auto& t : remove_threads) t.join();

    // 3. Финальная проверка
    int remaining_count = 0;
    bool has_even = false;
    list.for_each( [&](int n) {
        remaining_count++;
        if (n % 2 == 0) has_even = true;
    });

    sync_cout << "После удаления четных осталось: " << remaining_count << std::endl;
    
    int expected_remaining = total_items / 2;
    
    if (remaining_count == expected_remaining && !has_even) {

        sync_cout << "УСПЕХ: Все данные корректны, четных чисел не осталось." << std::endl;
    } else {

        sync_cout << "ОШИБКА: Ожидалось " << expected_remaining << ", получено " << remaining_count;
        
        if (has_even) sync_cout << " (Найдены четные числа!)";
        
        sync_cout << std::endl;
        
        exit(1);
    }
}

int main() {

    functional_correctness_test();
    return 0;
}