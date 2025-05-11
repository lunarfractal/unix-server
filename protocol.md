# Protocol
## Client -> Server
### Opcodes
| Opcode               | Value  | Description               |
|----------------------|--------|---------------------------|
| OPCODE_CS_PING       | 0x00   | Sent once, at the intial connection. |
| OPCODE_CS_PONG       | 0x01   | Sent to the server whenever it pings us. |
| OPCODE_SCREEN        | 0x02   | Also sent at the initial connection, and when the window is resized. |
| OPCODE_ENTER_ROOM    | 0x03   | Sent to enter the filesystem. Could be used for authorization |
| OPCODE_LEAVE_ROOM    | 0x04   | Sent to leave the filesystem. If you leave, you still receive updates. |
| OPCODE_MOUSE         | 0x05   | Sent to update cursor position. |
| OPCODE_CLICK         | 0x06   | Sent when page is clicked. |
| OPCODE_NICK          | 0x07   | Sent to change ur nick. |
| OPCODE_COLOR         | 0x08   | Sent to change ur color. |
| OPCODE_DISPATCH      | 0x09   | Sent to dispatch an event. |

## Server -> Client
### Opcodes
| Opcode               | Value  | Description               |
|----------------------|--------|---------------------------|
| OPCODE_SC_PING       | 0x00   | Sent to the client whenever the server feels like it. |
| OPCODE_SC_PONG       | 0x01   | Sent when the client pings. |
| OPCODE_CONFIG | 0xA0 | Maybe list all default directories? |
| OPCODE_ENTERED_ROOM | 0xA1 | Send id to client. |
| OPCODE_INFO | 0xB0 | Info about the server. |
| OPCODE_EVENTS | 0xA2 | Event was dispatched. |

### Packet Structure
**Config**<br>
This packet starts with a uint8 header byte 0xA0. It contains information about the directories.
It's a relatively simple packet. After the header byte, it has a repeating pattern of bytes, the directory id, the null terminated nick, the parent directory's id. if the directory id is 0x00, it means you should stop reading further.<br><br>
Example: `u8 [u32 [u16] u32]`
