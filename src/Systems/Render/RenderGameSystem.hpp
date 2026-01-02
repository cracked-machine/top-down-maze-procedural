#ifndef __SYS_RENDERGAMESYSTEM_HPP__
#define __SYS_RENDERGAMESYSTEM_HPP__

#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/SpriteAnimation.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <Shaders/BackgroundShader.hpp>
#include <Shaders/DarkModeShader.hpp>
#include <Shaders/FloodWaterShader.hpp>
#include <Shaders/MistShader.hpp>
#include <Shaders/PulsingShader.hpp>
#include <Shaders/ViewFragmentShader.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>
#include <Utils/Optimizations.hpp>

namespace ProceduralMaze::Sys
{

class RenderGameSystem : public RenderSystem
{
public:
  enum class DarkMode { OFF = 0, ON = 1 };

  RenderGameSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~RenderGameSystem() = default;

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  //! @brief Refreshes the Z-order rendering queue
  void refresh_z_order_queue();

  //! @brief Initializes the views used for rendering
  void init_views();

  //! @brief Initializes the shaders used for rendering
  void init_shaders();

  //! @brief Entrypoint for rendering the game
  //! @param deltaTime
  //! @param render_overlay_sys anything that is not part of the game world itself. i.e. UI, debug
  //! info, etc..
  //! @param render_player_sys anything that walks about in the game world, i.e. player, NPCs, etc..
  //! as well as death animations/effects
  void render_game( sf::Time globalDeltaTime, RenderOverlaySystem &render_overlay_sys, Sprites::Containers::TileMap &floormap,
                    DarkMode dark_mode = DarkMode::OFF );

private:
  //! @brief Renders the game world floor
  //! @param offset The offset to apply to the floor rendering
  void render_floormap( Sprites::Containers::TileMap &floormap, const sf::Vector2f &offset = { 0.f, 0.f } );

  //! @brief Renders the armed obstacles in the game world
  void render_armed();

  void render_shockwaves();

  //! @brief Renders the flood water background effect
  //! @param player_position
  void render_background_water( sf::FloatRect player_position );

  //! @brief Renders the mist effect over the game world
  void render_mist( sf::FloatRect player_position );

  //! @brief Renders the arrow compass effect
  void render_arrow_compass();

  void render_wormhole_effect( Sprites::Containers::TileMap &floormap );

  void render_dark_mode_shader();

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
    for ( auto [entity, component] : getReg().view<Component>().each() )
    {
      if ( Utils::is_visible_in_view( view_bounds, component ) )
      {
        // check the component entity has required components: Cmp::SpriteAnimation and Cmp::Position
        auto sprite_cmp = getReg().try_get<Cmp::SpriteAnimation>( entity );
        auto pos_cmp = getReg().try_get<Cmp::Position>( entity );

        if ( sprite_cmp && pos_cmp )
        {
          auto z_order_cmp = getReg().try_get<Cmp::ZOrderValue>( entity );
          if ( z_order_cmp ) { zorder_queue.push_back( ZOrder{ z_order_cmp->getZOrder(), entity } ); }
        }
      }
    }
  }

  //! @brief This is the section of the game world that is visible to the player
  sf::View m_local_view;

  //! @brief m_local_view dimension
  const sf::Vector2u kLocalMapViewSize{ 300u, 200u };
  const sf::Vector2f kLocalMapViewSizeF{ static_cast<float>( kLocalMapViewSize.x ), static_cast<float>( kLocalMapViewSize.y ) };

  // Shaders - we dont know the size of the texture yet so set to 1,1 and resize later
  Sprites::FloodWaterShader m_water_shader{ "res/shaders/FloodWater2.glsl", { 1u, 1u } };
  Sprites::ViewFragmentShader m_wormhole_shader{ "res/shaders/SimpleDistortionField.frag",
                                                 Constants::kGridSquareSizePixels.componentWiseMul( { 3u, 3u } ) };
  Sprites::PulsingShader m_pulsing_shader{ "res/shaders/RedPulsingSand.frag", { 1u, 1u } };
  Sprites::MistShader m_mist_shader{ "res/shaders/MistShader.frag", { 1u, 1u } };
  Sprites::DarkModeShader m_dark_mode_shader{ "res/shaders/DarkMode.frag", { 1u, 1u } };

  // optimize the debug overlay updates to every n milliseconds
  const sf::Time m_debug_update_interval{ sf::milliseconds( 10 ) };
  sf::Clock m_debug_update_timer;

  // compass arrow variables
  float m_compass_scale{ 1.f };
  sf::Clock m_compass_osc_clock;
  float m_compass_freq{ 4.0f }; // oscillations per second
  float m_compass_min_scale{ 0.5f };
  float m_compass_max_scale{ 1.5f };

  //! @brief The z-order queue for rendering
  //! Each frame, this queue is refreshed to ensure correct rendering order
  std::vector<ZOrder> m_zorder_queue_;
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERGAMESYSTEM_HPP__