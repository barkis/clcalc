#ifndef BADENTRYEXCEPTION_H
#define BADENTRYEXCEPTION_H
#include <exception>
#include <string>
class BadEntryException: public std::exception  {
public:
    BadEntryException(std::string sMessage){m_sMessage = sMessage;}
    virtual ~BadEntryException(){}
    virtual const char *what() const noexcept {return ("Invalid entry: " + m_sMessage).c_str();}
private:
    std::string m_sMessage;
};

#endif // BADENTRYEXCEPTION_H
