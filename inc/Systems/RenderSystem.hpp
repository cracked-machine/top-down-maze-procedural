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

#include <exception>
#include <imgui.h>
#include <memory>
#include <sstream>

#include <imgui-SFML.h>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class RenderSystem : public BaseSystem
{
public:
  RenderSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg ) : BaseSystem( reg )
  {

    SPDLOG_INFO( "RenderSystem initialisation starting..." );

    if ( not ImGui::SFML::Init( getWindow() ) )
    {
      SPDLOG_CRITICAL( "ImGui-SFML initialization failed" );
      std::get_terminate();
    }
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f;
    io.IniFilename = "res/imgui.ini"; // store settings in the res folder
    std::ignore = ImGui::SFML::UpdateFontTexture();

    SPDLOG_INFO( "RenderSystem initialisation finished" );
  }

  ~RenderSystem() { SPDLOG_DEBUG( "~RenderSystem()" ); }

  // External access to the window
  sf::RenderWindow &window() { return getWindow(); }

protected:
  // Derived class accessor for the static window instance
  static sf::RenderWindow &getWindow()
  {
    if ( !m_window )
    {
      m_window =
          std::make_unique<sf::RenderWindow>( sf::VideoMode( DISPLAY_SIZE ), "ProceduralMaze" );
    }
    return *m_window;
  }

  // Font for rendering text
  Cmp::Font m_font = Cmp::Font( "res/tuffy.ttf" );

  bool m_show_path_distances = false;
  bool m_show_armed_obstacles = false;

private:
  // Static to prevent multiple windows being created
  // Private to prevent uninitialised access
  // Use the singleton getWindow() method to access
  static std::unique_ptr<sf::RenderWindow> m_window;
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_RENDER_SYSTEM_HPP__