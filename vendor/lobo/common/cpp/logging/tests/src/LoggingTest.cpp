#include <lobo/platform/logging/Logging.h>
#include <gtest/gtest.h>

TEST(LoggingTest, LogDoesNotCrash) {
    lobo::platform::Logging::log("test", "hello");
}

TEST(LoggingTest, LogErrorDoesNotCrash) {
    lobo::platform::Logging::logError("test", "error");
}
