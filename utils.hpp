#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <chrono>
#include <random>

uint32_t getUniqueId();
void getString(std::vector<uint16_t> &dest, std::vector<uint8_t> &buffer, int &offset);

#endif //UTILS_HPP
