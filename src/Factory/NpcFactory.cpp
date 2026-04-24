#include <Components/Armable.hpp>
#include <Components/DeathPosition.hpp>
#include <Components/Direction.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcContainer.hpp>
#include <Components/Npc/NpcFriendly.hpp>
#include <Components/Npc/NpcLerpSpeed.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Npc/NpcShockwaveTimer.hpp>
#include <Components/Persistent/NpcShockwaveFreq.hpp>
#include <Components/Persistent/NpcShockwaveResolution.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/Factory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Persistent/NpcGhostAnimFramerate.hpp>
#include <Persistent/NpcLerpSpeedGhost.hpp>
#include <Persistent/NpcLerpSpeedPriest.hpp>
#include <Persistent/NpcLerpSpeedSkele.hpp>
#include <Persistent/NpcLerpSpeedWitch.hpp>
#include <Persistent/NpcSkeleAnimFramerate.hpp>
#include <Persistent/NpcWitchAnimFramerate.hpp>
#include <Player.hpp>
#include <SpatialHashGrid.hpp>
#include <SpriteAnimation.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Stats/BaseAction.hpp>
#include <Stats/DestroyAction.hpp>
#include <Stats/ExhumeAction.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Stores/NpcStore.hpp>
#include <Utils/Random.hpp>
#include <entt/entity/entity.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Factory
{

void create_npc_container( entt::registry &registry, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                           std::size_t sprite_tile_idx, float zorder )
{
  registry.emplace_or_replace<Cmp::ReservedPosition>( entt );
  registry.emplace_or_replace<Cmp::Armable>( entt );
  registry.emplace_or_replace<Cmp::NpcContainer>( entt );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, true, sprite_type, sprite_tile_idx );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entt, pos_cmp.position.y - zorder );
}

void destroy_npc_container( entt::registry &registry, entt::entity npc_container_entity )
{
  registry.remove<Cmp::NpcContainer>( npc_container_entity );
  registry.remove<Cmp::SpriteAnimation>( npc_container_entity );
  registry.remove<Cmp::ZOrderValue>( npc_container_entity );
}

void create_shockwave( entt::registry &registry, entt::entity npc_entt )
{
  // get the shockwave timer for the NPC
  auto shockwave_timer = registry.try_get<Cmp::NpcShockwaveTimer>( npc_entt );
  if ( not shockwave_timer )
  {
    SPDLOG_DEBUG( "Unable to get Cmp::NpcShockwaveTimer from NPC entity" );
    return;
  }

  // check cooldown on this NPC shockwave timer
  sf::Time sw_emit_freq{ sf::milliseconds( Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveFreq>( registry ).get_value() ) };
  if ( shockwave_timer->getElapsedTime() > sw_emit_freq )
  {
    // create a new entity for adding the shockwave component to the NPC position
    auto *npc_pos = registry.try_get<Cmp::Position>( npc_entt );
    if ( not npc_pos )
    {
      SPDLOG_WARN( "Unable to get position from NPC entity" );
      return;
    }
    auto npc_sw_entt = registry.create();
    int circle_resolution = Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveResolution>( registry ).get_value();
    registry.emplace_or_replace<Cmp::NpcShockwave>( npc_sw_entt, npc_pos->getCenter(), circle_resolution );

    shockwave_timer->restart(); // make sure we restart the timer
  }
}

