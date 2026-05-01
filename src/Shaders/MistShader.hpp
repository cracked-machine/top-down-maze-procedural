#ifndef __SPRITES_MISTSHADER_HPP__
#define __SPRITES_MISTSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class MistShader : public BaseShaderSprite
{
public:
  MistShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShaderSprite( vert_shader_path, frag_shader_path, texture_size )
  {
    setup();
  }

  ~MistShader() override = default;

  void pre_setup_texture() override { m_render_texture.clear( sf::Color( 16, 128, 32 ) ); }

  void post_setup_shader() override {}
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MISTSHADER_HPP__