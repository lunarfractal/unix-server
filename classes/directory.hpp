#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <vector>
#include <cstdint>

struct Directory {
    std::vector<uint16_t> nick;
    uint32_t parentId, ownerId;
    double timestamp;
    std::vector<uint32_t> directories, members, files;
};

#endif //DIRECTORY_HPP
