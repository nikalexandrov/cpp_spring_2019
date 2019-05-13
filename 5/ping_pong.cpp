#include <thread>
#include <atomic>
#include <iostream>

static const int max_count = 500000;

std::atomic<bool> fl;

void ping_pong(const char *str, bool value) {
    for(int i = 0; i < max_count; i++) {
        while(fl == value);
        std::cout << str << std::endl;
        fl = value;
    }
}

int main() {
    fl = false;

    std::thread t1(ping_pong, "ping", true);
    std::thread t2(ping_pong, "pong", false);

    t2.join();
    t1.join();
    
    return 0;
}