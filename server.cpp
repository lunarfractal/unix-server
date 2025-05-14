#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstring>

#include "unix.hpp"
#include "utils.hpp"

#define NGINX_PORT 9091
#define WEBSOCKETPP_PORT 8081

#define OPCODE_CS_PING 0x00
#define OPCODE_CS_PONG 0x01
#define OPCODE_SCREEN 0x02
#define OPCODE_ENTER_ROOM 0x03
#define OPCODE_LEAVE_ROOM 0x04
#define OPCODE_CURSOR 0x05
#define OPCODE_CLICK 0x06
#define OPCODE_NICK 0x07
#define OPCODE_COLOR 0x08
#define OPCODE_DISPATCH 0x09

#define OPCODE_SC_PING 0x00
#define OPCODE_SC_PONG 0x01
#define OPCODE_CONFIG 0xA0
#define OPCODE_ENTERED_ROOM 0xA1
#define OPCODE_INFO 0xB0
#define OPCODE_EVENTS 0xA2

#define FLAG_CURSOR 0x00

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;
typedef server::message_ptr message_ptr;

struct ws_hdl {
    bool sentHello;
    bool sentPing;
    uint16_t screen_width, screen_height;
    uint32_t memberId;
    uint32_t roomId;
    connection_hdl hdl;
};

struct connection_hdl_hash {
    std::size_t operator()(const websocketpp::connection_hdl& hdl) const {
        return std::hash<std::uintptr_t>()(
            reinterpret_cast<std::uintptr_t>(hdl.lock().get())
        );
    }
};

struct connection_hdl_equal {
    bool operator()(const websocketpp::connection_hdl& lhs, const websocketpp::connection_hdl& rhs) const {
        return !lhs.owner_before(rhs) && !rhs.owner_before(lhs);  // compares weak_ptr identity
    }
};

class WebSocketServer {
    public:
        WebSocketServer() {
            m_server.init_asio();
    
            m_server.set_open_handler(bind(&WebSocketServer::on_open,this,::_1));
            m_server.set_close_handler(bind(&WebSocketServer::on_close,this,::_1));
            m_server.set_message_handler(bind(&WebSocketServer::on_message,this,::_1,::_2));

            m_server.clear_access_channels(websocketpp::log::alevel::all);
        }

        void ping(connection_hdl hdl) {
            uint8_t buffer[] = {OPCODE_SC_PING};
            try {
                m_server.send(hdl, buffer, sizeof(buffer), websocketpp::frame::opcode::binary);
            } catch (websocketpp::exception const & e) {
                std::cout << "Ping failed because: "
                    << "(" << e.what() << ")" << std::endl;
            }
        }

        void pong(connection_hdl hdl) {
            uint8_t buffer[] = {OPCODE_SC_PONG};
            try {
                m_server.send(hdl, buffer, sizeof(buffer), websocketpp::frame::opcode::binary);
            } catch (websocketpp::exception const & e) {
                std::cout << "Pong failed because: "
                    << "(" << e.what() << ")" << std::endl;
            }
        }

        void sendId(connection_hdl hdl, uint32_t id) {
            uint8_t buffer[5];
            buffer[0] = OPCODE_ENTERED_ROOM;
            std::memcpy(&buffer[1], &id, sizeof(uint32_t));
            try {
                m_server.send(hdl, buffer, sizeof(buffer), websocketpp::frame::opcode::binary);
            } catch (websocketpp::exception const & e) {
                std::cout << "Send failed because: "
                    << "(" << e.what() << ")" << std::endl;
            }
        }

