#ifndef __SPRITES_MISTSHADER_HPP__
#define __SPRITES_MISTSHADER_HPP__

#include <spdlog/spdlog.h>

#include <SFML/System/Time.hpp>

#include <Shaders/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class MistShader : public BaseFragmentShader
{
public:
  MistShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
      : BaseFragmentShader( shader_path, texture_size )
  {
  }
  ~MistShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 16, 32, 32 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override
  {
    m_shader.setUniform( "resolution", sf::Vector2f{ m_render_texture.getSize() } );
    SPDLOG_DEBUG( "FloodWaterShader initialized" );
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