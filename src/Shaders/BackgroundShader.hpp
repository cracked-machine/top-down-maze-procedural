#ifndef __SPRITES_BACKGROUND_SHADER_HPP__
#define __SPRITES_BACKGROUND_SHADER_HPP__

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <Shaders/BaseShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class BackgroundShader : public BaseShader
{
public:
  BackgroundShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShader( vert_shader_path, frag_shader_path, texture_size )
  {
    setup();
  }
  ~BackgroundShader() override = default;

  auto get_render_texture() -> sf::RenderTexture & { return m_render_texture; }
  void pre_setup_texture() override {}
  void post_setup_shader() override {}
  void update() override { /* unused */ }

  sf::Time getElapsedTime() const { return m_clock.getElapsedTime(); }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BACKGROUND_SHADER_HPP__