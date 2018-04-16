#include <iostream>
#include <functional>

// A renvoie une méthode qui renvoie un F
// qui implicitement castable en RecursiveHelper
// qui prend aucun argument et qui renvoie un RecursiveHelper
// et qui est castable implicitement en RecursiveHelper

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

int main()
{
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