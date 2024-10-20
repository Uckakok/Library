#include "Book.h"

Book::Book(std::string newTitle, std::string newAuthor)
{
	m_title = newTitle;
	m_author = newAuthor;
	m_status = BookStatus::available;
	CurrentOwner = nullptr;
}

std::string Book::GetTitle()
{
	return m_title;
}

BookStatus Book::GetStatus()
{
	return m_status;
}

std::string Book::GetAuthor()
{
	return m_author;
}

void Book::ChangeStatus(BookStatus newStatus)
{
	m_status = newStatus;
}
