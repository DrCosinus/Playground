//#include <functional>
//#include <any>
//#include <limits>

//using magic=std::function<std::any()>

template<int N>
auto A()
{
    return A<N-1>;
}

auto B()
{
    return A<64>;
}

template<>
auto A<0>()
{
    return B;
}

template<typename T>
struct zero_input_one_output
{
    auto operator()() -> T;
};

int main(void)
{
    auto state = A<64>;
    state = state();
}