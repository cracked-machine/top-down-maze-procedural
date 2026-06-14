#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
#include <Components/Armed.hpp>
#include <Components/DeathPosition.hpp>
#include <Components/Direction.hpp>
#include <Components/LerpPosition.hpp>
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
#include <Components/UUID.hpp>
#include <Factory/Factory.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ParticleFactory.hpp>
#include <SpatialHashGrid.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Stats/DestroyAction.hpp>
#include <Stats/SpawnAction.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Stores/NpcStore.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>

#include <entt/entity/entity.hpp>
#include <spdlog/spdlog.h>

namespace Game::Factory
{

void create_npc_container( entt::registry &reg, entt::entity entt, Cmp::Position pos_cmp, Sprites::SpriteMetaType sprite_type,
                           std::size_t sprite_tile_idx, float zorder )
{
  reg.emplace_or_replace<Cmp::ReservedPosition>( entt );
  reg.emplace_or_replace<Cmp::Armable>( entt );
  reg.emplace_or_replace<Cmp::NpcContainer>( entt );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( entt, Cmp::AnimData::Config{ 
        .sprite_type = sprite_type, 
        .frame_index_offset = sprite_tile_idx,
        .enabled = true
  });
  // clang-format on
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, pos_cmp.position.y - zorder );
}

void destroy_npc_container( entt::registry &registry, entt::entity npc_container_entity )
{
  registry.remove<Cmp::ReservedPosition>( npc_container_entity );
  registry.remove<Cmp::Armed>( npc_container_entity );
  registry.remove<Cmp::NpcContainer>( npc_container_entity );
  registry.remove<Cmp::AnimData>( npc_container_entity );
  registry.remove<Cmp::ZOrderValue>( npc_container_entity );
}

bool create_shockwave( entt::registry &registry, entt::entity npc_entt )
{
  // get the shockwave timer for the NPC
  auto *shockwave_timer = registry.try_get<Cmp::NpcShockwaveTimer>( npc_entt );
  if ( not shockwave_timer )
  {
    SPDLOG_DEBUG( "Unable to get Cmp::NpcShockwaveTimer from NPC entity" );
    return false;
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
      return false;
    }
    auto npc_sw_entt = registry.create();
    int circle_resolution = Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveResolution>( registry ).get_value();
    registry.emplace_or_replace<Cmp::NpcShockwave>( npc_sw_entt, npc_pos->getCenter(), circle_resolution );

    shockwave_timer->restart(); // make sure we restart the timer
    return true;
  }
  return false;
}

entt::entity create_npc( entt::registry &reg, entt::entity position_entity, const std::string &npc_type )
{

  auto *pos_cmp = reg.try_get<Cmp::Position>( position_entity );
  if ( not pos_cmp )
  {
    SPDLOG_ERROR( "Cannot add NPC entity {} without a Position component", static_cast<int>( position_entity ) );
    return entt::null;
  }

  // create a new entity for the NPC using the existing position
  auto new_pos_entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( new_pos_entity, pos_cmp->position, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::Armable>( new_pos_entity );
  reg.emplace_or_replace<Cmp::ZOrderValue>( new_pos_entity, pos_cmp->position.y );
  reg.emplace_or_replace<Cmp::Direction>( new_pos_entity, sf::Vector2f{ 0, 0 } );
  reg.emplace_or_replace<Cmp::UUID>( new_pos_entity, Cmp::UUID::generate() );

  SPDLOG_DEBUG( "Spawned NPC entity {} of type {} at position ({}, {})", static_cast<int>( new_pos_entity ), npc_type, pos_cmp->position.x,
                pos_cmp->position.y );

  if ( npc_type == "npc.ghost" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( npc_type );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    // clang-format off
    reg.emplace_or_replace<Cmp::AnimData>( new_pos_entity, Cmp::AnimData::Config{ 
          .sprite_type = npc_cmp.sprite_type_list.front(), 
          .framerate = npc_cmp.m_frame_rate,
          .enabled = true
    });
    // clang-format on

    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, npc_cmp.m_lerp_speed );

    auto action_timer_pair = npc_cmp.actions.at( std::type_index( typeid( Cmp::SpawnAction ) ) );
    Utils::Player::get_player_stats( reg ).apply_modifiers( action_timer_pair.action );
  }
  else if ( npc_type == "npc.skeleton" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( "npc.skeleton" );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    // clang-format off
    reg.emplace_or_replace<Cmp::AnimData>( new_pos_entity, Cmp::AnimData::Config{ 
          .sprite_type = npc_cmp.sprite_type_list.front(), 
          .framerate = npc_cmp.m_frame_rate,
          .enabled = true
    });
    // clang-format on

    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, npc_cmp.m_lerp_speed );

    auto action_timer_pair = npc_cmp.actions.at( std::type_index( typeid( Cmp::SpawnAction ) ) );
    Utils::Player::get_player_stats( reg ).apply_modifiers( action_timer_pair.action );

    Factory::destroy_npc_container( reg, position_entity );
  }
  else if ( npc_type == "npc.priest" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( npc_type );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    // clang-format off
    reg.emplace_or_replace<Cmp::AnimData>( new_pos_entity, Cmp::AnimData::Config{ 
          .sprite_type = npc_cmp.sprite_type_list.front(), 
          .framerate = npc_cmp.m_frame_rate,
          .enabled = true
    });
    // clang-format on

    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, npc_cmp.m_lerp_speed );

    reg.emplace_or_replace<Cmp::NpcShockwaveTimer>( new_pos_entity );
    Factory::create_shockwave( reg, new_pos_entity );

    auto action_timer_pair = npc_cmp.actions.at( std::type_index( typeid( Cmp::SpawnAction ) ) );
    Utils::Player::get_player_stats( reg ).apply_modifiers( action_timer_pair.action );
  }
  else if ( npc_type == "npc.witch" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( npc_type );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    // clang-format off
    reg.emplace_or_replace<Cmp::AnimData>( new_pos_entity, Cmp::AnimData::Config{ 
          .sprite_type = npc_cmp.sprite_type_list.front(), 
          .framerate = npc_cmp.m_frame_rate,
          .enabled = true
    });
    // clang-format on

    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, npc_cmp.m_lerp_speed );

    auto action_timer_pair = npc_cmp.actions.at( std::type_index( typeid( Cmp::SpawnAction ) ) );
    Utils::Player::get_player_stats( reg ).apply_modifiers( action_timer_pair.action );
  }
  else if ( npc_type == "npc.wisp" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( npc_type );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    // clang-format off
    reg.emplace_or_replace<Cmp::AnimData>( new_pos_entity, Cmp::AnimData::Config{ 
          .sprite_type = npc_cmp.sprite_type_list.front(), 
          .framerate = npc_cmp.m_frame_rate,
          .enabled = true
    });
    // clang-format on 

    reg.emplace_or_replace<Cmp::NpcLerpSpeed>( new_pos_entity, npc_cmp.m_lerp_speed );
    reg.emplace_or_replace<Cmp::Direction>( new_pos_entity, sf::Vector2f{ 1, 1 } );
    reg.emplace_or_replace<Cmp::NpcFriendly>( new_pos_entity );

  }
  else if ( npc_type == "npc.drknox" )
  {
    auto npc_cmp = Sys::NpcStore::instance().get_item( npc_type );
    reg.emplace_or_replace<Cmp::NPC>( new_pos_entity, npc_cmp );

    // clang-format off
    reg.emplace_or_replace<Cmp::AnimData>( new_pos_entity, Cmp::AnimData::Config{ 
          .sprite_type = npc_cmp.sprite_type_list.front(), 
          .framerate = npc_cmp.m_frame_rate,
          .enabled = false
    });
    // clang-format on

    reg.emplace_or_replace<Cmp::NpcFriendly>( new_pos_entity );

    auto action_timer_pair = npc_cmp.actions.at( std::type_index( typeid( Cmp::SpawnAction ) ) );
    Utils::Player::get_player_stats( reg ).apply_modifiers( action_timer_pair.action );
  }

  return new_pos_entity;
}

