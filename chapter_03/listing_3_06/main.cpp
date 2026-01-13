#include <thread>
#include <mutex>

class some_big_object {

    public:

        some_big_object(int id_) : id(id_) {}

    private:

        int id;
};

void swap(some_big_object& lhs, some_big_object& rhs) {

    std::swap(lhs, rhs);
}

class X {

    public:

        X(some_big_object const& sd) : some_detail(sd) {}

    private:

        some_big_object some_detail;
        mutable std::mutex m;


    friend void swap( X& lhs, X& rhs ) {

        if ( &lhs == &rhs )
            return;

        std::lock( lhs.m, rhs.m);
        
        std::lock_guard<std::mutex> lock_a( lhs.m, std::adopt_lock );
        std::lock_guard<std::mutex> lock_b( rhs.m, std::adopt_lock );
        
        swap(lhs.some_detail, rhs.some_detail);
    }
    
};

int main() {

    some_big_object sbo1(1), sbo2(2);
    X x1(sbo1), x2(sbo2);

    std::thread t( [&x1, &x2]() {

        swap(x1, x2);
    } ) ;

    t.join();

    return EXIT_SUCCESS;
}