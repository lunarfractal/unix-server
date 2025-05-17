#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstring>

#include "classes/unix.hpp"
#include "utils.hpp"
#include "opcodes.hpp"
#include "io.hpp"

#define NGINX_PORT 9091
#define WEBSOCKETPP_PORT 8081

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
            buffer[0] = OPCODE_ENTERED_GAME;
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
            buffer[0] = OPCODE_CONFIG;
            buffer[1] = FLAG_CURSOR;
            int offset = 2;
            for(auto &pair: unixSystem.members) {
                buffer.resize(offset + 4 + 2 + 2 + 2 * pair.second.nick.size() + 2 + 3);
                std::memcpy(&buffer[offset], &pair.first, sizeof(uint32_t));
                offset += 4;
                std::memcpy(&buffer[offset], &pair.second.x, sizeof(uint16_t));
                offset += 2;
                std::memcpy(&buffer[offset], &pair.second.y, sizeof(uint16_t));
                offset += 2;
                std::memcpy(&buffer[offset], pair.second.nick.data(), 2 * pair.second.nick.size());
                offset += 2 * pair.second.nick.size();
                uint16_t nt = 0x00;
                std::memcpy(&buffer[offset], &nt, sizeof(uint16_t));
                offset += 2;
                buffer[offset++] = pair.second.r;buffer[offset++] = pair.second.g;buffer[offset++] = pair.second.b;
            }
            buffer.resize(offset + 4);
            uint32_t unt = 0x00;
            std::memcpy(&buffer[offset], &unt, sizeof(uint32_t));
            sendBuffer(hdl, buffer);
        }

        void processDispatch(std::vector<uint8_t> &buffer, connection_hdl hdl, IO &io) {
            uint8_t _byte = buffer[1];

            int offset = 2;
            switch(_byte) {
                case DISPATCH_CHANGE_DIRECTORY:
                {
                    if(io.isInGame() && buffer.size() >= 5) {
                        uint32_t originalRoomId = io.roomId;
                        uint32_t directoryId;
                        std::memcpy(&directoryId, &buffer[offset], sizeof(uint32_t));
                        offset += 4;
                        if(unixSystem.directories.find(directoryId) != unixSystem.directories.end()) {
                            io.roomId = directoryId; // is it that simple? probably
                            // now send it to everyone
                            std::vector<uint8_t> buffer(1+1+4+1+4);
                            buffer[0] = OPCODE_EVENTS;
                            buffer[1] = FLAG_CURSOR;
                            std::memcpy(&buffer[2], &io.memberId, sizeof(uint32_t));
                            buffer[6] = EVENT_CHANGE_DIRECTORY;
                            uint32_t nt = 0x00;
                            std::memcpy(&buffer[7], &nt, sizeof(uint32_t));
                            sendToRoom(originalRoomId, buffer);
                            std::vector<uint8_t> ebuffer(1+1+4+1+4);
                            ebuffer[0] = OPCODE_EVENTS;
                            ebuffer[1] = FLAG_CURSOR;
                            std::memcpy(&ebuffer[2], &io.memberId, sizeof(uint32_t));
                            ebuffer[6] = EVENT_ENTER_DIRECTORY;
                            std::memcpy(&ebuffer[7], &nt, sizeof(uint32_t));
                            sendToRoom(directoryId, ebuffer);
                        }
                    }
                    break;
                }
                case DISPATCH_LIST_DIRECTORY:
                {
                    break;
                }
            }
        }

        void processMessage(std::vector<uint8_t> &buffer, connection_hdl hdl) {
            uint8_t op = buffer[0];
            IO &io = m_connections[hdl];
            switch(op) {
                case OPCODE_CS_PING:
                {
                    pong(hdl);
                    io.pingCount++;
                    if(!io.sentPing) io.sentPing = true;
                    break;
                }
                case OPCODE_CS_PONG:
                {
                    std::cout << "Ping!" << std::endl;
                    io.pongCount++;
                    if(io.pongCount == 1) {
                        sendConfigCursors(hdl);
                    }
                    break;
                }
                case OPCODE_DIMENSIONS:
                {
                    if(buffer.size() >= 5) {
                        std::memcpy(&io.screen_width, &buffer[1], sizeof(uint16_t));
                        std::memcpy(&io.screen_height, &buffer[3], sizeof(uint16_t));
                    }
                    if(!io.sentHello) io.sentHello = true;
                    break;
                }
                case OPCODE_ENTER_GAME:
                {
                    if(!io.isInGame() && io.didSendHello() && buffer.size() > 1) {
                        uint32_t id = getUniqueId();io.memberId = id;io.roomId = 3;/* /home */
                        Member m = {{}, 0, 0, 0, 0, 0};unixSystem.members[id] = m;
                        sendId(hdl, id);
                        ping(hdl);
                        // add cursor to everyone's screen
                        std::vector<uint8_t> buffer(1+1+4+1+2+2+m.nick.size()+2+3+4);
                        buffer[0] = OPCODE_EVENTS;
                        buffer[1] = FLAG_CURSOR;
                        int offset = 2;
                        std::memcpy(&buffer[offset], &id, sizeof(uint32_t));
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
                        sendToRoom(io.roomId, buffer);
                    }
                    break;
                }
                case OPCODE_LEAVE_GAME:
                {
                    if(io.isInGame()) {
                        unixSystem.members.erase(io.memberId);
                        std::vector<uint8_t> buffer(1+1+4+1+4);
                        buffer[0] = OPCODE_EVENTS;
                        buffer[1] = FLAG_CURSOR;
                        std::memcpy(&buffer[2], &io.memberId, sizeof(uint32_t));
                        buffer[6] = EVENT_CURSOR_DELETE;
                        uint32_t nt = 0x00;
                        std::memcpy(&buffer[7], &nt, sizeof(uint32_t));
                        sendAll(buffer);
                        io.leave();
                    }
                    break;
                }
                case OPCODE_CURSOR:
                {
                    if(io.isInGame()) {
                        uint16_t x, y;
                        std::memcpy(&x, &buffer[1], sizeof(uint16_t));
                        std::memcpy(&y, &buffer[3], sizeof(uint16_t));
                        // this just normalizes the value for different screen dimensions
                        x = (x * 65535) / io.screen_width;
                        y = (y * 65535) / io.screen_height;
                        Member &m = unixSystem.members[io.memberId];m.x = x;m.y = y;
                        std::vector<uint8_t> buffer(1+1+4+2+2);
                        buffer[0] = OPCODE_INFO;
                        buffer[1] = FLAG_CURSOR;
                        std::memcpy(&buffer[2], &io.memberId, sizeof(uint32_t));
                        std::memcpy(&buffer[6], &x, sizeof(uint16_t));
                        std::memcpy(&buffer[8], &y, sizeof(uint16_t));
                        sendToRoom(io.roomId, buffer);
                    }
                    break;
                }
                case OPCODE_CLICK:
                {
                    if(io.isInGame()) {
                        unixSystem.members[io.memberId].click = buffer[1];
                    }
                    break;
                }
                case OPCODE_DISPATCH:
                {
                    processDispatch(buffer, hdl, io);
                    break;
                }
            }
        }

        void on_open(connection_hdl hdl) {
            IO io;
            m_connections[hdl] = io;
        }

        void on_close(connection_hdl hdl) {
            IO &io = m_connections[hdl];
            unixSystem.members.erase(io.memberId);
            std::vector<uint8_t> buffer(1+1+4+1+4);
            buffer[0] = OPCODE_EVENTS;
            buffer[1] = FLAG_CURSOR;
            std::memcpy(&buffer[2], &io.memberId, sizeof(uint32_t));
            buffer[6] = EVENT_CURSOR_DELETE;
            uint32_t nt = 0x00;
            std::memcpy(&buffer[7], &nt, sizeof(uint32_t));
            sendToRoom(io.roomId, buffer);
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
        typedef struct {
            std::size_t operator()(const websocketpp::connection_hdl& hdl) const {
                return std::hash<std::uintptr_t>()(reinterpret_cast<std::uintptr_t>(hdl.lock().get()));
            }
        } connection_hdl_hash;
        typedef struct {
            bool operator()(const websocketpp::connection_hdl& lhs, const websocketpp::connection_hdl& rhs) const {
                return !lhs.owner_before(rhs) && !rhs.owner_before(lhs);
            }
        } connection_hdl_equal;
        std::unordered_map<connection_hdl, IO, connection_hdl_hash, connection_hdl_equal> m_connections;
        UnixSystem unixSystem;
};

int main() {
    WebSocketServer wsServer;
    wsServer.run(WEBSOCKETPP_PORT);
}
