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
    setup();
    SPDLOG_INFO( "FloodWaterShader initialized" );
  }
  ~FloodWaterShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 64, 128, 255 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override { m_shader.setUniform( "resolution", sf::Vector2f{ m_render_texture.getSize() } ); }

  void update( float waterLevel )
  {
    m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );
    m_shader.setUniform( "waterLevel", waterLevel );
    m_sprite.setPosition( { 0, waterLevel } );
  }
  void update() override { /* unused */ }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_FLOODERWATERSHADER_HPP__