#include <random>

int random_int() {
    static std::random_device device;
    static std::uniform_int_distribution<int> distribution(-1000, 1000);
    return distribution(device);
}
