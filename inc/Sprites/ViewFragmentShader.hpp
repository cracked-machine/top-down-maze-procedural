#ifndef __SPRITES_VIEWFRAGMENTSHADER_HPP__
#define __SPRITES_VIEWFRAGMENTSHADER_HPP__

#include <Random.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/BaseFragmentShader.hpp>
#include <Systems/BaseSystem.hpp>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sprites {

class ViewFragmentShader : public BaseFragmentShader
{
public:
  ViewFragmentShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
      : BaseFragmentShader( shader_path, texture_size ),
        m_shader_view( sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ texture_size } )
  {
    setup();
    SPDLOG_INFO( "ViewFragmentShader initialized" );
  }
  ~ViewFragmentShader() override = default;

  auto get_render_texture() -> sf::RenderTexture & { return m_render_texture; }
  void pre_setup_texture() override {}
  void post_setup_shader() override {}
  void update() override { /* unused */ }

  enum class Align
  {
    CENTER,
    TOPLEFT
  };

  sf::Time getElapsedTime() const { return m_clock.getElapsedTime(); }
  sf::Vector2f get_view_size() const { return m_shader_view.getSize(); }

  /**
   * @brief Updates the shader view and position based on the provided position and alignment.
   *
   * This function sets the shader view's center and position based on the input parameters.
   * If the alignment is CENTER, the position is converted from center coordinates to top-left
   * coordinates for consistency with internal calculations.
   *
   * @param pos The position vector. If align is CENTER, this represents the center position;
   *            otherwise, it represents the top-left position.
   * @param align The alignment mode for position interpretation. Defaults to Align::TOPLEFT.
   *              - Align::TOPLEFT: pos is treated as top-left corner position
   *              - Align::CENTER: pos is treated as center position and converted to top-left
   */
  void update_shader_view_and_position( sf::Vector2f pos, Align align = Align::TOPLEFT )
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