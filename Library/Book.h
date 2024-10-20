#pragma once

#include "iostream"

class Reader;

using namespace std;

enum BookStatus {
	available,
	borrowed,
	nonexistant
};


class Book {
private:
	string Title;
	string Author;
	BookStatus Status;
public:
	Book(string NewTitle, string NewAuthor);
	Reader* CurrentOwner;
	string GetTitle();
	BookStatus GetStatus();
	string GetAuthor();
	void ChangeStatus(BookStatus NewStatus);
};