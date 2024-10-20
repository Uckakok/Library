#pragma once
#include <iostream>
#include "Book.h"
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

#include "Structs.h"
#include "vector"

class Reader {
private:
	SOCKET ServerSocket;
	void SendPacket(const Packet& packet);
	void ListenToMessages();
	void ProcessPacket(Packet packet);
public:
	std::string Id;
	std::vector<std::string> Notifications;
	Reader(std::string NewSessionId, SOCKET NewServerSocket) : ServerSocket(NewServerSocket), SessionId(NewSessionId) { ; };
	void ClientMenu();
	void AddBorrowedBook(Book* NewBook);
	void PostUserNotification(std::string Message);
	std::string Username;
	std::string HashedPassword;
	std::string SessionId;
	std::vector<Book*> Books;
};