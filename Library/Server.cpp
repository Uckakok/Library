#include "Server.h"
#include <string.h>
#include <chrono>
#include <algorithm>

Server* Server::Instance{ nullptr };

Server::~Server()
{
#ifdef _WIN32
    closesocket(serverSocket);
    WSACleanup();
#elif __linux__
    close(serverSocket);
#endif
}

void Server::Start() {
    if (!readConfig())
    {
        std::cout << "Couldn't read config file. Trying with fallback values" << std::endl;
    }
    m_system = LibrarySystem::GetInstance();

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef _WIN32
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return;
    }
#elif __linux__
    if (serverSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return;
    }
#endif

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(g_Port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "Error binding socket: " << g_IpAddress << ", " << g_Port << std::endl;
#ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
#elif __linux__
        close(serverSocket);
#endif
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) 
    {
        std::cerr << "Error listening on socket." << std::endl;
#ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
#elif __linux__
        close(serverSocket);
#endif
        return;
    }

    std::cout << "Server started. Listening on port " << g_Port << "..." << std::endl;

    AcceptConnections();

#ifdef _WIN32
    closesocket(serverSocket);
    WSACleanup();
#elif __linux__
    close(serverSocket);
#endif
}

void Server::AcceptConnections() {
    while (true) {
        sockaddr_in clientAddr;
#ifdef _WIN32
        int clientAddrSize = sizeof(clientAddr);
#elif __linux__
        socklen_t clientAddrSize = sizeof(clientAddr);
#endif
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection." << std::endl;
            continue;
        }

#ifdef _WIN32
        u_long mode = 1;
        if (ioctlsocket(clientSocket, FIONBIO, &mode) != NO_ERROR) {
            std::cerr << "Error setting socket to non-blocking mode." << std::endl;
            closesocket(clientSocket);
            continue;
        }
#elif __linux__
        int flags = fcntl(clientSocket, F_GETFL, 0);
        if (flags == -1) {
            std::cerr << "Error getting socket flags." << std::endl;
            close(clientSocket);
            continue;
        }
        if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
            std::cerr << "Error setting socket to non-blocking mode." << std::endl;
            close(clientSocket);
            continue;
        }
#endif

        std::cout << "Client connected." << std::endl;

        std::thread clientThread(&Server::HandleClient, this, clientSocket);
        clientThread.detach();
    }
}

void Server::HandleClient(SOCKET clientSocket) {

    std::cout << "Started handling new client" << std::endl;

    Reader* handledUser = nullptr;

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    while (true) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);

#ifdef _WIN32
        int selectResult = select(0, &readSet, nullptr, nullptr, &timeout);
#elif __linux__
        int selectResult = select(clientSocket + 1, &readSet, nullptr, nullptr, &timeout);
