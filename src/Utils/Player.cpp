#include <Components/Inventory/CarryItem.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerCurse.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Player/PlayerSpeedPenalty.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Position.hpp>
#include <Components/ZOrderValue.hpp>
#include <Direction.hpp>
#include <Player/PlayerCadaverCount.hpp>
#include <Player/PlayerLevelDepth.hpp>
#include <SpawnArea.hpp>
#include <SpriteAnimation.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Utils/Player.hpp>
#include <stdexcept>

namespace ProceduralMaze::Utils::Player
{

entt::entity get_entity( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerCharacter, Cmp::Position>();
  if ( player_view.front() == entt::null ) throw std::runtime_error( "Player entity could not be found" );
  return player_view.front();
}

//! @brief Get the player position object
//! @throws runtime_error if player has no Cmp::Position
//! @param reg
//! @return Cmp::Position&
Cmp::Position &get_position( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerCharacter, Cmp::Position>();
  for ( auto [entt, player_cmp, pos_cmp] : player_view.each() )
  {
    return pos_cmp;
  }
  throw std::runtime_error( "Player entt has no component: Cmp::Position" );
}

//! @brief Get the player direction object
//! @throws runtime_error if player has no Cmp::Direction
//! @param reg
//! @return Cmp::Direction&
Cmp::Direction &get_direction( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerCharacter, Cmp::Direction>();
  for ( auto [entt, player_cmp, dir_cmp] : player_view.each() )
  {
    return dir_cmp;
  }
  throw std::runtime_error( "Player entt has no component: Cmp::Direction" );
}

//! @brief Get the player sprite anim object
//! @throws runtime_error if player has no Cmp::SpriteAnimation
//! @param reg
//! @return Cmp::SpriteAnimation&
Cmp::SpriteAnimation &get_sprite_anim( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerCharacter, Cmp::SpriteAnimation>();
  for ( auto [entt, player_cmp, anim_cmp] : player_view.each() )
  {
    return anim_cmp;
  }
  throw std::runtime_error( "Player entt has no component: Cmp::SpriteAnimation" );
}

int get_ruin_location( entt::registry &reg )
{
  auto player_location = reg.try_get<Cmp::PlayerRuinLocation>( get_entity( reg ) );
  if ( player_location ) { return static_cast<int>( player_location->m_floor ); }
  else { return static_cast<int>( Cmp::PlayerRuinLocation::Floor::NONE ); }
}

//! @brief Get the player last graveyard position object. Return may be nullptr if non-existent.
//! @param reg
//! @return Cmp::PlayerLastGraveyardPosition*
Cmp::PlayerLastGraveyardPosition *get_last_graveyard_pos( entt::registry &reg )
{
  auto player_entt = get_entity( reg );
  return reg.try_get<Cmp::PlayerLastGraveyardPosition>( player_entt );
}

Cmp::PlayerLevelDepth &get_level_depth( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerLevelDepth>();
  if ( player_view.empty() ) throw std::runtime_error( "Player entt has no component: Cmp::PlayerLevelDepth" );
  return player_view.get<Cmp::PlayerLevelDepth>( get_entity( reg ) );
}

Cmp::PlayerWealth &get_wealth( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerWealth>();
  if ( player_view.empty() ) throw std::runtime_error( "Player entt has no component: Cmp::PlayerWealth" );
  return player_view.get<Cmp::PlayerWealth>( get_entity( reg ) );
}

Cmp::PlayerBlastRadius &get_blast_radius( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerBlastRadius>();
  if ( player_view.empty() ) throw std::runtime_error( "Player entt has no component: Cmp::PlayerBlastRadius" );
  return player_view.get<Cmp::PlayerBlastRadius>( get_entity( reg ) );
}

Cmp::PlayerMortality &get_mortality( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerMortality>();
  if ( player_view.empty() ) throw std::runtime_error( "Player entt has no component: Cmp::PlayerMortality" );
  return player_view.get<Cmp::PlayerMortality>( get_entity( reg ) );
}

Cmp::ZOrderValue &get_zorder( entt::registry &reg )
{
  auto zorder_cmp = reg.try_get<Cmp::ZOrderValue>( get_entity( reg ) );
  if ( not zorder_cmp ) throw std::runtime_error( "Player entt has no component: Cmp::ZOrderValue" );

  auto player_view = reg.view<Cmp::PlayerCharacter, Cmp::ZOrderValue>();
  return player_view.get<Cmp::ZOrderValue>( get_entity( reg ) );
}

Cmp::PlayerCurse &get_curse( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerCurse>();
  if ( player_view.empty() ) throw std::runtime_error( "Player entt has no component: Cmp::PlayerCurse" );
  auto &curse = player_view.get<Cmp::PlayerCurse>( get_entity( reg ) );
  SPDLOG_DEBUG( "Cmp::PlayerCurse == {}", curse.active );
  return curse;
}

void reset_curse( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerCurse>();
  if ( player_view.empty() ) throw std::runtime_error( "Player entt has no component: Cmp::PlayerCurse" );
  auto &curse = player_view.get<Cmp::PlayerCurse>( get_entity( reg ) );
  curse.active = false;
  curse.shader_alpha.reset();
  SPDLOG_DEBUG( "Cmp::PlayerCurse == {}", curse.active );
}

float get_speed_penalty( entt::registry &reg )
{
  auto penalty_cmp = reg.try_get<Cmp::PlayerSpeedPenalty>( get_entity( reg ) );
  if ( penalty_cmp ) { return penalty_cmp->m_penalty; }
  else { return 1.f; }
}

void remove_lerp_cmp( entt::registry &reg )
{
  // Clear any ongoing lerp from the previous scene to prevent invalid player re-positioning
  auto player_entt = get_entity( reg );
  if ( reg.any_of<Cmp::LerpPosition>( player_entt ) )
  {
    reg.remove<Cmp::LerpPosition>( player_entt );
    SPDLOG_DEBUG( "Cleared LerpPosition component to prevent position interpolation" );
  }
}

std::pair<entt::entity, Sprites::SpriteMetaType> get_inventory_type( entt::registry &reg )
{
  auto inv_view = reg.view<Cmp::PlayerInventorySlot>();
  Sprites::SpriteMetaType found_type = "";
  entt::entity found_entt = entt::null;
  // this assumes there is only one slot in the inventory, so warn if there is a bug somewhere
  if ( inv_view.size() > 1 ) SPDLOG_WARN( "Found multiple slots in signle slot inventory" );
  for ( auto [inv_entt, inv_cmp] : inv_view.each() )
  {
    found_type = inv_cmp.type;
    found_entt = inv_entt;
  }
  return { found_entt, found_type };
}

float get_inventory_wear_level( entt::registry &reg )
{
  auto inventory_wear_view = reg.view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
  for ( auto [inventory_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
  {
    return wear_level.m_level;
  }
  SPDLOG_DEBUG( "Player Inventory slot has no appropriate InventoryWearLevel component" );
  return -1;
}

void reduce_inventory_wear_level( entt::registry &reg, float amount )
{
  auto inventory_wear_view = reg.view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
  for ( auto [inventory_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
  {
    wear_level.m_level -= amount;
    return;
  }
  SPDLOG_DEBUG( "Player Inventory slot has no appropriate InventoryWearLevel component" );
}

bool is_in_spawn( entt::registry &reg, const Cmp::Position &player_pos_cmp )
{
  bool result = false;
  for ( auto [spawn_entt, spawn_cmp, spawn_pos_cmp] : reg.view<Cmp::SpawnArea, Cmp::Position>().each() )
  {
    if ( spawn_pos_cmp.findIntersection( player_pos_cmp ) ) result = true;
  }
  return result;
}

Cmp::PlayerCadaverCount &get_cadaver_count( entt::registry &reg )
{

  auto player_view = reg.view<Cmp::PlayerCadaverCount>();
  if ( player_view.empty() ) throw std::runtime_error( "Player entt has no component: Cmp::PlayerCadaverCount" );
  auto &curse = player_view.get<Cmp::PlayerCadaverCount>( get_entity( reg ) );
  SPDLOG_DEBUG( "Cmp::PlayerCadaverCount == {}", curse.active );
  return curse;
}

Cmp::PlayerStats &get_player_stats( entt::registry &reg )
{
  auto player_stats_view = reg.view<Cmp::PlayerStats>();
  for ( auto [entity, cmp] : player_stats_view.each() )
  {
    if ( reg.all_of<Cmp::PlayerCharacter>( entity ) ) { return cmp; }
  }
  throw std::runtime_error( "Player entt has no component: Cmp::PlayerStats" );
}

} // namespace ProceduralMaze::Utils::Player