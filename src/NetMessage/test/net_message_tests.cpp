#include "net_message.h"
#include <gtest/gtest.h>

/**
 * enum class to contain out message types. Uses uint32_t's
 */
enum class CustomMsgTypes : uint32_t
{
    FireBullet,
    MovePlayer
};

TEST(ModuleTest, SampleTest)
{
    // create our custom message type
    olc::net::message<CustomMsgTypes> msg;
    // Use custom enum class
    msg.header.id = CustomMsgTypes::FireBullet;
    uint32_t a = 0xFFFF0000;
    bool b = true;
    float c = 3.14159f;

    uint32_t a_hold = 0xFFFF0000;
    bool b_hold = true;
    float c_hold = 3.14159f;
    msg << a << b << c;

    a = 99;
    b = false;
    c = 99.0f;

    msg >> c >> b >> a;

    // Example test
    EXPECT_EQ(a, a_hold);
    EXPECT_EQ(b, b_hold);
}