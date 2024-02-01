#include "object.h"
#include "error.h"

Object* Cell::GetFirst() const {
    return first_;
}

Object* Cell::GetSecond() const {
    return second_;
}

void Cell::SetFirst(Object* ptr) {
    first_ = ptr;
}

void Cell::SetSecond(Object* ptr) {
    second_ = ptr;
}

bool Boolean::GetValue() const {
    return value_;
}

Boolean::Boolean(bool value) : value_(value) {
}

const std::string& Symbol::GetName() const {
    return name_;
}

Symbol::Symbol(const std::string& s) : name_(s) {
}

int64_t Number::GetValue() const {
    return value_;
}

Number::Number(int64_t value) : value_(value) {
}

Cell::Iterator::Iterator(Cell* ptr) : ptr_(ptr) {
    checked_elements_ = 0;
    Advance();
}

Object* Cell::Iterator::Get() {
    if (checked_elements_ == 0) {
        return nullptr;
        // Means that I'm at an empty point
    } else if (checked_elements_ == 1) {
        return ptr_->GetFirst();
    } else if (checked_elements_ == 2) {
        return ptr_->GetSecond();
    }
}

bool Cell::Iterator::Advance() {
    if (checked_elements_ == 0) {
        if (ptr_->GetFirst() == nullptr) {
            return false;
        }
        checked_elements_ = 1;
        return true;
    } else if (checked_elements_ == 1) {
        if (ptr_->GetSecond() == nullptr) {
            return false;
        } else if (Is<Cell>(ptr_->GetSecond())) {
            ptr_ = dynamic_cast<Cell*>(ptr_->GetSecond());
            checked_elements_ = 0;
            return Advance();
        } else {
            checked_elements_ = 2;
            return true;
        }
    } else if (checked_elements_ == 2) {
        return false;
    }
}

Cell::BoarIterator::BoarIterator(Cell* ptr) : ptr_(ptr) {
    checked_elements_ = 0;
    Advance();
}

Object* Cell::BoarIterator::Get() {
    if (checked_elements_ == 0) {
        return nullptr;
        // Means that I'm at an empty point
    } else if (checked_elements_ == 1) {
        return ptr_->GetFirst();
    } else if (checked_elements_ == 2) {
        return ptr_->GetSecond();
    }
}

bool Cell::BoarIterator::Advance() {
    if (checked_elements_ == 0) {
        checked_elements_ = 1;
        return true;
    } else if (checked_elements_ == 1) {
        if (Is<Cell>(ptr_->GetSecond())) {
            ptr_ = dynamic_cast<Cell*>(ptr_->GetSecond());
            checked_elements_ = 1;
            return true;
        } else {
            checked_elements_ = 2;
            return true;
        }
    } else if (checked_elements_ == 2) {
        return false;
    }
}

Object* Eval(Object* root, Scope* scope);

template <class Error = RuntimeError>
void RequireNArgs(size_t n, const std::vector<Object*>& args) {
    if (args.size() < n) {
        throw Error{"Not enough arguments in a function call"};
    } else if (args.size() > n) {
        throw Error{"Too many arguments in a function call"};
    }
}

template <class Error = RuntimeError>
void RequireAtLeastNArgs(size_t n, const std::vector<Object*>& args) {
    if (args.size() < n) {
        throw Error{"Not enough arguments in a function call"};
    }
}

template <class Error = RuntimeError>
void RequireNotMoreNArgs(size_t n, const std::vector<Object*>& args) {
    if (args.size() > n) {
        throw Error{"Too many arguments in a function call"};
    }
}

Object* IsBooleanFunction::Invoke(const std::vector<Object*>& args) {
    Object* evaled = Eval(args[0], CurrentScope::Get());
    return Heap::Make<Boolean>(Is<Boolean>(evaled));
}

Object* NotFunction::Invoke(const std::vector<Object*>& args) {
    RequireNArgs(1, args);
    Object* evaled = Eval(args[0], CurrentScope::Get());
    return Heap::Make<Boolean>(Is<Boolean>(evaled) ? !As<Boolean>(evaled)->GetValue() : false);
}

Object* IsNumberFunction::Invoke(const std::vector<Object*>& args) {
    Object* evaled = Eval(args[0], CurrentScope::Get());
    return Heap::Make<Boolean>(Is<Number>(evaled));
}

