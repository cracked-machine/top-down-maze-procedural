#ifndef SRC_SHADERS_FEARDISTORTIONSHADER_HPP__
#define SRC_SHADERS_FEARDISTORTIONSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>

namespace Game::Sprites
{

//! @brief Full-screen post-process shader that warps the already-rendered game frame with a wave
//! distortion whose intensity tracks the player's fear stat. Unlike the other IShaderSprite overlays,
//! its render texture is not left as a flat color: RenderGameSystem redirects the frame's world/UI
//! drawing into it (via BaseShaderSprite::get_render_texture()) before compositing the distorted
//! result back onto the window as the final step of the frame.
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
};

} // namespace Game::Sprites

#endif // SRC_SHADERS_FEARDISTORTIONSHADER_HPP__
