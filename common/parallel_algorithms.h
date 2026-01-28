#pragma once

#include <future>
#include <algorithm>
#include <vector>
#include <atomic>
#include <numeric>
#include <barrier>
#include <thread>


class join_threads {

        std::vector<std::thread>& threads;

    public:

        explicit join_threads( std::vector<std::thread>& threads_ ): threads( threads_ ) {}

        ~join_threads() {

            for(int i = 0; i < threads.size(); ++i) {

                if ( threads[i].joinable() )
                    threads[i].join();
                }
        }
};


template<typename Iterator,typename Func>
void parallel_for_each( Iterator first, Iterator last, Func f ) {

    unsigned long const length = std::distance( first, last );

    if ( !length )
        return;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;

    unsigned long const hardware_threads = std::thread::hardware_concurrency();

    unsigned long const num_threads = std::min( hardware_threads != 0 ? hardware_threads : 2, max_threads );

    unsigned long const block_size = length / num_threads;

    std::vector< std::future<void> > futures( num_threads - 1 );
    std::vector<std::thread> threads( num_threads - 1 );
    join_threads joiner( threads );

    Iterator block_start = first;

    for (unsigned long i = 0; i < (num_threads - 1); ++i ) {

        Iterator block_end = block_start;
        
        std::advance( block_end, block_size );
        
        std::packaged_task< void( void ) > task(
        
            [=] () {

                std::for_each( block_start, block_end, f );
            });
        
        futures[i] = task.get_future();
        threads[i] = std::thread( std::move( task ) );
        
        block_start = block_end;
    }

    std::for_each( block_start, last, f );

    for ( unsigned long i = 0; i < (num_threads-1); ++i ) {

        futures[i].get();
    }
}


template<typename Iterator, typename MatchType>
Iterator parallel_find_impl( Iterator first, Iterator last, MatchType match, std::atomic<bool>& done ) {

    try {

        unsigned long const length = std::distance( first, last );
        unsigned long const min_per_thread = 25;
        
        if ( length < ( 2 * min_per_thread) ) {

            for ( ; ( first != last) && !done.load(); ++first ) {

                if ( *first == match ) {

                    done = true;
                    return first;
                }
            }
            return last;
        }
        else {

            Iterator const mid_point = first + ( length/2 );
            
            std::future<Iterator> async_result = std::async( &parallel_find_impl<Iterator, MatchType>, mid_point, last, match, std::ref( done ) );
            
            Iterator const direct_result = parallel_find_impl( first, mid_point, match, done );
            return ( direct_result == mid_point ) ? async_result.get() : direct_result ;
        }
    }
    catch (...) {

        done = true;
        throw;
    }
}

template<typename Iterator, typename MatchType>
Iterator parallel_find(Iterator first, Iterator last, MatchType match) {

    std::atomic<bool> done{false};
    return parallel_find_impl( first, last, match, done );
}



template<typename Iterator>
void parallel_partial_sum( Iterator first, Iterator last ) {

    typedef typename Iterator::value_type value_type;

    struct process_chunk {

        void operator()( Iterator begin, 
                         Iterator last,
                         std::future<value_type>* previous_end_value,
                         std::promise<value_type>* end_value)
        {
            try
            {
                Iterator end = last;
                ++end;
                std::partial_sum( begin, end, begin );
                
                if ( previous_end_value ) {

                    const value_type addend = previous_end_value->get();
                    *last += addend;
                    
                    if ( end_value ) {
                        end_value->set_value(*last);
                    }

                    std::for_each( begin, last, [addend] (value_type& item)
                                  {
                                      item += addend;
                                  });
                }
                else if ( end_value ) 
                {
                    end_value->set_value( *last );
                }
            }
            catch (...) {

                if (end_value)
                {
                    end_value->set_exception( std::current_exception() );
                }
                else
                {
                    throw;
                }
            }
        }
    };

    unsigned long const length = std::distance(first, last);

    if ( !length )
        return;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;

    unsigned long const hardware_threads = std::thread::hardware_concurrency();

    unsigned long const num_threads = std::min( hardware_threads != 0 ? hardware_threads : 2, max_threads );

    unsigned long const block_size = length / num_threads;

    typedef typename Iterator::value_type value_type;

    std::vector<std::thread> threads( num_threads - 1);
    
    std::vector<std::promise<value_type> > end_values( num_threads - 1 );
    
    std::vector<std::future<value_type> > previous_end_values;
    
    previous_end_values.reserve( num_threads - 1 );
    
    join_threads joiner(threads);

    Iterator block_start = first;
    
    for( unsigned long i = 0; i < (num_threads - 1); ++i) {

        Iterator block_last = block_start;
        std::advance( block_last, block_size - 1);

        threads[i] = std::thread( process_chunk(),
                               block_start, block_last,
                               (i!=0) ? &previous_end_values[i-1] : 0,
                               &end_values[i]);
        block_start=block_last;
        ++block_start;
        previous_end_values.push_back(end_values[i].get_future());
    }

    Iterator final_element = block_start;
    std::advance( final_element, std::distance(block_start, last)-1);

    process_chunk()( block_start, final_element,
                    (num_threads > 1) ? &previous_end_values.back() : 0, 0 );
}



