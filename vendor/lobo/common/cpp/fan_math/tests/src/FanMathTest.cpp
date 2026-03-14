#include <lobo/platform/fan_math/FanMath.h>
#include <gtest/gtest.h>

using lobo::platform::FanMath;

TEST(FanMathTest, BelowMinTempGivesZeroDuty) {
    EXPECT_EQ(0, FanMath::tempToDutyCycle(20.0f, 40.0f, 80.0f));
}

TEST(FanMathTest, AboveMaxTempGivesFullDuty) {
    EXPECT_EQ(100, FanMath::tempToDutyCycle(90.0f, 40.0f, 80.0f));
}

TEST(FanMathTest, MidpointGivesHalfDuty) {
    EXPECT_EQ(50, FanMath::tempToDutyCycle(60.0f, 40.0f, 80.0f));
}

TEST(FanMathTest, ClampLow) {
    EXPECT_EQ(0, FanMath::clamp(-5, 0, 100));
}

TEST(FanMathTest, ClampHigh) {
    EXPECT_EQ(100, FanMath::clamp(200, 0, 100));
}
