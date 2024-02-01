#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <string>

#include "error.h"

struct SymbolToken {
    std::string name;

    SymbolToken(const std::string& name);

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

enum class BooleanToken { FALSE, TRUE };

struct ConstantToken {
    int64_t value;

    ConstantToken(const int64_t& value);

    bool operator==(const ConstantToken& other) const;
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, BooleanToken>;

class Tokenizer {
private:
    std::istream* in_ = nullptr;
    Token current_token_ = ConstantToken(0);
    bool end_ = false;

    bool IsStartForSymbol(char nc);

    bool IsInnerForSymbol(char nc);

    void ParseTokenAndStore();

public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();
};