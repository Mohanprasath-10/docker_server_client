#include <iostream>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <vector>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LENGTH_NAME 31
#define LENGTH_MSG 101
#define LENGTH_SEND 201

class ClientList {
public:
    int data;
    ClientList* prev;
    ClientList* link;
    char ip[16];
    char name[LENGTH_NAME];
};

class Server {
private:
    ClientList* root;
    ClientList* now;
    int server_sockfd;
    sockaddr_in server_info, client_info;
    socklen_t s_addrlen, c_addrlen;

public:
    Server() : root(nullptr), now(nullptr), server_sockfd(-1), s_addrlen(sizeof(server_info)), c_addrlen(sizeof(client_info)) {}

    ~Server() {
        close(server_sockfd);
        while (root != nullptr) {
            ClientList* tmp = root;
            root = root->link;
            delete tmp;
        }
    }

    void str_trim_lf(char* arr, int length) {
        for (int i = 0; i < length; i++) {
            if (arr[i] == '\n') {
                arr[i] = '\0';
                break;
            }
        }
    }

    void str_overwrite_stdout() {
        std::cout << "\r> " << std::flush;
    }

    static void catch_ctrl_c_and_exit(int sig) {
        std::cout << "\nBye\n";
        exit(EXIT_SUCCESS);
    }

    void send_to_all_clients(ClientList* np, char tmp_buffer[]) {
        ClientList* tmp = root->link;
        while (tmp != nullptr) {
            if (np == nullptr || np->data != tmp->data) {
                std::cout << "Send to sockfd " << tmp->data << ": \"" << tmp_buffer << "\"\n";
                send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
            }
            tmp = tmp->link;
        }
    }

    void client_handler(ClientList* np) {
        int leave_flag = 0;
        char nickname[LENGTH_NAME] = {};
        char recv_buffer[LENGTH_MSG] = {};
        char send_buffer[LENGTH_SEND] = {};

        // Naming
        if (recv(np->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME - 1) {
            std::cout << np->ip << " didn't input name.\n";
            leave_flag = 1;
        }
        else {
            strncpy(np->name, nickname, LENGTH_NAME);
            std::cout << np->name << "(" << np->ip << ")(" << np->data << ") join the server.\n";
            sprintf(send_buffer, "%s(%s) join the server.", np->name, np->ip);
            send_to_all_clients(np, send_buffer);
        }

        // Conversation
        while (1) {
            if (leave_flag) {
                break;
            }
            int receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
            if (receive > 0) {
                if (strlen(recv_buffer) == 0) {
                    continue;
                }
                std::cout << "Received from sockfd " << np->data << ": \"" << recv_buffer << "\"\n"; // Added line
                sprintf(send_buffer, "%s: %s", np->name, recv_buffer);
            }
            else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
                std::cout << np->name << "(" << np->ip << ")(" << np->data << ") leave the server.\n";
                sprintf(send_buffer, "%s(%s) leave the server.", np->name, np->ip);
                leave_flag = 1;
            }
            else {
                std::cerr << "Fatal Error: -1\n";
                leave_flag = 1;
            }
            send_to_all_clients(np, send_buffer);
        }

        // Remove Node
        close(np->data);
        if (np == now) {
            now = np->prev;
            now->link = nullptr;
        }
        else {
            np->prev->link = np->link;
            np->link->prev = np->prev;
        }
        delete np;
    }

    void server_sender() {
        char message[LENGTH_MSG] = {};
        while (1) {
            str_overwrite_stdout();
            while (fgets(message, LENGTH_MSG, stdin) != nullptr) {
                str_trim_lf(message, LENGTH_MSG);
                if (strlen(message) == 0) {
                    str_overwrite_stdout();
                }
                else {
                    break;
                }
            }
            char send_buffer[LENGTH_SEND] = {};
            sprintf(send_buffer, "Server: %s", message);
            send_to_all_clients(nullptr, send_buffer);
        }
    }

    void startServer() {
        signal(SIGINT, catch_ctrl_c_and_exit);

        // Create socket
        server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sockfd == -1) {
            std::cerr << "Fail to create a socket.";
            exit(EXIT_FAILURE);
        }

        // Socket information
        memset(&server_info, 0, s_addrlen);
        server_info.sin_family = AF_INET;
        server_info.sin_addr.s_addr = INADDR_ANY;
        server_info.sin_port = htons(8888);

        // Bind and Listen
        bind(server_sockfd, reinterpret_cast<sockaddr*>(&server_info), s_addrlen);
        listen(server_sockfd, 5);

        // Print Server IP
        getsockname(server_sockfd, reinterpret_cast<sockaddr*>(&server_info), &s_addrlen);
        std::cout << "Start Server on: " << inet_ntoa(server_info.sin_addr) << ":" << ntohs(server_info.sin_port) << std::endl;

        // Initial linked list for clients
        root = new ClientList;
        root->data = server_sockfd;
        root->prev = nullptr;
        root->link = nullptr;
        strncpy(root->ip, inet_ntoa(server_info.sin_addr), 16);
        strncpy(root->name, "Server", LENGTH_NAME);
        now = root;

        std::thread server_send_thread(&Server::server_sender, this);
        server_send_thread.detach();

        while (1) {
            int client_sockfd = accept(server_sockfd, reinterpret_cast<sockaddr*>(&client_info), &c_addrlen);

            // Print Client IP
            getpeername(client_sockfd, reinterpret_cast<sockaddr*>(&client_info), &c_addrlen);
            std::cout << "Client " << inet_ntoa(client_info.sin_addr) << ":" << ntohs(client_info.sin_port) << " come in.\n";

            // Append linked list for clients
            ClientList* c = new ClientList;
            c->data = client_sockfd;
            c->prev = now;
            c->link = nullptr;
            strncpy(c->ip, inet_ntoa(client_info.sin_addr), 16);
            strncpy(c->name, "NULL", 5);
            now->link = c;
            now = c;

            std::thread client_thread(&Server::client_handler, this, c);
            client_thread.detach();
        }
    }
};

int main() {
    Server server;
    server.startServer();
    return 0;
}
