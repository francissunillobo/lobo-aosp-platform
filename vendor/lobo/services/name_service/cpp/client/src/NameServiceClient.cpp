#include <lobo/platform/name_service/NameServiceClient.h>
#include <lobo/platform/logging/Logging.h>

namespace lobo {
namespace platform {
namespace name_service {

NameServiceClient::NameServiceClient() {
    Logging::log("NameServiceClient", "Client created");
}

std::string NameServiceClient::resolve(const std::string& key) {
    Logging::log("NameServiceClient", "Resolving: " + key);
    return "";
}

} // namespace name_service
} // namespace platform
} // namespace lobo
