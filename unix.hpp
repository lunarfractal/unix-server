#ifndef UNIX_HPP
#define UNIX_HPP

#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <vector>

#include "utils.hpp"

typedef std::vector<uint16_t> u16string;

typedef struct {u16string nick;uint8_t click,r,g,b;uint16_t x,y;} member_hdl;
typedef struct {uint32_t authorId;u16string content;uint8_t type;double timestamp;} message_hdl;
typedef struct {uint32_t parentId;u16string nick;std::unordered_map<uint32_t, message_hdl> messages;} directory_hdl;

class Unix {
public:
    Unix(u16string n);

    u16string nick;

    uint32_t addMember(std::vector<uint8_t> &buffer);

    void setMemberNick(uint32_t id, std::vector<uint8_t> &buffer);

    void setMemberColor(uint32_t id, std::vector<uint8_t> &buffer);

    void setMemberCursor(uint32_t id, uint16_t x, uint16_t y);

    void setMemberClick(uint32_t id, uint8_t flag);

    void deleteMember(uint32_t id);

    member_hdl getMember(uint32_t id);

    void getConfigCursors(std::vector<uint8_t> &buffer);
    void getConfigDirectories(std::vector<uint8_t> &buffer);
private:
    std::unordered_map<uint32_t, member_hdl> m_members;
    std::unordered_map<uint32_t, directory_hdl> m_directories;
};

#endif //UNIX_HPP
