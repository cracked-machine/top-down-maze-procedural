#ifndef __SPRITES_DARKMODESHADER_HPP__
#define __SPRITES_DARKMODESHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class DarkModeShader : public BaseShaderSprite
{
public:
  DarkModeShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShaderSprite( vert_shader_path, frag_shader_path, texture_size )
  {
    setup();
  }

  ~DarkModeShader() override = default;

  void pre_setup_texture() override
  {
    // Clear the render texture with a dark color
    m_render_texture.clear( sf::Color( 0, 0, 0 ) );
  }

  void post_setup_shader() override
  {
    // nothing special to do here
  }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_DARKMODESHADER_HPP__