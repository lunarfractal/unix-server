#include <vector>
#include "utils.hpp"
#include <iostream>
int main() {
    std::u16string m;
    int offset = 1;
    std::vector<uint8_t> buffer = {1, 2, 3, 4,5 ,6, 7, 8, 9};
    getString(m, buffer, offset);
    std::cout <<(int)getUniqueId() << std::endl;
    return 0;
}
