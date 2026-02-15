#ifndef __SPRITES_PULSINGSHADER_HPP__
#define __SPRITES_PULSINGSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class PulsingShader : public BaseFragmentShader
{
public:
  PulsingShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
      : BaseFragmentShader( shader_path, texture_size )
  {
  }
  ~PulsingShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 16, 32, 32 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override
  {
    m_shader.setUniform( "resolution", sf::Vector2f{ m_render_texture.getSize() } );
  }

  void update( sf::Vector2f position, float alpha )
  {
    m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );
    m_shader.setUniform( "alpha", alpha ); // Set the alpha value
    m_sprite.setPosition( { position } );
  }
  void update() override { /* unused */ }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MISTSHADER_HPP__