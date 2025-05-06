#ifndef CHATROOM_HPP
#define CHATROOM_HPP

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

#include "utils.hpp"

struct member {
    std::string nick;
    uint8_t click;
    uint8_t r, g, b;
    uint16_t x, y;
};

struct message {
    std::string content;
    double timestamp;
    member author;
};


class ChatRoom {
public:
    ChatRoom(std::string n);

    std::string name;

    std::unordered_map<uint32_t, member> members;
    std::unordered_map<uint32_t, message> messages;

    uint32_t addMember(std::vector<uint8_t> &buffer);
    void updateMember(uint32_t id, std::vector<uint8_t> &buffer);
    void updateMemberMouse(uint32_t id, std::vector<uint8_t> &buffer);
    void updateMemberClick(uint32_t id, std::vector<uint8_t> &buffer);
    void deleteMember(uint32_t id);

    uint32_t addMessage(std::vector<uint8_t> &buffer);
    void updateMessage(uint32_t id, std::vector<uint8_t> &buffer);
    void deleteMessage(uint32_t id);

    void encodeMember(uint32_t id, std::vector<uint8_t> &buffer, int &offset);
    void encodeMember(uint32_t id, member &m, std::vector<uint8_t> &buffer, int &offset);
    void encodeAllMembers(std::vector<uint8_t> &buffer, int &offset);

    void encodeMessage(uint32_t id, std::vector<uint8_t> &buffer, int &offset);
    void encodeAllMessages(std::vector<uint8_t> &buffer, int &offset);

private:
}

#endif //CHATROOM_HPP
