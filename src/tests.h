#ifndef LOGSTALGIA_TESTER_H
#define LOGSTALGIA_TESTER_H

#include <string>

class TesterException : public std::exception {
    std::string message;
public:
    TesterException(const std::string& message) : message(message) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class LogstalgiaTester {
public:
    LogstalgiaTester();

    void runTests();
};

#endif
