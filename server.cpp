#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

struct Client {
    int socket;
    std::string username;
    bool authenticated;
};

class ChatServer {
private:
    int server_socket;
    int port;
    std::vector<Client> clients;
    fd_set master_set;
    int max_fd;

    // Initialize server socket and bind to port
    bool initializeServer() {
        // Create socket
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            std::cerr << "Error: Failed to create socket" << std::endl;
            return false;
        }

        // Set socket options to reuse address
        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Error: Failed to set socket options" << std::endl;
            close(server_socket);
            return false;
        }

        // Bind socket to address and port
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Error: Failed to bind socket to port " << port << std::endl;
            close(server_socket);
            return false;
        }

        // Listen for connections
        if (listen(server_socket, MAX_CLIENTS) < 0) {
            std::cerr << "Error: Failed to listen on socket" << std::endl;
            close(server_socket);
            return false;
        }

        std::cout << "Server: Listening on port " << port << "..." << std::endl;
        return true;
    }

    // Accept new client connection
    void acceptNewClient() {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Error: Failed to accept client connection" << std::endl;
            return;
        }

        // Check if max clients reached
        if (clients.size() >= MAX_CLIENTS) {
            std::string msg = "Server is full. Please try again later.\n";
            send(client_socket, msg.c_str(), msg.length(), 0);
            close(client_socket);
            return;
        }

        // Request username from client
        std::string welcome = "Enter your username: ";
        send(client_socket, welcome.c_str(), welcome.length(), 0);

        // Add client to list (temporarily unauthenticated)
        Client new_client;
        new_client.socket = client_socket;
        new_client.authenticated = false;
        clients.push_back(new_client);

        // Update master set
        FD_SET(client_socket, &master_set);
        if (client_socket > max_fd) {
            max_fd = client_socket;
        }

        std::cout << "Server: New connection from " 
                  << inet_ntoa(client_addr.sin_addr) << std::endl;
    }

    // Handle client message
    void handleClientMessage(int client_index) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        int bytes_received = recv(clients[client_index].socket, buffer, BUFFER_SIZE - 1, 0);

        // Client disconnected or error
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Server: Client " << clients[client_index].username 
                          << " disconnected" << std::endl;
                broadcastMessage("Server: " + clients[client_index].username + 
                               " has left the chat\n", clients[client_index].socket);
            } else {
                std::cerr << "Error: Failed to receive data from client" << std::endl;
            }

            // Remove client
            FD_CLR(clients[client_index].socket, &master_set);
            close(clients[client_index].socket);
            clients.erase(clients.begin() + client_index);
            return;
        }

        // Remove newline character
        buffer[strcspn(buffer, "\n")] = 0;

        // Handle username setup for new clients
        if (!clients[client_index].authenticated) {
            clients[client_index].username = std::string(buffer);
            clients[client_index].authenticated = true;
            
            std::cout << "Server: Client " << clients[client_index].username 
                      << " connected" << std::endl;
            
            std::string welcome = "Welcome to the chat, " + 
                                clients[client_index].username + "!\n";
            send(clients[client_index].socket, welcome.c_str(), welcome.length(), 0);
            
            broadcastMessage("Server: " + clients[client_index].username + 
                           " has joined the chat\n", clients[client_index].socket);
            return;
        }

        // Broadcast message to all other clients
        std::string message = clients[client_index].username + ": " + 
                             std::string(buffer) + "\n";
        std::cout << message;
        broadcastMessage(message, clients[client_index].socket);
    }

    // Broadcast message to all clients except sender
    void broadcastMessage(const std::string& message, int sender_socket) {
        for (const auto& client : clients) {
            if (client.socket != sender_socket && client.authenticated) {
                if (send(client.socket, message.c_str(), message.length(), 0) < 0) {
                    std::cerr << "Error: Failed to send message to client" << std::endl;
                }
            }
        }
    }

    // Clean up and close all connections
    void cleanup() {
        for (auto& client : clients) {
            close(client.socket);
        }
        close(server_socket);
        std::cout << "Server: Shut down successfully" << std::endl;
    }

public:
    ChatServer(int p = DEFAULT_PORT) : server_socket(-1), port(p), max_fd(0) {
        FD_ZERO(&master_set);
    }

    ~ChatServer() {
        cleanup();
    }

    // Main server loop
    void run() {
        if (!initializeServer()) {
            return;
        }

        // Add server socket to master set
        FD_SET(server_socket, &master_set);
        max_fd = server_socket;

        while (true) {
            // Create working copy of master set
            fd_set working_set = master_set;

            // Wait for activity on any socket
            int activity = select(max_fd + 1, &working_set, NULL, NULL, NULL);
            if (activity < 0) {
                std::cerr << "Error: select() failed" << std::endl;
                break;
            }

            // Check for new connection
            if (FD_ISSET(server_socket, &working_set)) {
                acceptNewClient();
            }

            // Check all client sockets for data
            for (size_t i = 0; i < clients.size(); i++) {
                if (FD_ISSET(clients[i].socket, &working_set)) {
                    handleClientMessage(i);
                    // After handling, check if client was removed
                    if (i >= clients.size()) {
                        break;
                    }
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    // Parse command line arguments
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Error: Invalid port number" << std::endl;
            return 1;
        }
    }

    ChatServer server(port);
    server.run();

    return 0;
}
