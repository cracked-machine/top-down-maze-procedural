#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptLever.hpp>
#include <Components/HolyWell/HolyWellSegment.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Systems/BaseSystem.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Wall.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Utils/Utils.hpp>

#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sys
{

BaseSystem::BaseSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : m_reg( reg ),
      m_window( window ),
      m_sprite_factory( sprite_factory ),
      m_sound_bank( sound_bank )
{
  SPDLOG_DEBUG( "BaseSystem constructor called" );
}

// initialised by first call to getEventDispatcher()
std::unique_ptr<entt::dispatcher> BaseSystem::m_systems_event_queue = nullptr;

bool BaseSystem::is_valid_move( const sf::FloatRect &target_position )
{

  // Prevent the player from walking through NPCs
  auto &pc_damage_delay = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PcDamageDelay>( getReg() );
  auto npc_view = getReg().view<Cmp::NPC, Cmp::Position, Cmp::LerpPosition>();
  auto pc_view = getReg().view<Cmp::PlayerCharacter>();
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
  auto obstacle_view = getReg().view<Cmp::Obstacle, Cmp::Position>();
  for ( auto [entity, obs_cmp, pos_cmp] : obstacle_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  // Check walls
  auto wall_view = getReg().view<Cmp::Wall, Cmp::Position>();
  for ( auto [entity, wall_cmp, pos_cmp] : wall_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  // Check doors
  auto door_view = getReg().view<Cmp::Exit, Cmp::Position>();
  for ( auto [entity, exit_cmp, pos_cmp] : door_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) )
    {
      if ( exit_cmp.m_locked == false ) { return true; }
      else { return false; }
    }
  }

  auto grave_view = getReg().view<Cmp::GraveSegment, Cmp::Position>();
  for ( auto [entity, grave_cmp, pos_cmp] : grave_view.each() )
  {
    if ( not grave_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto altar_view = getReg().view<Cmp::AltarSegment, Cmp::Position>();
  for ( auto [entity, altar_cmp, pos_cmp] : altar_view.each() )
  {
    if ( not altar_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto crypt_view = getReg().view<Cmp::CryptSegment, Cmp::Position>();
  for ( auto [entity, crypt_cmp, pos_cmp] : crypt_view.each() )
  {
    if ( not crypt_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto holywell_view = getReg().view<Cmp::HolyWellSegment, Cmp::Position>();
  for ( auto [entity, holywell_cmp, pos_cmp] : holywell_view.each() )
  {
    if ( not holywell_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto ruin_view = getReg().view<Cmp::RuinSegment, Cmp::Position>();
  for ( auto [entity, ruin_cmp, pos_cmp] : ruin_view.each() )
  {
    if ( not ruin_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto ruin_stair_view = getReg().view<Cmp::RuinStairsSegment, Cmp::Position>();
  for ( auto [entity, ruin_cmp, pos_cmp] : ruin_stair_view.each() )
  {
    if ( not ruin_cmp.isSolidMask() ) continue;
    if ( pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto crypt_obj_view = getReg().view<Cmp::CryptObjectiveSegment, Cmp::Position>();
  for ( auto [entity, crypt_obj_cmp, crypt_obj_pos_cmp] : crypt_obj_view.each() )
  {
    if ( not crypt_obj_cmp.isSolidMask() ) continue;
    if ( crypt_obj_pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto crypt_int_view = getReg().view<Cmp::CryptInteriorSegment, Cmp::Position>();
  for ( auto [entity, crypt_int_cmp, crypt_int_pos_cmp] : crypt_int_view.each() )
  {
    if ( not crypt_int_cmp.isSolidMask() ) continue;
    if ( crypt_int_pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto plant_view = getReg().view<Cmp::PlantObstacle, Cmp::Position>();
  for ( auto [entity, plant_cmp, plant_pos_cmp] : plant_view.each() )
  {
    if ( plant_pos_cmp.findIntersection( target_position ) ) { return false; }
  }

  auto crypt_chest_view = getReg().view<Cmp::CryptChest, Cmp::Position>();
  for ( auto [entity, crypt_chest_cmp, crypt_chest_pos_cmp] : crypt_chest_view.each() )
  {
    if ( crypt_chest_pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_INFO( "Blocking player at {},{} with CryptChest", crypt_chest_pos_cmp.position.x, crypt_chest_pos_cmp.position.y );
      return false;
    }
  }

  return true;
}

bool BaseSystem::isDiagonalMovementBetweenObstacles( const sf::FloatRect &current_pos, const sf::Vector2f &direction )
{
  if ( !( ( direction.x != 0.0f ) && ( direction.y != 0.0f ) ) ) return false; // Not diagonal

  sf::Vector2f grid_size{ Constants::kGridSquareSizePixels };

  // Calculate the two orthogonal positions the diagonal movement would "cut through"
  sf::FloatRect horizontal_check = sf::FloatRect{ sf::Vector2f{ current_pos.position.x + ( direction.x * grid_size.x ), current_pos.position.y },
                                                  grid_size };

  sf::FloatRect vertical_check = sf::FloatRect{ sf::Vector2f{ current_pos.position.x, current_pos.position.y + ( direction.y * grid_size.y ) },
                                                grid_size };

  // Check if both orthogonal positions have obstacles
  bool horizontal_blocked = !is_valid_move( horizontal_check );
  if ( !horizontal_blocked ) return false;
  bool vertical_blocked = !is_valid_move( vertical_check );
  if ( !vertical_blocked ) return false;

  // Both orthogonal paths are blocked, diagonal movement is between obstacles
  return true;
}

} // namespace ProceduralMaze::Sys