#pragma once

#include <atomic>
class spinlock_mutex {

    public:
        spinlock_mutex(): flag{ATOMIC_FLAG_INIT} {}

        void lock() {

            while( flag.test_and_set(std::memory_order_acquire) );
        }

        void unlock() {

            flag.clear(std::memory_order_release);
        }

    private:

        std::atomic_flag flag;
};



class smart_mutex {

    std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:

    void lock() {
        // Пытаемся захватить флаг. Если он уже установлен (true),
        // переходим к ожиданию.
        while ( flag.test_and_set(std::memory_order_acquire) ) {
            // wait(true) блокирует поток, пока значение флага остается true.
            // Как только другой поток вызовет notify, wait проверит флаг снова.
            flag.wait(true, std::memory_order_relaxed);
        }
    }

    void unlock() {
        // Сбрасываем флаг в false
        flag.clear(std::memory_order_release);
        // Будим один из потоков, который уснул в методе wait()
        flag.notify_one();
    }
};
