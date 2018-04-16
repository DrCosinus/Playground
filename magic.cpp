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

struct classA
{
};

struct classB : classA
{
    void foo() const {};
};

struct classC
{
    virtual const classA* get() const { return nullptr; }
};

struct classD : classC
{
    classD(const classB& _objB) : objB_{_objB} {}
    const classB* get() const override { return &objB_; }
private:
    const classB& objB_;
};

//              OPTIMIZATION
// |        |     RETAIL    |        NON       |            |
// |        |               |       RETAIL     |    ANY     |
// |--------|---------------|------------------|------------|
// | EDITOR | EDITOR_RETAIL | EDITOR_NONRETAIL | EDITOR_ANY |
// | CLIENT | CLIENT_RETAIL | CLIENT_NONRETAIL | CLIENT_ANY |
// |  ANY   | ANY_RETAIL    | ANY_NONRETAIL    |    ALL     |

class OptimizationRetail;
class OptimizationNonRetail;
using Optimization = OptimizationRetail;
template<typename Optimization>
[[maybe_unused]]constexpr auto IsOptimization = std::is_same<Optimization, OptimizationRetail>::value || std::is_same<Optimization, OptimizationNonRetail>::value;

class ModeClient;
class ModeEditor;
using Mode = ModeEditor;
template<typename Mode>
[[maybe_unused]]constexpr auto IsMode = std::is_same<Mode, ModeClient>::value || std::is_same<Mode, ModeEditor>::value;

template<typename... Wrappers>
struct DebugModulesInitTemplate
{
    static void Run() {}
};

template<>
struct DebugModulesInitTemplate<ModeEditor>
{
    static void Run() { std::cout << "Mode: Editor" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeClient>
{
    static void Run() { std::cout << "Mode: Client" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<OptimizationRetail>
{
    static void Run() { std::cout << "Optimization: Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<OptimizationNonRetail>
{
    static void Run() { std::cout << "Optimization: Non Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeEditor, OptimizationRetail>
{
    static void Run() { std::cout << "Mode: Editor && Optimization: Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeClient, OptimizationRetail>
{
    static void Run() { std::cout << "Mode: Client && Optimization: Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeEditor, OptimizationNonRetail>
{
    static void Run() { std::cout << "Mode: Editor && Optimization: Non Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeClient, OptimizationNonRetail>
{
    static void Run() { std::cout << "Mode: Client && Optimization: Non Retail" << std::endl; }
};

template<typename T, typename U>
struct DebugModulesInitTemplate<T, U> : DebugModulesInitTemplate<U, T>
{
};

void Debug_RegisterModules() { std::cout << "Debug_RegisterModules();" << std::endl; }

template<typename...>
struct Debug_RegisterModules_
{
    void static Run()
    {
        Debug_RegisterModules();
    }
};

template<>
struct Debug_RegisterModules_<ModeClient, OptimizationRetail>
{
    void static Run()
    {
        std::cout << "/* Debug_RegisterModules(); */" << std::endl;
    }
};

template<>
struct Debug_RegisterModules_<OptimizationRetail, ModeClient> : Debug_RegisterModules_<ModeClient, OptimizationRetail>
{
};

template<template<typename...> class F>
void Klerdenne()
{
    F<Mode, Optimization>::Run();
    F<Mode>::Run();
    F<Optimization>::Run();
}

void DebugModulesInit([[maybe_unused]]int a)
{
    Debug_RegisterModules_<Optimization, Mode>();
    // Debug_RegisterModules<OptimizationRetail, ModeClient>();
    // Debug_RegisterModules<OptimizationRetail, ModeEditor>();
    // Debug_RegisterModules<OptimizationNonRetail, ModeClient>();
    // Debug_RegisterModules<OptimizationNonRetail, ModeEditor>();
    Klerdenne<DebugModulesInitTemplate>();
    //DebugModulesInit<Mode, Optimization>();
    //DebugModulesInit<Mode>();
    //DebugModulesInit<Optimization>();
}
//     static inline auto DebugModulesInit() -> void
//     {
//         if (OptimizationWrapper::IsNonRetail() || ModeWrapper::IsClient())
//         {
//             Debug_RegisterModules();
//         }
//         OptimizationWrapper::DebugModulesInit();
//         ModeWrapper::DebugModulesInit();
//         Optimization
//     }
// };

int main()
{
    DebugModulesInit(17);
    classB objB;
    classD{objB}.get()->foo();
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