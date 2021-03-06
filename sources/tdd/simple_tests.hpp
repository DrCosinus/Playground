#pragma once
#include <cstddef>
#include <sstream>
#include <vector>
#include <memory>

namespace tdd
{
    struct Message final
    {
        Message() { }
        Message(const Message& _msg) : Message() { ss_ << _msg.GetString(); }
        Message(const char* _cstr) : Message() { ss_ << _cstr; }

        template <typename T>
        auto operator<<(const T& _value)
        {
            //using ::operator<<;
            ss_ << _value;
            return *this;
        }

        auto operator<<(const std::nullptr_t&)
        {
            ss_ << "(null)";
            return *this;
        }

        auto operator<<(bool _boolean)
        {
            ss_ << (_boolean ? "true" : "false");
            return *this;
        }

        auto GetString() const -> std::string { return ss_.str(); }
    private:
        std::stringstream ss_;
    };

    struct AssertionResult
    {
        template<typename T> explicit AssertionResult(const T& _success, typename std::enable_if_t<!std::is_convertible<T, AssertionResult>::value, std::nullptr_t> = nullptr) : success_(_success) {}
        AssertionResult(const AssertionResult& _other) : success_(_other.success_), message_(_other.message_ ? ::std::make_unique<::std::string>(*_other.message_) : nullptr) {}
        AssertionResult(AssertionResult&&) = default;
        auto swap(AssertionResult& _other) { ::std::swap(success_, _other.success_); ::std::swap(message_, _other.message_); }
        auto operator=(AssertionResult _other) { swap(_other); return *this; }
        operator bool() const { return success_; }
        auto message() const { return message_.get() != nullptr ? message_->c_str() : ""; }
        template<typename T> auto operator<<(const T& _value) { AppendMessage(Message() << _value); return *this; }
    private:
        auto AppendMessage(const Message& _message) -> void { if (message_.get() == nullptr) { message_ = ::std::make_unique<::std::string>(); } message_->append(_message.GetString().c_str()); }
        bool success_;
        ::std::unique_ptr<::std::string> message_;
    };

    inline auto AssertionSuccess() { return AssertionResult(true); }
    inline auto AssertionFailure() { return AssertionResult(false); }

    auto OpFailure(const char* _file, size_t _line, const char* _message)
    {
        return AssertionFailure() << _file << "(" << _line << "): " << _message;
    }

    inline std::size_t FailureCount = 0;
    inline std::size_t CheckCount = 0;
    inline std::vector<AssertionResult> Results;

    template<typename F>
    inline void PrintTestResults(F _writeline_callback)
    {
        for(auto& s: Results)
        {
            if (!s)
            {
                _writeline_callback(s.message());
            }
        }
        _writeline_callback("========================================================================");
        _writeline_callback((tdd::Message() << "Tests: " << (CheckCount-FailureCount) << "/" << CheckCount).GetString().c_str());
    }

    void CheckUnaryPostOperator(bool condition, const char* condition_text, const char* filename, std::size_t lineindex)
    {
        ++tdd::CheckCount;
        if (!condition)
        {
            Results.emplace_back(OpFailure(filename, lineindex, condition_text));
            ++FailureCount;
        }
    }
    template<typename T1, typename T2>
    void CheckBinaryOperator(bool condition, const char* expr1, const char* expr2, const T1& val1, const T2& val2, const char* op, const char* filename, std::size_t lineindex )
    {
        ++tdd::CheckCount;
        if (!condition)
        {
            Results.emplace_back(AssertionFailure() << filename << "(" << lineindex << "): " << "Expected: (" << expr1 << ") " << op << " (" << expr2 << "), actual: " << val1 << op << val2);
            ++FailureCount;
        }
    }
} // namespace tdd

#define CHECK_EQ(__E,__V) tdd::CheckBinaryOperator((__E)==(__V), #__E, #__V, __E,__V, "==",__FILE__,__LINE__)
#define CHECK_NE(__E,__V) tdd::CheckBinaryOperator((__E)!=(__V), #__E, #__V, __E,__V, "!=",__FILE__,__LINE__)
#define CHECK_LT(__E,__V) tdd::CheckBinaryOperator((__E)< (__V), #__E, #__V, __E,__V, "<" ,__FILE__,__LINE__)
#define CHECK_GT(__E,__V) tdd::CheckBinaryOperator((__E)> (__V), #__E, #__V, __E,__V, ">" ,__FILE__,__LINE__)
#define CHECK_LE(__E,__V) tdd::CheckBinaryOperator((__E)<=(__V), #__E, #__V, __E,__V, "<=",__FILE__,__LINE__)
#define CHECK_GE(__E,__V) tdd::CheckBinaryOperator((__E)>=(__V), #__E, #__V, __E,__V, ">=",__FILE__,__LINE__)
#define CHECK_TRUE(__C) tdd::CheckUnaryPostOperator((__C), "Expected: " #__C " == true", __FILE__,__LINE__)
#define CHECK_FALSE(__C) tdd::CheckUnaryPostOperator(!(__C), "Expected: " #__C " == false", __FILE__,__LINE__)
#define FAIL(__MESSAGE) tdd::Results.emplace_back(tdd::OpFailure(__FILE__, __LINE__, #__MESSAGE));
