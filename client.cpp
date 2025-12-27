#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

class ChatClient {
private:
    int client_socket;
    std::string server_address;
    int port;
    bool connected;

    // Connect to server
    bool connectToServer() {
        // Create socket
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket < 0) {
            std::cerr << "Error: Failed to create socket" << std::endl;
            return false;
        }

        // Set up server address
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        // Convert address from text to binary
        if (inet_pton(AF_INET, server_address.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Error: Invalid address" << std::endl;
            close(client_socket);
            return false;
        }

        // Connect to server
        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Error: Failed to connect to server" << std::endl;
            close(client_socket);
            return false;
        }

        connected = true;
        std::cout << "Connected to server at " << server_address 
                  << ":" << port << std::endl;
        return true;
    }

    // Receive messages from server (runs in separate thread)
    void receiveMessages() {
        char buffer[BUFFER_SIZE];
        
        while (connected) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    std::cout << "\nServer closed connection" << std::endl;
                } else {
                    std::cerr << "\nError: Failed to receive data" << std::endl;
                }
                connected = false;
                break;
            }

            // Display received message
            std::cout << buffer;
            std::cout.flush();
        }
    }

    // Send messages to server
    void sendMessages() {
        std::string message;
        
        while (connected && std::getline(std::cin, message)) {
            if (!connected) {
                break;
            }

            // Send message to server
            message += "\n";
            if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
                std::cerr << "Error: Failed to send message" << std::endl;
                connected = false;
                break;
            }

            // Check for quit command
            if (message == "quit\n" || message == "exit\n") {
                connected = false;
                break;
            }
        }
    }

    // Close connection and cleanup
    void cleanup() {
        if (client_socket >= 0) {
            close(client_socket);
        }
        connected = false;
    }

public:
    ChatClient(const std::string& addr = "127.0.0.1", int p = DEFAULT_PORT) 
        : client_socket(-1), server_address(addr), port(p), connected(false) {}

    ~ChatClient() {
        cleanup();
    }

    // Main client loop
    void run() {
        if (!connectToServer()) {
            return;
        }

        std::cout << "\n--- Multi-Client Chat ---" << std::endl;
        std::cout << "Type 'quit' or 'exit' to disconnect" << std::endl;
        std::cout << "-------------------------\n" << std::endl;

        // Start receive thread
        std::thread receive_thread(&ChatClient::receiveMessages, this);

        // Send messages in main thread
        sendMessages();

        // Wait for receive thread to finish
        if (receive_thread.joinable()) {
            receive_thread.join();
        }

        cleanup();
        std::cout << "Disconnected from server" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string server_address = "127.0.0.1";
    int port = DEFAULT_PORT;

    // Parse command line arguments
    if (argc > 1) {
        server_address = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Error: Invalid port number" << std::endl;
            return 1;
        }
    }

    ChatClient client(server_address, port);
    client.run();

    return 0;
}
