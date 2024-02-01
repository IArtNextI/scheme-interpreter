#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

class Object {
protected:
    bool marked_ = false;

public:
    virtual void Mark() = 0;
    virtual void UnMark();
    virtual bool IsMarked();
    virtual ~Object() = default;
};

class Number : public Object {
private:
    int64_t value_;

public:
    Number(int64_t value);
    int64_t GetValue() const;
    virtual void Mark() override;
};

class Symbol : public Object {
private:
    std::string name_;

public:
    Symbol(const std::string& s);
    const std::string& GetName() const;
    virtual void Mark() override;
};

class Boolean : public Object {
private:
    bool value_;

public:
    Boolean(bool value);
    bool GetValue() const;
    virtual void Mark() override;
};

// Probably should be a quote here

class Cell : public Object {
private:
    Object* first_ = nullptr;
    Object* second_ = nullptr;

public:
    class Iterator {
    private:
        Cell* ptr_ = nullptr;
        int checked_elements_ = 0;

    public:
        Iterator(Cell* ptr);
        bool Advance();
        Object* Get();
    };

    class BoarIterator {
    private:
        Cell* ptr_ = nullptr;
        int checked_elements_ = 0;

    public:
        BoarIterator(Cell* ptr);
        bool Advance();
        Object* Get();
    };

    void SetFirst(Object* ptr);
    void SetSecond(Object* ptr);

    Object* GetFirst() const;
    Object* GetSecond() const;

    virtual void Mark() override;
};

class SchemaFunction : public Object {
public:
    virtual Object* Invoke(const std::vector<Object*>&) = 0;
    virtual void Mark() override;
};

class IsBooleanFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class NotFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class IsNumberFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class NumberEqFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class NumberLeFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class NumberGeFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class NumberLeqFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class NumberGeqFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class AddFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class SubFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class MulFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class DivFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class MaxFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class MinFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class AbsFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class QuoteFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class AndFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class OrFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class IsPairFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class IsNullFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class IsListFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class ConsFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class CarFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class CdrFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class MakeListFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class ListTailFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class ListRefFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class IsSymbolFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class DefineFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class IfFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class SetFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class SetCarFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class SetCdrFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class Scope : public Object {
private:
    Scope* parent_ = nullptr;
    std::unordered_map<std::string, Object*> known_symbols_;

public:
    Scope(Scope* parent);
    Object* LookUpSymbol(const std::string&);
    Scope* IsDefined(const std::string&);
    Scope* GetParent();
    void DefineSymbol(const std::string&, Object*);
    virtual void Mark() override;
};

class CurrentScope {
private:
    explicit CurrentScope();
    static CurrentScope& Instance();

public:
    static void Set(Scope* new_scope);
    static Scope* Get();

private:
    Scope* ptr_ = nullptr;
};

class LambdaFunction : public SchemaFunction {
public:
    virtual Object* Invoke(const std::vector<Object*>&) override;
};

class LambdaImplFunction : public SchemaFunction {
private:
    std::vector<std::string> args_fmt_;
    std::vector<Object*> cmds_;
    Scope* lsc_;

public:
    LambdaImplFunction(const std::vector<std::string>&, const std::vector<Object*>&);
    Scope* GetScope();
    virtual Object* Invoke(const std::vector<Object*>&) override;
    virtual void Mark() override;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
static T* As(Object* obj) {
    return dynamic_cast<T*>(obj);
}

template <class T>
static bool Is(Object* obj) {
    return dynamic_cast<T*>(obj) != nullptr;
}

class Heap {
private:
    std::vector<Object*> objs_;
    explicit Heap();

public:
    template <class T, class... Args>
    static T* Make(Args... args) {
        Instance().objs_.push_back(new T(std::forward<Args>(args)...));
        return As<T>(Instance().objs_.back());
    }

    static Heap& Instance();

    static void Cleanup(Scope* global_scope);

    ~Heap();
};
