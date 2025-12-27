# Multi-Client Chat Server

A TCP-based multi-client chat server implementation in C++ for Linux systems. The server uses select() for I/O multiplexing to handle multiple simultaneous client connections efficiently.

## Project Overview

**Current Phase:** Development

**Tech Stack:** C++11, POSIX sockets, Linux

**Project Type:** Network Application / Chat Server

## Features

- TCP socket-based server supporting multiple simultaneous clients
- Uses select() for I/O multiplexing to manage connections
- Broadcast messages to all connected clients
- Username-based messaging system
- Connect/disconnect notifications
- Support for up to 10 concurrent clients
- Non-blocking I/O operations
- Proper error handling and resource cleanup

## Requirements

- Linux operating system
- g++ compiler with C++11 support
- POSIX-compliant system libraries

## Installation

### Build from Source

1. Clone or download this repository
2. Navigate to the project directory
3. Build the project:

```bash
make
```

This will create two executables:

- `server` - The chat server
- `client` - The chat client

### Build Options

```bash
make all        # Build both server and client (default)
make server     # Build server only
make client     # Build client only
make clean      # Remove all build files
make rebuild    # Clean and rebuild everything
make help       # Show all available commands
```

## Usage

### Starting the Server

Run the server with default port (8080):

```bash
./server
```

Or specify a custom port:

```bash
./server 9000
```

The server will display:

```
Server: Listening on port 8080...
```

### Connecting Clients

In separate terminals, run the client:

```bash
./client
```

Or connect to a specific server and port:

```bash
./client 192.168.1.100 9000
```

Arguments:

- First argument: Server IP address (default: 127.0.0.1)
- Second argument: Port number (default: 8080)

### Using the Chat

1. When you connect, you'll be prompted to enter your username
2. After entering your username, you'll join the chat room
3. Type your message and press Enter to send
4. Messages from other users will appear automatically
5. Type `quit` or `exit` to disconnect

### Example Session

Terminal 1 (Server):

```
Server: Listening on port 8080...
Server: New connection from 127.0.0.1
Server: Client Alice connected
Server: New connection from 127.0.0.1
Server: Client Bob connected
Alice: Hello everyone!
Bob: Hi Alice!
Server: Client Alice disconnected
```

Terminal 2 (Client - Alice):

```
Connected to server at 127.0.0.1:8080
--- Multi-Client Chat ---
Type 'quit' or 'exit' to disconnect
-------------------------

Enter your username: Alice
Welcome to the chat, Alice!
Server: Bob has joined the chat
Hello everyone!
Bob: Hi Alice!
quit
Disconnected from server
```

Terminal 3 (Client - Bob):

```
Connected to server at 127.0.0.1:8080
--- Multi-Client Chat ---
Type 'quit' or 'exit' to disconnect
-------------------------

Enter your username: Bob
Welcome to the chat, Bob!
Alice: Hello everyone!
Hi Alice!
Server: Alice has left the chat
```

## Technical Details

### Server Architecture

- **Socket Type:** TCP (SOCK_STREAM)
- **I/O Multiplexing:** select() system call
- **Max Clients:** 10 concurrent connections
- **Message Size:** 1024 bytes maximum
- **Port Range:** 1-65535 (default: 8080)

### Message Format

- User messages: `[username]: [message]`
- Server notifications: `Server: [notification]`

### Connection Flow

1. Client connects to server
2. Server requests username
3. Client sends username
4. Server broadcasts join notification
5. Clients exchange messages
6. Client disconnects
7. Server broadcasts leave notification

## Error Handling

The application handles various error scenarios:

- Failed socket creation
- Port binding failures
- Connection errors
- Send/receive failures
- Invalid port numbers
- Server capacity limits

## Security Considerations

- No authentication mechanism (usernames are not verified)
- Messages are transmitted in plain text
- No message encryption
- No input sanitization for usernames/messages
- Suitable for local network or learning purposes only

## Limitations

- Maximum 10 concurrent clients
- Maximum message size: 1024 bytes
- No message history or persistence
- No private messaging
- No room/channel support
- No administrative controls

## Troubleshooting

### Port Already in Use

If you get "Failed to bind socket to port" error:

- Another process is using that port
- Try a different port number
- Or wait for the port to be released (TIME_WAIT state)

### Connection Refused

If client cannot connect:

- Ensure server is running
- Verify correct IP address and port
- Check firewall settings
- Ensure network connectivity

### Build Errors

If compilation fails:

- Verify g++ is installed: `g++ --version`
- Ensure C++11 support
- Check for missing system libraries

## Project Structure

```
Cpp-Multi-client-Chat-Server/
├── server.cpp              # Server implementation
├── client.cpp              # Client implementation
├── Makefile                # Build configuration
├── README.md               # This file
├── dev-log.md              # Development log and documentation
└── prompt_requirement.txt  # Project documentation guidelines
```

## Development

This project follows a strict documentation-first workflow. See [dev-log.md](dev-log.md) for detailed development history and technical decisions.

## License

This is an educational project. Feel free to use and modify as needed.

## Contributing

1. Read [dev-log.md](dev-log.md) completely before making changes
2. Follow the coding standards defined in the project
3. Update [dev-log.md](dev-log.md) after any changes
4. Test thoroughly before committing

## Future Enhancements

Potential improvements for future versions:

- Implement epoll() for better scalability
- Add SSL/TLS encryption
- Implement user authentication
- Add private messaging functionality
- Support for multiple chat rooms
- Message persistence and history
- File transfer capabilities
- Administrative commands (kick, ban, etc.)
- Configurable message size limits
- IPv6 support