template <class Type>
void RequireArgsAre(const std::vector<Object*>& args) {
    for (auto arg : args) {
        if (!Is<Type>(arg)) {
            throw RuntimeError{"Wrong argument type"};
        }
    }
}

Object* NumberEqFunction::Invoke(const std::vector<Object*>& args) {
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    for (size_t i = 1; i < evals.size(); ++i) {
        if (As<Number>(evals[i - 1])->GetValue() != As<Number>(evals[i])->GetValue()) {
            return Heap::Make<Boolean>(false);
        }
    }
    return Heap::Make<Boolean>(true);
}

Object* NumberLeFunction::Invoke(const std::vector<Object*>& args) {
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    for (size_t i = 1; i < evals.size(); ++i) {
        if (As<Number>(evals[i - 1])->GetValue() >= As<Number>(evals[i])->GetValue()) {
            return Heap::Make<Boolean>(false);
        }
    }
    return Heap::Make<Boolean>(true);
}

Object* NumberLeqFunction::Invoke(const std::vector<Object*>& args) {
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    for (size_t i = 1; i < evals.size(); ++i) {
        if (As<Number>(evals[i - 1])->GetValue() > As<Number>(evals[i])->GetValue()) {
            return Heap::Make<Boolean>(false);
        }
    }
    return Heap::Make<Boolean>(true);
}

Object* NumberGeFunction::Invoke(const std::vector<Object*>& args) {
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    for (size_t i = 1; i < evals.size(); ++i) {
        if (As<Number>(evals[i - 1])->GetValue() <= As<Number>(evals[i])->GetValue()) {
            return Heap::Make<Boolean>(false);
        }
    }
    return Heap::Make<Boolean>(true);
}

Object* NumberGeqFunction::Invoke(const std::vector<Object*>& args) {
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    for (size_t i = 1; i < evals.size(); ++i) {
        if (As<Number>(evals[i - 1])->GetValue() < As<Number>(evals[i])->GetValue()) {
            return Heap::Make<Boolean>(false);
        }
    }
    return Heap::Make<Boolean>(true);
}

Object* AddFunction::Invoke(const std::vector<Object*>& args) {
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    int64_t result = 0;
    for (size_t i = 0; i < evals.size(); ++i) {
        result += As<Number>(evals[i])->GetValue();
    }
    return Heap::Make<Number>(result);
}

Object* SubFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    int64_t result = As<Number>(evals[0])->GetValue();
    for (size_t i = 1; i < evals.size(); ++i) {
        result -= As<Number>(evals[i])->GetValue();
    }
    return Heap::Make<Number>(result);
}

Object* MulFunction::Invoke(const std::vector<Object*>& args) {
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    int64_t result = 1;
    for (size_t i = 0; i < evals.size(); ++i) {
        result *= As<Number>(evals[i])->GetValue();
    }
    return Heap::Make<Number>(result);
}

Object* DivFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    int64_t result = As<Number>(evals[0])->GetValue();
    for (size_t i = 1; i < evals.size(); ++i) {
        result /= As<Number>(evals[i])->GetValue();
    }
    return Heap::Make<Number>(result);
}

Object* MinFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    int64_t result = As<Number>(evals[0])->GetValue();
    for (size_t i = 1; i < evals.size(); ++i) {
        result = std::min(result, As<Number>(evals[i])->GetValue());
    }
    return Heap::Make<Number>(result);
}

Object* MaxFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    int64_t result = As<Number>(evals[0])->GetValue();
    for (size_t i = 1; i < evals.size(); ++i) {
        result = std::max(result, As<Number>(evals[i])->GetValue());
    }
    return Heap::Make<Number>(result);
}

Object* AbsFunction::Invoke(const std::vector<Object*>& args) {
    RequireNArgs(1, args);
    std::vector<Object*> evals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        evals[i] = Eval(args[i], CurrentScope::Get());
    }
    RequireArgsAre<Number>(evals);
    int64_t result = As<Number>(evals[0])->GetValue();
    return Heap::Make<Number>(result < 0 ? -result : result);
}

Object* QuoteFunction::Invoke(const std::vector<Object*>& args) {
    RequireNArgs(1, args);
    return args[0];
}

Object* AndFunction::Invoke(const std::vector<Object*>& args) {
    Object* last_eval = Heap::Make<Boolean>(true);
    for (auto& x : args) {
        auto evaled = Eval(x, CurrentScope::Get());
        bool is_false = Is<Boolean>(evaled) && As<Boolean>(evaled)->GetValue() == false;
        if (is_false) {
            return evaled;
        }
        last_eval = evaled;
    }
    return last_eval;
}

