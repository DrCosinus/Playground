#include "cuppa/cuppa.hpp"

#define DEMO  0x44454D4F
#define PROTO 0x50524F54

#define CURRENT PROTO

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