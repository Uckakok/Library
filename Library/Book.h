#pragma once

#include "iostream"

class Reader;

enum BookStatus {
	available,
	borrowed,
	nonexistant
};


class Book {
private:
	std::string m_title;
	std::string m_author;
	BookStatus m_status;
public:
	Book(std::string NewTitle, std::string NewAuthor);
	Reader* CurrentOwner;
	std::string GetTitle();
	BookStatus GetStatus();
	std::string GetAuthor();
	void ChangeStatus(BookStatus NewStatus);
};