#include <DiggingSystem.hpp>
#include <Persistent/DiggingCooldownThreshold.hpp>
#include <Persistent/DiggingDamagePerHit .hpp>
#include <RenderSystem.hpp>
#include <ReservedPosition.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/System/Time.hpp>
#include <SelectedPosition.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

DiggingSystem::DiggingSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  // register the event handler
  std::ignore = getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&DiggingSystem::on_player_action>(
      this );

  // load pool of pickaxe sound effects
  for ( auto &pickaxe_sound : m_pickaxe_sounds )
  {
    if ( not pickaxe_sound.buffer.loadFromFile( pickaxe_sound.path ) )
    {
      SPDLOG_ERROR( "Failed to load dig sound effect: {}", pickaxe_sound.path.string() );
      std::terminate();
    }
  }
  // load final smash sound effect
  if ( not m_pickaxe_final_sound.buffer.loadFromFile( m_pickaxe_final_sound.path ) )
  {
    SPDLOG_ERROR( "Failed to load smash sound effect: {}", m_pickaxe_final_sound.path.string() );
    std::terminate();
  }
}

void DiggingSystem::update()
{

  // abort if still in cooldown
  auto digging_cooldown_amount = get_persistent_component<Cmp::Persistent::DiggingCooldownThreshold>()();
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
  auto digging_cooldown_amount = get_persistent_component<Cmp::Persistent::DiggingCooldownThreshold>()();
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

  auto position_view = m_reg->view<Cmp::Position, Cmp::Obstacle>(
      entt::exclude<Cmp::ReservedPosition, Cmp::SelectedPosition> );

  // Iterate through all entities with Position and Obstacle components
  for ( auto [entity, pos_cmp, obst_cmp] : position_view.each() )
  {
    // skip positions with non diggable obstacles
    if ( not obst_cmp.m_type.contains( "ROCK" ) or not obst_cmp.m_enabled ) continue;

    // Remap the mouse position to game view coordinates (a subset of the actual game area)
    sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( RenderSystem::getWindow() );
    sf::Vector2f mouse_world_pos = RenderSystem::getWindow().mapPixelToCoords( mouse_pixel_pos,
                                                                               RenderSystem::getGameView() );
    // Check if the mouse position intersects with the entity's position
    auto mouse_position_bounds = sf::FloatRect( mouse_world_pos, sf::Vector2f( 2.f, 2.f ) );
    auto pos_cmp_bounds = get_hitbox( pos_cmp );
    if ( mouse_position_bounds.findIntersection( pos_cmp_bounds ) )
    {
      SPDLOG_DEBUG( "Found diggable entity at position: [{}, {}]!", pos_cmp.x, pos_cmp.y );

      // Check player proximity to the entity
      auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
      bool player_nearby = false;
      for ( auto [_pc_entt, _pc_cmp, _pc_pos_cmp] : player_view.each() )
      {
        auto player_hitbox = Cmp::RectBounds( _pc_pos_cmp,
                                              sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions }, 1.5f );
        if ( player_hitbox.findIntersection( pos_cmp_bounds ) )
        {
          player_nearby = true;
          break;
        }
      }

      // skip this iteration of the loop if player too far away
      if ( not player_nearby )
      {
        SPDLOG_DEBUG( " Player not close enough to dig at position ({}, {})!", pos_cmp.x, pos_cmp.y );
        continue;
      }

      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.
      // Add a new SelectedPosition component to the entity
      m_reg->emplace_or_replace<Cmp::SelectedPosition>( entity, pos_cmp );

      // Apply digging damage, play a sound depending on whether the obstacle was destroyed
      m_dig_cooldown_clock.restart();
      obst_cmp.m_integrity -= get_persistent_component<Cmp::Persistent::DiggingDamagePerHit>()();
      if ( obst_cmp.m_integrity <= 0.0f )
      {
        // select the final smash sound
        m_dig_sound.setBuffer( m_pickaxe_final_sound.buffer );
        obst_cmp.m_enabled = false;
        SPDLOG_DEBUG( "Digged through obstacle at position ({}, {})!", pos_cmp.x, pos_cmp.y );
      }
      else
      {
        // select a random pickaxe sound
        Cmp::Random random_picker( 0, m_pickaxe_sounds.size() - 1 );
        auto selected_pickaxe_sound_index = random_picker.gen();
        SPDLOG_INFO( "Random pickaxe sound index: {}", selected_pickaxe_sound_index );
        m_dig_sound.setBuffer( m_pickaxe_sounds[selected_pickaxe_sound_index].buffer );
        SPDLOG_INFO( "Playing pickaxe sound: {}", m_pickaxe_sounds[selected_pickaxe_sound_index].path.string() );
        SPDLOG_DEBUG( "Digged obstacle at position ({}, {}), remaining integrity: {}!", pos_cmp.x, pos_cmp.y,
                      obst_cmp.m_integrity );
      }
      m_dig_sound.play();
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