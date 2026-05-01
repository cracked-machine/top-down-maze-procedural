#ifndef __SPRITES_TITLESCREENSHADER_HPP__
#define __SPRITES_TITLESCREENSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class TitleScreenShader : public BaseShaderSprite
{
public:
  TitleScreenShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShaderSprite( vert_shader_path, frag_shader_path, texture_size )
  {
    setup();
  }
  ~TitleScreenShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 128, 128, 128 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override { m_shader.setUniform( "texture", sf::Shader::CurrentTexture ); }
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_TITLESCREENSHADER_HPP__