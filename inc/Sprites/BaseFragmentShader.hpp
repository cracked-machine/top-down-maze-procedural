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

  // set the Sprite position
  void set_position( const sf::Vector2f &position );

  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

protected:
  sf::RenderTexture m_texture;
  sf::Sprite m_sprite{ m_texture.getTexture() };
  sf::Shader m_shader;
  sf::Clock m_clock{};

private:
  std::filesystem::path m_shader_path{};

  void setup_shader();
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BASEFRAGMENTSHADER_HPP__