#pragma once

#include <lobo/platform/name_service/NameService.h>
#include <unordered_map>
#include <string>

namespace lobo {
namespace platform {
namespace name_service {

class NameServiceImpl : public NameService {
public:
    std::string resolveName(const std::string& key) override;
    bool registerName(const std::string& key, const std::string& value) override;
    bool unregisterName(const std::string& key) override;

private:
    std::unordered_map<std::string, std::string> mRegistry;
};

} // namespace name_service
} // namespace platform
} // namespace lobo
