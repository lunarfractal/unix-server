#ifndef FILE_HPP
#define FILE_HPP

#include <cstdint>

struct File {
    std::vector<uint16_t> nick, content;
    double timestamp;
    uint32_t parentId, ownerId;
    bool executable;
};

#endif //FILE_CPP
