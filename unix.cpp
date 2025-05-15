#include "unix.hpp"

bool debug = true;

Unix::Unix(std::u16string n) : nick(n) {}

uint32_t Unix::addMember(std::vector<uint8_t> &buffer) {
    uint32_t id = getUniqueId();member_hdl m;int offset=1;getString(m.nick, buffer, offset);m.click=m.r=m.g=m.b=m.x=m.y=0;
    if(debug) {
        std::cout << "[Unix::addMember]: Added id: " << (int)id
            << std::endl;
    }
    return id;
}

void Unix::setMemberNick(uint32_t id, std::vector<uint8_t> &buffer) {
    member_hdl &m = m_members[id];int offset=1;getString(m.nick, buffer, offset);
    if(debug) {
        std::cout << "[Unix::setMemberNick]: Updated id: " << (int)id
            << std::endl;
    }
}


void Unix::setMemberColor(uint32_t id, std::vector<uint8_t> &buffer) {
    member_hdl &m = m_members[id];int offset=1;m.r=buffer[offset++];m.g=buffer[offset++];m.b=buffer[offset++];
    if(debug) {
        std::cout << "[Unix::setMemberColor]: Updated id: " << (int)id
            << " and color: " << (int)m.r << " " << (int)m.g << " " << (int)m.b
            << std::endl;
    }
}

void Unix::setMemberCursor(uint32_t id, uint16_t x, uint16_t y) {
    member_hdl &m = m_members[id];
    m.x = x; m.y = y;
    if(debug) {
        std::cout << "[Unix::setMemberCursor]: Set id: " << (int)id
            << " and coordinates: " << (int)m.x << " " << (int)m.y
            << std::endl;
    }
}

void Unix::setMemberClick(uint32_t id, uint8_t flag) {
    member_hdl &m = m_members[id];m.click=flag;
    if(debug) {
        std::cout << "[Unix::setMemberClick]: Set id: " << (int)id
            << " and click: " << (int)m.click
            << std::endl;
    }
}

void Unix::deleteMember(uint32_t id) {
    if(debug) {
        member_hdl &m = m_members[id];
        std::cout << "[Unix::deleteMember]: Deleting id: " << (int)id
            << std::endl;
    }
    m_members.erase(id);
}

member_hdl Unix::getMember(uint32_t id) {
    return m_members[id];
}

void Unix::getConfigCursors(std::vector<uint8_t> &buffer) {
    buffer[0] = 0xA0;
    buffer[1] = 0x00;

    int offset = 2;
    for(auto &pair: m_members) {
        int requiredSize = offset + 4 + 2 + 2 + 2 * pair.second.nick.length() + 2 + 3;
        buffer.resize(requiredSize);
        std::memcpy(&buffer[offset], &pair.first, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        std::memcpy(&buffer[offset], &pair.second.x, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        std::memcpy(&buffer[offset], &pair.second.y, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        std::memcpy(&buffer[offset], pair.second.nick.data(), 2 * pair.second.nick.length());
        offset += 2 * pair.second.nick.length();
        uint16_t nt = 0x00;
        std::memcpy(&buffer[offset], &nt, sizeof(uint16_t));
        offset += 2;
        buffer[offset++] = pair.second.r;buffer[offset++] = pair.second.g;buffer[offset++] = pair.second.b;
    }
    buffer.resize(offset + 4);
    uint32_t nt = 0x00;
    std::memcpy(&buffer[offset], &nt, sizeof(uint32_t));
}

void Unix::getConfigDirectories(std::vector<uint8_t> &buffer) {
    buffer[0] = 0xA0;

    int offset = 1;
    for(auto &pair: m_directories) {
        int requiredSize = offset + 4 + (2 * pair.second.nick.length()) + 2 + 4;
        buffer.resize(requiredSize);
        std::memcpy(&buffer[offset], &pair.first, sizeof(uint32_t));
        offset += 4;
        std::memcpy(&buffer[offset], pair.second.nick.data(), 2 * pair.second.nick.length());
        offset += 2 * pair.second.nick.length();
        uint16_t nt = 0x00;
        std::memcpy(&buffer[offset], &nt, sizeof(uint16_t));
        offset += 2;
        std::memcpy(&buffer[offset], &pair.second.parentId, sizeof(uint32_t));
        offset += 4;
    }
    uint32_t nt = 0x00;
    std::memcpy(&buffer[offset], &nt, sizeof(uint32_t));
}

