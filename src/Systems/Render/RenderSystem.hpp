#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <Persistent/DisplayResolution.hpp>
#include <Shaders/TitleScreenShader.hpp>
#include <entt/entity/fwd.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include <imgui.h>

#include <Components/Font.hpp>

#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Cmp
{
class RectBounds;
}

namespace ProceduralMaze::Sys
{

class RenderSystem : public BaseSystem
{
public:
  //! @brief Construct a new Render System object
  //! @param reg
  RenderSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief polymorphic destructor for derived classes
  virtual ~RenderSystem();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  //! @brief Get the current view of the game world. See RenderSystem::kWorldViewSize.
  //! @return const sf::View&
  static const sf::View &get_world_view() { return s_world_view; }

  //! @brief Get the screen resolution view. See Cmp::Persist::DisplayResolution.
  //! @return const sf::View&
  const sf::View &get_screen_view() { return m_window.getDefaultView(); }

protected:
  //! @brief Z-order entry for rendering queue
  struct ZOrder
  {
    float z;
    entt::entity e;
  };

  //! @brief Text alignment options
  enum class Alignment {
    //! @brief Left align text (respects position.x)
    LEFT,
    //! @brief Center align text (ignores position.x)
    CENTER
  };

  //! @brief Convert the world position to the equivalent position in the screen view
  //! @param world_pos
  //! @return sf::Vector2f
  sf::Vector2f world_to_screen( sf::Vector2f world_pos ) const { return sf::Vector2f( m_window.mapCoordsToPixel( world_pos, get_world_view() ) ); }

  //! @brief Draw in screen view coordinates. This restores the view afterwards.
  //! @param drawable
  void draw_screen( const sf::Drawable &drawable );

  //! @brief Draw in world view coordinates. This restores the view afterwards.
  //! @param drawable
  void draw_world( const sf::Drawable &drawable );

  //! @brief Current view of the game world.
  static sf::View s_world_view;

  //! @brief Dimension for `s_world_view`.
  constexpr static sf::Vector2u kWorldViewSize{ 300u, 200u };
  constexpr static sf::Vector2f kWorldViewSizeF{ static_cast<float>( kWorldViewSize.x ), static_cast<float>( kWorldViewSize.y ) };

  //! @brief Default font for rendering text
  Cmp::Font m_font = Cmp::Font( "res/fonts/tuffy.ttf" );

  // System mode flags
  bool m_show_path_finding{ false };
  bool m_show_debug_stats{ false };
  bool m_show_npcnopath{ false };
  bool m_show_playernopath{ false };
  bool m_shaders_enabled{ true };

  //! @brief Renders text to the screen with specified formatting and alignment options.
  //!
  //! @param text The string content to be rendered
  //! @param size The font size for the text in pixels
  //! @param position The screen coordinates where the text should be positioned
  //! @param align The alignment mode for the text (left or center). Left will
  //! respect position.x, center will ignore it.
  //! @param padding Optional spacing around the text in pixels (default: 10.0f)
  //! @param fill_color Optional color for the text fill (default: White)
  //! @param outline_color Optional color for the text outline (default:
  //! Transparent). Outline thickness is 0.f if set to Transparent.
  void render_text( std::string text, unsigned int size, sf::Vector2f position, Alignment align, float letter_spacing = 1.f,
                    sf::Color fill_color = sf::Color::White, sf::Color outline_color = sf::Color::Transparent );

  // Variant that renders to a specific render target (shader, texture, etc.)
  void safe_render_sprite_to_target( sf::RenderTarget &target, const std::string &sprite_type, const sf::FloatRect &pos_cmp,
                                     std::size_t sprite_index = 0, sf::Vector2f scale = { 1.f, 1.f }, uint8_t alpha = 255,
                                     sf::Vector2f origin = { 0.f, 0.f }, sf::Angle angle = sf::degrees( 0.f ) );

  // Fallback rendering for missing sprites (also target-aware)
  void render_fallback_square_to_target( sf::RenderTarget &target, const sf::FloatRect &pos_cmp, const sf::Color &color = sf::Color::Magenta );

  // Safe sprite accessor that renders a fallback square if sprite is missing
  void safe_render_sprite_screen( const std::string &sprite_type, const sf::FloatRect &position, std::size_t sprite_index = 0,
                                  sf::Vector2f scale = { 1.f, 1.f }, uint8_t alpha = 255, sf::Vector2f origin = { 0.f, 0.f },
                                  sf::Angle angle = sf::degrees( 0.f ) );

  void safe_render_sprite_world( const std::string &sprite_type, const sf::FloatRect &position, std::size_t sprite_index = 0,
                                 sf::Vector2f scale = { 1.f, 1.f }, uint8_t alpha = 255, sf::Vector2f origin = { 0.f, 0.f },
                                 sf::Angle angle = sf::degrees( 0.f ) );

  // Fallback rendering for missing sprites
  void render_fallback_square_world( const sf::FloatRect &pos_cmp, const sf::Color &color = sf::Color::Magenta );

  void render_rectbounds( Cmp::RectBounds &bounds, sf::Color color );

  //! @brief Common window options for ImGui windows
  const int kImGuiWindowOptions = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__