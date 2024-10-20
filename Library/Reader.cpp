#include "Reader.h"
#include "string"
#include <chrono>
#include <thread>
#include <string.h>
#include "iostream"

using namespace std;

void Reader::SendPacket(const Packet& packet)
{
	// Send packet
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
		// Read from the socket and process messages
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

		// Sleep for 100 milliseconds
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void Reader::ProcessPacket(Packet packet)
{
	//Simply print what happened
	switch (packet.action) {
	case BORROW_BOOK:
		cout << "Book " << packet.Title << " borrowed successfully." << endl;
		break;
	case BORROW_FAIL:
		cout << "Failed to borrow book " << packet.Title << "." << endl;
		break;
	case RETURN_BOOK:
		cout << "Book " << packet.Title << " returned successfully." << endl;
		break;
	case RETURN_FAIL:
		cout << "Failed to return book" << packet.Title << "." << endl;
		break;
	case SUBSCRIBE_BOOK:
		cout << "Subscribed to book " << packet.Title << " successfully." << endl;
		break;
	case SUBSCRIBE_FAIL:
		cout << "Failed to subscribe to book " << packet.Title << "." << endl;
		break;
	case BOOK_AVAILABLE:
		cout << "Book " << packet.Title << " is available." << endl;
		break;
	case BOOK_UNAVAILABLE:
		cout << "Book " << packet.Title << " is unavailable." << endl;
		break;
	case NOTIFICATION:
		cout << "Notification: " << packet.Username << endl;
		break;
	case BOOK_INFO:
		cout << "Title: " << packet.Title << "\tAuthor: " << packet.Author << endl;
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
		cout << "1 - borrow a book" << endl;
		cout << "2 - display borrowed books" << endl;
		cout << "3 - return a book" << endl;
		cout << "4 - subscribe to a book" << endl;
		cout << "5 - check book availability" << endl;
		cout << "6 - display catalog" << endl;
		cout << "0 - logout" << endl;
		int Choice;
		while (!(cin >> Choice)) {
			cout << "Invalid input. Please enter a valid option." << endl;
			cin.clear(); // Clear fail state
			cin.ignore(1000, '\n');
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
			cout << "Enter title of the book you want to borrow: ";
			while (getchar() != '\n');
			if (scanf("%[^\n]", Title) != 1) {
				cout << "Invalid input. Please enter a valid title." << endl;
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
			cout << "Enter title of the book you want to return: ";
			while (getchar() != '\n');
			if (scanf("%[^\n]", Title) != 1) {
				cout << "Invalid input. Please enter a valid title." << endl;
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
			cout << "Enter title of the book you want to subscribe to: ";
			while (getchar() != '\n');
			if (scanf("%[^\n]", Title) != 1) {
				cout << "Invalid input. Please enter a valid title." << endl;
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
			cout << "Enter title of the book you want to check availability for: ";
			while (getchar() != '\n');
			if (scanf("%[^\n]", Title) != 1) {
				cout << "Invalid input. Please enter a valid title." << endl;
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

void Reader::PostUserNotification(string Message)
{
	Notifications.push_back(Message);
}
