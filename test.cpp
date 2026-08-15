#include <iostream>
#include <vector>

void example(std::vector<int>& arr) {

    for (int i = 0; i <= arr.size(); i++) {

        for (int j = 0; j < arr.size(); j++) {

            std::cout << i + j;
        }
    }

    int x = arr[0];
}