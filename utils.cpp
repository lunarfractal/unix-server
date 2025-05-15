#include "utils.hpp"

void getString(std::vector<uint16_t> &dest, std::vector<uint8_t> &buffer, int &offset) {
    dest.clear();
    const size_t len = buffer.size();

    while (offset + 1 < len) {
        uint16_t c;
        std::memcpy(&c, &buffer[offset], sizeof(uint16_t));
        offset += 2;
        if (c == 0x00) break;
        dest.push_back(c);
    }
}

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
