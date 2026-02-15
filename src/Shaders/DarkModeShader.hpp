#ifndef __SPRITES_DARKMODESHADER_HPP__
#define __SPRITES_DARKMODESHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class DarkModeShader : public BaseFragmentShader
{
public:
  DarkModeShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
      : BaseFragmentShader( shader_path, texture_size )
  {
  }

  ~DarkModeShader() override = default;

  void pre_setup_texture() override
  {
    // Clear the render texture with a dark color
    m_render_texture.clear( sf::Color( 0, 0, 0 ) );
  }

  void post_setup_shader() override
  {
    // nothing special to do here
  }

  void update( sf::Vector2f shader_position, sf::Vector2f aperture_half_size, sf::Vector2u local_resolution, sf::Vector2u display_resolution )
  {

    m_sprite.setPosition( shader_position );
    m_shader.setUniform( "local_resolution", sf::Vector2f{ local_resolution } );
    m_shader.setUniform( "display_resolution", sf::Vector2f{ display_resolution } );
    m_shader.setUniform( "aperture_half_size", aperture_half_size );
    m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );
  }
  void update() override { /* unused */ }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_DARKMODESHADER_HPP__