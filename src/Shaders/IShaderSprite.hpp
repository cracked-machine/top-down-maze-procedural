#ifndef SRC_SHADERS_ISHADERSPRITE_HPP_
#define SRC_SHADERS_ISHADERSPRITE_HPP_

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

class ISpriteShader
{
public:
  //! @brief polymorphic destructor for derived classes
  virtual ~ISpriteShader() = default;

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

  virtual void update( UniformBuilder builder ) = 0;
};

} // namespace ProceduralMaze::Sprites

#endif // SRC_SHADERS_ISHADERSPRITE_HPP_