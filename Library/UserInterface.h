#pragma once

#include "Structs.h"
#include <string>
#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <WinSock2.h>
#elif __linux__
    #include <sys/socket.h>
    #include <fcntl.h>
    #include <arpa/inet.h>
    #include <unistd.h>     
    #define SOCKET int      
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR   (-1)
#endif

class UserInterface {
public:
    ~UserInterface();

    UserInterface();

    void MainMenu();

private:
    void CreateAccount();

    void Login();

    void SendPacket(const Packet& packet);
    void StartUserSession(std::string SessionId, SOCKET ServerSocket);
};