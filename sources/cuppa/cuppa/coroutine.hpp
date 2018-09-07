#include <experimental/coroutine>
// for clang need -fcoroutines-ts -stdlib=libc++
// for msvc need /await

template <typename T>
struct generator
{
    struct promise_type
    {
        T current_value;
        std::experimental::suspend_always yield_value(T value)
        {
            this->current_value = value;
            return {};
        }
        std::experimental::suspend_always initial_suspend() { return {}; }
        std::experimental::suspend_always final_suspend() { return {}; }
        generator get_return_object() { return generator{this}; };
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    struct iterator
    {
        std::experimental::coroutine_handle<promise_type> _Coro;
        bool _Done;

        iterator(std::experimental::coroutine_handle<promise_type> Coro, bool Done)
            : _Coro(Coro), _Done(Done) {}

        iterator &operator++()
        {
            _Coro.resume();
            _Done = _Coro.done();
            return *this;
        }

        bool operator==(iterator const &_Right) const
        {
            return _Done == _Right._Done;
        }

        bool operator!=(iterator const &_Right) const { return !(*this == _Right); }
        T const &operator*() const { return _Coro.promise().current_value; }
        T const *operator->() const { return &(operator*()); }
    };

    iterator begin()
    {
        p.resume();
        return {p, p.done()};
    }

    iterator end() { return {p, true}; }

    generator(generator const &) = delete;
    generator(generator &&rhs) : p(rhs.p) { rhs.p = nullptr; }

    ~generator()
    {
        if (p)
            p.destroy();
    }

  private:
    explicit generator(promise_type *p)
        : p(std::experimental::coroutine_handle<promise_type>::from_promise(*p)) {}

    std::experimental::coroutine_handle<promise_type> p;
};