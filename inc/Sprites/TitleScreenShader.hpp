#ifndef __SPRITES_TITLESCREENSHADER_HPP__
#define __SPRITES_TITLESCREENSHADER_HPP__

#include <Random.hpp>
#include <SFML/System/Time.hpp>
#include <Sprites/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sprites {

class TitleScreenShader : public BaseFragmentShader
{
public:
  TitleScreenShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
      : BaseFragmentShader( shader_path, texture_size )
  {
    setup();
    SPDLOG_INFO( "TitleScreenShader initialized" );
  }
  ~TitleScreenShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 128, 128, 128 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override
  {
    m_shader.setUniform( "texture", sf::Shader::CurrentTexture );
  }

  void update( sf::Vector2f mousePos )
  {
    m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );
    m_shader.setUniform( "pixel_threshold", ( mousePos.x + mousePos.y ) / 30 );
    m_shader.setUniform( "mouse_cursor", mousePos );
  }

  void update() override { /* unused */ }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_TITLESCREENSHADER_HPP__