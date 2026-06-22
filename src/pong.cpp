#include <string_view>

#include "core/application.h"
#include "utils/exception.h"
#include "utils/log.h"

int main()
{
    using namespace std::literals;

    try
    {
        arm::log::info("Hello Pong");

        const auto application_info = pong::ApplicationInfo{
            .application_name = "Pong",
            .engine_name = "not_an_engine",
            .application_root_dir = "C:/dev/Pong",
            .version =
                {
                    .major = APP_VERSION_MAJOR,
                    .minor = APP_VERSION_MINOR,
                    .patch = APP_VERSION_PATCH,
                },
        };

        auto app = pong::Application(application_info);
        app.run();

        return 0;
    }
    catch (arm::Exception &e)
    {
        arm::log::error("Pong error: {}", e.to_string());
        return 1;
    }
    catch (...)
    {
        arm::log::error("Unknown error, exiting...");
        return 1;
    }
} // main()
