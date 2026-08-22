#ifndef SRC_SHADERS_FEARDISTORTIONSHADER_HPP__
#define SRC_SHADERS_FEARDISTORTIONSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>

namespace Game::Sprites
{

//! @brief Full-screen post-process shader that warps the already-rendered game frame with a wave
//! distortion whose intensity tracks the player's fear stat. Unlike the other IShaderSprite overlays,
//! its render texture is not left as a flat color: registered with a normal Cmp::ZOrderValue (see
//! Factory::Shader::add_fear_distortion) like any other shader, its ZOrderValue is what decides how
//! much of the frame gets captured into it before RenderGameSystem's z-order loop reaches this
//! sprite, finalizes the capture, and composites the distorted result back onto the window.
class FearDistortionShader : public BaseShaderSprite
{
public:
  FearDistortionShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShaderSprite( vert_shader_path, frag_shader_path, texture_size )
  {
    setup();
  }
  ~FearDistortionShader() override = default;

  void pre_setup_texture() override { m_render_texture.clear( sf::Color::Black ); }

  void post_setup_shader() override { m_shader.setUniform( "texture", sf::Shader::CurrentTexture ); }
  void update( entt::registry &reg ) override;

  [[nodiscard]] bool is_post_process() const override { return true; }

private:
  // The `fear` stat (see Cmp::PlayerStats) only changes in coarse, discrete steps as gameplay ticks
  // apply +/-N modifiers - most notably a -6/second drop while carrying a lit candle, six times the
  // size of the +1/second darkness gain. FREQUENCY_EXP_K/VIGNETTE_LOG_K in FearDistortion.frag both
  // curve steeply at low fear, so feeding that raw step straight into the shader every frame produced
  // a visible snap in the wave whenever fear dropped. These smooth it into a continuous ramp instead.
  float m_smoothed_fear{ 0.f };
  sf::Time m_last_fear_update;
};

} // namespace Game::Sprites

#endif // SRC_SHADERS_FEARDISTORTIONSHADER_HPP__
