#ifndef __SPRITES_VIEWFRAGMENTSHADER_HPP__
#define __SPRITES_VIEWFRAGMENTSHADER_HPP__

#include <SFML/Graphics/View.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <spdlog/spdlog.h>

#include <Components/Random.hpp>
#include <Shaders/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sprites {

//  Usage example:
//
//  // create a ViewFragmentShader instance with a shader and view size of 64x64
//  ViewFragmentShader view_shader("path/to/shader.frag", sf::Vector2u(64, 64));
//
//  // update the shader view and position to center at (400, 300)
//  view_shader.update_shader_view_and_position(
//      sf::Vector2f(400, 300),
//      ViewFragmentShader::Align::CENTER);
//
//  // Draw something sf::Drawable to the shaders render texture to be post-processed.
//  my_drawable_thing.draw(view_shader.get_render_texture(), sf::RenderStates::Default);
//
//  // finally draw the shader effect to the main window
//  window.draw(view_shader);

/**
 * @class ViewFragmentShader
 * @brief A fragment shader implementation that provides view-based rendering capabilities.
 *
 * ViewFragmentShader extends BaseFragmentShader to add view management functionality,
 * allowing for dynamic positioning and alignment of shader effects within a texture.
 * This class is particularly useful for rendering effects that need to be positioned
 * relative to a specific view or camera in a 2D graphics application.
 *
 * The class maintains an internal sf::View object that can be updated with different
 * positions and alignment modes, providing flexibility in how shader effects are
 * applied to the render texture.
 *
 * Key features:
 * - View-based positioning with CENTER and TOPLEFT alignment options
 * - Automatic coordinate conversion between alignment modes
 * - Access to elapsed time for time-based shader effects
 * - Integration with SFML's view system for consistent rendering
 *
 * @note ViewFragmentShader manages its own internal view.
 * Therefore, your shader should follow these rules:
 * 1. Always center at (0.5, 0.5) in texture coordinate space
 * 2. Convert from world units to texture coordinate units using a "screen size" uniform
 * 3. Work entirely in texture space rather than trying to use world coordinates
 * @see BaseFragmentShader
 */
class ViewFragmentShader : public BaseFragmentShader
{
public:
  ViewFragmentShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
      : BaseFragmentShader( shader_path, texture_size ),
        m_shader_view( sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ texture_size } )
  {
  }
  ~ViewFragmentShader() override = default;

  auto get_render_texture() -> sf::RenderTexture & { return m_render_texture; }
  void pre_setup_texture() override {}
  void post_setup_shader() override { SPDLOG_INFO( "ViewFragmentShader initialized" ); }
  void update() override { /* unused */ }

  enum class Align
  {
    CENTER,
    TOPLEFT
  };

  sf::Time getElapsedTime() const { return m_clock.getElapsedTime(); }
  sf::Vector2f get_view_size() const { return m_shader_view.getSize(); }
  sf::Vector2f get_view_center() const { return m_shader_view.getCenter(); }

  /**
   * @brief Updates the shader position. Supports different sprite alignments.
   *        If your sprite is center aligned then use Align::CENTER.
   *
   * @param pos The position vector. If align is CENTER, this represents the center position;
   *            otherwise, it represents the top-left position.
   * @param align Sprite alignment. Defaults to Align::TOPLEFT.
   *              - Align::TOPLEFT: pos is treated as top-left corner position
   *              - Align::CENTER: pos is treated as center position and converted to top-left
   */
  void update_shader_position( sf::Vector2f pos, Align align = Align::TOPLEFT )
  {
    if ( align == Align::CENTER )
    {
      // convert center pos to top-left to simplify callsite calculations
      pos -= sf::Vector2f{ m_shader_view.getSize() * 0.5f };
    }

    m_shader_view.setCenter( pos + sf::Vector2f{ m_shader_view.getSize() * 0.5f } );
    BaseFragmentShader::set_texture_view( m_shader_view );
    BaseFragmentShader::set_position( pos );
  }

private:
  sf::View m_shader_view{};
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_VIEWFRAGMENT_SHADER_HPP__