Object* OrFunction::Invoke(const std::vector<Object*>& args) {
    Object* last_eval = Heap::Make<Boolean>(false);
    for (auto& x : args) {
        auto evaled = Eval(x, CurrentScope::Get());
        bool is_false = Is<Boolean>(evaled) && As<Boolean>(evaled)->GetValue() == false;
        if (!is_false) {
            return evaled;
        }
        last_eval = evaled;
    }
    return last_eval;
}

Object* IsPairFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    Object* evaled = Eval(args[0], CurrentScope::Get());
    if (!Is<Cell>(evaled)) {
        return Heap::Make<Boolean>(false);
    }
    std::vector<Object*> els;
    Cell::BoarIterator it = Cell::BoarIterator(As<Cell>(evaled));
    els.push_back(it.Get());
    while (it.Advance()) {
        els.push_back(it.Get());
    }
    if (els.back() == nullptr) {
        els.pop_back();
    }
    return Heap::Make<Boolean>(els.size() == 2);
}

Object* IsNullFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    Object* evaled = Eval(args[0], CurrentScope::Get());
    return Heap::Make<Boolean>(evaled == nullptr);
}

bool IsProperList(Object* ptr);
bool IsImProperList(Object* ptr);

Object* IsListFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    Object* evaled = Eval(args[0], CurrentScope::Get());
    return Heap::Make<Boolean>(evaled == nullptr || IsProperList(evaled));
}

Object* ConsFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(2, args);
    Object* evaled1 = Eval(args[0], CurrentScope::Get());
    Object* evaled2 = Eval(args[1], CurrentScope::Get());
    Cell* result = Heap::Make<Cell>();
    result->SetFirst(evaled1);
    result->SetSecond(evaled2);
    return result;
}

template <class Type>
void RequireIs(Object* arg) {
    if (!Is<Type>(arg)) {
        throw RuntimeError{"Invalid parameter type"};
    }
}

Object* CarFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    Object* evaled = Eval(args[0], CurrentScope::Get());
    RequireIs<Cell>(evaled);
    return As<Cell>(evaled)->GetFirst();
}

Object* CdrFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(1, args);
    Object* evaled = Eval(args[0], CurrentScope::Get());
    RequireIs<Cell>(evaled);
    return As<Cell>(evaled)->GetSecond();
}

Object* MakeListFunction::Invoke(const std::vector<Object*>& args) {
    if (args.empty()) {
        return nullptr;
    }
    std::vector<Object*> evaled(args);
    for (size_t i = 0; i < args.size(); ++i) {
        evaled[i] = Eval(args[i], CurrentScope::Get());
    }
    Cell* result = Heap::Make<Cell>();
    Cell* cur = result;
    for (size_t i = 0; i < args.size() - 1; ++i) {
        cur->SetFirst(evaled[i]);
        cur->SetSecond(Heap::Make<Cell>());
        cur = As<Cell>(cur->GetSecond());
    }
    cur->SetFirst(args.back());
    return result;
}

Object* ListTailFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(2, args);
    Object* evaled = Eval(args[0], CurrentScope::Get());
    Object* evaled2 = Eval(args[1], CurrentScope::Get());
    RequireIs<Cell>(evaled);
    RequireIs<Number>(evaled2);
    Cell* result = As<Cell>(evaled);
    for (int i = 0; i < As<Number>(evaled2)->GetValue(); ++i) {
        if (result == nullptr) {
            throw RuntimeError{"List-tail was called with a parameter greater than length"};
        }
        result = As<Cell>(result->GetSecond());
    }
    return result;
}

Object* ListRefFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs(2, args);
    Object* evaled = Eval(args[0], CurrentScope::Get());
    Object* evaled2 = Eval(args[1], CurrentScope::Get());
    RequireIs<Cell>(evaled);
    RequireIs<Number>(evaled2);
    Cell* result = As<Cell>(evaled);
    for (int i = 0; i < As<Number>(evaled2)->GetValue(); ++i) {
        if (!Is<Cell>(result->GetSecond())) {
            throw RuntimeError{
                "List-ref was called on an improper list with intent to reference last element"};
        }
        result = As<Cell>(result->GetSecond());
        if (result == nullptr) {
            throw RuntimeError{"List-ref was called with a parameter greater than length"};
        }
    }
    return result->GetFirst();
}

