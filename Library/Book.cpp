#include "Book.h"

Book::Book(string NewTitle, string NewAuthor)
{
	Title = NewTitle;
	Author = NewAuthor;
	Status = BookStatus::available;
	CurrentOwner = nullptr;
}

string Book::GetTitle()
{
	return Title;
}

BookStatus Book::GetStatus()
{
	return Status;
}

string Book::GetAuthor()
{
	return Author;
}

void Book::ChangeStatus(BookStatus NewStatus)
{
	Status = NewStatus;
}
