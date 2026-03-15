#include <lobo/platform/logging/Logging.h>
#include <log/log.h>

namespace lobo {
namespace platform {

void Logging::log(const std::string& tag, const std::string& message) {
    __android_log_print(ANDROID_LOG_INFO, tag.c_str(), "%s", message.c_str());
}

void Logging::logError(const std::string& tag, const std::string& message) {
    __android_log_print(ANDROID_LOG_ERROR, tag.c_str(), "%s", message.c_str());
}

} // namespace platform
} // namespace lobo
