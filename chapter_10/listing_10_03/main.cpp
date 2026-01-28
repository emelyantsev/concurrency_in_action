#include <vector>
#include <execution>
#include <cmath>
#include <chrono>
#include <iostream>


inline double heavy_compute(double x) {
   
    for (int i = 0; i < 500; ++i) {
        x = std::sin(x) + std::cos(x);
    }
   
    return x;
}


int main() {

    const size_t N = 50'000'000;

    std::vector<double> data(N, 1.0);

    auto start = std::chrono::steady_clock::now();

    std::for_each(
        std::execution::par,
        data.begin(), data.end(),
        [](double& x) {
            x = heavy_compute(x);
        }
    );

    auto end = std::chrono::steady_clock::now();

    std::cout << "seq time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";
}
