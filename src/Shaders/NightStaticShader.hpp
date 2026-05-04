#ifndef __SPRITES_PULSINGSHADER_HPP__
#define __SPRITES_PULSINGSHADER_HPP__

#include <SFML/System/Time.hpp>

#include <Shaders/BaseShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites
{

class NightStaticShader : public BaseShaderSprite
{
public:
  NightStaticShader( std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
      : BaseShaderSprite( vert_shader_path, frag_shader_path, texture_size )
  {
    setup();
  }
  ~NightStaticShader() override = default;

  void pre_setup_texture() override
  {
    m_render_texture.clear( sf::Color( 16, 32, 32 ) );
    // std::ignore = m_texture.resize(texture_size);
  }

  void post_setup_shader() override { m_shader.setUniform( "resolution", sf::Vector2f{ m_render_texture.getSize() } ); }
  void update( entt::registry &reg ) override;
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MISTSHADER_HPP__