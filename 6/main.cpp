#include "MultiThreadSorter.h"

int main() {
    try {
    MultiThreadSorter sorter("in", 2, 6*1024*1024);
    sorter.sort();
    } catch (std::logic_error err) {
        std::cerr << "An Error occured. " << err.what() << std:: endl;
    }
	return 0;
}