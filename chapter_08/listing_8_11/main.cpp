#include <vector>
#include <iomanip>

#include "common.h"
#include "parallel_algorithms.h"


int main() {

    std::vector<long long > numbers( 1'000'000'000, 1 );
    
    parallel_partial_sum( numbers.begin(), numbers.end() );
    
    //std::partial_sum( numbers.begin(), numbers.end(), numbers.begin() ) ;

    sync_cout << numbers.back() << std::endl;

    return 0;
}