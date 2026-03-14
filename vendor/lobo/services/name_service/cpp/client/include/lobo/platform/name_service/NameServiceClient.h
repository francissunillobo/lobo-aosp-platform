#pragma once

#include <string>

namespace lobo {
namespace platform {
namespace name_service {

class NameServiceClient {
public:
    NameServiceClient();
    std::string resolve(const std::string& key);

private:
    // In a real implementation this would hold a Binder proxy
};

} // namespace name_service
} // namespace platform
} // namespace lobo
