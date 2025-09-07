#ifndef __SYS_RENDERMENUSYSTEM_HPP__
#define __SYS_RENDERMENUSYSTEM_HPP__

#include <Systems/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

class RenderMenuSystem : public RenderSystem
{
public:
  RenderMenuSystem( std::shared_ptr<entt::basic_registry<entt::entity>> registry )
      : RenderSystem( registry )
  {
  }
  ~RenderMenuSystem() = default;

  void render_menu();
  void render_settings( PlayerSystem &m_settings_sys, FloodSystem &m_flood_sys );
  void render_paused();
  void render_defeat_screen();
  void render_victory_screen();
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERMENUSYSTEM_HPP__