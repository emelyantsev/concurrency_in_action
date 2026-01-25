#include <expected>
#include <string>
#include <thread>
#include <iostream>

std::expected<int, std::string> fetchData(bool shouldFail) {

    if ( shouldFail) {

        return std::unexpected( "Network error" );
    } 
    else {

        return 42;
    }            
}

int main() {
    
    std::thread t( [] () {

        auto res = fetchData(false);
        
        if (res) std::cout << "Data: " << *res << std::endl;
        
        else std::cerr << "Error: " << res.error() << std::endl;
    } );
    
    t.join();
}