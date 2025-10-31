#ifndef __SPRITES_BACKGROUND_SHADER_HPP__
#define __SPRITES_BACKGROUND_SHADER_HPP__

#include <spdlog/spdlog.h>

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Random.hpp>
#include <Sprites/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites {

class BackgroundShader : public BaseFragmentShader
{
public:
  BackgroundShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
      : BaseFragmentShader( shader_path, texture_size )
  {
    setup();
    SPDLOG_INFO( "BackgroundShader initialized" );
  }
  ~BackgroundShader() override = default;

  auto get_render_texture() -> sf::RenderTexture & { return m_render_texture; }
  void pre_setup_texture() override {}
  void post_setup_shader() override {}
  void update() override { /* unused */ }

  sf::Time getElapsedTime() const { return m_clock.getElapsedTime(); }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BACKGROUND_SHADER_HPP__