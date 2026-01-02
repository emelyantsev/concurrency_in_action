#include <thread>
#include <exception>
#include <iostream>

class thread_guard {

    public:
        
        explicit thread_guard(std::thread& t_): t(t_) {}

        thread_guard(thread_guard const&) = delete;
        thread_guard& operator=(thread_guard const&) = delete;

        ~thread_guard() {

            if ( t.joinable() ) {
                
                t.join();
            }
        }

    private:

        std::thread& t;

};

void do_something(int& i) {

    ++i;
}

struct func {
    
    int& i;

    func(int& i_) : i(i_) {}

    void operator()()
    {
        for ( unsigned j = 0; j < 1000000; ++j )
        {
            do_something( i );
        }
    }
};

void do_something_in_current_thread() {

    throw std::logic_error("a very dangerous error!");
}


void f()
{
    int some_local_state;

    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);
    //std::jthread t(my_func);
        
    do_something_in_current_thread();
}

int main()
{

    try {

        f();
    }

    catch (const std::logic_error& e) {

        std::cout << e.what() << std::endl;
    }
    
}