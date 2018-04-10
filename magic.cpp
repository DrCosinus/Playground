#include <functional>
#include <iostream>

template<typename... Ts>
struct RecursiveHelper
{
  using type = std::function<RecursiveHelper(Ts...)>;
  /*explicit*/ RecursiveHelper(type f) : func(f) {}
  operator type() const { return func; }

private:
  const type func;
};

using F = RecursiveHelper<int&, int&>::type;
F B(int& first, int& second);

F A(int& first, int& second)
{
    ++first;
    return B;
}

F B(int& first, int& second)
{
    second+=3;
    return A;
}


// A renvoie une méthode (ici A lui-même) qui renvoie un F
// qui implicitement castable en RecursiveHelper
// qui prend aucun argument et qui renvoie un RecursiveHelper
// et qui est castable implicitement en RecursiveHelper

int main()
{
    int plop = 0;
    int plip = 0;

    F a = A;
    a = a(plop, plip);
    a = a(plop, plip);
    a = a(plop, plip);
    a = a(plop, plip);
    a = a(plop, plip);
    // ...
    std::cout << plop << " " << plip << std::endl;
}