#ifndef IO_HPP
#define IO_HPP

#include <websocketpp/common/connection_hdl.hpp>
typedef websocketpp::connection_hdl connection_hdl;

class IO {
public:
    IO();
    bool sentHello;
    bool sentPing;
    bool locked;
    uint16_t screen_width, screen_height;
    uint32_t memberId, roomId;
    uint16_t pingCount, pongCount;

    void lock();
    void unlock();
    bool isInGame();
    bool didSendHello();
    void leave();
private:
};

#endif //IO_HPP
