#include <vector>

#include "common.h"
#include "parallel_algorithms.h"


int main() {

    std::vector<int> numbers( 1'000'000, 1 );
    
    parallel_for_each( numbers.begin(), numbers.end(), [](int& num){  num = -num;  }  ) ;

    int sum = 0;

    for (int num : numbers) {
        sum += num;
    }

    sync_cout << sum << std::endl;

    return 0;
}