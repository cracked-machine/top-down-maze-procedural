#include <AbsoluteAlpha.hpp>
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
#include <Ruin/RuinShadowHand.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Factory
{

// void create_staircase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms )
// {
//   auto stairs_entt = reg.create();
//   reg.emplace_or_replace<Cmp::Position>( stairs_entt, spawn_position, stairs_ms.getSpriteSizePixels() );
//   reg.emplace_or_replace<Cmp::SpriteAnimation>( stairs_entt, 0, 0, true, stairs_ms.get_sprite_type(), 0 );
//   reg.emplace_or_replace<Cmp::ZOrderValue>( stairs_entt, spawn_position.y );
// }

void create_bookcase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &bookcase_ms, int sprite_index )
{
  auto bookcase_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( bookcase_entt, spawn_position, bookcase_ms.getSpriteSizePixels() );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( bookcase_entt, 0, 0, true, bookcase_ms.get_sprite_type(), sprite_index );
  reg.emplace_or_replace<Cmp::ZOrderValue>( bookcase_entt, -spawn_position.y * 10 );
  reg.emplace_or_replace<Cmp::RuinBookcase>( bookcase_entt );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( bookcase_entt );
}

void create_cobweb( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &cobweb_ms, int sprite_index )
{
  auto cobweb_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( cobweb_entt, spawn_position, cobweb_ms.getSpriteSizePixels() );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( cobweb_entt, 0, 0, true, cobweb_ms.get_sprite_type(), sprite_index );
  reg.emplace_or_replace<Cmp::ZOrderValue>( cobweb_entt, spawn_position.y );
  reg.emplace_or_replace<Cmp::RuinCobweb>( cobweb_entt, 100 );
}

void create_shadow_hand( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::MultiSprite &ms, int sprite_index )
{
  bool exists = false;
  for ( auto [hand_entt, hand_cmp] : reg.view<Cmp::RuinShadowHand>().each() )
  {
    if ( hand_cmp.active ) exists = true;
  }

  if ( not exists )
  {
    auto shadowhand_entt = reg.create();
    reg.emplace_or_replace<Cmp::Position>( shadowhand_entt, spawn_position, ms.getSpriteSizePixels() );
    reg.emplace_or_replace<Cmp::SpriteAnimation>( shadowhand_entt, 0, 0, true, ms.get_sprite_type(), sprite_index );
    reg.emplace_or_replace<Cmp::ZOrderValue>( shadowhand_entt, spawn_position.y * 100 ); // above everythign
    reg.emplace_or_replace<Cmp::AbsoluteAlpha>( shadowhand_entt, 200 );
    reg.emplace_or_replace<Cmp::RuinShadowHand>( shadowhand_entt );
  }
}

void create_witch( entt::registry &reg, sf::Vector2f spawn_position )
{

  bool witch_exists = false;
  for ( auto [npc_entt, npc_cmp, npc_sprite_cmp] : reg.view<Cmp::NPC, Cmp::SpriteAnimation>().each() )
  {
    if ( npc_sprite_cmp.m_sprite_type == "NPCWITCH" ) { witch_exists = true; }
  }
  if ( not witch_exists )
  {
    auto position_entity = reg.create();
    Cmp::Position position_cmp = reg.emplace<Cmp::Position>( position_entity, spawn_position, Constants::kGridSizePxF );
    [[maybe_unused]] Cmp::ZOrderValue zorder_cmp = reg.emplace<Cmp::ZOrderValue>( position_entity, position_cmp.position.y );
    Factory::createNPC( reg, position_entity, "NPCWITCH" );
  }
}

} // namespace ProceduralMaze::Factory