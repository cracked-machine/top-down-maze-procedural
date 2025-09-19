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
// The shader draws to the render texture which is then applied to the sprite
// When you call draw on this class it draws the sprite with the shader applied
// to the given render target
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
/**
 * @brief Abstract base class for fragment shader-based drawable objects in SFML.
 *
 * This class provides a framework for creating drawable objects that apply fragment shaders
 * to render textures.
 *
 * The class manages a render texture, sprite, and shader, providing a standardized
 * initialization sequence and interface for derived classes to implement custom
 * shader effects.
 *
 * @note This class is move-only (copy operations are deleted).
 * @note Derived classes must implement the pure virtual functions to define
 *       texture setup, shader configuration, and runtime updates.
 *
 * Usage pattern:
 * 1. Create derived class implementing pure virtual functions
 * 2. Call setup() to initialize the shader system;
 *    You can do this from the constructor if you wish
 * 3. Use set_position() to position the drawable
 * 4. Call update() in your render loop for dynamic effects
 * 5. Draw using SFML's standard drawing mechanisms
 *
 * @note If you need to update with additional uniforms,
 *     create a new update function in your derived class
 */
class BaseFragmentShader : public sf::Drawable, public sf::Transformable
{
public:
  BaseFragmentShader( std::filesystem::path shader_path, sf::Vector2u texture_size );

  BaseFragmentShader( const BaseFragmentShader & ) = delete;
  BaseFragmentShader &operator=( const BaseFragmentShader & ) = delete;
  BaseFragmentShader( BaseFragmentShader && ) = default;
  BaseFragmentShader &operator=( BaseFragmentShader && ) = default;

  virtual ~BaseFragmentShader() = default;

  /**
   * @brief Prepares texture-related operations before the main texture setup process.
   *
   * This pure virtual method is called before the primary texture configuration
   * and allows derived fragment shader classes to perform any necessary
   * preprocessing steps such as texture parameter initialization, texture unit
   * binding preparation, or shader uniform setup.
   *
   * @note This method must be implemented by all derived classes.
   * @note This function is called automatically by setup()
   */
  virtual void pre_setup_texture() = 0;

  /**
   * @brief Pure virtual function called after shader setup is complete.
   *
   * This method is invoked after the base shader initialization process
   * has finished. Derived classes must implement this function to perform
   * any additional setup or configuration specific to their shader type.
   *
   * @note This is a pure virtual function that must be overridden by
   *       concrete implementations.
   * @note This function is called automatically by setup()
   */
  virtual void post_setup_shader() = 0;

  /**
   * @brief Updates the fragment shader state or parameters.
   *
   * This pure virtual function must be implemented by derived classes to handle
   * shader-specific update logic such as uniform variables, time-based parameters,
   * or other dynamic shader properties that need to be refreshed each frame.
   *
   * @note This function be called by the user from their game loop to ensure
   *       dynamic shader effects are applied.
   * @note If you need to update with additional uniforms, set this to {} and
   *     create a new parameterized update function in your derived class
   */
  virtual void update() = 0;

  /**
   * @brief Initializes and configures the base fragment shader.
   *
   * This method performs the necessary setup operations for the fragment shader:
   * 1. pre_setup_texture()
   * 2. m_texture.display()
   * 3. post_setup_shader()
   *
   * @throws std::runtime_error if shader compilation or linking fails
   */
  void setup();

  // Use this to set the position of the renderable sprite
  void set_position( const sf::Vector2f &position );
  // override sf::Transformable::setPosition since that has no meaning in the context of this class
  // and we want to avoid unexpected behavior where the position of the sprite is not set correctly
  void setPosition( const sf::Vector2f &position ) { set_position( position ); }
  // Use this to set the view of the internal render texture
  void set_texture_view( sf::View view_update ) { m_render_texture.setView( view_update ); }
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