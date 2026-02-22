#ifndef __SPRITES_TITLESCREENSHADER_HPP__
#define __SPRITES_TITLESCREENSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class TitleScreenShader : public BaseShader
{
public:
  TitleScreenShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShader( vert_shader_path, frag_shader_path, texture_size )
  {
  }
  ~TitleScreenShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 128, 128, 128 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override { m_shader.setUniform( "texture", sf::Shader::CurrentTexture ); }

  void update( sf::Vector2f mousePos, sf::Vector2u resolution )
  {
    m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );
    m_shader.setUniform( "pixel_threshold", ( mousePos.x + mousePos.y ) / 30 );
    m_shader.setUniform( "mouse_cursor", mousePos );
    m_shader.setUniform( "resolution", sf::Vector2f{ resolution } );
  }

  void update() override { /* unused */ }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_TITLESCREENSHADER_HPP__