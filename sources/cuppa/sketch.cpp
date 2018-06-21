#include "cuppa.hpp"
//#include "nonintegral_enum.hpp"
//#include "simple_tests.hpp"

#include <iostream>

struct sketch : cuppa::app
{
    void setup() override
    {

    }
    void update() override
    {

    }
    void draw() override
    {
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}