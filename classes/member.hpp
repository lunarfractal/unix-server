#ifndef MEMBER_HPP
#define MEMBER_HPP

#include <cstdint>
#include <vector>

struct Member {
    std::vector<uint16_t> nick;
    uint16_t x, y;
    uint8_t click;
    uint8_t r,g,b;
};

#endif //MEMBER_HPP
