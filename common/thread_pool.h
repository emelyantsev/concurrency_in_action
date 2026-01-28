#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <functional>
#include <algorithm>

#include "threadsafe_queue.h"

class thread_pool {

    std::atomic_bool done;
    threadsafe_queue<std::function<void()> > work_queue;
    std::vector<std::jthread> threads;

    void worker_thread() {

        while( !done ) {

            std::function<void()> task;

            if ( work_queue.try_pop( task ) ) {

                task();
            }
            else {
                std::this_thread::yield();
            }
        }
    }
public:

    thread_pool() : done{false} {

        unsigned const thread_count = std::max( 1u, std::thread::hardware_concurrency() ) ;
        
        try {

            for (unsigned i = 0; i < thread_count ; ++i) {

                threads.push_back( std::jthread( &thread_pool::worker_thread, this) );
            }
        }
        catch (...) {
            
            done = true;
            throw;
        }
    }

    ~thread_pool () {

        done = true;
    }

    template<typename FunctionType>
    void submit(FunctionType f) {

        work_queue.push( std::function<void()>(f) );
    }
};