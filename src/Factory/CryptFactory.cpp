#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptLever.hpp>
#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Crypt/CryptPassageSpikeTrap.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomLavaPit.hpp>
#include <Components/Crypt/CryptRoomLavaPitCell.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Exit.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Player.hpp>
#include <Player/PlayerNoPath.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Random.hpp>

namespace ProceduralMaze::Factory
{

entt::entity create_crypt_exit( entt::registry &reg, sf::Vector2f spawn_pos_px )
{
  auto entity = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entity, spawn_pos_px, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::Exit>( entity, false ); // unlocked at start
  reg.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "CRYPT.interior_sb", 1 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entity, spawn_pos_px.y );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
  reg.emplace_or_replace<Cmp::Exit>( entity );
  return entity;
}

entt::entity create_crypt_lever( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder )
{
  auto entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entt, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::CryptLever>( entt );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, true, sprite_type, sprite_idx );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  return entt;
}

void destroy_crypt_lever( entt::registry &reg, entt::entity entt )
{
  if ( reg.all_of<Cmp::CryptLever>( entt ) ) reg.remove<Cmp::CryptLever>( entt );
  if ( reg.all_of<Cmp::Position>( entt ) ) reg.remove<Cmp::Position>( entt );
  if ( reg.all_of<Cmp::SpriteAnimation>( entt ) ) reg.remove<Cmp::SpriteAnimation>( entt );
  if ( reg.all_of<Cmp::ZOrderValue>( entt ) ) reg.remove<Cmp::ZOrderValue>( entt );
  if ( reg.valid( entt ) ) { reg.destroy( entt ); }
}

entt::entity create_crypt_chest( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder )
{
  auto entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( entt, pos, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::CryptChest>( entt );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, false, sprite_type, sprite_idx );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::PlayerNoPath>( entt );
  return entt;
}

void destroy_crypt_chest( entt::registry &reg, entt::entity entt )
{
  if ( reg.all_of<Cmp::CryptChest>( entt ) ) reg.remove<Cmp::CryptLever>( entt );
  if ( reg.all_of<Cmp::Position>( entt ) ) reg.remove<Cmp::Position>( entt );
  if ( reg.all_of<Cmp::SpriteAnimation>( entt ) ) reg.remove<Cmp::SpriteAnimation>( entt );
  if ( reg.all_of<Cmp::ZOrderValue>( entt ) ) reg.remove<Cmp::ZOrderValue>( entt );
  if ( reg.all_of<Cmp::PlayerNoPath>( entt ) ) { reg.remove<Cmp::PlayerNoPath>( entt ); }
  if ( reg.valid( entt ) ) { reg.destroy( entt ); }
}

void create_crypt_lava_pit( entt::registry &reg, const Cmp::CryptRoomOpen &room,
                            std::shared_ptr<ProceduralMaze::PathFinding::SpatialHashGrid> pathfinding_navmesh )
{
  // add the lava pit area
  sf::Vector2f adjusted_position = { room.position.x + Constants::kGridSizePxF.x, room.position.y + Constants::kGridSizePxF.y };
  sf::Vector2f adjusted_size = { room.size.x - ( Constants::kGridSizePxF.x * 2 ), room.size.y - ( Constants::kGridSizePxF.y * 2 ) };
  sf::FloatRect lava_pit_bounds( adjusted_position, adjusted_size );

  auto lava_pit_entt = reg.create();
  reg.emplace<Cmp::CryptRoomLavaPit>( lava_pit_entt, lava_pit_bounds );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( lava_pit_entt );

  const auto player_pos = Utils::Player::get_position( reg );

  // add the inidividual lava cells
  for ( auto [pos_entt, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( reg.all_of<Cmp::FootStepTimer>( pos_entt ) ) continue; // dont add lava to footstep positions
    if ( player_pos.findIntersection( pos_cmp ) ) continue;     // dont add lava cell to player pos!

    if ( not lava_pit_bounds.findIntersection( pos_cmp ) ) continue; // only add lava to this lava pit
    auto lava_cell_entt = reg.create();
    reg.emplace_or_replace<Cmp::Position>( lava_cell_entt, pos_cmp.position, pos_cmp.size );
    reg.emplace_or_replace<Cmp::NpcNoPathFinding>( lava_cell_entt );
    reg.emplace_or_replace<Cmp::CryptRoomLavaPitCell>( lava_cell_entt, pos_cmp.position, pos_cmp.size );
    reg.emplace_or_replace<Cmp::SpriteAnimation>( lava_cell_entt, 0, 0, true, "CRYPT.interior_lava", 0 );
    if ( pathfinding_navmesh ) { pathfinding_navmesh->remove( pos_entt, pos_cmp ); }
    // reg.emplace<Cmp::ZOrderValue>( lava_cell_entt, pos_cmp.size.y );
  }
}

void destroy_crypt_lava_pit( entt::registry &reg, entt::entity entt,
                             std::shared_ptr<ProceduralMaze::PathFinding::SpatialHashGrid> pathfinding_navmesh )
{
  auto lava_pit_cmp = reg.try_get<Cmp::CryptRoomLavaPit>( entt );
  if ( lava_pit_cmp )
  {
    for ( auto [lava_cell_entt, lava_cell_cmp] : reg.view<Cmp::CryptRoomLavaPitCell>().each() )
    {
      if ( not lava_cell_cmp.findIntersection( *lava_pit_cmp ) ) continue;
      if ( pathfinding_navmesh ) { pathfinding_navmesh->remove( lava_cell_entt, lava_cell_cmp ); }
      reg.destroy( lava_cell_entt );
    }
  }
  reg.destroy( entt );
}

void add_spike_trap( entt::registry &reg, const entt::entity entt, const int passage_id )
{
  auto pblock_cmp = reg.try_get<Cmp::CryptPassageBlock>( entt );
  if ( not pblock_cmp ) return;
  sf::Vector2f position = *pblock_cmp;

  auto spike_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( spike_entt, position, Constants::kGridSizePxF );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( spike_entt, 0, 0, false, "CRYPT.interior_spiketrap", 0 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( spike_entt, position.y - 16.f ); // always behind player
  reg.emplace_or_replace<Cmp::CryptPassageSpikeTrap>( spike_entt, position, passage_id );
}

} // namespace ProceduralMaze::Factory