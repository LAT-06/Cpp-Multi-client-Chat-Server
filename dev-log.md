# Development Log

---

## [2025-12-27 Initial Implementation] - Multi-Client Chat Server

**Type:** Backend / Network Application

### What was done:

- Created a complete multi-client TCP chat server implementation in C++
- Implemented server using POSIX sockets with select() for I/O multiplexing
- Built a client application for testing and interaction
- Set up build system with Makefile
- Created comprehensive documentation and usage instructions
- Implemented username-based messaging system
- Added connect/disconnect notification broadcasting

### Technical Details:

**File paths:**

- `/server.cpp` - Main server implementation (230 lines)
- `/client.cpp` - Client application for testing (140 lines)
- `/Makefile` - Build configuration with multiple targets
- `/README.md` - Complete project documentation and usage guide

**Architecture Components:**

#### Server (server.cpp):

- **Class:** `ChatServer`

  - Manages server socket and client connections
  - Handles I/O multiplexing using select()
  - Broadcasts messages to all connected clients

- **Struct:** `Client`

  - `int socket` - Client socket file descriptor
  - `std::string username` - Client's username
  - `bool authenticated` - Whether username has been set

- **Key Functions:**
  - `initializeServer()` - Creates socket, binds to port, starts listening
  - `acceptNewClient()` - Accepts new connections, requests username
  - `handleClientMessage(int client_index)` - Processes messages from clients
  - `broadcastMessage(string message, int sender_socket)` - Sends message to all clients except sender
  - `run()` - Main server loop with select() for multiplexing
  - `cleanup()` - Closes all connections and frees resources

#### Client (client.cpp):

- **Class:** `ChatClient`

  - Manages connection to server
  - Handles sending and receiving messages in separate threads

- **Key Functions:**
  - `connectToServer()` - Establishes TCP connection to server
  - `receiveMessages()` - Runs in separate thread, receives messages from server
  - `sendMessages()` - Handles user input and sends to server
  - `run()` - Main client loop, spawns receive thread
  - `cleanup()` - Closes connection and cleans up resources

**Dependencies:**

- Standard C++ libraries: `<iostream>`, `<cstring>`, `<vector>`, `<thread>`, `<algorithm>`
- POSIX socket libraries: `<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`, `<unistd.h>`
- Compiler: g++ with C++11 support
- Threading library: pthread (linked via -pthread flag)

**Configuration Constants:**

- `MAX_CLIENTS` - Maximum concurrent clients (10)
- `BUFFER_SIZE` - Message buffer size (1024 bytes)
- `DEFAULT_PORT` - Default server port (8080)

**Network Protocol:**

- TCP/IP sockets (SOCK_STREAM)
- IPv4 (AF_INET)
- Text-based protocol with newline-delimited messages
- Message format: `[username]: [message]\n`
- Server notifications: `Server: [notification]\n`

**I/O Multiplexing (select()):**

- Uses fd_set to track all active sockets
- master_set maintains all client sockets
- working_set is a copy used for each select() call
- Monitors server socket for new connections
- Monitors all client sockets for incoming data

**Connection Workflow:**

1. Server creates socket and binds to port
2. Server calls listen() to accept connections
3. select() monitors server socket and all client sockets
4. New connection: accept() creates client socket
5. Server requests username from new client
6. Client sends username (first message)
7. Server marks client as authenticated
8. Server broadcasts join notification to all other clients
9. Clients exchange messages through server
10. Client disconnect triggers broadcast notification
11. Server removes client from list and closes socket

**Error Handling:**

- Socket creation failures
- Port binding errors (with SO_REUSEADDR option)
- Connection acceptance errors
- Send/receive failures
- Invalid port number validation (1-65535)
- Maximum client limit enforcement
- Graceful disconnect handling (both normal and error cases)

### How to use/test:

#### Building the Project:

```bash
# Build both server and client
make

# Or build individually
make server
make client

# Clean build files
make clean

# Rebuild everything
make rebuild
```

#### Running the Server:

```bash
# Default port (8080)
./server

# Custom port
./server 9000
```

Expected output:

```
Server: Listening on port 8080...
```

#### Running Clients:

Open multiple terminals and run:

```bash
# Connect to localhost on default port
./client

# Connect to specific server and port
./client 192.168.1.100 9000
```

#### Testing the Chat:

