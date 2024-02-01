#include <tokenizer.h>

SymbolToken::SymbolToken(const std::string& name) : name(name) {
}

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

ConstantToken::ConstantToken(const int64_t& value) : value(value) {
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

bool Tokenizer::IsStartForSymbol(char nc) {
    return std::isalpha(nc) || nc == '<' || nc == '=' || nc == '>' || nc == '*' || nc == '/' ||
           nc == '#';
}

bool Tokenizer::IsInnerForSymbol(char nc) {
    return std::isalnum(nc) || nc == '<' || nc == '=' || nc == '>' || nc == '*' || nc == '/' ||
           nc == '?' || nc == '!' || nc == '.' || nc == '-' || nc == '#';
}

void Tokenizer::ParseTokenAndStore() {
    char c = in_->peek();
    while (std::isspace(c)) {
        in_->get();
        c = in_->peek();
    }
    if (c == EOF) {
        end_ = true;
        return;
    }
    // else it's some kind of token
    // let's do the easy ones first
    if (c == '\'') {
        in_->get();
        current_token_ = QuoteToken();
        return;
    } else if (c == '(') {
        in_->get();
        current_token_ = BracketToken::OPEN;
        return;
    } else if (c == ')') {
        in_->get();
        current_token_ = BracketToken::CLOSE;
        return;
    } else if (c == '.') {
        in_->get();
        current_token_ = DotToken();
        return;
    }
    // left options are boolean, number of a string
    // dumb check : strings are not permitted to start with numbers
    if (std::isdigit(c)) {
        // then this is for sure a number, read the numbers while valid
        int64_t current = 0;
        while (std::isdigit(c)) {
            c = in_->get();
            current = 10 * current + c - 48;
            c = in_->peek();
        }
        // not a number anymore, end
        current_token_ = ConstantToken(current);
        return;
    } else {
        if (c == '#') {
            // either boolean or a string;
            c = in_->get();
            char nc = in_->peek();
            if (nc == 't' || nc == 'f') {
                nc = in_->get();
                // can be a boolean
                char nnc = in_->peek();
                if (!IsInnerForSymbol(nnc)) {
                    current_token_ = (nc == 't' ? BooleanToken::TRUE : BooleanToken::FALSE);
                    return;
                } else {
                    // definitely a string;
                    std::string result;
                    result += c;
                    result += nc;
                    while (IsInnerForSymbol(nnc)) {
                        nnc = in_->get();
                        result += nnc;
                        nnc = in_->peek();
                    }
                    current_token_ = SymbolToken(result);
                }
            } else {
                if (nc == EOF) {
                    nc = in_->get();
                    current_token_ = SymbolToken("#");
                    return;
                }
                // definitely a string
                std::string result;
                result += c;
                while (IsInnerForSymbol(nc)) {
                    nc = in_->get();
                    result += nc;
                    nc = in_->peek();
                }
                current_token_ = SymbolToken(result);
                return;
            }
        } else if (c == '+') {
            // either positive integer or a string
            c = in_->get();
            char nc = in_->peek();
            if (!std::isdigit(nc)) {
                current_token_ = SymbolToken("+");
                return;
            } else {
                // it's a positive integer
                int64_t current = 0;
                while (std::isdigit(nc)) {
                    nc = in_->get();
                    current = 10 * current + nc - 48;
                    nc = in_->peek();
                }
                // not a number anymore, end
                current_token_ = ConstantToken(current);
                return;
            }
        } else if (c == '-') {
            // either negative integer or a string
            c = in_->get();
            char nc = in_->peek();
            if (!std::isdigit(nc)) {
                current_token_ = SymbolToken("-");
                return;
            } else {
                // it's a positive integer
                int64_t current = 0;
                while (std::isdigit(nc)) {
                    nc = in_->get();
                    current = 10 * current + nc - 48;
                    nc = in_->peek();
                }
                // not a number anymore, end
                current_token_ = ConstantToken(-current);
                return;
            }
        } else {
            // definitely a string
            // could be some real shit, let's throw for this one;
            std::string result;
            c = in_->get();
            if (!IsStartForSymbol(c)) {
                throw SyntaxError{std::string() + "Unrecognized token starting with symbol '" + c +
                                  "'"};
            }
            result += c;
            char nc = in_->peek();
            while (IsInnerForSymbol(nc)) {
                nc = in_->get();
                result += nc;
                nc = in_->peek();
            }
            current_token_ = SymbolToken(result);
            return;
        }
    }
}

Tokenizer::Tokenizer(std::istream* in) : in_(in) {
    ParseTokenAndStore();
}

bool Tokenizer::IsEnd() {
    return end_;
}

void Tokenizer::Next() {
    ParseTokenAndStore();
}

Token Tokenizer::GetToken() {
    return current_token_;
}
