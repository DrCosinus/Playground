#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

template<typename... Ts>
struct RecursiveHelper
{
    using CALLBACK_TYPE = std::function<RecursiveHelper(Ts...)>;
    RecursiveHelper(CALLBACK_TYPE _f) : func_(_f) {}
    operator CALLBACK_TYPE() const { return func_; }

private:
    const CALLBACK_TYPE func_;
};

struct Params
{
    struct Counter
    {
        auto Touch()             { hit_count_++;      }
        auto GetHitCount() const { return hit_count_; }
    private:
        size_t hit_count_ = 0;
    };
    Counter A;
    Counter B;
};

using F = RecursiveHelper<Params&>::CALLBACK_TYPE;

F B(Params& _params);

F A(Params& _params)
{
    _params.A.Touch();
    return B;
}

F B(Params& _params)
{
    _params.B.Touch();
    return A;
}
/*
template<char... chars>
struct State
{

};*/
/*
template<typename CHAR_TYPE, CHAR_TYPE... CHARS>
constexpr State<CHARS...> operator "" _state ()
{
    return {};
}*/
//template<const char* STR> struct plop { static constexpr auto value = 1; };
//constexpr int operator"" _etat(const char* S, size_t N) { return plop<S>::value; }

// A renvoie une méthode qui renvoie un F
// qui implicitement castable en RecursiveHelper
// qui prend aucun argument et qui renvoie un RecursiveHelper
// et qui est castable implicitement en RecursiveHelper
template<typename OBJECT_TYPE, typename RETURN_TYPE, typename... ARG_TYPES>
RETURN_TYPE& deconstify(OBJECT_TYPE* _object, const RETURN_TYPE& (OBJECT_TYPE::*_method)(ARG_TYPES...)const, ARG_TYPES&&... _args)
{
    return const_cast<RETURN_TYPE&>(((static_cast<const OBJECT_TYPE*>(_object))->*(_method))(std::forward<ARG_TYPES>(_args)...));
}
template<typename OBJECT_TYPE, typename RETURN_TYPE, typename... ARG_TYPES>
RETURN_TYPE* deconstify(OBJECT_TYPE* _object, const RETURN_TYPE* (OBJECT_TYPE::*_method)(ARG_TYPES...)const, ARG_TYPES&&... _args)
{
    return const_cast<RETURN_TYPE*>(((static_cast<const OBJECT_TYPE*>(_object))->*(_method))(std::forward<ARG_TYPES>(_args)...));
}

struct choupi
{
    const int& get() const { return value_; }
          int& get()       { return deconstify(this, &choupi::get); }
    const int* get_ptr() const { return &value_; }
          int* get_ptr()       { return deconstify(this, &choupi::get_ptr); }
private:
    int value_ = 42;
};

int main()
{
    //std::cout << "-> " << "plop"_state << std::endl;
    //auto i = "init"_state;
    /* = []
    {

    };*/
    choupi c;
    const choupi& ccr = c;
    c.get() = 78;
    std::cout << "Choupi A :" << ccr.get() << std::endl;
    *c.get_ptr() = 509;
    std::cout << "Choupi B :" << *ccr.get_ptr() << std::endl;

    Params params;

    F state = A;
    state = state(params);
    state = state(params);
    state = state(params);
    state = state(params);
    state = state(params);
    // ...
    std::cout << "Hit count of A:" << params.A.GetHitCount() << std::endl;
    std::cout << "Hit count of B:" << params.B.GetHitCount() << std::endl;
}