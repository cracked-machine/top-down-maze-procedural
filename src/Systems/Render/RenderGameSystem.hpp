#ifndef SRC_SYSTEMS_RENDER_RENDERGAMESYSTEM_HPP__
#define SRC_SYSTEMS_RENDER_RENDERGAMESYSTEM_HPP__

#include <Components/AnimData.hpp>
#include <Components/Inventory/ScryingBall.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Sprites
{
class SpriteSheet;
class SpriteFactory;
class FloodWaterShader;
class ViewFragmentShader;
class NightStaticShader;
class MistShader;
class DarkModeShader;
class DrippingBloodShader;
} // namespace Game::Sprites

namespace Game::Sprites::Containers
{
class VertexFloor;
} // namespace Game::Sprites::Containers

namespace Game::PathFinding
{
class SpatialHashGrid;
} // namespace Game::PathFinding

namespace Game::Cmp
{
class ZOrderValue;
class Position;
} // namespace Game::Cmp

namespace Game::Cmp::Particle
{
class IParticleSprite;
}

namespace Game::Sys
{

class RenderOverlaySystem;

class RenderGameSystem : public RenderSystem
{
public:
  RenderGameSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~RenderGameSystem();

  //! @brief Entrypoint for rendering the game
  //! @param deltaTime
  //! @param render_overlay_sys anything that is not part of the game world itself. i.e. UI, debug
  //! info, etc..
  //! @param render_player_sys anything that walks about in the game world, i.e. player, NPCs, etc..
  //! as well as death animations/effects
void render_game( sf::Time dt, RenderOverlaySystem &render_overlay_sys, PathFinding::SpatialHashGridSharedPtr npc_navmesh );

  //! @brief Refreshes the Z-order rendering queue
  void refresh_z_order_queue();

  //! @brief This should be called by scene "on enter" functions.
  void init_world_view();

  //! @brief Update the camera center on the player using lerp for smooth transform.
  //! @param deltaTime
  void update_camera( sf::Time deltaTime );

private:
  sf::Vector2f m_camera_position{ 0.f, 0.f }; // Smoothed camera position
  bool m_camera_initialized{ false };

  //! @brief Renders the armed obstacles in the game world

  //! @brief Used by CryptScene for Priest NPC weapon
  //! @param floormap
  void render_shockwaves();

  //! @brief Used by GraveyardScene when player has key and relic carryitems
  void render_arrow_compass();

  //! @brief Used by GraveyardScene when player places a seeing stone
  void render_seeingstone_doglegs( const Cmp::SeeingStone &stone_cmp, const Cmp::Position &pos_cmp );

  //! @brief Used by GraveyardScene when player is struck by lightning
  void render_lightning_strike();

  void render_obstacle_cracks();

  //! @brief Flashes the screen
  //! @param color
  void render_screen_flash( sf::Color color );

  //! @brief Adds visible entities of a specific component type to the Z-order queue
  //! Optimized (single-type view) query on entt components for visibility check and Z-order queue
  //! addition
  //! @note The Component-owning entity must also have a Cmp::ZOrderValue component to be added to
  //! the queue
  //! @tparam Component The component type to check for visibility
  //! @param zorder_queue The Z-order queue to add visible entities to
  //! @param view_bounds The view bounds to check against
  template <typename Component>
  void add_visible_entity_to_z_order_queue( std::vector<ZOrder> &zorder_queue, sf::FloatRect view_bounds )
  {
    for ( auto [entity, component] : reg().view<Component>().each() )
    {
      if constexpr ( std::is_base_of_v<sf::FloatRect, Component> )
      {
        if ( not Utils::is_visible_in_view( view_bounds, component ) ) continue;
      }
      if constexpr ( std::is_same_v<Sys::ParticleSpriteOwner, Component> )
      {
        if ( component.sprite && component.sprite->get_view_type() == Cmp::Particle::ViewType::WORLD &&
             not Utils::is_visible_in_view( view_bounds, { component.sprite->get_emitter_position(), Constants::kGridSizePxF } ) )
          continue;
      }

      auto z_order_cmp = reg().try_get<Cmp::ZOrderValue>( entity );
      if ( z_order_cmp ) { zorder_queue.push_back( ZOrder{ z_order_cmp->getZOrder(), entity } ); }
    }
  }

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  //! @brief Delay for updating the debug UI
  const sf::Time m_debug_update_interval{ sf::milliseconds( 500 ) };
  //! @brief The timer for managing the debug UI update delays
  sf::Clock m_debug_update_timer;

  //! @brief Time component of the sine wave for the compass arrow bouncing movement
  sf::Clock m_compass_osc_clock;
  //! @brief Frequency component of the sine wave for the compass arrow bouncing movement
  float m_compass_freq{ 4.0f };
  //! @brief Min range for the compass arrow dynamic resize
  float m_compass_min_scale{ 0.5f };
  //! @brief Max range for the compass arrow dynamic resize
  float m_compass_max_scale{ 1.5f };

  //! @brief The z-order queue for rendering
  //! Each frame, this queue is refreshed to ensure correct rendering order
  std::vector<ZOrder> m_zorder_queue_;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_RENDER_RENDERGAMESYSTEM_HPP__