void create_npc( entt::registry &reg, entt::entity position_entity, const Sprites::SpriteMetaType &type )
{

  auto *pos_cmp = reg.try_get<Cmp::Position>( position_entity );
  if ( not pos_cmp )
  {
    SPDLOG_ERROR( "Cannot add NPC entity {} without a Position component", static_cast<int>( position_entity ) );
    return;
  }

  // create a new entity for the NPC using the existing position
  auto new_pos_entity = reg.create();
  reg.emplace<Cmp::Position>( new_pos_entity, pos_cmp->position, Constants::kGridSizePxF );
  reg.emplace<Cmp::Armable>( new_pos_entity );
  reg.emplace_or_replace<Cmp::Direction>( new_pos_entity, sf::Vector2f{ 0, 0 } );
  if ( type == "NPCGHOST" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( "npc.ghost" );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    auto framerate = Sys::PersistSystem::get<Cmp::Persist::NpcGhostAnimFramerate>( reg ).get_value();
    reg.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, true, "NPCGHOST.walk.east", framerate );

    reg.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );

    float lerpspeed = Sys::PersistSystem::get<Cmp::Persist::NpcLerpSpeedGhost>( reg ).get_value();
    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, lerpspeed );

    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );

    Utils::Player::get_player_stats( reg ).action( npc_cmp.action_fx_map.at( std::type_index( typeid( Cmp::ExhumeAction ) ) ) );
  }
  else if ( type == "NPCSKELE" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( "npc.skeleton" );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    auto framerate = Sys::PersistSystem::get<Cmp::Persist::NpcSkeleAnimFramerate>( reg ).get_value();
    reg.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, true, "NPCSKELE.walk.east", framerate );

    reg.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );

    float lerpspeed = Sys::PersistSystem::get<Cmp::Persist::NpcLerpSpeedSkele>( reg ).get_value();
    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, lerpspeed );

    // Remove the npc container component from the original entity
    reg.remove<Cmp::NpcContainer>( position_entity );
    reg.remove<Cmp::ZOrderValue>( position_entity );
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
    Utils::Player::get_player_stats( reg ).action( npc_cmp.action_fx_map.at( std::type_index( typeid( Cmp::ExhumeAction ) ) ) );
  }
  else if ( type == "NPCPRIEST" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( "npc.priest" );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    reg.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, false, "NPCPRIEST" );

    reg.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );

    float lerpspeed = Sys::PersistSystem::get<Cmp::Persist::NpcLerpSpeedPriest>( reg ).get_value();
    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, lerpspeed );

    // Remove the npc container component from the original entity
    reg.remove<Cmp::NpcContainer>( position_entity );
    reg.remove<Cmp::ZOrderValue>( position_entity );

    reg.emplace_or_replace<Cmp::NpcShockwaveTimer>( new_pos_entity );
    Factory::create_shockwave( reg, position_entity );
    SPDLOG_DEBUG( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                  pos_cmp->position.y );
    Utils::Player::get_player_stats( reg ).action( npc_cmp.action_fx_map.at( std::type_index( typeid( Cmp::ExhumeAction ) ) ) );
  }
  else if ( type == "NPCWITCH" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( "npc.witch" );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    auto framerate = Sys::PersistSystem::get<Cmp::Persist::NpcWitchAnimFramerate>( reg ).get_value();
    reg.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, false, "NPCWITCH", framerate );
    reg.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );

    float lerpspeed = Sys::PersistSystem::get<Cmp::Persist::NpcLerpSpeedWitch>( reg ).get_value();
    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, lerpspeed );

    // Remove the npc container component from the original entity
    reg.remove<Cmp::NpcContainer>( position_entity );
    reg.remove<Cmp::ZOrderValue>( position_entity );
    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
    Utils::Player::get_player_stats( reg ).action( npc_cmp.action_fx_map.at( std::type_index( typeid( Cmp::ExhumeAction ) ) ) );
  }
  else if ( type == "NPC.dr_knox" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( "npc.drknox" );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    reg.emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity, 0, 0, false, "NPC.dr_knox" );
    reg.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );
    reg.emplace_or_replace<Cmp::NpcFriendly>( new_pos_entity );

    SPDLOG_INFO( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), type, pos_cmp->position.x,
                 pos_cmp->position.y );
    Utils::Player::get_player_stats( reg ).action( npc_cmp.action_fx_map.at( std::type_index( typeid( Cmp::ExhumeAction ) ) ) );
  }
}

entt::entity destroy_npc( entt::registry &reg, entt::entity npc_entity )
{

  // check for position component
  entt::entity loot_entity = entt::null;
  auto *npc_pos_cmp = reg.try_get<Cmp::Position>( npc_entity );
  if ( not npc_pos_cmp ) { SPDLOG_WARN( "Cannot process loot drop for NPC entity {} without a Position component", static_cast<int>( npc_entity ) ); }

  // apply destroy action effect to player
  auto *npc_cmp = reg.try_get<Cmp::NPC>( npc_entity );
  if ( npc_cmp ) { Utils::Player::get_player_stats( reg ).action( npc_cmp->action_fx_map.at( std::type_index( typeid( Cmp::DestroyAction ) ) ) ); }

  // kill npc once we are done
  reg.remove<Cmp::NPC>( npc_entity );
  reg.remove<Cmp::Position>( npc_entity );
  reg.remove<Cmp::Direction>( npc_entity );
  reg.remove<Cmp::SpriteAnimation>( npc_entity );
  reg.remove<Cmp::ZOrderValue>( npc_entity );

  return loot_entity;
}

entt::entity create_npc_explosion( entt::registry &registry, Cmp::Position npc_pos_cmp )
{
  auto npc_death_entity = registry.create();
  registry.emplace<Cmp::Position>( npc_death_entity, npc_pos_cmp.position, npc_pos_cmp.size );
  registry.emplace_or_replace<Cmp::DeathPosition>( npc_death_entity, npc_pos_cmp.position, npc_pos_cmp.size );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( npc_death_entity, 0, 0, true, "EXPLOSION", 0, 0.1, Cmp::AnimType::ONESHOTRESET );
  registry.emplace_or_replace<Cmp::ZOrderValue>( npc_death_entity, npc_pos_cmp.position.y );
  return npc_death_entity;
}

void remove_npc_explosion( entt::registry &registry, entt::entity entity )
{
  registry.remove<Cmp::DeathPosition>( entity );
  registry.remove<Cmp::SpriteAnimation>( entity );
  registry.remove<Cmp::ZOrderValue>( entity );
  registry.remove<Cmp::Position>( entity );
  SPDLOG_DEBUG( "Explosion animation complete, removing component from entity {}", static_cast<int>( entity ) );
}

std::vector<entt::entity> gen_npc_containers( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size )
{
  std::vector<entt::entity> assigned_entts;

  auto num_npc_containers = map_grid_size.x * map_grid_size.y / 120; // one NPC container per N grid squares

  for ( std::size_t i = 0; i < num_npc_containers; ++i )
  {
    auto [random_entity, random_origin_position] = Utils::Rnd::get_random_position(
        reg, {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // pick a random loot container type and texture index
    // clang-format off
    auto [npc_type, rand_npc_tex_idx] =
      sprite_factory.get_random_type_and_texture_index( {
        "BONES"
      } );
    // clang-format on

    Factory::create_npc_container( reg, random_entity, random_origin_position, npc_type, rand_npc_tex_idx, 0.f );
    assigned_entts.push_back( random_entity );
  }
  return assigned_entts;
}

} // namespace ProceduralMaze::Factory