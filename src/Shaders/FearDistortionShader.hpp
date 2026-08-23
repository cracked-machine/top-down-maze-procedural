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
  //! @brief Construct a new Fear Distortion Shader object and immediately run setup().
  //! @param vert_shader_path Path to the vertex shader file.
  //! @param frag_shader_path Path to the fragment shader file.
  //! @param texture_size Size of the backing render texture, in pixels.
  FearDistortionShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShaderSprite( vert_shader_path, frag_shader_path, texture_size )
  {
    setup();
  }
  //! @brief Destroy the Fear Distortion Shader object
  ~FearDistortionShader() override = default;

  //! @brief Clear the render texture to black before the current frame is captured into it.
  void pre_setup_texture() override { m_render_texture.clear( sf::Color::Black ); }

  //! @brief Bind the shader's `texture` uniform to whatever is currently bound (the captured frame).
  void post_setup_shader() override { m_shader.setUniform( "texture", sf::Shader::CurrentTexture ); }

  //! @brief Refresh the smoothed fear level and player-position uniforms each frame.
  //! @param reg The entt registry, used to source the player's fear stat and position.
  void update( entt::registry &reg ) override;

  //! @brief This shader samples the already-rendered frame rather than blending a self-contained texture.
  //! @return Always true; see IShaderSprite::is_post_process() for how this changes composite behaviour.
  [[nodiscard]] bool is_post_process() const override { return true; }

private:
  //! @brief Frame-rate independent exponential smoothing (Utils::Maths::exp_decay) of the raw `fear`
  //! stat towards the value fed into the shader as its `fear` uniform.
  //! @note The `fear` stat (see Cmp::PlayerStats) only changes in coarse, discrete steps as gameplay ticks
  //! apply +/-N modifiers - most notably a -6/second drop while carrying a lit candle, six times the
  //! size of the +1/second darkness gain. FREQUENCY_EXP_K/VIGNETTE_LOG_K in FearDistortion.frag both
  //! curve steeply at low fear, so feeding that raw step straight into the shader every frame produced
  //! a visible snap in the wave whenever fear dropped. These smooth it into a continuous ramp instead.
  float m_smoothed_fear{ 0.f };

  //! @brief elapsed() timestamp of the last update() call, used to compute the per-frame dt fed into
  //! the m_smoothed_fear exponential decay.
  sf::Time m_last_fear_update;
};

} // namespace Game::Sprites

#endif // SRC_SHADERS_FEARDISTORTIONSHADER_HPP__
