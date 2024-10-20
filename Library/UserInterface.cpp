#include "UserInterface.h"

#include <iostream>
#include <string.h>
#include "Reader.h"

UserInterface::~UserInterface()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

UserInterface::UserInterface()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Critical error" << std::endl;
        exit(1);
    }
#endif
}

void UserInterface::MainMenu()
{
    do {
        try {
            int choiceCode;
            std::cout << "1 - add a new account" << std::endl;
            std::cout << "2 - log in" << std::endl;
            std::cout << "0 - exit" << std::endl;

           while (!(std::cin >> choiceCode)) {
               std::cout << "Invalid input. Please enter a valid option." << std::endl;
               std::cin.clear();
               std::cin.ignore(1000, '\n');
           }
           std::cin.clear();
           std::cin.ignore(1000, '\n');

            switch (choiceCode) {
            case 0:
                return;
            case 1:
                CreateAccount();
                break;
            case 2:
                Login();
                break;
            default:
                std::cout << "unknown option" << std::endl;
            }
        }
        catch (const LibraryException& e) {
            std::cerr << "Error encountered: " << e.what() << std::endl;
        }

    } while (true);
}

void UserInterface::CreateAccount() {
    Packet packet;
    packet.action = CREATE_ACCOUNT;

    std::cout << "Enter the username: ";
    std::string username;
    getline(std::cin, username);
    if (username.length() > MAXPACKETTEXT) {
        throw LibraryException("Password is too long.");
    }
    strncpy(packet.Username, username.c_str(), sizeof(packet.Username) - 1);

    std::cout << "Enter the password: ";
    std::string password;
    getline(std::cin, password);
    if (password.length() > MAXPACKETTEXT) {
        throw LibraryException("Password is too long.");
    }
    strncpy(packet.Password, password.c_str(), sizeof(packet.Password) - 1);

    SendPacket(packet);
}

void UserInterface::Login() {
    Packet packet;
    packet.action = LOGIN;

    std::cout << "Enter the username: ";
    std::string username;
    getline(std::cin, username);
    if (username.length() > MAXPACKETTEXT) {
        throw LibraryException("Username is too long.");
    }
    strncpy(packet.Username, username.c_str(), sizeof(packet.Username) - 1);

    std::cout << "Enter the password: ";
    std::string password;
    getline(std::cin, password);
    if (password.length() > MAXPACKETTEXT) {
        throw LibraryException("Password is too long.");
    }
    strncpy(packet.Password, password.c_str(), sizeof(packet.Password) - 1);

    SendPacket(packet);
}

void UserInterface::SendPacket(const Packet& packet)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        throw LibraryException("Error creating socket.");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IPADDRESS);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
#ifdef _WIN32
        closesocket(sock);
#elif __linux__
        close(sock);
#endif
        throw LibraryException("Error connecting to server.");
    }

    if (send(sock, reinterpret_cast<const char*>(&packet), sizeof(packet), 0) == SOCKET_ERROR) {
#ifdef _WIN32
        closesocket(sock);
#elif __linux__
        close(sock);
#endif
        throw LibraryException("Error sending packet to server.");
    }

    timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sock, &readSet);

    int selectResult = select(0, &readSet, nullptr, nullptr, &timeout);
    if (selectResult == SOCKET_ERROR) {
#ifdef _WIN32
        closesocket(sock);
#elif __linux__
        close(sock);
#endif
        throw LibraryException("Error in select.");
    }
    else if (selectResult == 0) {
#ifdef _WIN32
        closesocket(sock);
#elif __linux__
        close(sock);
#endif
        throw LibraryException("Timeout occurred. No reply from server.");
    }

    Packet replyPacket;
    int bytesReceived = recv(sock, reinterpret_cast<char*>(&replyPacket), sizeof(replyPacket), 0);
    if (bytesReceived == SOCKET_ERROR) {
        throw LibraryException("Error receiving reply from server.");
    }
    else if (bytesReceived == 0) {
        throw LibraryException("server disconnected");
    }
    else {
        if (replyPacket.action == LOGIN_SUCCESS && replyPacket.SessionId != "") {
            try {
#ifdef _WIN32
                u_long mode = 1;
                if (ioctlsocket(sock, FIONBIO, &mode) != NO_ERROR) {
                    closesocket(sock);
                    throw LibraryException("Error setting socket to non-blocking mode.");
                }
#elif __linux__
                int flags = fcntl(sock, F_GETFL, 0);
                if (flags == -1) {
                    close(sock);
                    throw LibraryException("Error getting socket flags.");
                }
                if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
                    close(sock);
                    throw LibraryException("Error setting socket to non-blocking mode.");
                }
#endif

                StartUserSession(replyPacket.SessionId, sock);
            }
            catch (const LibraryException& e) {
                std::cerr << "Error encountered: " << e.what() << std::endl;
            }
        }
        else {
            throw LibraryException("Unsuccessful login atempt");
        }
    }

    // Close socket
#ifdef _WIN32
    closesocket(sock);
#elif __linux__
    close(sock);
#endif
}

void UserInterface::StartUserSession(std::string SessionId, SOCKET ServerSocket)
{
    Reader user(SessionId, ServerSocket);
    user.ClientMenu();
}
