#ifndef __SPRITES_BASEFRAGMENTSHADER_HPP__
#define __SPRITES_BASEFRAGMENTSHADER_HPP__

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>

#include <filesystem>
#include <map>

namespace ProceduralMaze::Sprites
{

/**
 * @class UniformBuilder
 * @brief A builder class for setting multiple shader uniforms in a chainable manner.
 *
 * The UniformBuilder class provides a fluent interface for setting shader uniforms
 * before applying them all at once to an sf::Shader object. It supports various
 * data types and provides special handling for sf::Color objects by converting
 * them to normalized Vec4 values.
 *
 * @details The class uses a function composition pattern to build up a series
 * of uniform assignments that are executed when apply() is called. Each call
 * to set() chains the new uniform assignment with the previous ones.
 *
 * Special type handling:
 * - sf::Color: Automatically converts RGBA values from 0-255 range to 0.0-1.0 range
 * - Other types: Passed directly to sf::Shader::setUniform()
 *
 * @example
 * UniformBuilder builder;
 * builder.set("color", sf::Color::Red)
 *        .set("time", 1.5f)
 *        .set("position", sf::Vector2f(10.0f, 20.0f))
 *        .apply(shader);
 */
class UniformBuilder
{
private:
  std::function<void( sf::Shader & )> m_chain;

public:
  //! @brief Update the chain with the new uniform. Save the existing chain,
  //         build a new chain that calls the existing chain first,
  //         then set this new uniform.
  //! @tparam T
  //! @param name
  //! @param value
  //! @return UniformBuilder&
  template <typename T>
  UniformBuilder &set( const std::string &name, const T &value )
  {
    auto existing_chain = std::move( m_chain );

    if constexpr ( std::is_same_v<T, sf::Color> )
    {
      m_chain = [existing_chain, name, value]( sf::Shader &shader )
      {
        if ( existing_chain ) existing_chain( shader );
        shader.setUniform( name, sf::Glsl::Vec4( value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f ) );
      };
    }
    else
    {
      m_chain = [existing_chain, name, value]( sf::Shader &shader )
      {
        if ( existing_chain ) existing_chain( shader );
        shader.setUniform( name, value );
      };
    }
    return *this;
  }

  void apply( sf::Shader &shader )
  {
    if ( m_chain ) m_chain( shader );
  }
};

/**
 * @brief Base class for fragment shader-based drawable objects in SFML.
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
class BaseShader : public sf::Drawable, public sf::Transformable
{
public:
  BaseShader( std::filesystem::path vertex_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size );

  BaseShader( const BaseShader & ) = delete;
  BaseShader &operator=( const BaseShader & ) = delete;
  BaseShader( BaseShader && ) = default;
  BaseShader &operator=( BaseShader && ) = default;

  //! @brief polymorphic destructor for derived classes
  virtual ~BaseShader() = default;

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

  void update( UniformBuilder builder );

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
  void setPosition( const sf::Vector2f &position );

  void set_center_at_position( sf::Vector2f pos );

  // Use this to set the view of the internal render texture
  void set_texture_view( sf::View view_update );
  // internal draw function called by SFML
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

  auto get_texture_size() const { return m_render_texture.getSize(); }
  void resize_texture( sf::Vector2u new_size );

  sf::Time elapsed() { return m_clock.getElapsedTime(); }

protected:
  // this is the pallette texture that the shader will be applied to
  sf::RenderTexture m_render_texture;
  // the sprite that uses the texture
  sf::Sprite m_sprite{ m_render_texture.getTexture() };
  // the vertex/fragment shader to be applied to the sprite
  sf::Shader m_shader;
  // clock for timing shader effects
  sf::Clock m_clock{};

private:
  std::filesystem::path m_vert_shader_path;
  std::filesystem::path m_frag_shader_path;

  void load_shader_files();
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BASEFRAGMENTSHADER_HPP__