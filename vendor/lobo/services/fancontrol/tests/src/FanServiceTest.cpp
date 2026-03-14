#include <lobo/platform/fancontrol/FanService.h>
#include <gtest/gtest.h>

using namespace lobo::platform;
using namespace lobo::platform::fancontrol;

TEST(FanServiceTest, StartsAndStops) {
    FanService svc;
    EXPECT_TRUE(svc.start());
    EXPECT_TRUE(svc.isRunning());
    svc.stop();
    EXPECT_FALSE(svc.isRunning());
}

TEST(FanServiceTest, SetValidDutyCycle) {
    FanService svc;
    EXPECT_EQ(Result::OK, svc.setDutyCycle(50));
    EXPECT_EQ(50, svc.getDutyCycle());
}

TEST(FanServiceTest, SetInvalidDutyCycleReturnsError) {
    FanService svc;
    EXPECT_EQ(Result::INVALID_ARG, svc.setDutyCycle(101));
}
