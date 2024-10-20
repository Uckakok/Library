#include "LibrarySystem.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>

LibrarySystem* LibrarySystem::Instance{ nullptr };

void LibrarySystem::SaveUsers()
{
	std::ofstream outputFile("users", std::ios::trunc);

	if (!outputFile.is_open()) {
		std::cerr << "Error opening file: users" << std::endl;
		return;
	}

	for (auto& reader : m_readers) {
		outputFile << reader->Username << "|" << reader->Id << "|" << reader->HashedPassword << std::endl;
	}

	outputFile.close();
}

void LibrarySystem::SaveBooks()
{
	std::ofstream outputFile("books", std::ios::trunc);

	if (!outputFile.is_open()) {
		std::cerr << "Error opening file: books " << std::endl;
		return;
	}

	for (auto& book : m_bookCollection) {
		std::string ownerId = (book.CurrentOwner != nullptr) ? book.CurrentOwner->Id : "0";
		outputFile << book.GetTitle() << "|" << book.GetAuthor() << "|" << ownerId << std::endl;
	}

	outputFile.close();
}

void LibrarySystem::LoadBooks()
{
	std::ifstream inputFile("books");

	if (!inputFile.is_open()) {
		std::cerr << "Error opening file: books" << std::endl;
		return;
	}

	std::string line;
	while (getline(inputFile, line)) {
		std::istringstream iss(line);
		std::string title, author;
		std::string ownerId;

		getline(iss, title, '|');
		getline(iss, author, '|');
		getline(iss, ownerId);

		Book newBook(title, author);
		
		for (auto& user : m_readers) {
			if (user->Id == ownerId) {
				newBook.CurrentOwner = user;
				newBook.ChangeStatus(BookStatus::borrowed);
				break;
			}
		}

		m_bookCollection.push_back(newBook);
		if (newBook.CurrentOwner && newBook.GetStatus() != BookStatus::available) {
			newBook.CurrentOwner->AddBorrowedBook(&m_bookCollection.back());
		}
	}

	inputFile.close();
}

void LibrarySystem::LoadUsers()
{
	std::ifstream inputFile("users");

	if (!inputFile.is_open()) {
		std::cerr << "Error opening file: users" << std::endl;
		return;
	}

	std::string line;
	while (getline(inputFile, line)) {
		std::istringstream iss(line);
		std::string username, id, password;

		getline(iss, username, '|');
		getline(iss, id, '|');
		getline(iss, password);

		Reader* newUser = new Reader("", SOCKET_ERROR);
		newUser->Id = id;
		newUser->Username = username;
		newUser->HashedPassword = password;

		m_readers.push_back(newUser);
	}

	inputFile.close();
}

std::vector<Book*> LibrarySystem::ShowOwned(Reader* user)
{
	return user->Books;
}

std::vector<Book*> LibrarySystem::ShowCatalogue()
{
	std::vector<Book*> cat;
	for (auto& book : m_bookCollection) {
		cat.push_back(&book);
	}
	return cat;
}

Reader* LibrarySystem::Login(std::string username, std::string password)
{
	for (auto& user : m_readers) {
		if (user->Username == username && user->HashedPassword == password) {
			return user;
		}
	}
	std::cout << "User not found" << std::endl;
	return nullptr;
}

Reader* LibrarySystem::CreateAccount(std::string username, std::string password)
{
	for (auto& user : m_readers) {
		if (user->Username == username) {
			std::cout << "This username is already being used" << std::endl;
			return nullptr;
		}
	}
	Reader* newUser = new Reader("", SOCKET_ERROR);
	newUser->Username = username;
	newUser->HashedPassword = password;
	std::ostringstream userId;
	userId << "USER";
	std::time_t currentTime = time(nullptr);
	userId << currentTime;
	userId << rand() % 10000;
	newUser->Id = userId.str();
	m_readers.push_back(newUser);
	//update db
	SaveUsers();
	return newUser;
}

LibrarySystem * LibrarySystem::GetInstance()
{
	if (Instance) {
		return Instance;
	}
	Instance = new LibrarySystem();
	return Instance;
}

bool LibrarySystem::SubscribeUserToBook(Reader * User, std::string Title)
{
	for (const auto& existingSubscription : m_subscribers) {
		if (existingSubscription.User == User && existingSubscription.Title == Title) {
			//user is already subscribed to this book
			return false;
		}
	}

	bool bookExists = false;
	for (auto& currentBook : m_bookCollection) {
		if (currentBook.GetTitle() == Title) {
			if (currentBook.CurrentOwner == User) {
				//user already has this book, no need to subscribe
				return false;
			}
			bookExists = true;
			if (currentBook.GetStatus() == BookStatus::available) {
				//book available, no need to subscribe
				return false;
			}
		}
	}

	//book doesn't exist
	if (!bookExists) return false;

	Subscriber newSub;
	newSub.User = User;
	newSub.Title = Title;
	//add observer to the list
	m_subscribers.push_back(newSub);
	return true;
}

BookStatus LibrarySystem::CheckBookAvailibility(std::string Title)
{
	bool exists = false;
	for (auto& elem : m_bookCollection) {
		if (elem.GetTitle() == Title) {
			exists = true;
			if (elem.GetStatus() == BookStatus::available) {
				return BookStatus::available;
			}
		}
	}
	if (exists) return BookStatus::borrowed;
	return BookStatus::nonexistant;
}

bool LibrarySystem::ReturnBook(Reader* User, std::string BookTitle)
{
	Book* userBook = nullptr;
	for (auto& elem : m_bookCollection) {
		if (elem.GetTitle() == BookTitle && elem.CurrentOwner == User) {
			userBook = &elem;
			break;
		}
	}

	if (!userBook) return false;

	userBook->CurrentOwner = nullptr;
	userBook->ChangeStatus(BookStatus::available);
	auto it = std::find(User->Books.begin(), User->Books.end(), userBook);
    if (it != User->Books.end()) {
        User->Books.erase(it);
    }
	
	//notify observers
	for (int i = 0; i < (int)m_subscribers.size(); ++i) {
		if (m_subscribers[i].Title == userBook->GetTitle()) {
			std::string notification = "Book: " + userBook->GetTitle() + " is now available";
			m_subscribers[i].User->PostUserNotification(notification);
			m_subscribers.erase(m_subscribers.begin() + i);
			i--;
		}
	}

	SaveBooks();
	return true;
}

bool LibrarySystem::BorrowBook(Reader* User, std::string Title)
{
	if (CheckBookAvailibility(Title) != BookStatus::available) {
		return false;
	}
	Book* borrowedBook = nullptr;
	for (auto& elem : m_bookCollection) {
		if (elem.GetTitle() == Title && elem.GetStatus() == BookStatus::available) {
			borrowedBook = &elem;
			break;
		}
	}
	if (borrowedBook) {
		borrowedBook->CurrentOwner = User;
		borrowedBook->ChangeStatus(BookStatus::borrowed);
		User->AddBorrowedBook(borrowedBook);
	}

	SaveBooks();

	return true;
}
