#pragma once
#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <WinSock2.h>
#elif __linux__
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>     
    #define SOCKET int      
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR   (-1)
#endif
#include <mutex>
#include <thread>
#include "LibrarySystem.h"
#include <unordered_map>
#include "Structs.h"
#include <iostream>

class Server {
public:
    static Server* GetInstance() {
        if (Instance) {
            return Instance;
        }
        Instance = new Server();
        Instance->Start();
        return Instance;
    }

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    ~Server();

private:
    void SendUserNotification(SOCKET clientSocket, std::string message);

    LibrarySystem* m_system;
    // Method to start the server
    void Start();

    void HandleClient(SOCKET clientSocket);
    static Server* Instance;
    Server() = default;

    SOCKET serverSocket;
    std::vector<std::string> m_activeSessionIds;
    std::unordered_map<std::string, Reader*> m_sessionReaders;

    void AcceptConnections();

    Reader* ProcessPacket(const Packet& packet, SOCKET clientSocket);

    std::string GenerateSessionId();

    bool VerifySessionId(const std::string& sessionId);

    void UserVerificationFailed(SOCKET clientSocket);
};