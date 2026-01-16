#include <list>
#include <algorithm>
#include <future>
#include <thread>
#include <cassert>

#include "common.h"

template<typename T, bool Verbose = false >
std::list<T> parallel_quick_sort(std::list<T> input) {

    if (Verbose) {

        sync_cout << std::this_thread::get_id() << std::endl; 
    }
  
    if ( input.empty() ) {

        return input;
    }

    std::list<T> result;

    result.splice( result.begin(), input, input.begin() );
    
    T const& pivot = *result.begin();

    auto divide_point = std::partition( input.begin(), input.end(), [ & ]( T const& t ) { return t < pivot; } );
    
    std::list<T> lower_part;
    
    lower_part.splice( lower_part.end(), input, input.begin(), divide_point );
    
    std::future<std::list<T> > new_lower( std::async( &parallel_quick_sort<T, Verbose>, std::move(lower_part) ) );
    
    auto new_higher( parallel_quick_sort<T, Verbose>( std::move(input) ) );
    
    result.splice( result.end(), new_higher);
    
    result.splice( result.begin(), new_lower.get() );
    
    return result;
}


int main() {

    std::list l{ 1, 4, 5, 0, 2, 3, -2, -6, 10};

    auto result = parallel_quick_sort<int, false>( std::move(l) );

    assert( std::is_sorted( result.begin(), result.end() ) ) ;

    return 0;
}