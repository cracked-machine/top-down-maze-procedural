#include "Debug/AssertHandler.hpp" // Include this first for custom assertions

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include <Engine.hpp>
#include <Logging/BasicLogController.hpp>

int main()
{
  // Logging: make sure errors - exceptions and failed asserts - go to log file
  fclose( stderr );

  using Logger = ProceduralMaze::Logging::BasicLogController;
  std::unique_ptr<Logger> logger{ std::make_unique<Logger>( "logger", "log.txt" ) };
  spdlog::set_level( spdlog::level::trace );

  SPDLOG_DEBUG( "Entering Engine" );

  ProceduralMaze::Engine engine;
  engine.run();
}