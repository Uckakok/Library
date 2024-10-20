#pragma once

#include "iostream"
#include "vector"
#include "Book.h"
#include "Reader.h"

using namespace std;

typedef struct {
	Reader* User;
	string Title;
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
	vector<Subscriber> Subscribers;
	vector<Reader*> Readers;
	vector<Book> BookCollection;
public:
	vector<Book*> ShowOwned(Reader* User);
	vector<Book*> ShowCatalogue();
	Reader* Login(string Username, string Password);
	Reader* CreateAccount(string Username, string Password);
	static LibrarySystem * GetInstance();
	bool SubscribeUserToBook(Reader* Subscriber, string Title);
	BookStatus CheckBookAvailibility(string Title);
	bool ReturnBook(Reader* User, string BookTitle);
	bool BorrowBook(Reader* User, string Title);
};
