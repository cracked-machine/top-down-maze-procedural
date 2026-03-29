#include <AbsoluteAlpha.hpp>
#include <Collision.hpp>
#include <Components/Position.hpp>
#include <Components/Ruin/RuinBookcase.hpp>
#include <Components/Ruin/RuinCobweb.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Constants.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <Npc/Npc.hpp>
#include <Npc/NpcNoPathFinding.hpp>
#include <Obstacle.hpp>
#include <Player/PlayerNoPath.hpp>
#include <RectBounds.hpp>
#include <ReservedPosition.hpp>
#include <Ruin/RuinShadowHand.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Utils/Random.hpp>
#include <entt/entity/registry.hpp>
#include <ranges>

namespace ProceduralMaze::Factory
{

void create_bookcase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &bookcase_ms, int sprite_index )
{
  // We must modify the **existing** Cmp::Position-owning entity so that we don't have
  // new entity with Cmp::NpcNoPathFinding and existing entity without. This screws up path finding
  Cmp::Position search_pos( spawn_position, bookcase_ms.getSpriteSizePixels() );
  for ( auto [existing_entt, existing_pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( search_pos.findIntersection( existing_pos_cmp ) )
    {
      reg.emplace_or_replace<Cmp::Position>( existing_entt, spawn_position, bookcase_ms.getSpriteSizePixels() );
      reg.emplace_or_replace<Cmp::SpriteAnimation>( existing_entt, 0, 0, true, bookcase_ms.get_sprite_type(), sprite_index );
      reg.emplace_or_replace<Cmp::ZOrderValue>( existing_entt, -spawn_position.y * 10 );
      reg.emplace_or_replace<Cmp::RuinBookcase>( existing_entt );
      reg.emplace_or_replace<Cmp::NpcNoPathFinding>( existing_entt );
      reg.emplace_or_replace<Cmp::PlayerNoPath>( existing_entt );
      break;
    }
  }
}

void create_cobweb( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &cobweb_ms, int sprite_index )
{
  auto cobweb_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( cobweb_entt, spawn_position, cobweb_ms.getSpriteSizePixels() );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( cobweb_entt, 0, 0, true, cobweb_ms.get_sprite_type(), sprite_index );
  reg.emplace_or_replace<Cmp::ZOrderValue>( cobweb_entt, spawn_position.y );
  reg.emplace_or_replace<Cmp::RuinCobweb>( cobweb_entt, 100 );
}

void create_shadow_hand( entt::registry &reg, sf::Vector2f scene_dimensions, const Sprites::MultiSprite &ms, int sprite_index )
{
  bool exists = false;
  for ( auto [hand_entt, hand_cmp] : reg.view<Cmp::RuinShadowHand>().each() )
  {
    if ( hand_cmp.active ) exists = true;
  }

  if ( not exists )
  {
    const auto hand_ms_size = ms.getSpriteSizePixels();
    sf::Vector2f starting_pos = { 0 - hand_ms_size.x, scene_dimensions.y / 2 - hand_ms_size.y / 2 };

    auto shadowhand_entt = reg.create();
    reg.emplace_or_replace<Cmp::Position>( shadowhand_entt, starting_pos, ms.getSpriteSizePixels() );
    reg.emplace_or_replace<Cmp::SpriteAnimation>( shadowhand_entt, 0, 0, true, ms.get_sprite_type(), sprite_index );
    reg.emplace_or_replace<Cmp::ZOrderValue>( shadowhand_entt, ms.get_zorder( 0 ) ); // above everythign
    reg.emplace_or_replace<Cmp::AbsoluteAlpha>( shadowhand_entt, 200 );
    reg.emplace_or_replace<Cmp::RuinShadowHand>( shadowhand_entt );
  }
}

} // namespace ProceduralMaze::Factory