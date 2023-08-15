#include <thread>
#include <stdexcept>
#include <iostream>

void do_something( int& i )
{
    ++i;
}

struct func {
    int& i;

    func( int& i_ ) :i( i_ ) {}

    void operator()()
    {
        for( unsigned j = 0; j < 1000000; ++j ) {
            do_something( i );
        }
    }
};

void do_something_in_current_thread()
{
    throw std::logic_error::logic_error( "Logic error" );

}

void f()
{
    int some_local_state = 0;

    func my_func( some_local_state );
    
    std::thread t( my_func );
    
    try {
        do_something_in_current_thread();
    } catch( ... ) {
        t.join();
        throw;
    }
    t.join();

}

int main()
{
    try {

        f();
    }
    catch( std::exception& e ) {
        std::cout << e.what() << std::endl;
    }
    
}