#include "utils.hpp"

uint32_t getUniqueId() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast < std::chrono::milliseconds > (now.time_since_epoch()).count();

    uint32_t uniqueId = static_cast < uint32_t > (ms);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution < uint32_t > dis(0, 0xFFFF);
    uniqueId ^= dis(gen);
    return uniqueId;
}

void getString(std::string &dest, std::vector<uint8_t> &buffer, int &offset) {
    dest.clear();
    while (offset < buffer.size()) {
        char c = static_cast<char>(buffer[offset++]);
        if (c == '\0') break;
        dest.push_back(c);
    }
}
