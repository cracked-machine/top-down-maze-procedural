#ifndef __SYSTEMS_RENDER_SYSTEM_HPP__
#define __SYSTEMS_RENDER_SYSTEM_HPP__

#include <Components/Armed.hpp>
#include <Components/Direction.hpp>
#include <Components/Font.hpp>
#include <Components/Loot.hpp>
#include <Components/NPC.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Components/WaterLevel.hpp>
#include <FloodSystem.hpp>
#include <NPCScanBounds.hpp>
#include <PCDetectionBounds.hpp>
#include <PlayerSystem.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <Sprites/BasicSprite.hpp>
#include <Sprites/FloodWaterShader.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PathFindSystem.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <imgui-SFML.h>

#include <entt/entity/fwd.hpp>

#include <imgui.h>
#include <memory>

#include <imgui-SFML.h>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class RenderSystem : public BaseSystem
{
public:
  RenderSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg );
  virtual ~RenderSystem() = default;

  // External access to the window
  sf::RenderWindow &window() { return getWindow(); }

protected:
  // Font for rendering text
  Cmp::Font m_font = Cmp::Font( "res/fonts/tuffy.ttf" );

  bool m_show_path_distances = false;
  bool m_show_armed_obstacles = false;

  // common window options for ImGui windows
  const int kImGuiWindowOptions = ImGuiWindowFlags_NoTitleBar
      // | ImGuiWindowFlags_NoResize
      // | ImGuiWindowFlags_NoMove
      ;

  // Text alignment options
  enum class Alignment
  {
    LEFT,
    CENTER
  };

  /**
   * @brief Renders text to the screen with specified formatting and alignment options.
   *
   * @param text The string content to be rendered
   * @param size The font size for the text in pixels
   * @param position The screen coordinates where the text should be positioned
   * @param align The alignment mode for the text (left or center). Left will respect position.x,
   * center will ignore it.
   * @param padding Optional spacing around the text in pixels (default: 10.0f)
   * @param fill_color Optional color for the text fill (default: White)
   * @param outline_color Optional color for the text outline (default: Transparent). Outline
   * thickness is 0.f if set to Transparent.
   */
  void render_text( std::string text,
                    unsigned int size,
                    sf::Vector2f position,
                    Alignment align,
                    float padding = 10.f,
                    sf::Color fill_color = sf::Color::White,
                    sf::Color outline_color = sf::Color::Transparent );

  // Derived class accessor for the static window instance
  static sf::RenderWindow &getWindow()
  {
    if ( !RenderSystem::m_window )
    {
      RenderSystem::m_window = std::make_unique<sf::RenderWindow>(
          sf::VideoMode( kDisplaySize ), "ProceduralMaze", sf::State::Fullscreen );
    }
    return *RenderSystem::m_window;
  }

private:
  // Static to prevent multiple windows being created
  // Private to prevent uninitialised access
  // Use the singleton getWindow() method to access
  static std::unique_ptr<sf::RenderWindow> m_window;
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__