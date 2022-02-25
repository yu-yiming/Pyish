#pragma once
#include <exception>
#include <string>

namespace python {

template<std::size_t N>
struct string_literal {
    char data[N];

    constexpr string_literal(char const (&str)[N])
        : data{} {
        std::copy(str, str + N, data);
    }
};

template<std::size_t N>
string_literal(char const (&)[N]) -> string_literal<N>;

class BasicError {
public:
    BasicError() = default;
    BasicError(BasicError const&) = default;
    BasicError(BasicError&&) = default;
    BasicError& operator =(BasicError const&) = default;
    BasicError& operator =(BasicError&&) = default;
    virtual ~BasicError() = default;

    virtual std::string info() const = 0;
};

template<string_literal ErrStr>
class Error : BasicError {
public:
    static constexpr char const* error_type = ErrStr.data;

    Error()
        : Error("") {}
    Error(char const* msg)
        : m_msg(msg) {}
    Error(std::string const& msg)
        : m_msg(msg) {}
    
    Error(Error const& other) = default;
    Error(Error&& other) = default;

    Error& operator =(Error const& other) = default;
    Error& operator =(Error&& other) = default;

    std::string info() const override {
        return error_type + '\n' + m_msg;
    }

private:
    std::string m_msg;
};

#define ADD_ERROR_TYPE(T) using T ## Error = Error<#T " Error">

ADD_ERROR_TYPE(Index);
ADD_ERROR_TYPE(Internal);
ADD_ERROR_TYPE(Value);
ADD_ERROR_TYPE(Type);

} // namespace python