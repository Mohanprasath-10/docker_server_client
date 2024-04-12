#include <iostream>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LENGTH_NAME 31
#define LENGTH_MSG 101
#define LENGTH_SEND 201

class Client {
private:
    int sockfd;
    sockaddr_in server_info;
    socklen_t s_addrlen;

public:
    Client() : sockfd(-1), s_addrlen(sizeof(server_info)) {}

    ~Client() {
        close(sockfd);
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

    void recv_msg_handler() {
        char receiveMessage[LENGTH_SEND] = {};
        while (1) {
            int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
            if (receive > 0) {
                std::cout << "\r" << receiveMessage << std::endl;
                str_overwrite_stdout();
            }
            else if (receive == 0) {
                break;
            }
        }
    }

    void send_msg_handler() {
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
            send(sockfd, message, LENGTH_MSG, 0);
            if (strcmp(message, "exit") == 0) {
                break;
            }
        }
        catch_ctrl_c_and_exit(2);
    }

    void startClient(const char* ip) {
        // Create socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            std::cerr << "Fail to create a socket.";
            exit(EXIT_FAILURE);
        }

        // Socket information
        memset(&server_info, 0, s_addrlen);
        server_info.sin_family = AF_INET;
        server_info.sin_addr.s_addr = inet_addr(ip);
        server_info.sin_port = htons(8888);

        // Connect to Server
        int err = connect(sockfd, reinterpret_cast<sockaddr*>(&server_info), s_addrlen);
        if (err == -1) {
            std::cerr << "Connection to Server error!\n";
            exit(EXIT_FAILURE);
        }

        // Naming
        char nickname[LENGTH_NAME] = {};
        std::cout << "Please enter your name: ";
        if (fgets(nickname, LENGTH_NAME, stdin) != nullptr) {
            str_trim_lf(nickname, LENGTH_NAME);
        }
        if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME - 1) {
            std::cerr << "\nName must be more than one and less than thirty characters.\n";
            exit(EXIT_FAILURE);
        }

        send(sockfd, nickname, LENGTH_NAME, 0);

        std::thread send_msg_thread(&Client::send_msg_handler, this);
        std::thread recv_msg_thread(&Client::recv_msg_handler, this);

        send_msg_thread.detach();
        recv_msg_thread.detach();

        while (1) {
            // Loop until interrupted
        }
    }
};

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: ./client <IP>\n";
        return EXIT_FAILURE;
    }
    Client client;
    client.startClient(argv[1]);
    return 0;
}
