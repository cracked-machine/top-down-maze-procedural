#ifndef __SYS_RENDERMENUSYSTEM_HPP__
#define __SYS_RENDERMENUSYSTEM_HPP__

#include <BombSystem.hpp>
#include <SFML/System/Time.hpp>
#include <Systems/RenderSystem.hpp>
#include <TitleScreenShader.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class RenderMenuSystem : public RenderSystem
{
public:
  RenderMenuSystem( std::shared_ptr<entt::basic_registry<entt::entity>> registry )
      : RenderSystem( registry )
  {
  }
  ~RenderMenuSystem() = default;

  void render_loading_screen( const std::string &status );
  void render_title();

  void render_settings_widgets( sf::Time deltaTime );
  void render_settings( sf::Time deltaTime );
  void render_paused();
  void render_defeat_screen();
  void render_victory_screen();

private:
  Sprites::TitleScreenShader m_title_screen_shader{ "res/shaders/TitleScreen.frag", kDisplaySize };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERMENUSYSTEM_HPP__