struct barrier {

    std::atomic<unsigned> count;
    std::atomic<unsigned> spaces;
    std::atomic<unsigned> generation;
    
    barrier( unsigned count_ ) : count( count_ ), spaces(count_), generation(0) {}
    
    void wait() {

        unsigned const gen = generation.load();

        if ( !--spaces ) {

            spaces = count.load();
            ++generation;
        }
        else {

            while ( generation.load() == gen ) {

                std::this_thread::yield();
            }
        }
    }

    void done_waiting() {

        --count;
        
        if (!--spaces)
        {
            spaces = count.load();
            ++generation;
        }
    }
};



template<typename Iterator>
void parallel_partial_sum2( Iterator first, Iterator last ) {

    typedef typename Iterator::value_type value_type;

    struct process_element {

        void operator() ( Iterator first, Iterator last, std::vector<value_type>& buffer, unsigned i, barrier& b ) {

            value_type& ith_element = *(first + i);
            
            bool update_source = false;
            
            for ( unsigned step = 0, stride = 1; stride <= i; ++step, stride *= 2 ) {

                value_type const& source = ( step % 2 ) ? buffer[i] : ith_element;

                value_type& dest = ( step % 2 ) ? ith_element : buffer[i];
                
                value_type const& addend = ( step % 2 ) ? buffer[ i - stride ] : *( first + i - stride );
                dest = source + addend;
                update_source = ! (step % 2 );

                b.wait();
            }

            if (update_source) {

                ith_element = buffer[i];
            }

            b.done_waiting();
        }
    };

    unsigned long const length = std::distance( first, last );

    if ( length <= 1 )
        return;

    std::vector<value_type> buffer( length );
    buffer[0] = *first;
    barrier b( length );
    std::vector<std::thread> threads( length - 1 );
    join_threads joiner( threads );

    Iterator block_start = first;
    
    for (unsigned long i = 0; i < (length-1); ++i) {

        threads[i] = std::thread( process_element(), first, last, std::ref(buffer), i, std::ref(b) );
    }

    process_element()( first, last, buffer, length - 1, b);
}



template<typename Iterator>
void parallel_partial_sum3( Iterator first, Iterator last ) {

    typedef typename Iterator::value_type value_type;

    struct process_element {

        void operator() ( Iterator first, Iterator last, std::vector<value_type>& buffer, unsigned i, std::barrier<>& b ) {

            value_type& ith_element = *(first + i);
            
            bool update_source = false;
            
            for ( unsigned step = 0, stride = 1; stride <= i; ++step, stride *= 2 ) {

                value_type const& source = ( step % 2 ) ? buffer[i] : ith_element;

                value_type& dest = ( step % 2 ) ? ith_element : buffer[i];
                
                value_type const& addend = ( step % 2 ) ? buffer[ i - stride ] : *( first + i - stride );
                dest = source + addend;
                update_source = ! (step % 2 );

                b.arrive_and_wait();
            }

            if (update_source) {

                ith_element = buffer[i];
            }

            b.arrive_and_drop();
        }
    };

    unsigned long const length = std::distance( first, last );

    if ( length <= 1 )
        return;

    std::vector<value_type> buffer( length );
    buffer[0] = *first;
    std::barrier<> b( length );
    std::vector<std::jthread> threads( length - 1 );

    Iterator block_start = first;
    
    for (unsigned long i = 0; i < (length-1); ++i) {

        threads[i] = std::jthread( process_element(), first, last, std::ref(buffer), i, std::ref(b) );
    }

    process_element()( first, last, buffer, length - 1, b);
}