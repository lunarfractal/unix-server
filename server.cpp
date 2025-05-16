#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstring>

#include "unix.hpp"
#include "utils.hpp"
#include "opcodes.hpp"
#include "io.hpp"

#define NGINX_PORT 9091
#define WEBSOCKETPP_PORT 8081

#undef unix

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;
typedef server::message_ptr message_ptr;


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
                        && pair.second.isInGame()
                    ) {
                        m_server.send(pair.first, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
                    }
                } catch (websocketpp::exception const & e) {
                    std::cout << "Send failed because: "
                        << "(" << e.what() << ")" << std::endl;
                }
            }
        }

        void sendToRoom(uint32_t roomId, std::vector<uint8_t> &buffer) {
            for (auto &pair: m_connections) {
                try {
                    if (
                        m_server.get_con_from_hdl(pair.first)->get_state() == websocketpp::session::state::open
                        && pair.second.roomId == roomId
                        && pair.second.isInGame()
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

        void sendConfigCursors(connection_hdl hdl) {
            std::vector<uint8_t> buffer(10);
            unix.getConfigCursors(buffer);
            sendBuffer(hdl, buffer);
        }

        void processMessage(std::vector<uint8_t> &buffer, connection_hdl hdl) {
            uint8_t op = buffer[0];
            IO &ws = m_connections[hdl];
            switch(op) {
                case OPCODE_CS_PING:
                {
                    pong(hdl);
                    break;
                }
                case OPCODE_CS_PONG:
                {
                    std::cout << "Ping!" << std::endl;
                    sendConfigCursors(hdl);
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
                        sendId(hdl, ws.memberId);
                        ping(hdl);
                        // add cursor to everyone's screen
                        member_hdl m = unix.getMember(ws.memberId);
                        std::vector<uint8_t> buffer(1+1+4+1+2+2+m.nick.size()+2+3+4);
                        buffer[0] = OPCODE_EVENTS;
                        buffer[1] = FLAG_CURSOR;
                        int offset = 2;
                        std::memcpy(&buffer[offset], &ws.memberId, sizeof(uint32_t));
                        offset += sizeof(uint32_t);
                        buffer[offset++] = EVENT_CURSOR_ADD;
                        std::memcpy(&buffer[offset], &m.x, sizeof(uint16_t));
                        offset += sizeof(uint16_t);
                        std::memcpy(&buffer[offset], &m.y, sizeof(uint16_t));
                        offset += sizeof(uint16_t);
                        std::memcpy(&buffer[offset], m.nick.data(), m.nick.size());
                        offset += m.nick.size();
                        uint16_t nt = 0x00;
                        std::memcpy(&buffer[offset], &nt, sizeof(uint16_t));
                        offset += sizeof(uint16_t);
                        buffer[offset++] = m.r;buffer[offset++] = m.g;buffer[offset++] = m.b;
                        uint32_t unt = 0x00;
                        std::memcpy(&buffer[offset], &unt, sizeof(uint32_t));
                        sendToRoom(ws.roomId, buffer);
                    }
                    break;
                }
                case OPCODE_LEAVE_ROOM:
                {
                    if(ws.roomId > 0) {
                        unix.deleteMember(ws.memberId);
                        std::vector<uint8_t> buffer(1+1+4+1+4);
                        buffer[0] = OPCODE_EVENTS;
                        buffer[1] = FLAG_CURSOR;
                        std::memcpy(&buffer[2], &ws.memberId, sizeof(uint32_t));
                        buffer[6] = EVENT_CURSOR_DELETE;
                        uint32_t nt = 0x00;
                        std::memcpy(&buffer[7], &nt, sizeof(uint32_t));
                        sendToRoom(ws.roomId, buffer);
                        ws.leave();
                    }
                    break;
                }
                case OPCODE_CURSOR:
                {
                    if(ws.roomId > 0) {
                        uint16_t x, y;
                        std::memcpy(&x, &buffer[1], sizeof(uint16_t));
                        std::memcpy(&y, &buffer[3], sizeof(uint16_t));
                        // this just normalizes the value for different screen dimensions
                        x = (x * 65535) / ws.screen_width;
                        y = (y * 65535) / ws.screen_height;
                        unix.setMemberCursor(ws.memberId, x, y);
                        std::vector<uint8_t> buffer(1 + 1 + 4 + 2 + 2);
                        buffer[0] = OPCODE_INFO;
                        buffer[1] = FLAG_CURSOR;
                        std::memcpy(&buffer[2], &ws.memberId, sizeof(uint32_t));
                        std::memcpy(&buffer[6], &x, sizeof(uint16_t));
                        std::memcpy(&buffer[8], &y, sizeof(uint16_t));
                        sendToRoom(ws.roomId, buffer);
                    }
                    break;
                }
                case OPCODE_CLICK:
                {
                    if(ws.roomId > 0) {
                        unix.setMemberClick(ws.memberId, buffer[1]);
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
            IO ws(hdl);
            m_connections[hdl] = ws;
        }
    
        void on_close(connection_hdl hdl) {
            IO &ws = m_connections[hdl];
            unix.deleteMember(ws.memberId);
            std::vector<uint8_t> buffer(1+1+4+1+4);
            buffer[0] = OPCODE_EVENTS;
            buffer[1] = FLAG_CURSOR;
            std::memcpy(&buffer[2], &ws.memberId, sizeof(uint32_t));
            buffer[6] = EVENT_CURSOR_DELETE;
            uint32_t nt = 0x00;
            std::memcpy(&buffer[7], &nt, sizeof(uint32_t));
            sendToRoom(ws.roomId, buffer);
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
        typedef struct {std::size_t operator()(const websocketpp::connection_hdl& hdl) const {return std::hash<std::uintptr_t>()(reinterpret_cast<std::uintptr_t>(hdl.lock().get()));}} connection_hdl_hash;
        typedef struct {bool operator()(const websocketpp::connection_hdl& lhs, const websocketpp::connection_hdl& rhs) const {return !lhs.owner_before(rhs) && !rhs.owner_before(lhs); }} connection_hdl_equal;
        std::unordered_map<connection_hdl, IO, connection_hdl_hash, connection_hdl_equal> m_connections;
        Unix unix;
};

int main() {
    WebSocketServer wsServer;
    wsServer.run(WEBSOCKETPP_PORT);
}
