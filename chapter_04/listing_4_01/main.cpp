#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <iostream>
#include <syncstream>
#include <chrono>

#ifndef sync_cout
#define sync_cout std::osyncstream(std::cout)
#endif

using namespace std::chrono_literals;

constexpr int NUM_COUNTS = 100;


bool more_data_to_prepare() {

    static int count = 0;
    ++count;
    return count < NUM_COUNTS  ? true : false ;
}

struct data_chunk  {
    int id;
    bool is_last;
};


data_chunk prepare_data(int id, bool is_last = false) {

    std::this_thread::sleep_for(100ms) ;

    sync_cout << "[" << std::this_thread::get_id() << "]" << " prepared chunk " << id << std::endl;

    return data_chunk{id, is_last};
}


void process(const data_chunk& chunk) {

    //std::this_thread::sleep_for(200ms) ;

    sync_cout << "[" << std::this_thread::get_id() << "]" << " processed chunk " << chunk.id << std::endl;
}


bool is_last_chunk(const data_chunk& chunk) {

    return chunk.is_last;
}


std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;


void data_preparation_thread() {

    int id = 0;

    while ( more_data_to_prepare() ) {

        data_chunk const data = prepare_data( id++ ) ;
        std::lock_guard<std::mutex> lk( mut ) ;
        data_queue.push( data );
        data_cond.notify_one();
    }

    data_chunk const data = prepare_data(id++, true) ;
    std::lock_guard<std::mutex> lk( mut ) ;
    data_queue.push( data );
    data_cond.notify_one();
}

void data_processing_thread() {

    while ( true ) {

        std::unique_lock<std::mutex> lk( mut );
        
        data_cond.wait( lk, [] { return !data_queue.empty(); } ) ;

        data_chunk data = data_queue.front();
        data_queue.pop();
        
        lk.unlock();
        
        process( data );

        if ( is_last_chunk( data ) ) {

            break;
        }
            
    }
}

int main() {

    std::thread t1( data_preparation_thread );
    std::thread t2( data_processing_thread );
    
    t1.join();
    t2.join();

    return EXIT_SUCCESS;
}