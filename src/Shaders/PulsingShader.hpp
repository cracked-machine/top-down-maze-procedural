#ifndef __SPRITES_PULSINGSHADER_HPP__
#define __SPRITES_PULSINGSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class PulsingShader : public BaseShader
{
public:
  PulsingShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShader( vert_shader_path, frag_shader_path, texture_size )
  {
  }
  ~PulsingShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 16, 32, 32 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override { m_shader.setUniform( "resolution", sf::Vector2f{ m_render_texture.getSize() } ); }

  void update( sf::Vector2f position, float alpha, sf::Vector2u resolution )
  {
    m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );
    m_shader.setUniform( "alpha", alpha ); // Set the alpha value
    m_shader.setUniform( "resolution", sf::Vector2f{ resolution } );
    m_sprite.setPosition( { position } );
  }
  void update() override { /* unused */ }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MISTSHADER_HPP__