#ifndef __SYS_RENDERMENUSYSTEM_HPP__
#define __SYS_RENDERMENUSYSTEM_HPP__

#include <SFML/System/Time.hpp>

#include <spdlog/spdlog.h>

#include <Shaders/TitleScreenShader.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Sys {

class RenderMenuSystem : public RenderSystem
{
public:
  RenderMenuSystem( ProceduralMaze::SharedEnttRegistry registry, sf::RenderWindow &window,
                    Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : RenderSystem( registry, window, sprite_factory, sound_bank )
  {
    SPDLOG_DEBUG( "RenderMenuSystem constructor called" );
  }
  ~RenderMenuSystem() = default;

  void init_title();
  void render_title();

  void render_settings_widgets( sf::Time deltaTime );
  void render_settings( sf::Time deltaTime );
  void render_paused();
  void render_defeat_screen();
  void render_victory_screen();

private:
  std::unique_ptr<Sprites::TitleScreenShader> m_title_screen_shader;
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERMENUSYSTEM_HPP__