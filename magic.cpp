#include <iostream>

using namespace std;

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
    static void Run() { std::cout << "- Mode: Editor" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeClient>
{
    static void Run() { std::cout << "- Mode: Client" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<OptimizationRetail>
{
    static void Run() { std::cout << "- Optimization: Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<OptimizationNonRetail>
{
    static void Run() { std::cout << "- Optimization: Non Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeEditor, OptimizationRetail>
{
    static void Run() { std::cout << "- Mode: Editor && Optimization: Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeClient, OptimizationRetail>
{
    static void Run() { std::cout << "- Mode: Client && Optimization: Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeEditor, OptimizationNonRetail>
{
    static void Run() { std::cout << "- Mode: Editor && Optimization: Non Retail" << std::endl; }
};

template<>
struct DebugModulesInitTemplate<ModeClient, OptimizationNonRetail>
{
    static void Run() { std::cout << "- Mode: Client && Optimization: Non Retail" << std::endl; }
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

void DebugModulesInit()
{
    cout << "client retail:" << endl;
    DebugModulesInitTemplate<OptimizationRetail, ModeClient>::Run();
    cout << "editor retail" << endl;
    DebugModulesInitTemplate<OptimizationRetail, ModeEditor>::Run();
    cout << "client non retail" << endl;
    DebugModulesInitTemplate<OptimizationNonRetail, ModeClient>::Run();
    cout << "editor non retail" << endl;
    DebugModulesInitTemplate<OptimizationNonRetail, ModeEditor>::Run();
    cout << endl;
    cout << "current configuration" << endl;
    Debug_RegisterModules_<Optimization, Mode>::Run();
    Klerdenne<DebugModulesInitTemplate>();
}

int main()
{
    DebugModulesInit();
}