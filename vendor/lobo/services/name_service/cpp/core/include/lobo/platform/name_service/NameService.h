#pragma once

#include <string>

namespace lobo {
namespace platform {
namespace name_service {

class NameService {
public:
    virtual ~NameService() = default;
    virtual std::string resolveName(const std::string& key) = 0;
    virtual bool registerName(const std::string& key, const std::string& value) = 0;
    virtual bool unregisterName(const std::string& key) = 0;
};

} // namespace name_service
} // namespace platform
} // namespace lobo
