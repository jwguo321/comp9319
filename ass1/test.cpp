#include <iostream>
#include <memory>

int main() {
    std::unique_ptr<double[]> test = std::make_unique<double[]>(5);
    for (auto i = 0; i < 6; i++)
        test[i] = i*2.0;
    for (auto i = 0; i < 5; i++)
        std::cout << test[i] << "\n";
    return 0;
}