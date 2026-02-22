#ifndef __SPRITES_FLOODERWATERSHADER_HPP__
#define __SPRITES_FLOODERWATERSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class FloodWaterShader : public BaseShader
{
public:
  FloodWaterShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShader( vert_shader_path, frag_shader_path, texture_size )
  {
  }
  ~FloodWaterShader() override = default;

  void pre_setup_texture() override { m_render_texture.clear( sf::Color( 16, 32, 32 ) ); }

  void post_setup_shader() override { m_shader.setUniform( "resolution", sf::Vector2f{ m_render_texture.getSize() } ); }

  void update( sf::Vector2f position )
  {
    m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );
    m_shader.setUniform( "waterLevel", 0 );
    m_sprite.setPosition( { position } );
  }
  void update() override { /* unused */ }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_FLOODERWATERSHADER_HPP__