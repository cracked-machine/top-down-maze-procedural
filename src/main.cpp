#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include <basic_log_controller.hpp>
#include <components/position.hpp>
#include <engine.hpp>
#include <systems/render_system.hpp>

int main()
{
    // logging: make sure errors - exceptions and failed asserts - go to log file
    fclose(stderr);

    using Logger = ProceduralMaze::Logging::BasicLogController;
    std::unique_ptr<Logger> logger{ std::make_unique<Logger>("logger", "log.txt") };
    spdlog::set_level(spdlog::level::trace);

    SPDLOG_INFO("Init");

    ProceduralMaze::Engine engine;
    engine.run();

}