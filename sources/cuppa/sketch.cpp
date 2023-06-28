#include "cuppa/cuppa.hpp"

constexpr auto DEMO = 0x44454D4F;
constexpr auto PROTO = 0x50524F54;

constexpr auto CURRENT = DEMO;

#if CURRENT == DEMO
#include "demo.hpp"
#elif CURRENT == PROTO
#include "proto/proto.hpp"
#else
#error "Unknown App kind"
#endif

int main(void)
{
#if CURRENT == DEMO
    std::unique_ptr<cuppa::app> app = createDemo();
#elif CURRENT == PROTO
    std::unique_ptr<cuppa::app> app = createProto();
#else
#error "Unknown App kind"
#endif
    app->run();
}