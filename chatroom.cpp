#include "chatroom.hpp"
#include <cstring>
#include <iostream>
#include <cstdbool>

bool debug = true;

ChatRoom::ChatRoom(std::string n) : name(n) {}

uint32_t ChatRoom::addMember(std::vector<uint8_t> &buffer) {
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

void ChatRoom::updateMember(uint32_t id, std::vector<uint8_t> &buffer) {
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

void ChatRoom::updateMemberMouse(uint32_t id, std::vector<uint8_t> &buffer) {
    member &m = members[id];
    if(buffer.size() >= 5) {
        std::memcpy(&m.x, &buffer[1], sizeof(uint16_t));
        std::memcpy(&m.y, &buffer[3], sizeof(uint16_t));
    }

    if(debug) {
        std::cout << "[updateMemberMouse] Updating member with id: " << (int)id << " and nick: "
            << m.nick << " mouse position: " << (int)m.x << " " << (int)m.y
            << std::endl;
    }
}

void ChatRoom::updateMemberClick(uint32_t id, std::vector<uint8_t> &buffer) {
    member &m = members[id];
    if(buffer.size() >= 2) {
        m.click = buffer[1];
    }

    if(debug) {
        std::cout << "[updateMemberClick] Updating member with id: " << (int)id << " and nick: "
            << m.nick << " click: " << (int)m.click << std::endl;
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
    auto it = members.find(id);
    if (it == members.end()) return;
    member &m = it->second;

    size_t requiredSize = offset + 4 + m.nick.length() + 1 + 3 + 4 + 1;
    buffer.resize(buffer.size() + requiredSize);

    std::memcpy(&buffer[offset], &id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    std::memcpy(&buffer[offset], m.nick.c_str(), m.nick.length() + 1);
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
    size_t requiredSize = offset + 4 + m.nick.length() + 1 + 3 + 4 + 1;
    
    size_t requiredSize = offset + 4 + m.nick.length() + 1 + 3 + 4 + 1;
    buffer.resize(buffer.size() + requiredSize);

    std::memcpy(&buffer[offset], &id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    std::memcpy(&buffer[offset], m.nick.c_str(), m.nick.length() + 1);
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
