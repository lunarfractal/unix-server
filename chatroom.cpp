#include "chatroom.hpp"
#include <cstring>
#include <iostream>
#include <cstdbool>

bool debug = true;

ChatRoom::ChatRoom(std::string n) : name(n) {}

uint32_t ChatRoom::addMember(uint8_t *buffer) {
    member m;

    uint32_t id = getUniqueId();
    members[id] = m;

    int offset = 1;

    getString(m.nick, offset);
    m.r = buffer[offset++];
    m.g = buffer[offset++];
    m.b = buffer[offset++];

   if(debug) {
        std::cout << "[addMember] Adding member with id: " << (int)id << " and nick: "
            << m.nick << " plus rgb values: " << (int)m.r << " " << (int)m.g << " " << (int)m.b
            << std::endl
            << "Size of member map: " << members.size()
            << std::endl;
    }

   return id;
}

void ChatRoom::updateMember(uint32_t id, uint8_t *buffer) {
    member &m = members[id];
    int offset = 1;
    getString(m.nick, offset);
    m.r = buffer[offset++];
    m.g = buffer[offset++];
    m.b = buffer[offset++];

    if(debug) {
        std::cout << "[updateMember] Updating member with id: " << (int)id << " and nick: "
            << m.nick << " plus rgb values: " << (int)m.r << " " << (int)m.g << " " << (int)m.b
            << std::endl;
    }
}

void ChatRoom::deleteMember(uint32_t id) {
    members.erase(id);

    if(debug) {
        std::cout << "[deleteMember] Deleting member with id: " << (int)id << " and nick: "
            << m.nick << std::endl
            << "Size of member map: " << members.size()
            << std::endl;
    }
}

void ChatRoom::encodeMember(uint32_t id, std::vector<uint8_t> &buffer, int &offset) {
    member &m = members[id];
    
    std::memcpy(&buffer[offset], &id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    std::memcpy(&buffer[offset], &m.nick.c_str(), m.nick.length() + 1);
    offset += m.nick.length() + 1;

    buffer[offset++] = m.r;
    buffer[offset++] = m.g;
    buffer[offset++] = m.b;

    std::memcpy(&buffer[offset], &m.x, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    std::memcpy(&buffer[offset], &m.y, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    buffer[offset++] = m.click;
}

void ChatRoom::encodeMember(uint32_t id, member &m, std::vector<uint8_t> &buffer, int &offset) {
    std::memcpy(&buffer[offset], &id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    std::memcpy(&buffer[offset], &m.nick.c_str(), m.nick.length() + 1);
    offset += m.nick.length() + 1;

    buffer[offset++] = m.r;
    buffer[offset++] = m.g;
    buffer[offset++] = m.b;

    std::memcpy(&buffer[offset], &m.x, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    std::memcpy(&buffer[offset], &m.y, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    buffer[offset++] = m.click;
}

void ChatRoom::encodeAllMembers(std::vector<uint8_t> &buffer, int &offset) {
    for(auto &pair : members) {
        encodeMember(pair.first, pair.second, buffer, offset);
    }
}
