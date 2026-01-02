#include <thread>
#include <exception>
#include <iostream>

void do_something(int& i)
{
    ++i;
}

struct func
{
    int& i;

    func(int& i_):i(i_){}

    void operator()()
    {
        for(unsigned j=0;j<1000000;++j)
        {
            do_something(i);
        }
    }
};

void do_something_in_current_thread() {

    throw std::logic_error("a very bad error");
}

void f()
{
    int some_local_state=0;
    func my_func(some_local_state);
    //std::thread t(my_func);
    std::jthread t(my_func);

    try
    {
        do_something_in_current_thread();
    }
    catch ( const std::logic_error& e)
    {
        std::cout << e.what() << std::endl;
        //t.join();
        throw e;
    }

    t.join();
}

int main()
{

    try {

        f();

    }
    catch ( const std::logic_error& e) {

       std::cout << "Caught in main: " << e.what() << std::endl;
    }
}