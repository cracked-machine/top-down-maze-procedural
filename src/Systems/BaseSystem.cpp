#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys {

// initialised by first call to getEventDispatcher()
std::unique_ptr<entt::dispatcher> BaseSystem::m_event_dispatcher = nullptr;

} // namespace ProceduralMaze::Sys