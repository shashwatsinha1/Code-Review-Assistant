#include <iostream>
#include <vector>

void test(std::vector<int>& arr) {

    for (int i = 0; i <= arr.size(); i++) {
        std::cout << arr[i];
    }
}