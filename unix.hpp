#ifndef UNIX_HPP
#define UNIX_HPP

#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <vector>

#include "utils.hpp"

bool debug = true;

typedef struct {std::u16string nick;uint8_t click,r,g,b;uint16_t x,y;} member_hdl;
typedef struct {uint32_t authorId;std::u16string content;uint8_t type;double timestamp;} message_hdl;
typedef struct {uint32_t parentId;std::u16string nick;std::unordered_map<uint32_t, message_hdl> messages;} directory_hdl;

class Unix {
public:
    Unix(std::u16string n) : nick(n) {}

    std::u16string nick;

    uint32_t addMember(std::vector<uint8_t> &buffer) {
        uint32_t id = getUniqueId();member_hdl m;int offset=1;getString(m.nick, buffer, offset);m.click=m.r=m.g=m.b=m.x=m.y=0;
        if(debug) {
            std::cout << "[Unix::addMember]: Added " << m.nick << " with id: " << (int)id
                << " and color: " << << (int)m.r << " " << (int)m.g << " " << (int)m.b
                << " and coordinates: " << (int)m.x << " " << (int)m.y
                << " and click: " << (int)m.click
                << std::endl;
        }
        return id;
    }

    void updateMemberNick(uint32_t id, std::vector<uint8_t> &buffer) {
        member_hdl &m = m_members[id];int offset=1;getString(m.nick, offset);
        if(debug) {
            std::cout << "[Unix::updateMemberNick]: Updated " << m.nick
                << " with id: " << (int)id
                << std::endl;
        }
    }

    void updateMemberColor(uint32_t id, std::vector<uint8_t> &buffer) {
        member_hdl &m = m_members[id];int offset=1;m.r=buffer[offset++];m.g=buffer[offset++];m.b=buffer[offset++];
        if(debug) {
            std::cout << "[Unix::updateMemberColor]: Updated " << m.nick << " with id: " << (int)id
                << " and color: " << << (int)m.r << " " << (int)m.g << " " << (int)m.b
                << std::endl;
        }
    }

    void setMemberMouse(uint32_t id, std::vector<uint8_t> &buffer) {
        member_hdl &m = m_members[id];int offset=1;
        std::memcpy(&m.x, &buffer[offset], sizeof(uint16_t));offset+=sizeof(uint16_t);
        std::memcpy(&m.y, &buffer[offset], sizeof(uint16_t));
        if(debug) {
            std::cout << "[Unix::setMemberMouse]: Set " << m.nick << " with id: " << (int)id
                << " and coordinates: " << (int)m.x << " " << (int)m.y
                << std::endl;
        }
    }

    void setMemberClick(uint32_t id, uint8_t flag) {
        member_hdl &m = m_members[id];m.click=flag;
        if(debug) {
            std::cout << "[Unix::setMemberClick]: Set " << m.nick << " with id: " << (int)id
                << " and click: " << (int)m.click
                << std::endl;
        }
    }

    void deleteMember(uint32_t id) {
        if(debug) {
            member_hdl &m = m_members[id];
            std::cout << "[Unix::deleteMember]: Deleting " << m.nick << " with id: " << (int)id
                << std::endl;
        }
        m_members.erase(id);
    }

    void getConfig(std::vector<uint8_t> &buffer) {
        buffer[0] = OPCODE_CONFIG;
        
    }

private:
    std::unordered_map<uint32_t, member_hdl> m_members;
    std::unordered_map<uint32_t, directory_hdl> m_directories;
};

#endif //UNIX_HPP
