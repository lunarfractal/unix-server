#ifndef OPCODES_HPP
#define OPCODES_HPP

#include <cstdint>

// Opcodes
// Client -> Server
constexpr uint8_t OPCODE_CS_PING = 0x00;
constexpr uint8_t OPCODE_CS_PONG = 0x01;
constexpr uint8_t OPCODE_DIMENSIONS = 0x02;
constexpr uint8_t OPCODE_ENTER_GAME = 0x03;
constexpr uint8_t OPCODE_LEAVE_GAME = 0x04;
constexpr uint8_t OPCODE_CURSOR = 0x05;
constexpr uint8_t OPCODE_CLICK = 0x06;
constexpr uint8_t OPCODE_NICK = 0x07;
constexpr uint8_t OPCODE_COLOR = 0x08;
constexpr uint8_t OPCODE_DISPATCH = 0x09;

// Server -> Client
constexpr uint8_t OPCODE_SC_PING = 0x00;
constexpr uint8_t OPCODE_SC_PONG = 0x01;
constexpr uint8_t OPCODE_CONFIG = 0xA0;
constexpr uint8_t OPCODE_ENTERED_GAME = 0xA1;
constexpr uint8_t OPCODE_INFO = 0xB0;
constexpr uint8_t OPCODE_EVENTS = 0xA2;

// Flags
// Server -> Client
constexpr uint8_t FLAG_CURSOR = 0x11;
constexpr uint8_t FLAG_DIRECTORY = 0x12;
constexpr uint8_t FLAG_FILE = 0x13;

// Events
// Client -> Server
constexpr uint8_t DISPATCH_CHANGE_DIRECTORY = 0x01;
constexpr uint8_t DISPATCH_LIST_DIRECTORY = 0x02;

// Server -> Client
constexpr uint8_t EVENT_CURSOR_ADD = 0x00;
constexpr uint8_t EVENT_CURSOR_DELETE = 0x01;
constexpr uint8_t EVENT_CHANGE_DIRECTORY = 0x02;

#endif //OPCODES_HPP
