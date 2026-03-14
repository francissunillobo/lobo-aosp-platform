#pragma once

#include <string>

namespace lobo {
namespace platform {

class Logging {
public:
    static void log(const std::string& tag, const std::string& message);
    static void logError(const std::string& tag, const std::string& message);
};

} // namespace platform
} // namespace lobo
