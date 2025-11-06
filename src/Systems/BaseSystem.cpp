#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/RectBounds.hpp>
#include <entt/entity/registry.hpp>

#include <Components/Door.hpp>
#include <Components/Exit.hpp>
#include <Components/GraveSprite.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/ShrineSprite.hpp>
#include <Components/Wall.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys {

BaseSystem::BaseSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                        Audio::SoundBank &sound_bank )
    : m_reg( reg ),
      m_window( window ),
      m_sprite_factory( sprite_factory ),
      m_sound_bank( sound_bank )
{
  SPDLOG_DEBUG( "BaseSystem constructor called" );
}

const sf::Vector2u BaseSystem::kDisplaySize{ 1920, 1024 };

const sf::Vector2u BaseSystem::kMapGridSize{ 100u, 124u };

const sf::Vector2f BaseSystem::kMapGridOffset{ 1.f, 1.f };

const sf::Vector2u BaseSystem::kGridSquareSizePixels{ 16u, 16u };

// initialised by first call to getEventDispatcher()
std::unique_ptr<entt::dispatcher> BaseSystem::m_event_dispatcher = nullptr;

bool BaseSystem::is_valid_move( const sf::FloatRect &target_position )
{

  // Prevent the player from walking through NPCs
  auto &pc_damage_delay = get_persistent_component<Cmp::Persistent::PcDamageDelay>();
  auto npc_view = m_reg->view<Cmp::NPC, Cmp::Position, Cmp::LerpPosition>();
  auto pc_view = m_reg->view<Cmp::PlayableCharacter>();
  for ( auto [pc_entity, pc_cmp] : pc_view.each() )
  {
    // However if player is in damage cooldown (blinking), let player walk through NPCs to escape
    if ( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_delay.get_value() ) continue;
    for ( auto [entity, npc_cmp, pos_cmp, lerp_pos_cmp] : npc_view.each() )
    {
      // relaxed bounds to allow player to sneak past during lerp transition
      Cmp::RectBounds npc_pos_cmp_bounds_current{ pos_cmp.position, pos_cmp.size, 0.1f };
      if ( target_position.findIntersection( npc_pos_cmp_bounds_current.getBounds() ) ) { return false; }
    }
  }
  // Check obstacles
  auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
  for ( auto [entity, obs_cmp, pos_cmp] : obstacle_view.each() )
  {
    if ( obs_cmp.m_enabled == false || obs_cmp.m_integrity <= 0.0f ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  // Check walls
  auto wall_view = m_reg->view<Cmp::Wall, Cmp::Position>();
  for ( auto [entity, wall_cmp, pos_cmp] : wall_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  // Check doors
  auto door_view = m_reg->view<Cmp::Door, Cmp::Position>();
  for ( auto [entity, door_cmp, pos_cmp] : door_view.each() )
  {
    auto exit_cmp = m_reg->try_get<Cmp::Exit>( entity );
    // if door is an exit and is unlocked, allow passage
    if ( exit_cmp && exit_cmp->m_locked == false ) return true;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto grave_view = m_reg->view<Cmp::GraveSprite, Cmp::Position>();
  for ( auto [entity, grave_cmp, pos_cmp] : grave_view.each() )
  {
    if ( not grave_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto shrine_view = m_reg->view<Cmp::ShrineSprite, Cmp::Position>();
  for ( auto [entity, shrine_cmp, pos_cmp] : shrine_view.each() )
  {
    if ( not shrine_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  return true;
}

bool BaseSystem::isDiagonalMovementBetweenObstacles( const sf::FloatRect &current_pos, const sf::Vector2f &direction )
{
  if ( !( ( direction.x != 0.0f ) && ( direction.y != 0.0f ) ) ) return false; // Not diagonal

  sf::Vector2f grid_size{ BaseSystem::kGridSquareSizePixels };

  // Calculate the two orthogonal positions the diagonal movement would "cut through"
  sf::FloatRect horizontal_check = sf::FloatRect{
      sf::Vector2f{ current_pos.position.x + ( direction.x * grid_size.x ), current_pos.position.y }, grid_size };

  sf::FloatRect vertical_check = sf::FloatRect{
      sf::Vector2f{ current_pos.position.x, current_pos.position.y + ( direction.y * grid_size.y ) }, grid_size };

  // Check if both orthogonal positions have obstacles
  bool horizontal_blocked = !is_valid_move( horizontal_check );
  if ( !horizontal_blocked ) return false;
  bool vertical_blocked = !is_valid_move( vertical_check );
  if ( !vertical_blocked ) return false;

  // Both orthogonal paths are blocked, diagonal movement is between obstacles
  return true;
}

sf::FloatRect BaseSystem::calculate_view_bounds( const sf::View &view ) const
{
  // Calculate view bounds in world coordinates
  return sf::FloatRect( view.getCenter() - view.getSize() / 2.f, view.getSize() );
}

bool BaseSystem::is_visible_in_view( const sf::FloatRect &viewbounds, const sf::FloatRect &position ) const
{
  // Check for intersection
  return viewbounds.findIntersection( position ).has_value();
}

bool BaseSystem::is_visible_in_view( const sf::View &view, const sf::FloatRect &position ) const
{
  auto viewBounds = calculate_view_bounds( view );

  // Check for intersection
  return viewBounds.findIntersection( position ).has_value();
}

} // namespace ProceduralMaze::Sys