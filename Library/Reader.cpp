#include "Reader.h"
#include "string"
#include <chrono>
#include <thread>
#include <string.h>
#include "iostream"

void Reader::SendPacket(const Packet& packet)
{
	if (send(ServerSocket, reinterpret_cast<const char*>(&packet), sizeof(packet), 0) == SOCKET_ERROR) {
		#ifdef _WIN32
    closesocket(ServerSocket);
#elif __linux__
    close(ServerSocket);
#endif
		throw LibraryException("Error sending packet to server.");
	}
}

void Reader::ListenToMessages()
{
	while (true) {
		Packet packet;
		int bytesReceived;
#ifdef _WIN32
		bytesReceived = recv(ServerSocket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
		if (bytesReceived == SOCKET_ERROR) {
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK) {
				std::cerr << "You were disconnected from the server" << std::endl;
				break;
			}
		}
#elif __linux__
		bytesReceived = recv(ServerSocket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
		if (bytesReceived == -1) {
			if (errno != EWOULDBLOCK) {
				std::cerr << "Error receiving packet from server: " << errno << std::endl;
				break;
			}
		}
#endif
		else if (bytesReceived == 0) {
			std::cout << "Server disconnected." << std::endl;
			break;
		}
		else {
			ProcessPacket(packet);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void Reader::ProcessPacket(Packet packet)
{
	//Simply print what happened
	switch (packet.action) {
	case BORROW_BOOK:
		std::cout << "Book " << packet.Title << " borrowed successfully." << std::endl;
		break;
	case BORROW_FAIL:
		std::cout << "Failed to borrow book " << packet.Title << "." << std::endl;
		break;
	case RETURN_BOOK:
		std::cout << "Book " << packet.Title << " returned successfully." << std::endl;
		break;
	case RETURN_FAIL:
		std::cout << "Failed to return book" << packet.Title << "." << std::endl;
		break;
	case SUBSCRIBE_BOOK:
		std::cout << "Subscribed to book " << packet.Title << " successfully." << std::endl;
		break;
	case SUBSCRIBE_FAIL:
		std::cout << "Failed to subscribe to book " << packet.Title << "." << std::endl;
		break;
	case BOOK_AVAILABLE:
		std::cout << "Book " << packet.Title << " is available." << std::endl;
		break;
	case BOOK_UNAVAILABLE:
		std::cout << "Book " << packet.Title << " is unavailable." << std::endl;
		break;
	case NOTIFICATION:
		std::cout << "Notification: " << packet.Username << std::endl;
		break;
	case BOOK_INFO:
		std::cout << "Title: " << packet.Title << "\tAuthor: " << packet.Author << std::endl;
	default:
		//cout << "Unexpected packet action encountered." << endl;
		;
	}
}

void Reader::ClientMenu()
{
	std::thread readerThread(&Reader::ListenToMessages, this);
	readerThread.detach();

	do {
		std::cout << "1 - borrow a book" << std::endl;
		std::cout << "2 - display borrowed books" << std::endl;
		std::cout << "3 - return a book" << std::endl;
		std::cout << "4 - subscribe to a book" << std::endl;
		std::cout << "5 - check book availability" << std::endl;
		std::cout << "6 - display catalog" << std::endl;
		std::cout << "0 - logout" << std::endl;
		int Choice;
		while (!(std::cin >> Choice)) {
			std::cout << "Invalid input. Please enter a valid option." << std::endl;
			std::cin.clear();
			std::cin.ignore(1000, '\n');
		}
		switch (Choice) {
		case 0:
			//send close session packet
			Packet packet;
			packet.action = CLOSE_SESSION;
			strncpy(packet.SessionId, SessionId.c_str(), sizeof(packet.SessionId) - 1);
			SendPacket(packet);
			return;
		case 1: {
			char Title[MAXPACKETTEXT];
			std::cout << "Enter title of the book you want to borrow: ";
			while (getchar() != '\n');
			if (scanf("%[^\n]", Title) != 1) {
				std::cout << "Invalid input. Please enter a valid title." << std::endl;
				while (getchar() != '\n');
			}
			while (getchar() != '\n');
			Title[sizeof(Title) - 1] = '\0';
			Packet packet;
			strncpy(packet.Title, Title, sizeof(packet.Title) - 1);
			packet.action = BORROW_BOOK;
			strncpy(packet.SessionId, SessionId.c_str(), sizeof(packet.SessionId) - 1);
			SendPacket(packet);
			break; }
		case 2:{
			Packet packet;
			packet.action = REQUEST_OWNED;
			strncpy(packet.SessionId, SessionId.c_str(), sizeof(packet.SessionId) - 1);
			SendPacket(packet);
			break; }
		case 3: {
			char Title[MAXPACKETTEXT];
			std::cout << "Enter title of the book you want to return: ";
			while (getchar() != '\n');
			if (scanf("%[^\n]", Title) != 1) {
				std::cout << "Invalid input. Please enter a valid title." << std::endl;
				while (getchar() != '\n');
			}
			while (getchar() != '\n');
			Title[sizeof(Title) - 1] = '\0';
			Packet packet;
			strncpy(packet.Title, Title, sizeof(packet.Title) - 1);
			packet.action = RETURN_BOOK;
			strncpy(packet.SessionId, SessionId.c_str(), sizeof(packet.SessionId) - 1);
			SendPacket(packet);
			break;
		}
		case 4: {
			char Title[MAXPACKETTEXT];
			std::cout << "Enter title of the book you want to subscribe to: ";
			while (getchar() != '\n');
			if (scanf("%[^\n]", Title) != 1) {
				std::cout << "Invalid input. Please enter a valid title." << std::endl;
				while (getchar() != '\n');
			}
			while (getchar() != '\n');
			Title[sizeof(Title) - 1] = '\0';
			Packet packet;
			strncpy(packet.Title, Title, sizeof(packet.Title) - 1);
			packet.action = SUBSCRIBE_BOOK;
			strncpy(packet.SessionId, SessionId.c_str(), sizeof(packet.SessionId) - 1);
			SendPacket(packet);
			break; }
		case 5: {
			char Title[MAXPACKETTEXT];
			std::cout << "Enter title of the book you want to check availability for: ";
			while (getchar() != '\n');
			if (scanf("%[^\n]", Title) != 1) {
				std::cout << "Invalid input. Please enter a valid title." << std::endl;
				while (getchar() != '\n');
			}
			while (getchar() != '\n');
			Title[sizeof(Title) - 1] = '\0';
			Packet packet;
			strncpy(packet.Title, Title, sizeof(packet.Title) - 1);
			packet.action = AVAILABILITY_BOOK;
			strncpy(packet.SessionId, SessionId.c_str(), sizeof(packet.SessionId) - 1);
			SendPacket(packet);
			break;
		}
		case 6: {
			Packet packet;
			packet.action = REQUEST_CATALOGUE;
			strncpy(packet.SessionId, SessionId.c_str(), sizeof(packet.SessionId) - 1);
			SendPacket(packet);
			break; }
		default:
			break;
		}

	} while (true);
}

void Reader::AddBorrowedBook(Book * NewBook)
{
	Books.push_back(NewBook);
}

void Reader::PostUserNotification(std::string Message)
{
	Notifications.push_back(Message);
}
