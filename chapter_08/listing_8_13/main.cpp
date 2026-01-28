#include <vector>
#include <iomanip>

#include "common.h"
#include "parallel_algorithms.h"


int main() {

    std::vector<long long > numbers( 16, 1 );
    
    parallel_partial_sum3( numbers.begin(), numbers.end() );
    
    //std::partial_sum( numbers.begin(), numbers.end(), numbers.begin() ) ;

    for (const auto& num : numbers) {

        std::cout << num << " " ;  
    }

    std::cout << std::endl;

    return 0;
}