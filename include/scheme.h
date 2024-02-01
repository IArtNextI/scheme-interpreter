#pragma once

#include <string>
#include "object.h"

class Interpreter {
private:
    Scope* global_scope_ = nullptr;

public:
    explicit Interpreter();
    std::string Run(const std::string&);
};
