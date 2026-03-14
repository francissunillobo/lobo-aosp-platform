#include <lobo/platform/name_service/NameServiceImpl.h>
#include <lobo/platform/logging/Logging.h>

int main() {
    lobo::platform::Logging::log("NameService", "Starting");
    lobo::platform::name_service::NameServiceImpl svc;
    svc.registerName("lobo.fan", "fancontrol");
    return 0;
}
