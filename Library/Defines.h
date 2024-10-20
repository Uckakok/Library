#pragma once
#include <exception>
#include <string>

#define MAXPACKETTEXT   64

extern std::string g_IpAddress;
extern int g_Port;

bool readConfig();


enum Action {
    CREATE_ACCOUNT,
    LOGIN,
    LOGIN_SUCCESS,
    LOGIN_FAIL,
    CLOSE_SESSION,
    BORROW_BOOK,
    BORROW_FAIL,
    RETURN_BOOK,
    RETURN_FAIL,
    SUBSCRIBE_BOOK,
    SUBSCRIBE_FAIL,
    AVAILABILITY_BOOK,
    BOOK_AVAILABLE,
    BOOK_UNAVAILABLE,
    NOTIFICATION,
    REQUEST_CATALOGUE,
    REQUEST_OWNED,
    BOOK_INFO,
};

struct Packet {
    char Username[MAXPACKETTEXT];
    char Password[MAXPACKETTEXT];
    char SessionId[MAXPACKETTEXT];
    char Title[MAXPACKETTEXT];
    char Author[MAXPACKETTEXT];
    Action action;
};


class LibraryException : public std::exception {
public:
    LibraryException(const std::string& message) : m_message(message) {}

    const char* what() const noexcept override {
        return m_message.c_str();
    }

private:
    std::string m_message;
};