#pragma once

#include "iostream"
#include "vector"
#include "Book.h"
#include "Reader.h"

typedef struct {
	Reader* User;
	std::string Title;
} Subscriber;


//singleton to ensure there is only one library system users connect to.
//using observer pattern tailored to the specific requirements of notifying users about book availability
class LibrarySystem {
private:
	static LibrarySystem* Instance;
	//loading data when singleton is first initialized.
	LibrarySystem() {
		LoadUsers();
		LoadBooks();
	};
	void SaveUsers();
	void SaveBooks();
	void LoadBooks();
	void LoadUsers();
	std::vector<Subscriber> m_subscribers;
	std::vector<Reader*> m_readers;
	std::vector<Book> m_bookCollection;
public:
	std::vector<Book*> ShowOwned(Reader* User);
	std::vector<Book*> ShowCatalogue();
	Reader* Login(std::string Username, std::string Password);
	Reader* CreateAccount(std::string Username, std::string Password);
	static LibrarySystem * GetInstance();
	bool SubscribeUserToBook(Reader* Subscriber, std::string Title);
	BookStatus CheckBookAvailibility(std::string Title);
	bool ReturnBook(Reader* User, std::string BookTitle);
	bool BorrowBook(Reader* User, std::string Title);
};
