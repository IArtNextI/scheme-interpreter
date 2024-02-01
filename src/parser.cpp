#include <parser.h>

Object* ReadList(Tokenizer* tokenizer);

Object* Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError{"Unexpected EOF"};
    }
    Token cur = tokenizer->GetToken();
    tokenizer->Next();
    if (std::get_if<BracketToken>(&cur)) {
        bool open = (*std::get_if<BracketToken>(&cur)) == BracketToken::OPEN;
        if (!open) {
            return nullptr;
        }
        return ReadList(tokenizer);
    } else if (std::get_if<ConstantToken>(&cur)) {
        return Heap::Make<Number>((std::get_if<ConstantToken>(&cur))->value);
    } else if (std::get_if<SymbolToken>(&cur)) {
        return Heap::Make<Symbol>((std::get_if<SymbolToken>(&cur))->name);
    } else if (std::get_if<DotToken>(&cur)) {
        throw SyntaxError{"Unexpected dot token"};
    } else if (std::get_if<BooleanToken>(&cur)) {
        return Heap::Make<Boolean>(*(std::get_if<BooleanToken>(&cur)) == BooleanToken::TRUE);
    } else if (std::get_if<QuoteToken>(&cur)) {
        Cell* result = Heap::Make<Cell>();
        result->SetFirst(Heap::Make<Symbol>("quote"));
        result->SetSecond(Heap::Make<Cell>());
        As<Cell>(result->GetSecond())->SetFirst(Read(tokenizer));
        return result;
    }
}

Object* ReadList(Tokenizer* tokenizer) {
    Token cur = tokenizer->GetToken();
    if (std::get_if<BracketToken>(&cur) &&
        (*std::get_if<BracketToken>(&cur)) == BracketToken::CLOSE) {
        tokenizer->Next();
        return nullptr;
    }
    Cell* result = Heap::Make<Cell>();
    Cell* current = result;
    while (!(std::get_if<BracketToken>(&cur) &&
             (*std::get_if<BracketToken>(&cur)) == BracketToken::CLOSE)) {
        current->SetFirst(Read(tokenizer));
        if (tokenizer->IsEnd()) {
            throw SyntaxError{"Unmatched open bracker in the expression"};
        }
        cur = tokenizer->GetToken();
        if (std::get_if<BracketToken>(&cur) &&
            (*std::get_if<BracketToken>(&cur)) == BracketToken::CLOSE) {
            tokenizer->Next();
            return result;
        }

        if (std::get_if<DotToken>(&cur)) {
            tokenizer->Next();
            current->SetSecond(Read(tokenizer));
            if (tokenizer->IsEnd()) {
                throw SyntaxError{"Unexpected EOF"};
            }
            cur = tokenizer->GetToken();
            if (!std::get_if<BracketToken>(&cur) ||
                (*std::get_if<BracketToken>(&cur)) != BracketToken::CLOSE) {
                throw SyntaxError{"Unexpected token"};
            }
            tokenizer->Next();
            return result;
        } else {
            current->SetSecond(Heap::Make<Cell>());
            current = As<Cell>(current->GetSecond());
        }
        cur = tokenizer->GetToken();
    }
    if (tokenizer->IsEnd()) {
        throw SyntaxError{"Unexpected EOF"};
    }
    cur = tokenizer->GetToken();
    if (!std::get_if<BracketToken>(&cur) ||
        (*std::get_if<BracketToken>(&cur)) != BracketToken::CLOSE) {
        throw SyntaxError{"Unexpected token"};
    }
    tokenizer->Next();
    return result;
}
