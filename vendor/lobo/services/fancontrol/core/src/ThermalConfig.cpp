#include <lobo/platform/fancontrol/ThermalConfig.h>

namespace lobo {
namespace platform {
namespace fancontrol {

ThermalConfig ThermalConfig::loadDefault() {
    return ThermalConfig{};
}

} // namespace fancontrol
} // namespace platform
} // namespace lobo