        void sendAll(std::vector<uint8_t> &buffer) {
            for (auto &pair: m_connections) {
                try {
                    if (
                        m_server.get_con_from_hdl(pair.first)->get_state() == websocketpp::session::state::open
                        && pair.second.roomId > 0
                        && pair.second.sentPing
                        && pair.second.sentHello
                    ) {
                        m_server.send(pair.first, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
                    }
                } catch (websocketpp::exception const & e) {
                    std::cout << "Send failed because: "
                        << "(" << e.what() << ")" << std::endl;
                }
            }
        }

        void sendBuffer(connection_hdl hdl, std::vector<uint8_t> &buffer) {
            try {
                m_server.send(hdl, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
            } catch (websocketpp::exception const & e) {
                std::cout << "Send failed because: "
                    << "(" << e.what() << ")" << std::endl;
            }
        }

        void processMessage(std::vector<uint8_t> &buffer, connection_hdl hdl) {
            uint8_t op = buffer[0];
            ws_hdl &ws = m_connections[hdl];
            switch(op) {
                case OPCODE_CS_PING:
                {
                    pong(hdl);
                    break;
                }
                case OPCODE_CS_PONG:
                {
                    std::cout << "Ping!" << std::endl;
                    break;
                }
                case OPCODE_SCREEN:
                {
                    if(buffer.size() >= 5) {
                        std::memcpy(&ws.screen_width, &buffer[1], sizeof(uint16_t));
                        std::memcpy(&ws.screen_height, &buffer[3], sizeof(uint16_t));
                    }
                    break;
                }
                case OPCODE_ENTER_ROOM:
                {
                    if(ws.roomId == 0 && buffer.size() > 1) {
                        ws.memberId = unix.addMember(buffer);
                        ws.roomId = 1000; // /home
                    }
                    break;
                }
                case OPCODE_LEAVE_ROOM:
                {
                    if(ws.roomId > 0) {
                        unix.deleteMember(ws.memberId);
                        ws.roomId = 0;
                    }
                    break;
                }
                case OPCODE_CURSOR:
                {
                    if(ws.roomId > 0) {
                        uint16_t x, y;
                        std::memcpy(&x, &buffer[1], sizeof(uint16_t));
                        std::memcpy(&y, &buffer[3], sizeof(uint16_t));
                        unix.setMemberCursor(ws.memberId, x, y);
                        // this just normalizes the value for different screen dimensions
                        x = x / ws.screen_width * 65535;
                        y = y / ws.screen_height * 65535;
                        std::vector<uint8_t> buffer(1 + 1 + 4 + 2 + 2);
                        buffer[0] = OPCODE_INFO;
                        buffer[1] = FLAG_CURSOR;
                        std::memcpy(&buffer[2], &ws.memberId, sizeof(uint32_t));
                        std::memcpy(&buffer[6], &x, sizeof(uint16_t));
                        std::memcpy(&buffer[8], &y, sizeof(uint16_t));
                        sendAll(buffer);
                    }
                    break;
                }
                case OPCODE_CLICK:
                {
                    if(ws.roomId > 0) {
                        unix.updateMemberClick(ws.memberId, buffer[1]);
                    }
                    break;
                }
                case OPCODE_DISPATCH:
                {
                    break;
                }
            }
        }

        void on_open(connection_hdl hdl) {
            ws_hdl ws; ws.hdl = hdl; ws.roomId = 0;
            m_connections[hdl] = ws;
        }
    
        void on_close(connection_hdl hdl) {
            ws_hdl &ws = m_connections[hdl];
            unix.deleteMember(ws.memberId);
            m_connections.erase(hdl);
        }
    
        void on_message(connection_hdl hdl, server::message_ptr msg) {
            if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
                std::vector<uint8_t> buffer(msg->get_payload().begin(), msg->get_payload().end());
                processMessage(buffer, hdl);
            } else {
                std::cout << "received text message from hdl: " << hdl.lock().get()
                    << " and message: " << msg->get_payload() << std::endl;
            }
        }
    
        void run(uint16_t port) {
            m_server.listen(port);
            m_server.start_accept();
            m_server.run();
        }
    private:
        server m_server;
        std::unordered_map<connection_hdl, ws_hdl, connection_hdl_hash, connection_hdl_equal> m_connections;
        Unix unix;
};

int main() {
    WebSocketServer wsServer;
    wsServer.run(WEBSOCKETPP_PORT);
}
