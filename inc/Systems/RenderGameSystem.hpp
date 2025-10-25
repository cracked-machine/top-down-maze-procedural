#ifndef __SYS_RENDERGAMESYSTEM_HPP__
#define __SYS_RENDERGAMESYSTEM_HPP__

#include <BackgroundShader.hpp>
#include <FootstepSystem.hpp>
#include <MultiSprite.hpp>
#include <RenderOverlaySystem.hpp>
#include <SFML/System/Time.hpp>

#include <HazardFieldSystem.hpp>
#include <SFML/System/Vector2.hpp>
#include <Systems/RenderSystem.hpp>
#include <ViewFragmentShader.hpp>
#include <unordered_map>

namespace ProceduralMaze::Sys {

class RenderGameSystem : public RenderSystem
{
public:
  RenderGameSystem( ProceduralMaze::SharedEnttRegistry reg );
  ~RenderGameSystem() = default;

  // Entry point for class
  void init_views();
  void init_multisprites();
  void render_game( sf::Time deltaTime );

private:
  void render_floormap( const sf::Vector2f &offset = { 0.f, 0.f } );
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

  // Variant that renders to a specific render target (shader, texture, etc.)
  void safe_render_sprite_to_target( sf::RenderTarget &target, const std::string &sprite_type,
                                     const sf::FloatRect &pos_cmp, int sprite_index = 0,
                                     sf::Vector2f scale = { 1.f, 1.f }, uint8_t alpha = 255,
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

  void update_view_center( sf::View &view, const Cmp::Position &player_pos, float smoothFactor = 0.1f );

  RenderOverlaySystem m_overlay_sys{ m_reg };
  FootstepSystem m_footstep_sys{ m_reg };

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
  Sprites::Containers::TileMap m_floormap{ kMapGridSize };

  std::unordered_map<Sprites::SpriteMetaType, std::optional<Sprites::MultiSprite>> m_multisprite_map;
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERGAMESYSTEM_HPP__