entt::entity destroy_npc( entt::registry &reg, entt::entity npc_entity )
{

  // check for position component
  entt::entity loot_entity = entt::null;
  auto *npc_pos_cmp = reg.try_get<Cmp::Position>( npc_entity );
  if ( not npc_pos_cmp ) { SPDLOG_WARN( "Cannot process loot drop for NPC entity {} without a Position component", static_cast<int>( npc_entity ) ); }

  // apply destroy action effect to player
  auto *npc_cmp = reg.try_get<Cmp::NPC>( npc_entity );
  if ( npc_cmp )
  {
    auto action_timer_pair = npc_cmp->actions.at( std::type_index( typeid( Cmp::DestroyAction ) ) );
    Utils::Player::get_player_stats( reg ).apply_modifiers( action_timer_pair.action );
  }

  // kill npc once we are done
  reg.remove<Cmp::NPC>( npc_entity );
  reg.remove<Cmp::Position>( npc_entity );
  reg.remove<Cmp::Direction>( npc_entity );
  reg.remove<Cmp::AnimData>( npc_entity );
  reg.remove<Cmp::ZOrderValue>( npc_entity );

  return loot_entity;
}

entt::entity create_npc_explosion( entt::registry &reg, Cmp::Position npc_pos_cmp )
{
  auto npc_death_entity = reg.create();
  reg.emplace<Cmp::Position>( npc_death_entity, npc_pos_cmp.position, npc_pos_cmp.size );
  reg.emplace_or_replace<Cmp::DeathPosition>( npc_death_entity, npc_pos_cmp.position, npc_pos_cmp.size );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( npc_death_entity, Cmp::AnimData::Config{ 
        .sprite_type = "sprite.death.anim.explosion", 
        .framerate = 0.1,
        .enabled = true,
        .anim_type =  Cmp::AnimType::ONESHOTRESET
  });
  // clang-format on
  reg.emplace_or_replace<Cmp::ZOrderValue>( npc_death_entity, npc_pos_cmp.position.y );
  return npc_death_entity;
}

void remove_npc_explosion( entt::registry &registry, entt::entity entity )
{
  registry.remove<Cmp::DeathPosition>( entity );
  registry.remove<Cmp::AnimData>( entity );
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
        "sprite.graveyard.bones"
      } );
    // clang-format on

    Factory::create_npc_container( reg, random_entity, random_origin_position, npc_type, rand_npc_tex_idx, 0.f );
    assigned_entts.push_back( random_entity );
  }
  return assigned_entts;
}

} // namespace Game::Factory