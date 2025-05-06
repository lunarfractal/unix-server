#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <random>

uint32_t getUniqueId();
void getString(std::string &dest, uint8_t *buffer, size_t size, int &offset);

#endif //UTILS_HPP