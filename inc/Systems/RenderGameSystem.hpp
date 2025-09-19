#ifndef __SYS_RENDERGAMESYSTEM_HPP__
#define __SYS_RENDERGAMESYSTEM_HPP__

#include <BackgroundShader.hpp>
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
  RenderGameSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg );
  ~RenderGameSystem() = default;

  // Entry point for class
  void render_game( sf::Time deltaTime );
  void init_views();
  void load_multisprites();

private:
  void render_floormap( const sf::Vector2f &offset = { 0.f, 0.f } );
  void render_obstacles();
  void render_armed();
  void render_loot();
  void render_walls();
  void render_player();
  void render_npc();
  void render_explosions( sf::Time deltaTime );
  void render_flood_waters();
  void render_player_distances_on_npc();
  void render_player_distances_on_obstacles();
  void render_npc_distances_on_obstacles();

  void
  update_view_center( sf::View &view, const Cmp::Position &player_pos, float smoothFactor = 0.1f );

  RenderOverlaySystem m_overlay_sys{ m_reg };

  // Views
  const sf::Vector2f kLocalMapViewSize{ 300.f, 200.f };
  const float kMiniMapViewZoomFactor = 0.25f;
  // const sf::Vector2f MINI_MAP_VIEW_SIZE{ kDisplaySize.x * 0.25f, kDisplaySize.y * 0.25f };
  sf::View m_local_view;
  sf::View m_minimap_view;
  const float kStartGameSmoothFactor = 1.f; // instant centering on start

  // Shaders
  Sprites::FloodWaterShader m_water_shader{ "res/shaders/FloodWater2.glsl", kDisplaySize };

  Sprites::ViewFragmentShader m_sand_storm_shader{ "res/shaders/CellularSand.frag", kDisplaySize };

  // Sprites
  Sprites::Containers::TileMap m_floormap;

  std::optional<Sprites::MultiSprite> m_rock_ms;
  std::optional<Sprites::MultiSprite> m_pot_ms;
  std::optional<Sprites::MultiSprite> m_bone_ms;
  std::optional<Sprites::MultiSprite> m_detonation_ms;

  std::optional<Sprites::MultiSprite> m_bomb_ms;
  std::optional<Sprites::MultiSprite> m_wall_ms;
  std::optional<Sprites::MultiSprite> m_player_ms;
  std::optional<Sprites::MultiSprite> m_npc_ms;

  std::optional<Sprites::MultiSprite> m_extra_health_ms;
  std::optional<Sprites::MultiSprite> m_extra_bombs_ms;
  std::optional<Sprites::MultiSprite> m_infinite_bombs_ms;
  std::optional<Sprites::MultiSprite> m_chain_bombs_ms;
  std::optional<Sprites::MultiSprite> m_lower_water_ms;
  std::optional<Sprites::MultiSprite> m_explosion_ms;
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERGAMESYSTEM_HPP__