1. Start the server in one terminal
2. Start first client, enter username (e.g., "Alice")
3. Start second client, enter username (e.g., "Bob")
4. Send messages from either client
5. Observe messages appear on both clients
6. Disconnect a client (type "quit" or "exit")
7. Observe disconnect notification on remaining client

#### Example Test Session:

Server Terminal:

```
Server: Listening on port 8080...
Server: New connection from 127.0.0.1
Server: Client Alice connected
Server: New connection from 127.0.0.1
Server: Client Bob connected
Alice: Hello everyone!
Bob: Hi Alice!
Alice: How are you?
Bob: I'm good, thanks!
Server: Client Alice disconnected
```

Client 1 (Alice):

```
Connected to server at 127.0.0.1:8080
Enter your username: Alice
Welcome to the chat, Alice!
Server: Bob has joined the chat
Hello everyone!
Bob: Hi Alice!
How are you?
Bob: I'm good, thanks!
quit
Disconnected from server
```

Client 2 (Bob):

```
Connected to server at 127.0.0.1:8080
Enter your username: Bob
Welcome to the chat, Bob!
Alice: Hello everyone!
Hi Alice!
Alice: How are you?
I'm good, thanks!
Server: Alice has left the chat
```

### Related files:

- `README.md` - Project documentation, setup, and usage guide
- `Makefile` - Build system configuration
- `prompt_requirement.txt` - Documentation guidelines and coding rules
- `.github/copilot-instructions.md` - Project-wide development guidelines

### Notes/Warnings:

**Security Considerations:**

- No authentication mechanism implemented
- Messages transmitted in plain text (no encryption)
- No input validation or sanitization for usernames/messages
- Suitable for local network or educational purposes only
- NOT production-ready without security enhancements

**Known Limitations:**

- Maximum 10 concurrent clients (defined by MAX_CLIENTS constant)
- Maximum message size 1024 bytes (defined by BUFFER_SIZE)
- No message persistence or chat history
- No private messaging capability
- No room/channel separation
- Username uniqueness not enforced
- No admin controls (kick, ban, mute, etc.)

**Performance Characteristics:**

- Uses select() for I/O multiplexing (O(n) complexity)
- For better scalability with many connections, consider epoll() (Linux-specific, O(1) complexity)
- Current implementation suitable for up to 10-20 clients
- For hundreds of clients, epoll() would be more efficient

**Port Considerations:**

- Uses SO_REUSEADDR socket option to allow quick restart after shutdown
- Avoids TIME_WAIT state issues when restarting server
- Port must not be in use by another application
- Privileged ports (<1024) require root/sudo access

**Threading Model:**

- Server is single-threaded (uses select() for multiplexing)
- Client uses two threads: one for receiving, one for sending
- Client's receive thread prevents blocking on user input
- Proper thread synchronization via connected flag

**Memory Management:**

- std::vector automatically manages client list
- Sockets properly closed in destructor and cleanup()
- No manual memory allocation (no new/delete used)
- RAII pattern for resource management

**Testing Recommendations:**

- Test with multiple simultaneous clients (stress test)
- Test disconnection scenarios (graceful and abrupt)
- Test maximum message size (1024 bytes)
- Test maximum client limit (11th client should be rejected)
- Test invalid port numbers
- Test server restart (port reuse)
- Test message ordering and delivery

**Future Enhancement Opportunities:**

- Implement epoll() for better scalability
- Add SSL/TLS for encrypted communication
- Implement user authentication system
- Add private messaging (/msg username message)
- Support multiple chat rooms/channels
- Add message persistence (database/file storage)
- Implement file transfer capability
- Add administrative commands (/kick, /ban, /mute)
- Make buffer size and max clients configurable
- Add IPv6 support
- Implement message rate limiting
- Add connection timeout handling
- Implement heartbeat/keepalive mechanism
- Add message encryption at application layer
- Create configuration file support
- Add logging system for debugging and monitoring

**Compilation Requirements:**

- C++11 or later (uses std::thread, std::string)
- pthread library (for threading support)
- POSIX-compliant system (Linux, Unix, macOS)
- Does not compile on Windows without modifications (Winsock instead of POSIX sockets)

**Code Quality:**

- Clear separation of concerns (connection management, message handling, broadcasting)
- Proper error checking on all system calls
- Resource cleanup in destructors (RAII)
- Self-documenting function and variable names
- Comments explain WHY, not WHAT
- Consistent coding style throughout

---
