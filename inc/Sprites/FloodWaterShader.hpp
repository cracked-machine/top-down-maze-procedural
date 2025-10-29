#ifndef __SPRITES_FLOODERWATERSHADER_HPP__
#define __SPRITES_FLOODERWATERSHADER_HPP__

#include <spdlog/spdlog.h>

#include <SFML/System/Time.hpp>
#include <Sprites/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites {

class FloodWaterShader : public BaseFragmentShader
{
public:
  FloodWaterShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
      : BaseFragmentShader( shader_path, texture_size )
  {
  }
  ~FloodWaterShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 16, 32, 32 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override
  {
    m_shader.setUniform( "resolution", sf::Vector2f{ m_render_texture.getSize() } );
    SPDLOG_INFO( "FloodWaterShader initialized" );
  }

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