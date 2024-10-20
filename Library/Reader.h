#pragma once
#include "iostream"
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

using namespace std; // Forward declaration of LibrarySystem class

class Reader {
private:
	SOCKET ServerSocket;
	void SendPacket(const Packet& packet);
	void ListenToMessages();
	void ProcessPacket(Packet packet);
public:
	string Id;
	vector<string> Notifications;
	Reader(string NewSessionId, SOCKET NewServerSocket) : ServerSocket(NewServerSocket), SessionId(NewSessionId) { ; };
	void ClientMenu();
	void AddBorrowedBook(Book* NewBook);
	void PostUserNotification(string Message);
	string Username;
	string HashedPassword;
	string SessionId;
	vector<Book*> Books;
};