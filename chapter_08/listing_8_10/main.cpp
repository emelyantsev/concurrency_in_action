#include <vector>
#include <iomanip>

#include "common.h"
#include "parallel_algorithms.h"


int main() {

    std::vector<int> numbers( 1'000'000, 1 );

    numbers[877356] = -1;
    
    auto it = parallel_find(numbers.begin(), numbers.end(), -1);

    bool is_found = it != numbers.end();

    int ind = std::distance(numbers.begin(), it );

    sync_cout << std::boolalpha << is_found << std::endl;

    if (is_found) {

        sync_cout << ind << " " << numbers[ind] << std::endl;
    }

    return 0;
}