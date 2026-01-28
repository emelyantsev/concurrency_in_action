#include <vector>
#include <execution>
#include <unordered_set>
#include <thread>
#include <iostream>


int main() {


    std::vector<int> numbers(10'000'000);

    for (int i = 0; i < numbers.size(); ++i) {

        numbers[i] = -i;
    }

    std::unordered_set<std::thread::id> ids;

    std::sort( std::execution::par, numbers.begin(), numbers.end(),

        [&ids] (int l, int r) {

            ids.insert( std::this_thread::get_id() ) ;

            return l < r;
        }
    ) ;

    std::cout << numbers.front() << " " << numbers.back() << std::endl;

    std::cout << ids.size() << std::endl;


    return 0;

}