Object* IsSymbolFunction::Invoke(const std::vector<Object*>& args) {
    RequireNArgs(1, args);
    return Heap::Make<Boolean>(Is<Symbol>(Eval(args[0], CurrentScope::Get())));
}

Object* DefineFunction::Invoke(const std::vector<Object*>& args) {
    if (Is<Symbol>(args[0])) {
        RequireNArgs<SyntaxError>(2, args);
        Object* eval1 = args[0];
        Object* eval2 = Eval(args[1], CurrentScope::Get());

        RequireIs<Symbol>(eval1);

        CurrentScope::Get()->DefineSymbol(As<Symbol>(eval1)->GetName(), eval2);
        return nullptr;
    } else if (Is<Cell>(args[0])) {
        RequireAtLeastNArgs(2, args);
        Cell::BoarIterator it(As<Cell>(args[0]));
        std::vector<Object*> items;
        items.push_back(it.Get());
        while (it.Advance()) {
            items.push_back(it.Get());
        }
        if (items.back() == nullptr) {
            items.pop_back();
        }
        std::string name;
        std::vector<std::string> params;
        RequireIs<Symbol>(items[0]);
        name = As<Symbol>(items[0])->GetName();
        for (size_t i = 1; i < items.size(); ++i) {
            RequireIs<Symbol>(items[i]);
            params.push_back(As<Symbol>(items[i])->GetName());
        }
        std::vector<Object*> body = args;
        body.erase(body.begin());
        CurrentScope::Get()->DefineSymbol(name, Heap::Make<LambdaImplFunction>(params, body));
        return nullptr;
    } else {
        throw SyntaxError{"Invalid use of 'define'"};
    }
}

Object* IfFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs<SyntaxError>(2, args);
    RequireNotMoreNArgs<SyntaxError>(3, args);

    Object* eval1 = Eval(args[0], CurrentScope::Get());

    if (!(Is<Boolean>(eval1) && As<Boolean>(eval1)->GetValue() == false)) {
        return Eval(args[1], CurrentScope::Get());
    } else if (args.size() == 3) {
        return Eval(args[2], CurrentScope::Get());
    } else {
        return nullptr;
    }
}

Object* SetFunction::Invoke(const std::vector<Object*>& args) {
    RequireNArgs<SyntaxError>(2, args);

    Object* eval1 = nullptr;
    if (!Is<Symbol>(args[0])) {
        eval1 = Eval(args[0], CurrentScope::Get());
    } else {
        if (!CurrentScope::Get()->IsDefined(As<Symbol>(args[0])->GetName())) {
            throw NameError{std::string() + "Undefined reference to symbol'" +
                            As<Symbol>(args[0])->GetName() + "'"};
        } else {
            eval1 = args[0];
        }
    }
    Object* eval2 = Eval(args[1], CurrentScope::Get());

    RequireIs<Symbol>(eval1);

    CurrentScope::Get()
        ->IsDefined(As<Symbol>(eval1)->GetName())
        ->DefineSymbol(As<Symbol>(eval1)->GetName(), eval2);
    return nullptr;
}

Object* SetCarFunction::Invoke(const std::vector<Object*>& args) {
    RequireNArgs(2, args);

    Object* eval1 = Eval(args[0], CurrentScope::Get());
    Object* eval2 = Eval(args[1], CurrentScope::Get());

    RequireIs<Cell>(eval1);
    As<Cell>(eval1)->SetFirst(eval2);
    return nullptr;
}

Object* SetCdrFunction::Invoke(const std::vector<Object*>& args) {
    RequireNArgs(2, args);

    Object* eval1 = Eval(args[0], CurrentScope::Get());
    Object* eval2 = Eval(args[1], CurrentScope::Get());

    RequireIs<Cell>(eval1);
    As<Cell>(eval1)->SetSecond(eval2);
    return nullptr;
}

