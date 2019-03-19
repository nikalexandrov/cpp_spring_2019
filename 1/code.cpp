#include <iostream>
#include "numbers.dat" // Data - массив, Size - размер массива

__attribute__((always_inline)) bool is_prime(int n) {
    if(n != 1 && n != 2 && n % 2 != 0) {
        for(int i = 3; i * i <= n; i += 2)
            if(n % i == 0)
                return false;
    }else if(n == 2)
        return true;
    else if(n == 1 || n % 2 == 0)
        return false;
    return true;
}

int prime_counter(int left, int right) {
    int counter = 0;
    for(int i = 0; i < Size && Data[i] <= right; i++) {
        if(Data[i] >= left && is_prime(Data[i])) {
            counter++;
            while(i < Size - 1 && Data[i+1] == Data[i]){
                i++;
                counter++;
            }
        } else {
            while(i < Size - 1 && Data[i+1] == Data[i]) {
                i++;
            }
        }
    }
    return counter;
}

int main(int argc, char* argv[]) {
    if(argc != 1 && argc % 2 != 0) {
        for(int i = 1; i < argc; i += 2)
            std::cout << prime_counter(std::atoi(argv[i]), std::atoi(argv[i+1])) << std::endl;
        return 0;
    }
    return -1;
}