#endif

        if (selectResult == SOCKET_ERROR) {
#ifdef _WIN32
            int error = WSAGetLastError();
#elif __linux__
            int error = errno;
#endif
            std::cerr << "Error in select: " << error << std::endl;
            break;
        }
        else if (selectResult != 0) {
            Packet packet;
            int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
            if (bytesReceived == SOCKET_ERROR) {
#ifdef _WIN32
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
#elif __linux__
                int error = errno;
                if (error == EWOULDBLOCK || error == EAGAIN) {
#endif
                    continue;
                }
                std::cerr << "Error receiving packet from client: " << error << std::endl;
                break;
            }
            else if (bytesReceived == 0) {
                std::cout << "Client disconnected." << std::endl;
                break;
            }
            else {
                Reader* processResult;
                processResult = ProcessPacket(packet, clientSocket);
                if (!processResult) {
                    break;
                }
                else if (processResult->SessionId == "Ok") {
                    delete processResult;
                }
                else {
                    handledUser = processResult;
                }
            }
        }

        if (handledUser) {
            if (!handledUser->Notifications.empty()) {
                SendUserNotification(clientSocket, handledUser->Notifications.front());
                handledUser->Notifications.erase(handledUser->Notifications.begin());
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Finished handling client" << std::endl;

#ifdef _WIN32
    closesocket(clientSocket);
#elif __linux__
    close(clientSocket);
#endif
}

Reader* Server::ProcessPacket(const Packet& packet, SOCKET clientSocket) {

    Packet response;

    switch (packet.action) {
    case CREATE_ACCOUNT:{
        std::cout << "Received request to create account for user: " << packet.Username << std::endl;

        Reader* newUser = m_system->CreateAccount(packet.Username, packet.Password);

        if (!newUser) {
            std::cout << "Failed to create new account" << std::endl;
            response.action = LOGIN_FAIL;
        }
        else {
            std::string sessionId = GenerateSessionId();
            m_activeSessionIds.push_back(sessionId);

            m_sessionReaders[sessionId] = newUser;

            response.action = LOGIN_SUCCESS;
            strncpy(response.SessionId, sessionId.c_str(), sizeof(response.SessionId) - 1);
        }

        send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
        return newUser;
    }
    case LOGIN: {
        std::cout << "Received login request for user: " << packet.Username << std::endl;

        Reader* newUser = m_system->Login(packet.Username, packet.Password);

        if (!newUser) {
            std::cout << "Someone tried to login as " << packet.Username << " with invalid login credentials" << std::endl;
            response.action = LOGIN_FAIL;
        }
        else {
            std::string sessionId = GenerateSessionId();
            m_activeSessionIds.push_back(sessionId);

            m_sessionReaders[sessionId] = newUser;

            response.action = LOGIN_SUCCESS;
            strncpy(response.SessionId, sessionId.c_str(), sizeof(response.SessionId) - 1);
        }

        send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
        return newUser; 
    }
    case CLOSE_SESSION:
    {
        std::cout << "Closing session for user: " << m_sessionReaders[packet.SessionId]->Username << std::endl;
        m_activeSessionIds.erase(std::remove(m_activeSessionIds.begin(), m_activeSessionIds.end(), packet.SessionId), m_activeSessionIds.end());

        auto it = m_sessionReaders.find(packet.SessionId);
        if (it != m_sessionReaders.end()) {
            m_sessionReaders.erase(it);
        }
        else {
            std::cout << "User associated with session ID " << packet.SessionId << " not found." << std::endl;
        }

        return nullptr;
    }
    case BORROW_BOOK: 
        if (!VerifySessionId(packet.SessionId)) {
            UserVerificationFailed(clientSocket);
            break;
        }
        std::cout << "Processing request to borrow book: " << packet.Title << " by user: " << m_sessionReaders[packet.SessionId]->Username << std::endl;

        if (!m_system->BorrowBook(m_sessionReaders[packet.SessionId], packet.Title)) {
            response.action = BORROW_FAIL;
        }
        else {
            response.action = BORROW_BOOK;
        }
        strncpy(response.Title, packet.Title, sizeof(response.Title) - 1);
        send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
        break;
    case RETURN_BOOK:
        if (!VerifySessionId(packet.SessionId)) {
            UserVerificationFailed(clientSocket);
            break;
        }

        std::cout << "Processing request to return a book: " << packet.Title << " by user: " << m_sessionReaders[packet.SessionId]->Username << std::endl;

        if (!m_system->ReturnBook(m_sessionReaders[packet.SessionId], packet.Title)) {
            response.action = RETURN_FAIL;
        }
        else {
            response.action = RETURN_BOOK;
        }

        strncpy(response.Title, packet.Title, sizeof(response.Title) - 1);
        send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
        break;
    case SUBSCRIBE_BOOK:
        if (!VerifySessionId(packet.SessionId)) {
            UserVerificationFailed(clientSocket);
            break;
        }

        std::cout << "Processing request to subscribe to book: " << packet.Title << " by user: " << m_sessionReaders[packet.SessionId]->Username << std::endl;

        if (!m_system->SubscribeUserToBook(m_sessionReaders[packet.SessionId], packet.Title)){
            response.action = SUBSCRIBE_FAIL;
        }
        else {
            response.action = SUBSCRIBE_BOOK;
        }

        strncpy(response.Title, packet.Title, sizeof(response.Title) - 1);
        send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
        break;
    case AVAILABILITY_BOOK:
        if (!VerifySessionId(packet.SessionId)) {
            UserVerificationFailed(clientSocket);
            break;
        }

        std::cout << "Processing request to check availability of book: " << packet.Title << " by user: " << m_sessionReaders[packet.SessionId]->Username << std::endl;

        if (m_system->CheckBookAvailibility(packet.Title) != BookStatus::available) {
            response.action = BOOK_UNAVAILABLE;
        }
        else {
            response.action = BOOK_AVAILABLE;
        }

        strncpy(response.Title, packet.Title, sizeof(response.Title) - 1);
        send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
        break;
    case REQUEST_CATALOGUE:
        if (!VerifySessionId(packet.SessionId)) {
            UserVerificationFailed(clientSocket);
            break;
        }

        std::cout << "Processing request to show catalogue by user: " << m_sessionReaders[packet.SessionId]->Username << std::endl;

        for (auto& book : m_system->ShowCatalogue()) {
            response.action = BOOK_INFO;
            strncpy(response.Title, book->GetTitle().c_str(), sizeof(response.Title) - 1);
            strncpy(response.Author, book->GetAuthor().c_str(), sizeof(response.Author) - 1);
            send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
        }
        break;
    case REQUEST_OWNED:
        if (!VerifySessionId(packet.SessionId)) {
            UserVerificationFailed(clientSocket);
            break;
        }

        std::cout << "Processing request to show owned books by user: " << m_sessionReaders[packet.SessionId]->Username << std::endl;

        for (auto& book : m_system->ShowOwned(m_sessionReaders[packet.SessionId])) {
            response.action = BOOK_INFO;
            strncpy(response.Title, book->GetTitle().c_str(), sizeof(response.Title) - 1);
            strncpy(response.Author, book->GetAuthor().c_str(), sizeof(response.Author) - 1);
            send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
        }
        break;
    default:
        std::cout << "Unexpected user action encountered" << std::endl;
    }
    return new Reader("Ok", SOCKET_ERROR);
}

std::string Server::GenerateSessionId()
{
    return "SESSION_" + std::to_string(rand());
}


bool Server::VerifySessionId(const std::string& sessionId)
{
    return find(m_activeSessionIds.begin(), m_activeSessionIds.end(), sessionId) != m_activeSessionIds.end();
}

void Server::SendUserNotification(SOCKET clientSocket, std::string Message)
{
    Packet response;
    response.action = NOTIFICATION;
    strncpy(response.Username, Message.c_str(), sizeof(response.Username) - 1);

    send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
}

void Server::UserVerificationFailed(SOCKET clientSocket)
{
    std::cout << "User sent a request with invalid session id!" << std::endl;

    Packet response;
    response.action = LOGIN_FAIL;

    send(clientSocket, reinterpret_cast<const char*>(&response), sizeof(response), 0);
}
