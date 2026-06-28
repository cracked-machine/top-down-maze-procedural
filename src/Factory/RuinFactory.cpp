#include <Components/AbsoluteAlpha.hpp>
#include <Components/AnimData.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/RuinBookcase.hpp>
#include <Components/Ruin/RuinCobweb.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <Factory/SpriteFactory.hpp>
#include <Systems/Stores/NpcStore.hpp>
#include <Utils/Collision.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Random.hpp>

#include <entt/entity/registry.hpp>

namespace Game::Factory
{

void create_bookcase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::SpriteSheet &bookcase_ms, int sprite_index )
{
  // We must modify the **existing** Cmp::Position-owning entity so that we don't have
  // new entity with Cmp::NpcNoPathFinding and existing entity without. This screws up path finding
  Cmp::Position search_pos( spawn_position, bookcase_ms.get_sprite_size() );
  for ( auto [existing_entt, existing_pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( search_pos.findIntersection( existing_pos_cmp ) )
    {
      reg.emplace_or_replace<Cmp::Position>( existing_entt, spawn_position, bookcase_ms.get_sprite_size() );
      // clang-format off
      reg.emplace_or_replace<Cmp::AnimData>( existing_entt, Cmp::AnimData::Config{ 
            .sprite_type = bookcase_ms.get_sprite_type(), 
            .frame_index_offset = static_cast<size_t>(sprite_index),
            .enabled = true
      });
      // clang-format on
      reg.emplace_or_replace<Cmp::ZOrderValue>( existing_entt, -5.f );
      reg.emplace_or_replace<Cmp::RuinBookcase>( existing_entt );
      reg.emplace_or_replace<Cmp::NpcNoPathFinding>( existing_entt );
      reg.emplace_or_replace<Cmp::PlayerNoPath>( existing_entt );
      break;
    }
  }
}

void create_cobweb( entt::registry &reg, entt::entity selected_entt, sf::Vector2f spawn_position, const Sprites::SpriteSheet &cobweb_ms,
                    int sprite_index )
{
  // auto cobweb_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( selected_entt, spawn_position, cobweb_ms.get_sprite_size() );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( selected_entt, Cmp::AnimData::Config{ 
        .sprite_type = cobweb_ms.get_sprite_type(), 
        .frame_index_offset = static_cast<size_t>(sprite_index),
        .enabled = true
  });
  // clang-format on
  reg.emplace_or_replace<Cmp::ReservedPosition>( selected_entt );
  reg.emplace_or_replace<Cmp::ZOrderValue>( selected_entt, spawn_position.y );
  reg.emplace_or_replace<Cmp::RuinCobweb>( selected_entt, 100 );
}

void create_shadow_hand( entt::registry &reg, sf::Vector2f scene_dimensions, const Sprites::SpriteSheet &hand_ms, int sprite_index )
{
  bool exists = false;
  for ( auto [hand_entt, hand_cmp] : reg.view<Cmp::NPC>().each() )
  {
    if ( hand_cmp.sprite_type_list.front() == "sprite.shadowhand" ) exists = true;
  }

  if ( not exists )
  {
    auto npc_shadowhand_cmp = Sys::NpcStore::instance().get_item( "npc.shadowhand" );
    const auto hand_ms_size = hand_ms.get_sprite_size();
    sf::Vector2f starting_pos = { 0 - hand_ms_size.x, ( scene_dimensions.y / 2 ) - ( hand_ms_size.y / 2 ) };

    auto shadowhand_entt = reg.create();
    reg.emplace_or_replace<Cmp::Position>( shadowhand_entt, starting_pos, hand_ms.get_sprite_size() );
    // clang-format off
    reg.emplace_or_replace<Cmp::AnimData>( shadowhand_entt, Cmp::AnimData::Config{ 
          .sprite_type = hand_ms.get_sprite_type(), 
          .frame_index_offset = static_cast<size_t>(sprite_index),
          .enabled = true
    });
    // clang-format on
    reg.emplace_or_replace<Cmp::ZOrderValue>( shadowhand_entt, hand_ms.get_zorder( 0 ) ); // above everythign
    reg.emplace_or_replace<Cmp::AbsoluteAlpha>( shadowhand_entt, 200 );
    reg.emplace_or_replace<Cmp::NPC>( shadowhand_entt, npc_shadowhand_cmp );
  }
}

} // namespace Game::Factory