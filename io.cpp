#include "io.hpp"

IO::IO(connection_hdl &x) : hdl(x), sentPing(false), sentHello(false), locked(false), screen_width(0), screen_height(0), memberId(0), roomId(0), ip("") {}

void IO::lock() {
    locked = true;
}

void IO::unlock() {
    locked = false;
}

bool IO::isInGame() {
    return roomId > 0 && memberId > 0 && sentHello && sentPing;
}

bool IO::didSendHello() {
    return sentHello && sentPing;
}

void IO::leave() {
    roomId = 0;
    memberId = 0;
}