Object* LambdaFunction::Invoke(const std::vector<Object*>& args) {
    RequireAtLeastNArgs<SyntaxError>(2, args);

    std::vector<Object*> fmt_ptrs;
    if (args[0] != nullptr) {
        RequireIs<Cell>(args[0]);
        Cell::BoarIterator it(As<Cell>(args[0]));

        fmt_ptrs.push_back(it.Get());
        while (it.Advance()) {
            fmt_ptrs.push_back(it.Get());
        }
        if (fmt_ptrs.back() == nullptr) {
            fmt_ptrs.pop_back();
        }

        RequireArgsAre<Symbol>(fmt_ptrs);
    }
    std::vector<Object*> body = args;
    body.erase(body.begin());

    std::vector<std::string> fmt;
    fmt.reserve(fmt_ptrs.size());
    for (auto& x : fmt_ptrs) {
        fmt.push_back(As<Symbol>(x)->GetName());
    }

    return Heap::Make<LambdaImplFunction>(fmt, body);
}

LambdaImplFunction::LambdaImplFunction(const std::vector<std::string>& fmt,
                                       const std::vector<Object*>& cmds) {
    args_fmt_ = fmt;
    cmds_ = cmds;
    lsc_ = Heap::Make<Scope>(CurrentScope::Get());
}

Scope* LambdaImplFunction::GetScope() {
    return lsc_;
}

Object* LambdaImplFunction::Invoke(const std::vector<Object*>& args) {
    Scope* safe_scope = Heap::Make<Scope>(lsc_);
    RequireNArgs<RuntimeError>(args_fmt_.size(), args);
    for (size_t i = 0; i < args.size(); ++i) {
        auto h = Eval(args[i], CurrentScope::Get());
        safe_scope->DefineSymbol(args_fmt_[i], h);
    }

    Object* last_eval = nullptr;
    for (auto& x : cmds_) {
        last_eval = Eval(x, safe_scope);
    }

    return last_eval;
}

Scope::Scope(Scope* parent) : parent_(parent) {
}

void Scope::DefineSymbol(const std::string& symbol, Object* obj) {
    known_symbols_[symbol] = obj;
}

Object* Scope::LookUpSymbol(const std::string& symbol) {
    Scope* current = this;
    while (current != nullptr && current->known_symbols_.find(symbol) == known_symbols_.end()) {
        current = current->parent_;
    }
    if (current == nullptr) {
        throw NameError{std::string() + "Reference to an unknown symbol '" + symbol + "'"};
    } else {
        return current->known_symbols_[symbol];
    }
}

Scope* Scope::IsDefined(const std::string& symbol) {
    Scope* current = this;
    while (current != nullptr && current->known_symbols_.find(symbol) == known_symbols_.end()) {
        current = current->parent_;
    }
    return current;
}

Scope* Scope::GetParent() {
    return parent_;
}

CurrentScope::CurrentScope() {
}

void CurrentScope::Set(Scope* new_scope) {
    Instance().ptr_ = new_scope;
}

Scope* CurrentScope::Get() {
    return Instance().ptr_;
}

CurrentScope& CurrentScope::Instance() {
    static CurrentScope sc;
    return sc;
}

Heap::Heap() {
}

Heap& Heap::Instance() {
    static Heap hp;
    return hp;
}

void Number::Mark() {
    marked_ = true;
}

void Symbol::Mark() {
    marked_ = true;
}

void Boolean::Mark() {
    marked_ = true;
}

void Cell::Mark() {
    if (!marked_) {
        marked_ = true;
        if (first_ != nullptr) {
            first_->Mark();
        }
        if (second_ != nullptr) {
            second_->Mark();
        }
    }
}

void SchemaFunction::Mark() {
    marked_ = true;
}

void Scope::Mark() {
    if (!marked_) {
        marked_ = true;
        for (auto& x : known_symbols_) {
            x.second->Mark();
        }
        if (parent_ != nullptr) {
            parent_->Mark();
        }
    }
}

void LambdaImplFunction::Mark() {
    if (!marked_) {
        marked_ = true;
        for (auto& x : cmds_) {
            x->Mark();
        }
        lsc_->Mark();
    }
}

void Object::UnMark() {
    marked_ = false;
}

bool Object::IsMarked() {
    return marked_;
}

void Heap::Cleanup(Scope* global_scope) {
    for (auto& x : Instance().objs_) {
        x->UnMark();
    }
    global_scope->Mark();
    std::vector<Object*> new_objs;
    for (auto& x : Instance().objs_) {
        if (!x->IsMarked()) {
            delete x;
        } else {
            new_objs.push_back(x);
        }
    }
    Instance().objs_ = new_objs;
}

Heap::~Heap() {
    for (auto& x : objs_) {
        delete x;
    }
    objs_.clear();
}