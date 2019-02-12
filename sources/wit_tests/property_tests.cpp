#include "simple_tests.hpp"

#include "property.hpp"

#include <iostream>

//using namespace wit;

// Holder type must be a union
// all fields must be declared in an anonymous struct
// default member initializer are not allowed with MSVC
// C2926: a default member initializer is not allowed for a member
//        of an anonymous struct within a union
union TestDummy {
  private:
    const int &getX() const { return x_; }
    void setX(const int &value) { x_ = value; }
    const int &getY() const { return y_; }
    void setY(const int &value) { y_ = value; }

  public:
    TestDummy() : TestDummy{5, 3} {} // this is a union no = default;
    TestDummy(int x, int y) : x_{x}, y_{y} {}
    TestDummy(int x) : TestDummy{x, 3} {}

    Property<TestDummy, int, &TestDummy::getX, &TestDummy::setX> x;
    Property<TestDummy, int, &TestDummy::getY, &TestDummy::setY> y;

  private:
    struct
    {
        int x_;
        int y_;
    };
};

int main()
{
    CHECK_EQ(sizeof(TestDummy), sizeof(int));

    TestDummy dummy{5, 3}, buddy{2, 3};
    CHECK_EQ(dummy.x, 5);
    CHECK_EQ(3, dummy.y);
    CHECK_EQ(dummy.y, buddy.y);

    dummy.x = 17;
    CHECK_EQ(dummy.x, 17);
    dummy.x = 7.3;
    CHECK_EQ(dummy.x, int(7.3));
    CHECK_EQ(dummy.x + 35, 42);
    CHECK_EQ(2 * dummy.x, 14);
    CHECK_EQ(dummy.y + buddy.x, 7);
    CHECK_LT(buddy.x, buddy.y);
    CHECK_LE(buddy.x, buddy.y);
    CHECK_GT(buddy.y, buddy.x);
    CHECK_GE(buddy.y, buddy.x);
    dummy.x += 3;
    CHECK_EQ(dummy.x, 10);
    dummy.x += dummy.y;
    CHECK_EQ(dummy.x, 13);
    dummy.x += 3.2;
    CHECK_EQ(dummy.x, int(13 + 3.2));
    dummy.x = 5;
    dummy.x *= 3.2;
    CHECK_EQ(dummy.x, int(5 * 3.2));

    tdd::PrintTestResults([](const char *line) { std::cout << line << std::endl; });
    return 0;
}