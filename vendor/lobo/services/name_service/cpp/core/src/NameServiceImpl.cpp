#include <lobo/platform/name_service/NameServiceImpl.h>

namespace lobo {
namespace platform {
namespace name_service {

std::string NameServiceImpl::resolveName(const std::string& key) {
    auto it = mRegistry.find(key);
    return (it != mRegistry.end()) ? it->second : "";
}

bool NameServiceImpl::registerName(const std::string& key, const std::string& value) {
    mRegistry[key] = value;
    return true;
}

bool NameServiceImpl::unregisterName(const std::string& key) {
    return mRegistry.erase(key) > 0;
}

} // namespace name_service
} // namespace platform
} // namespace lobo
