#include <DiggingSystem.hpp>
#include <Persistent/DiggingCooldownAmount.hpp>
#include <RenderSystem.hpp>
#include <ReservedPosition.hpp>
#include <SFML/System/Time.hpp>
#include <SelectedPosition.hpp>

namespace ProceduralMaze::Sys {

DiggingSystem::DiggingSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  // register the event sinks
  std::ignore = getEventDispatcher().sink<Events::PlayerActionEvent>().connect<&DiggingSystem::on_player_action>(
      this );
}

void DiggingSystem::check_player_dig_obstacle_collision()
{
  // abort if still in cooldown
  auto digging_cooldown_amount = get_persistent_component<Cmp::Persistent::DiggingCooldownAmount>()();
  if ( m_dig_cooldown_clock.getElapsedTime() < sf::seconds( digging_cooldown_amount ) )
  {
    SPDLOG_INFO( "Digging is on cooldown for {} more seconds!",
                 ( digging_cooldown_amount - m_dig_cooldown_clock.getElapsedTime().asSeconds() ) );
    return;
  }
  // Remove all SelectedPosition components from the registry
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
      if ( not player_nearby )
      {
        SPDLOG_DEBUG( " Player not close enough to dig at position ({}, {})!", pos_cmp.x, pos_cmp.y );
        continue;
      }
      // We are in proximity to an entity that is a candidate for a new SelectedPosition component.

      // then add a new SelectedPosition component to the entity
      m_reg->emplace_or_replace<Cmp::SelectedPosition>( entity, pos_cmp );
      m_dig_cooldown_clock.restart();
      obst_cmp.m_integrity -= 0.1f;
      if ( obst_cmp.m_integrity <= 0.0f )
      {
        obst_cmp.m_enabled = false;
        SPDLOG_INFO( "Digged through obstacle at position ({}, {})!", pos_cmp.x, pos_cmp.y );
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