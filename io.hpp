#ifndef IO_HPP
#define IO_HPP

#include <websocketpp/common/connection_hdl.hpp>
typedef websocketpp::connection_hdl connection_hdl;

class IO {
public:
    IO(connection_hdl &hdl);
    bool sentHello;
    bool sentPing;
    bool locked;
    uint16_t screen_width, screen_height;
    uint32_t memberId, roomId;

    void lock();
    void unlock();
private:
    connection_hdl &hdl;
    std::string ip;
}

#endif //IO_HPP
