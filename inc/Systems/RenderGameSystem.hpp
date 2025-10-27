#ifndef __SYS_RENDERGAMESYSTEM_HPP__
#define __SYS_RENDERGAMESYSTEM_HPP__

#include <BackgroundShader.hpp>
#include <FootstepSystem.hpp>
#include <HazardFieldSystem.hpp>
#include <MultiSprite.hpp>
#include <RenderOverlaySystem.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <Systems/RenderSystem.hpp>
#include <ViewFragmentShader.hpp>

namespace ProceduralMaze::Sys {

class RenderGameSystem : public RenderSystem
{
public:
  RenderGameSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window );
  ~RenderGameSystem() = default;

  // Entry point for class
  void init_views();
  // Add this method to share the sprite map with overlay system
  void init_shaders();
  void init_tilemap();
  void render_game( sf::Time deltaTime, RenderOverlaySystem &overlay_sys );

private:
  void render_floormap( const sf::Vector2f &offset = { 0.f, 0.f } );
  void render_npc_containers();
  void render_loot_containers();
  void render_small_obstacles();
  void render_large_obstacles();
  void render_sinkhole();
  void render_corruption();
  void render_wormhole();
  void render_armed();
  void render_loot();
  void render_walls();
  void render_player_spawn();
  void render_player();
  void render_player_footsteps();
  void render_npc();
  void render_explosions();
  void render_flood_waters();
  void render_player_distances_on_npc();
  void render_player_distances_on_obstacles();
  void render_npc_distances_on_obstacles();
  void render_positions();

  void update_view_center( sf::View &view, const Cmp::Position &player_pos, float smoothFactor = 0.1f );

  // Views
  const sf::Vector2f kLocalMapViewSize{ 300.f, 200.f };
  const float kMiniMapViewZoomFactor = 0.25f;

  sf::View m_local_view;
  sf::View m_minimap_view;
  const float kStartGameSmoothFactor = 1.f; // instant centering on start

  // Shaders
  Sprites::FloodWaterShader m_water_shader{ "res/shaders/FloodWater2.glsl", kDisplaySize };

  Sprites::ViewFragmentShader m_wormhole_shader{ "res/shaders/SimpleDistortionField.frag",
                                                 BaseSystem::kGridSquareSizePixels.componentWiseMul( { 3u, 3u } ) };

  // Sprites
  Sprites::Containers::TileMap m_floormap{};

  // restrict the path tracking data update to every 0.1 seconds (optimization)
  const sf::Time m_debug_update_interval{ sf::milliseconds( 10 ) };
  sf::Clock m_debug_update_timer;
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERGAMESYSTEM_HPP__