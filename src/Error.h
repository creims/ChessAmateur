#ifndef CHESSAMATEUR3_ERROR_H
#define CHESSAMATEUR3_ERROR_H

#include <string>

class Error : std::exception {
public:
    explicit Error(std::string msg) : _msg{std::move(msg)} {}

    std::string what() { return _msg; }
private:
    std::string _msg;
};

#endif //CHESSAMATEUR3_ERROR_H