#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

#include "common.h"

#include "threadsafe_queue.h"


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

    std::this_thread::sleep_for(200ms) ;

    sync_cout << "[" << std::this_thread::get_id() << "]" << " processed chunk " << chunk.id << std::endl;
}


bool is_last_chunk(const data_chunk& chunk) {

    return chunk.is_last;
}


threadsafe_queue<data_chunk> data_queue;

void data_preparation_thread() {

    int id = 0;

    while ( more_data_to_prepare() ) {

        data_chunk data = prepare_data( id++ ) ;
        data_queue.push( data );
    }

    data_chunk  data = prepare_data(id++, true) ;
    data_queue.push( data );
}

void data_processing_thread() {

    while ( true ) {


        data_chunk data;
        data_queue.wait_and_pop(data);
        
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