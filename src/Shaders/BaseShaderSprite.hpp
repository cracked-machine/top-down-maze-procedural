#ifndef __SPRITES_BASEFRAGMENTSHADER_HPP__
#define __SPRITES_BASEFRAGMENTSHADER_HPP__

#include <Shaders/IShaderSprite.hpp>

namespace ProceduralMaze::Sprites
{

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
class BaseShaderSprite : public IShaderSprite
{
public:
  BaseShaderSprite( std::filesystem::path vertex_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size );

  BaseShaderSprite( const BaseShaderSprite & ) = delete;
  BaseShaderSprite &operator=( const BaseShaderSprite & ) = delete;
  BaseShaderSprite( BaseShaderSprite && ) = default;
  BaseShaderSprite &operator=( BaseShaderSprite && ) = default;

  //! @brief polymorphic destructor for derived classes
  ~BaseShaderSprite() override = default;

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
  void resize_texture( sf::Vector2u new_size ) override;

  sf::Time elapsed() { return m_clock.getElapsedTime(); }

  void set_tag( const std::string &tag ) override { m_tag = tag; }
  std::string get_tag() const override { return m_tag; }

  void active( bool active ) override { m_active = active; }
  bool active() const override { return m_active; }

protected:
  sf::Shader &get_shader() override { return m_shader; }

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

  std::string m_tag;
  bool m_active{ true };
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BASEFRAGMENTSHADER_HPP__