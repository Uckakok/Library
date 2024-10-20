#include "LibrarySystem.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>

LibrarySystem* LibrarySystem::Instance{ nullptr };

void LibrarySystem::SaveUsers()
{
	//issue: if users in memory are corrupted, database will be wiped
	ofstream outputFile("users", ios::trunc);

	if (!outputFile.is_open()) {
		cerr << "Error opening file: users" << endl;
		return;
	}

	for (auto& reader : Readers) {
		outputFile << reader->Username << "|" << reader->Id << "|" << reader->HashedPassword << endl;
	}

	outputFile.close();
}

void LibrarySystem::SaveBooks()
{
	//issue: if books in memory are corrupted, database will be wiped
	ofstream outputFile("books", ios::trunc);

	if (!outputFile.is_open()) {
		cerr << "Error opening file: books " << endl;
		return;
	}

	for (auto& book : BookCollection) {
		string ownerId = (book.CurrentOwner != nullptr) ? book.CurrentOwner->Id : "0";
		outputFile << book.GetTitle() << "|" << book.GetAuthor() << "|" << ownerId << endl;
	}

	outputFile.close();
}

void LibrarySystem::LoadBooks()
{
	ifstream inputFile("books");

	if (!inputFile.is_open()) {
		std::cerr << "Error opening file: books" <<endl;
		return;
	}

	string line;
	while (getline(inputFile, line)) {
		istringstream iss(line);
		string title, author;
		string ownerId;

		getline(iss, title, '|');
		getline(iss, author, '|');
		getline(iss, ownerId);

		Book newBook(title, author);
		
		for (auto& User : Readers) {
			if (User->Id == ownerId) {
				newBook.CurrentOwner = User;
				newBook.ChangeStatus(BookStatus::borrowed);
				break;
			}
		}

		BookCollection.push_back(newBook);
		if (newBook.CurrentOwner && newBook.GetStatus() != BookStatus::available) {
			newBook.CurrentOwner->AddBorrowedBook(&BookCollection.back());
		}
	}

	inputFile.close();
}

void LibrarySystem::LoadUsers()
{
	ifstream inputFile("users");

	if (!inputFile.is_open()) {
		cerr << "Error opening file: users" << endl;
		return;
	}

	string line;
	while (getline(inputFile, line)) {
		istringstream iss(line);
		string username, id, password;

		getline(iss, username, '|');
		getline(iss, id, '|');
		getline(iss, password);

		Reader* NewUser = new Reader("", SOCKET_ERROR);
		NewUser->Id = id;
		NewUser->Username = username;
		NewUser->HashedPassword = password;

		Readers.push_back(NewUser);
	}

	inputFile.close();
}

vector<Book*> LibrarySystem::ShowOwned(Reader* User)
{
	return User->Books;
}

vector<Book*> LibrarySystem::ShowCatalogue()
{
	vector<Book*> Cat;
	for (auto& book : BookCollection) {
		Cat.push_back(&book);
	}
	return Cat;
}

Reader* LibrarySystem::Login(string Username, string Password)
{
	for (auto& User : Readers) {
		if (User->Username == Username && User->HashedPassword == Password) {
			return User;
		}
	}
	cout << "User not found" << endl;
	return nullptr;
}

Reader* LibrarySystem::CreateAccount(string Username, string Password)
{
	for (auto& User : Readers) {
		if (User->Username == Username) {
			cout << "This username is already being used" << endl;
			return nullptr;
		}
	}
	Reader* NewUser = new Reader("", SOCKET_ERROR);
	NewUser->Username = Username;
	NewUser->HashedPassword = Password;
	ostringstream userId;
	userId << "USER";
	std::time_t currentTime = time(nullptr);
	userId << currentTime;
	userId << rand() % 10000;
	NewUser->Id = userId.str();
	Readers.push_back(NewUser);
	//update db
	SaveUsers();
	return NewUser;
}

LibrarySystem * LibrarySystem::GetInstance()
{
	if (Instance) {
		return Instance;
	}
	Instance = new LibrarySystem();
	return Instance;
}

bool LibrarySystem::SubscribeUserToBook(Reader * User, string Title)
{
	for (const auto& existingSubscription : Subscribers) {
		if (existingSubscription.User == User && existingSubscription.Title == Title) {
			// User is already subscribed to this book
			return false;
		}
	}

	bool bBookExists = false;
	for (auto& CurrentBook : BookCollection) {
		if (CurrentBook.GetTitle() == Title) {
			if (CurrentBook.CurrentOwner == User) {
				//user already has this book, no need to subscribe
				return false;
			}
			bBookExists = true;
			if (CurrentBook.GetStatus() == BookStatus::available) {
				//book available, no need to subscribe
				return false;
			}
		}
	}

	//book doesn't exist
	if (!bBookExists) return false;

	Subscriber NewSub;
	NewSub.User = User;
	NewSub.Title = Title;
	//add observer to the list
	Subscribers.push_back(NewSub);
	return true;
}

BookStatus LibrarySystem::CheckBookAvailibility(string Title)
{
	bool bExists = false;
	for (auto& elem : BookCollection) {
		if (elem.GetTitle() == Title) {
			bExists = true;
			if (elem.GetStatus() == BookStatus::available) {
				return BookStatus::available;
			}
		}
	}
	if (bExists) return BookStatus::borrowed;
	return BookStatus::nonexistant;
}

bool LibrarySystem::ReturnBook(Reader* User, string BookTitle)
{
	Book* UserBook = nullptr;
	for (auto& elem : BookCollection) {
		if (elem.GetTitle() == BookTitle && elem.CurrentOwner == User) {
			UserBook = &elem;
			break;
		}
	}

	if (!UserBook) return false;

	UserBook->CurrentOwner = nullptr;
	UserBook->ChangeStatus(BookStatus::available);
	auto it = std::find(User->Books.begin(), User->Books.end(), UserBook);
    if (it != User->Books.end()) {
        User->Books.erase(it);
    }
	
	//notify observers
	for (int i = 0; i < (int)Subscribers.size(); ++i) {
		if (Subscribers[i].Title == UserBook->GetTitle()) {
			string notification = "Book: " + UserBook->GetTitle() + " is now available";
			Subscribers[i].User->PostUserNotification(notification);
			Subscribers.erase(Subscribers.begin() + i);
			i--;
		}
	}
	//update db
	SaveBooks();
	return true;
}

bool LibrarySystem::BorrowBook(Reader* User, string Title)
{
	if (CheckBookAvailibility(Title) != BookStatus::available) {
		return false;
	}
	Book* BorrowedBook = nullptr;
	for (auto& elem : BookCollection) {
		if (elem.GetTitle() == Title && elem.GetStatus() == BookStatus::available) {
			BorrowedBook = &elem;
			break;
		}
	}
	if (BorrowedBook) {
		BorrowedBook->CurrentOwner = User;
		BorrowedBook->ChangeStatus(BookStatus::borrowed);
		User->AddBorrowedBook(BorrowedBook);
	}
	//update db
	SaveBooks();

	return true;
}
