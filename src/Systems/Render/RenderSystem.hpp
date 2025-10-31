#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <entt/entity/fwd.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/WindowEnums.hpp>

#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <Components/Armed.hpp>
#include <Components/Direction.hpp>
#include <Components/Font.hpp>
#include <Components/Loot.hpp>
#include <Components/NPC.hpp>
#include <Components/NPCScanBounds.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Components/WaterLevel.hpp>
#include <Shaders/FloodWaterShader.hpp>
#include <Sprites/BasicSprite.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PathFindSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/Threats/FloodSystem.hpp>

namespace ProceduralMaze::Sys {

class RenderSystem : public BaseSystem
{
public:
  //! @brief Construct a new Render System object
  //! @param reg
  RenderSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory );

  //! @brief Destroy the Render System object
  virtual ~RenderSystem() = default;

  //! @brief Initialize multi-sprites
  void init_multisprites();

  //! @brief Accessor for the static game view
  //! @return const sf::View&
  static const sf::View &getGameView() { return s_game_view; }

protected:
  //! @brief Text alignment options
  enum class Alignment
  {
    //! @brief Left align text (respects position.x)
    LEFT,
    //! @brief Center align text (ignores position.x)
    CENTER
  };

  //! @brief Default font for rendering text
  Cmp::Font m_font = Cmp::Font( "res/fonts/tuffy.ttf" );

  // System mode flags
  bool m_show_path_distances{ false };
  bool m_show_armed_obstacles{ false };
  bool m_minimap_enabled{ false };
  bool m_show_debug_stats{ false };

  //! @brief Common window options for ImGui windows
  const int kImGuiWindowOptions = ImGuiWindowFlags_NoTitleBar
      // | ImGuiWindowFlags_NoResize
      // | ImGuiWindowFlags_NoMove
      ;

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
  void render_text( std::string text, unsigned int size, sf::Vector2f position, Alignment align, float padding = 10.f,
                    sf::Color fill_color = sf::Color::White, sf::Color outline_color = sf::Color::Transparent );

  // Variant that renders to a specific render target (shader, texture, etc.)
  void safe_render_sprite_to_target( sf::RenderTarget &target, const std::string &sprite_type, const sf::FloatRect &pos_cmp,
                                     int sprite_index = 0, sf::Vector2f scale = { 1.f, 1.f }, uint8_t alpha = 255,
                                     sf::Vector2f origin = { 0.f, 0.f }, sf::Angle angle = sf::degrees( 0.f ) );

  // Fallback rendering for missing sprites (also target-aware)
  void render_fallback_square_to_target( sf::RenderTarget &target, const sf::FloatRect &pos_cmp,
                                         const sf::Color &color = sf::Color::Magenta );

  // Safe sprite accessor that renders a fallback square if sprite is missing
  void safe_render_sprite( const std::string &sprite_type, const sf::FloatRect &position, int sprite_index = 0,
                           sf::Vector2f scale = { 1.f, 1.f }, uint8_t alpha = 255, sf::Vector2f origin = { 0.f, 0.f },
                           sf::Angle angle = sf::degrees( 0.f ) );

  // Fallback rendering for missing sprites
  void render_fallback_square( const sf::FloatRect &pos_cmp, const sf::Color &color = sf::Color::Magenta );

  // Shared multisprite map
  static std::unordered_map<Sprites::SpriteMetaType, std::reference_wrapper<Sprites::MultiSprite>> m_multisprite_map;

  static sf::View s_game_view;
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__