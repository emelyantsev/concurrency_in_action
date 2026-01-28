#include <vector>
#include <execution>
#include <unordered_set>
#include <thread>
#include <iostream>


int main() {


    std::vector<int> numbers(1'000'000, 0);

    // for (int i = 0; i < numbers.size(); ++i) {

    //     numbers[i] = -i;
    // }

    std::mutex m;
    std::unordered_set<std::thread::id> ids;


    bool res = std::all_of( std::execution::par, numbers.begin(), numbers.end(),

        [&](int num) {

            thread_local bool recorded = false;

            if ( !recorded ) {
                
                std::lock_guard<std::mutex> lock(m);
                ids.insert(std::this_thread::get_id());
                recorded = true;
            }

            // делаем работу
            volatile int x = 0;
            
            for (int i = 0; i < 1000; ++i)
                x += i;

            return num == 0;
        }
    );


    std::cout << std::boolalpha << res << std::endl;

    std::cout << ids.size() << std::endl;


    return 0;

}