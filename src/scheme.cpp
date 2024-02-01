#include "scheme.h"
#include "tokenizer.h"
#include "parser.h"

#include <sstream>
#include <memory>
#include <vector>
#include <iostream>

bool IsProperList(Object* ptr) {
    if (!Is<Cell>(ptr)) {
        return false;
    }
    Cell* cur = dynamic_cast<Cell*>(ptr);
    while (cur->GetSecond() != nullptr) {
        if (!Is<Cell>(cur->GetSecond())) {
            return false;
        }
        cur = dynamic_cast<Cell*>(cur->GetSecond());
    }
    return true;
}

bool IsImproperList(Object* ptr) {
    if (!Is<Cell>(ptr)) {
        return false;
    }
    Cell* cur = dynamic_cast<Cell*>(ptr);
    while (Is<Cell>(cur->GetSecond())) {
        cur = dynamic_cast<Cell*>(cur->GetSecond());
    }
    return cur->GetSecond() != nullptr;
}

Interpreter::Interpreter() {
    global_scope_ = Heap::Make<Scope>(nullptr);
    global_scope_->DefineSymbol("boolean?", Heap::Make<IsBooleanFunction>());
    global_scope_->DefineSymbol("not", Heap::Make<NotFunction>());
    global_scope_->DefineSymbol("number?", Heap::Make<IsNumberFunction>());
    global_scope_->DefineSymbol("=", Heap::Make<NumberEqFunction>());
    global_scope_->DefineSymbol("<", Heap::Make<NumberLeFunction>());
    global_scope_->DefineSymbol("<=", Heap::Make<NumberLeqFunction>());
    global_scope_->DefineSymbol(">", Heap::Make<NumberGeFunction>());
    global_scope_->DefineSymbol(">=", Heap::Make<NumberGeqFunction>());
    global_scope_->DefineSymbol("+", Heap::Make<AddFunction>());
    global_scope_->DefineSymbol("-", Heap::Make<SubFunction>());
    global_scope_->DefineSymbol("/", Heap::Make<DivFunction>());
    global_scope_->DefineSymbol("*", Heap::Make<MulFunction>());
    global_scope_->DefineSymbol("max", Heap::Make<MaxFunction>());
    global_scope_->DefineSymbol("min", Heap::Make<MinFunction>());
    global_scope_->DefineSymbol("abs", Heap::Make<AbsFunction>());
    global_scope_->DefineSymbol("quote", Heap::Make<QuoteFunction>());
    global_scope_->DefineSymbol("and", Heap::Make<AndFunction>());
    global_scope_->DefineSymbol("or", Heap::Make<OrFunction>());
    global_scope_->DefineSymbol("pair?", Heap::Make<IsPairFunction>());
    global_scope_->DefineSymbol("null?", Heap::Make<IsNullFunction>());
    global_scope_->DefineSymbol("list?", Heap::Make<IsListFunction>());
    global_scope_->DefineSymbol("cons", Heap::Make<ConsFunction>());
    global_scope_->DefineSymbol("car", Heap::Make<CarFunction>());
    global_scope_->DefineSymbol("cdr", Heap::Make<CdrFunction>());
    global_scope_->DefineSymbol("list", Heap::Make<MakeListFunction>());
    global_scope_->DefineSymbol("list-tail", Heap::Make<ListTailFunction>());
    global_scope_->DefineSymbol("list-ref", Heap::Make<ListRefFunction>());
    global_scope_->DefineSymbol("symbol?", Heap::Make<IsSymbolFunction>());
    global_scope_->DefineSymbol("define", Heap::Make<DefineFunction>());
    global_scope_->DefineSymbol("if", Heap::Make<IfFunction>());
    global_scope_->DefineSymbol("set!", Heap::Make<SetFunction>());
    global_scope_->DefineSymbol("set-car!", Heap::Make<SetCarFunction>());
    global_scope_->DefineSymbol("set-cdr!", Heap::Make<SetCdrFunction>());
    global_scope_->DefineSymbol("lambda", Heap::Make<LambdaFunction>());
}

Object* Eval(Object* root, Scope* scope) {
    if (Is<Cell>(root)) {
        std::vector<Object*> invocation_params;
        Cell::BoarIterator it = Cell::BoarIterator(dynamic_cast<Cell*>(root));
        if (it.Get() != nullptr) {
            invocation_params.push_back(it.Get());
            while (it.Advance()) {
                invocation_params.push_back(it.Get());
            }
            if (invocation_params.back() == nullptr) {
                invocation_params.pop_back();
            }
            // Now I have a list of objects to be evaluated
            // I therefore require the first param to be a string
            Object* evaled_first = Eval(invocation_params[0], scope);
            if (!Is<SchemaFunction>(evaled_first)) {
                throw RuntimeError{
                    "Could not evaluate a list without its first param being a function"};
            }
            // Fix : do not evaluate all the params
            // Evaluate the first param to get the function
            CurrentScope::Set(scope);
            SchemaFunction* invocable = As<SchemaFunction>(evaled_first);
            invocation_params.erase(invocation_params.begin());
            auto result = invocable->Invoke(invocation_params);
            CurrentScope::Set(scope);
            return result;
        } else {
            // I'm literally an empty list
            throw RuntimeError{"Could not evaluate an empty list"};
        }
    } else if (Is<Number>(root)) {
        return root;
    } else if (Is<Symbol>(root)) {
        // Return some kind of a function object
        std::string symb = As<Symbol>(root)->GetName();
        return scope->LookUpSymbol(symb);
    } else if (Is<Boolean>(root)) {
        return root;
    } else {
        throw RuntimeError{"I fucked up with parsing somehow (or smth other?)"};
    }
}

std::string Serialize(Object* root) {
    if (Is<Cell>(root)) {
        std::vector<Object*> invocation_params;
        Cell::BoarIterator it = Cell::BoarIterator(dynamic_cast<Cell*>(root));
        invocation_params.push_back(it.Get());
        while (it.Advance()) {
            invocation_params.push_back(it.Get());
        }
        if (invocation_params.back() == nullptr) {
            invocation_params.pop_back();
        }
        if (IsProperList(root)) {
            std::string result = "(";
            for (auto& x : invocation_params) {
                result += Serialize(x);
                result += " ";
            }
            result.back() = ')';
            return result;
        } else if (IsImproperList(root)) {
            std::string result = "(";
            for (size_t i = 0; i < invocation_params.size() - 1; ++i) {
                result += Serialize(invocation_params[i]);
                result += " ";
            }
            result += ". ";
            result += Serialize(invocation_params.back());
            result += ")";
            return result;
        }
    } else if (Is<Number>(root)) {
        return std::to_string(As<Number>(root)->GetValue());
    } else if (Is<Symbol>(root)) {
        return As<Symbol>(root)->GetName();
    } else if (Is<Boolean>(root)) {
        return std::string() + "#" + (As<Boolean>(root)->GetValue() ? "t" : "f");
    } else if (Is<SchemaFunction>(root)) {
        throw RuntimeError{"Tried to serialize a function"};
    } else if (root == nullptr) {
        return "()";
    } else {
        throw RuntimeError{"Fucked up, or not implemented yet"};
    }
}

std::string Interpreter::Run(const std::string& s) {
    std::stringstream ss;
    ss << s;
    Tokenizer tkn(&ss);
    Object* root = Read(&tkn);
    if (!tkn.IsEnd()) {
        throw SyntaxError{"Provided string is not a valid executable expression"};
    }
    Object* result = Eval(root, global_scope_);
    std::string serialized_result = Serialize(result);
    Heap::Cleanup(global_scope_);
    return serialized_result;
}
