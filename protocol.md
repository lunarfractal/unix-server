# Protocol
## Client -> Server
### Opcodes
| Opcode               | Value  | Description               |
|----------------------|--------|---------------------------|
| OPCODE_CS_PING       | 0x00   | Sent once, at the intial connection. |
| OPCODE_CS_PONG       | 0x01   | Sent to the server whenever it pings us. |
| OPCODE_SCREEN        | 0x02   | Also sent at the initial connection, and when the window is resized. |
| OPCODE_ENTER_ROOM    | 0x03   | Sent to enter the filesystem. Could be used for authorization |
| OPCODE_LEAVE_ROOM    | 0x04   | Sent to leave the filesystem. If leave, you still receive updates. |
| OPCODE_MOUSE         | 0x05   | Sent to update cursor position. |
| OPCODE_CLICK         | 0x06   | Sent when page is clicked. |
| OPCODE_NICK          | 0x07   | Sent to change ur nick. |
| OPCODE_COLOR         | 0x08   | Sent to change ur color. |
| OPCODE_DISPATCH      | 0x09   | Sent to dispatch an event. |
