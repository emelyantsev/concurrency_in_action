#include <thread>
#include <atomic>
#include <iostream>
#include <latch>

#include "common.h"


std::atomic<int> x(0), y(0), z(0);

unsigned const loop_count = 10;

std::memory_order mem_order{ std::memory_order_relaxed } ;

std::latch go_latch{5};

struct read_values {

    int x,y,z;
};

read_values values1[loop_count];
read_values values2[loop_count];
read_values values3[loop_count];
read_values values4[loop_count];
read_values values5[loop_count];


void increment(std::atomic<int>* var_to_inc, read_values* values) {
    
    go_latch.arrive_and_wait();
        
    for ( int i = 0; i < loop_count; ++i ) {

        values[i].x = x.load( mem_order );
        values[i].y = y.load( mem_order );
        values[i].z = z.load( mem_order );
        
        var_to_inc->store( i + 1 , mem_order );
        
        std::this_thread::yield();
    }
}

void read_vals(read_values* values) {

    go_latch.arrive_and_wait();
        
    for ( int i = 0; i < loop_count; ++i ) {
            
        values[i].x = x.load( mem_order );
        values[i].y = y.load( mem_order );
        values[i].z = z.load( mem_order );

        std::this_thread::yield();
    }
}

void print(read_values* v) {

    for ( int i = 0; i < loop_count; ++i ) {

        if (i) {

            sync_cout << ",";
        }
            
        sync_cout << "(" << v[i].x << "," << v[i].y << "," << v[i].z <<")";
    }
    
    sync_cout << std::endl;
}

int main() {

    std::thread t1( increment, &x, values1 );
    std::thread t2( increment, &y, values2 );
    std::thread t3( increment, &z, values3 );
    std::thread t4( read_vals, values4 );
    std::thread t5( read_vals, values5 );

    t5.join();
    t4.join();
    t3.join();
    t2.join();
    t1.join();
    
    print( values1 );
    print( values2 );
    print( values3 );
    print( values4 );
    print( values5 );
}