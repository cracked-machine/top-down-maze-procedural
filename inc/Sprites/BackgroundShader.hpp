#ifndef __SPRITES_BACKGROUND_SHADER_HPP__
#define __SPRITES_BACKGROUND_SHADER_HPP__

#include <Random.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>
#include <spdlog/spdlog.h>

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

  void update( sf::Vector2f kMapGridOffset,
               float intensity,
               float windStrength,
               float waveAmplitude,
               float timeScale )
  {

    m_shader.setUniform( "time", m_clock.getElapsedTime().asSeconds() );
    m_shader.setUniform( "sandIntensity", intensity );
    m_shader.setUniform(
        "screenSize",
        sf::Vector2f{
            m_render_texture.getSize() } ); // Assuming m_render_texture is your render texture
    m_shader.setUniform(
        "worldPosition",
        sf::Vector2f{ 0, kMapGridOffset.y * Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.y } );
    m_shader.setUniform( "windDirection", sf::Vector2f{ 1.0f, 0.0f } ); // Example wind direction
    m_shader.setUniform( "windStrength", windStrength );                // Example wind strength
    m_shader.setUniform( "waveAmplitude", waveAmplitude );              // Example wave amplitude
    m_shader.setUniform( "timeScale", timeScale );                      // Example time scale
  }

  sf::Time getElapsedTime() const { return m_clock.getElapsedTime(); }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BACKGROUND_SHADER_HPP__