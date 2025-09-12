#include "Debug/AssertHandler.hpp" // Include this first for custom assertions

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include <Logging/BasicLogController.hpp>

#include <Engine.hpp>

int main()
{
  // Logging: make sure errors - exceptions and failed asserts - go to log file
  fclose( stderr );

  using Logger = ProceduralMaze::Logging::BasicLogController;
  std::unique_ptr<Logger> logger{ std::make_unique<Logger>( "logger", "log.txt" ) };
  spdlog::set_level( spdlog::level::trace );

  SPDLOG_INFO( "Init" );

  auto registry = std::make_shared<entt::basic_registry<entt::entity>>();
  ProceduralMaze::Engine engine( registry );
  engine.run();
}