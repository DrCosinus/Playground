#include <iostream>
#include <memory>

using namespace std;

struct Foo
{
    static Foo Create() { return {}; }
    static unique_ptr<Foo> CreatePtr() { return make_unique<Foo>(); }

    Foo() { cout << "default constructor" << endl; }
    Foo(const Foo&) { cout << "copy constructor" << endl; }
    Foo(Foo&&) { cout << "move constructor" << endl; }
};

constexpr int get_size() { return 42; }
//int tableau_statique[get_size() + 17];
template<int I> struct Bar {};
Bar<get_size()> bar;

int main()
{
    #pragma region toto
  __pragma(region toto);
  {
    auto plip = std::make_unique<Foo>();
    // output:  default constructor
  }
  __pragma(endregion);
  #pragma endregion
  cout << endl;
  {
    auto plop = std::make_unique<Foo>(Foo::Create());
    // output:  default constructor
    //          move constructor
  }
  cout << endl;
  {
    auto plup = Foo::CreatePtr();
    // output:  default constructor
  }
}