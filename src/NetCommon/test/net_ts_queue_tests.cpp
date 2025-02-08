#include "net_tsqueue.h"
#include <gtest/gtest.h>

/**
 * enum class to contain out message types. Uses uint32_t's
 */
enum class CustomMsgTypes : uint32_t
{
    FireBullet,
    MovePlayer
};

TEST(NetTsQueueTests, dummy)
{
    ASSERT_EQ(1,1);
}