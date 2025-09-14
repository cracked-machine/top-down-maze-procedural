#ifndef __SPRITES_BASEFRAGMENTSHADER_HPP__
#define __SPRITES_BASEFRAGMENTSHADER_HPP__

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <filesystem>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sprites {

// Builder class that handles the setup and rendering of sprite shaders
// You must override the following functions:
// - `pre_setup`:     Handles texture pre-render initialization.
//                  Extra arguments should be passed to the derived class
//                  constructor and stored as members.
// - `post_setup`:    Handles texture post-initialization.
//                  Extra arguments should be passed to the derived class
//                  constructor and stored as members.
// - `update`:        Handles runtime updates. The base class does not call this
// function so you must call it
//                  from your game loop. Add more functions to your derived
//                  class if you need them.
class BaseFragmentShader : public sf::Drawable, public sf::Transformable
{
public:
  BaseFragmentShader( std::filesystem::path shader_path, sf::Vector2u texture_size );

  BaseFragmentShader( const BaseFragmentShader & ) = delete;
  BaseFragmentShader &operator=( const BaseFragmentShader & ) = delete;
  BaseFragmentShader( BaseFragmentShader && ) = default;
  BaseFragmentShader &operator=( BaseFragmentShader && ) = default;

  virtual ~BaseFragmentShader() = default;

  // Override this function to pre-initialise `m_texture`
  virtual void pre_setup_texture() = 0;
  // Override this function to set uniforms for `m_shader` during setup
  virtual void post_setup_shader() = 0;
  // Override this function to set uniforms for `m_shader` during runtime
  virtual void update() = 0;

  // Call this function to setup the shader. Initialisation order:
  // 1. pre_setup_texture()
  // 2. m_texture.display()
  // 3. setup_shader()
  // 4. post_setup_shader()
  void setup();

  // Use this to set the position of the overall sprite (that contains the shader effect)
  void set_position( const sf::Vector2f &position );

  // internal draw function called by SFML
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

protected:
  // this is the pallette texture that the shader will be applied to
  sf::RenderTexture m_render_texture;
  // the sprite that uses the texture
  sf::Sprite m_sprite{ m_render_texture.getTexture() };
  // the shader to be applied to the sprite
  sf::Shader m_shader;
  // clock for timing shader effects
  sf::Clock m_clock{};

private:
  std::filesystem::path m_shader_path{};

  void setup_shader();
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BASEFRAGMENTSHADER_HPP__