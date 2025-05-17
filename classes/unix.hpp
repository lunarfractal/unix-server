#ifndef UNIX_HPP
#define UNIX_HPP

#include <unordered_map>
#include <vector>

#include "member.hpp"
#include "directory.hpp"
#include "file.hpp"

struct UnixSystem {
    std::vector<uint16_t> nick;
    std::unordered_map<uint32_t, Member> members;
    std::unordered_map<uint32_t, Directory> directories;
};

#endif //UNIX_HPP
