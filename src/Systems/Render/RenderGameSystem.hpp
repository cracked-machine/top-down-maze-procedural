#ifndef __SYS_RENDERGAMESYSTEM_HPP__
#define __SYS_RENDERGAMESYSTEM_HPP__

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <Shaders/BackgroundShader.hpp>
#include <Shaders/MistShader.hpp>
#include <Shaders/PulsingShader.hpp>
#include <Shaders/ViewFragmentShader.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/Render/RenderPlayerSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>

namespace ProceduralMaze::Sys {

class RenderGameSystem : public RenderSystem
{
public:
  RenderGameSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                    Audio::SoundBank &sound_bank );
  ~RenderGameSystem() = default;

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void init_views();
  void init_shaders();
  void init_tilemap();

  //! @brief Entrypoint for rendering the game
  //!
  //! @param deltaTime
  //! @param render_overlay_sys anything that is not part of the game world itself. i.e. UI, debug info, etc..
  //! @param render_player_sys anything that walks about in the game world, i.e. player, NPCs, etc.. as well as death
  //! animations/effects
  void render_game( sf::Time globalDeltaTime, RenderOverlaySystem &render_overlay_sys, RenderPlayerSystem &render_player_sys );

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

  void render_explosions();
  void render_flood_waters( sf::FloatRect player_position );
  void render_mist( sf::FloatRect player_position );

  void render_arrow_compass();

  void update_view_center( sf::View &view, const Cmp::Position &player_pos, float smoothFactor = 0.1f );

  // Views
  const sf::Vector2f kLocalMapViewSize{ 300.f, 200.f };
  const float kMiniMapViewZoomFactor = 0.25f;

  sf::View m_local_view;
  sf::View m_minimap_view;
  const float kStartGameSmoothFactor = 1.f; // instant centering on start

  // Shaders
  Sprites::FloodWaterShader m_water_shader{ "res/shaders/FloodWater2.glsl", kDisplaySize };
  // make the view size 16x16 grid squares for the 9x9 wormhole shader effect so that it blurs the edges better
  Sprites::ViewFragmentShader m_wormhole_shader{ "res/shaders/SimpleDistortionField.frag",
                                                 BaseSystem::kGridSquareSizePixels.componentWiseMul( { 4u, 4u } ) };
  Sprites::PulsingShader m_pulsing_shader{ "res/shaders/RedPulsingSand.frag", kDisplaySize };
  Sprites::MistShader m_mist_shader{ "res/shaders/MistShader.frag", kDisplaySize };

  // Sprites
  Sprites::Containers::TileMap m_floormap{};

  // restrict the path tracking data update to every 0.1 seconds (optimization)
  const sf::Time m_debug_update_interval{ sf::milliseconds( 10 ) };
  sf::Clock m_debug_update_timer;

  float m_compass_scale{ 1.f };
  sf::Clock m_compass_osc_clock;
  float m_compass_freq{ 4.0f }; // oscillations per second
  float m_compass_min_scale{ 0.5f };
  float m_compass_max_scale{ 1.5f };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERGAMESYSTEM_HPP__