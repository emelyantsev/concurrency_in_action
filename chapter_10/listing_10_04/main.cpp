#include <vector>
#include <string>
#include <numeric>
#include <execution>
#include <iostream>

int main() {

    std::vector<std::string> data = {
        "10", "20", "30", "40", "50"
    };

    long long sum = std::transform_reduce(

        std::execution::seq,
        
        data.begin(), data.end(),
        
        0LL,                      // initial value
        
        std::plus<>(),            // reduction
        
        [](const std::string& s)  // transform
        
        {
            return std::stoll(s);
        }
    );

    std::cout << "sum = " << sum << "\n";
}
