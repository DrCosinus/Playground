#include "cuppa.hpp"

#include "demo.hpp"

int main(void)
{
    std::unique_ptr<cuppa::app> app = createDemo();
    app->run();
}