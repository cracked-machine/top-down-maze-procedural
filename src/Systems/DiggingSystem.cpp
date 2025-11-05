#include <Components/PlayableCharacter.hpp>
#include <Components/WeaponLevel.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Time.hpp>

#include <spdlog/spdlog.h>

#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

DiggingSystem::DiggingSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                              Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // register the event handler
  std::ignore = getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&DiggingSystem::on_player_action>(
      this );
  SPDLOG_DEBUG( "DiggingSystem initialized" );
}

// void DiggingSystem::load_sounds()
// {
//   // load pool of pickaxe sound effects
//   for ( auto &pickaxe_sound : m_pickaxe_sounds )
//   {
//     if ( not pickaxe_sound.buffer.loadFromFile( pickaxe_sound.path ) )
//     {
//       SPDLOG_ERROR( "Failed to load dig sound effect: {}", pickaxe_sound.path.string() );
//       std::terminate();
//     }
//   }
//   // load final smash sound effect
//   if ( not m_pickaxe_final_sound.buffer.loadFromFile( m_pickaxe_final_sound.path ) )
//   {
//     SPDLOG_ERROR( "Failed to load smash sound effect: {}", m_pickaxe_final_sound.path.string() );
//     std::terminate();
//   }
//   SPDLOG_INFO( "DiggingSystem sounds initialized" );
// }

void DiggingSystem::update()
{
  // abort if still in cooldown
  auto digging_cooldown_amount = get_persistent_component<Cmp::Persistent::DiggingCooldownThreshold>().get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) )
  {
    SPDLOG_DEBUG( "Digging is on cooldown for {} more seconds!",
                  ( digging_cooldown_amount - m_dig_cooldown_clock.getElapsedTime().asSeconds() ) );
    return;
  }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = m_reg->view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    m_reg->remove<Cmp::SelectedPosition>( existing_sel_entity );
    SPDLOG_DEBUG( "Removing previous Cmp::SelectedPosition {},{} from entity {}", sel_cmp.x, sel_cmp.y,
                  static_cast<int>( existing_sel_entity ) );
  }
}

void DiggingSystem::check_player_dig_obstacle_collision()
{
  // abort if still in cooldown
  auto digging_cooldown_amount = get_persistent_component<Cmp::Persistent::DiggingCooldownThreshold>().get_value();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) ) { return; }

  // Cooldown has expired: Remove any existing SelectedPosition components from the registry
  auto selected_position_view = m_reg->view<Cmp::SelectedPosition>();
  for ( auto [existing_sel_entity, sel_cmp] : selected_position_view.each() )
  {
    m_reg->remove<Cmp::SelectedPosition>( existing_sel_entity );
  }

  // Iterate through all entities with Position and Obstacle components
  auto position_view = m_reg->view<Cmp::Position, Cmp::Obstacle>(
      entt::exclude<Cmp::ReservedPosition, Cmp::SelectedPosition> );
  for ( auto [entity, pos_cmp, obst_cmp] : position_view.each() )
  {
    // skip positions with non diggable obstacles
    if ( not obst_cmp.m_type.contains( "ROCK" ) or not obst_cmp.m_enabled ) continue;

    // Remap the mouse position to game view coordinates (a subset of the actual game area)
    sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
    sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::getGameView() );

    // Check if the mouse position intersects with the entity's position
    auto mouse_position_bounds = sf::FloatRect( mouse_world_pos, sf::Vector2f( 2.f, 2.f ) );
    if ( mouse_position_bounds.findIntersection( pos_cmp ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", pos_cmp.position.x, pos_cmp.position.y );

      // TODO: check player is facing the obstacle
      // Check player proximity to the entity
      auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
      bool player_nearby = false;
      for ( auto [pc_entt, pc_cmp, pc_pos_cmp] : player_view.each() )
      {
        auto half_sprite_size = kGridSquareSizePixelsF;
        auto player_horizontal_bounds = Cmp::RectBounds( pc_pos_cmp.position, half_sprite_size, 1.5f,
                                                         Cmp::RectBounds::ScaleCardinality::HORIZONTAL );
        auto player_vertical_bounds = Cmp::RectBounds( pc_pos_cmp.position, half_sprite_size, 1.5f,
                                                       Cmp::RectBounds::ScaleCardinality::VERTICAL );
        if ( player_horizontal_bounds.findIntersection( pos_cmp ) ||
             player_vertical_bounds.findIntersection( pos_cmp ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby )
      {
        SPDLOG_DEBUG( " Player not close enough to dig at position ({}, {})!", pos_cmp.position.x, pos_cmp.position.y );
        continue;
      }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      m_reg->emplace_or_replace<Cmp::SelectedPosition>( entity, pos_cmp.position );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();
      obst_cmp.m_integrity -= get_persistent_component<Cmp::Persistent::DiggingDamagePerHit>().get_value();
      auto player_weapons_view = m_reg->view<Cmp::WeaponLevel, Cmp::PlayableCharacter>();
      for ( auto [weapons_entity, weapons_level, pc_cmp] : player_weapons_view.each() )
      {
        // Decrease weapons level based on damage dealt
        weapons_level.m_level -= get_persistent_component<Cmp::Persistent::WeaponDegradePerHit>().get_value();
        SPDLOG_INFO( "Player weapons level decreased to {} after digging!", weapons_level.m_level );
      }

      if ( obst_cmp.m_integrity <= 0.0f )
      {
        // select the final smash sound
        m_sound_bank.get_effect( "pickaxe_final" ).play();
        obst_cmp.m_enabled = false;
        SPDLOG_DEBUG( "Digged through obstacle at position ({}, {})!", pos_cmp.x, pos_cmp.y );
      }
      else
      {
        // select all pickaxe sounds except the final smash sound
        Cmp::RandomInt random_picker( 1, 6 );
        m_sound_bank.get_effect( "pickaxe" + std::to_string( random_picker.gen() ) ).play();

        SPDLOG_DEBUG( "Digged obstacle at position ({}, {}), remaining integrity: {}!", pos_cmp.x, pos_cmp.y,
                      obst_cmp.m_integrity );
      }
    }
  }
}

void DiggingSystem::on_player_action( const Events::PlayerActionEvent &event )
{
  if ( event.action == Events::PlayerActionEvent::GameActions::DIG )
  {
    // Check for collisions with diggable obstacles
    check_player_dig_obstacle_collision();
  }
}

} // namespace ProceduralMaze::Sys