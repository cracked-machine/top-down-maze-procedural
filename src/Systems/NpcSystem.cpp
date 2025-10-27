#include <Destructable.hpp>
#include <GraveSprite.hpp>
#include <NpcContainer.hpp>
#include <NpcSystem.hpp>
#include <Obstacle.hpp>
#include <PlayableCharacter.hpp>
#include <Random.hpp>
#include <ReservedPosition.hpp>
#include <ShrineSprite.hpp>
#include <SpawnAreaSprite.hpp>
#include <SpriteAnimation.hpp>

namespace ProceduralMaze::Sys {

NpcSystem::NpcSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window )
    : BaseSystem( reg, window )
{
  std::ignore = Sys::BaseSystem::getEventDispatcher()
                    .sink<Events::NpcCreationEvent>()
                    .connect<&Sys::NpcSystem::on_npc_creation>( this );
  std::ignore = Sys::BaseSystem::getEventDispatcher()
                    .sink<Events::NpcDeathEvent>()
                    .connect<&Sys::NpcSystem::on_npc_death>( this );
}

void NpcSystem::add_npc_entity( entt::entity position_entity )
{
  auto pos_cmp = m_reg->try_get<Cmp::Position>( position_entity );
  if ( not pos_cmp )
  {
    SPDLOG_ERROR( "Cannot add NPC entity {} without a Position component", static_cast<int>( position_entity ) );
    return;
  }

  auto npc_ms = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>()->get_multisprite_by_type( "NPC" );
  Cmp::RandomInt npc_sprite_tile_picker( 0, npc_ms->get_sprite_count() - 1 );
  auto tile_pick = npc_sprite_tile_picker.gen();
  SPDLOG_INFO( "Creating NPC: {} of {} possible sprites", tile_pick, npc_ms->get_sprite_count() );

  // create a new entity for the NPC using the existing position
  auto new_pos_entity = m_reg->create();
  m_reg->emplace<Cmp::Position>( new_pos_entity, pos_cmp->position, kGridSquareSizePixelsF );
  m_reg->emplace<Cmp::Destructable>( new_pos_entity );
  m_reg->emplace_or_replace<Cmp::NPC>( new_pos_entity, "NPC", tile_pick );
  m_reg->emplace_or_replace<Cmp::Direction>( new_pos_entity, sf::Vector2f{ 0, 0 } );
  auto &npc_scan_scale = get_persistent_component<Cmp::Persistent::NpcScanScale>();
  m_reg->emplace_or_replace<Cmp::NPCScanBounds>( new_pos_entity, pos_cmp->position, kGridSquareSizePixelsF,
                                                 npc_scan_scale.get_value() );
  m_reg->emplace_or_replace<Cmp::SpriteAnimation>( new_pos_entity );
  m_reg->remove<Cmp::NpcContainer>( new_pos_entity );
  m_reg->remove<Cmp::ReservedPosition>( new_pos_entity );
  m_reg->remove<Cmp::Obstacle>( new_pos_entity );

  // Remove the npc container component from the original entity
  m_reg->remove<Cmp::NpcContainer>( position_entity );
  m_reg->remove<Cmp::NpcContainer>( position_entity );
}

void NpcSystem::remove_npc_entity( entt::entity npc_entity )
{
  // kill npc
  m_reg->remove<Cmp::NPC>( npc_entity );
  m_reg->remove<Cmp::Position>( npc_entity );
  m_reg->remove<Cmp::NPCScanBounds>( npc_entity );
  m_reg->remove<Cmp::Direction>( npc_entity );
  SPDLOG_DEBUG( "NPC entity {} killed by explosion", static_cast<int>( npc_entity ) );
}

void NpcSystem::update_movement( sf::Time dt )
{
  auto exclusions = entt::exclude<Cmp::ShrineSprite, Cmp::SpawnAreaSprite, Cmp::PlayableCharacter>;
  auto view = m_reg->view<Cmp::Position, Cmp::LerpPosition, Cmp::NPCScanBounds>( exclusions );

  for ( auto [entity, pos_cmp, lerp_pos_cmp, npc_scan_bounds] : view.each() )
  {
    // skip over obstacles that are still enabled i.e. dont travel though them
    auto obst_cmp = m_reg->try_get<Cmp::Obstacle>( entity );
    if ( obst_cmp && obst_cmp->m_enabled ) continue;

    // If this is the first update, store the start position
    if ( lerp_pos_cmp.m_lerp_factor == 0.0f ) { lerp_pos_cmp.m_start = pos_cmp.position; }

    lerp_pos_cmp.m_lerp_factor += lerp_pos_cmp.m_lerp_speed * dt.asSeconds();

    if ( lerp_pos_cmp.m_lerp_factor >= 1.0f )
    {
      pos_cmp.position = lerp_pos_cmp.m_target;
      m_reg->remove<Cmp::LerpPosition>( entity );
    }
    else
    {
      // Lerp from start to target directly
      pos_cmp.position.x = std::lerp( lerp_pos_cmp.m_start.x, lerp_pos_cmp.m_target.x, lerp_pos_cmp.m_lerp_factor );
      pos_cmp.position.y = std::lerp( lerp_pos_cmp.m_start.y, lerp_pos_cmp.m_target.y, lerp_pos_cmp.m_lerp_factor );
    }

    // clang-format off
    m_reg->patch<Cmp::NPCScanBounds>( entity, 
      [&]( auto &npc_scan_bounds ) 
      { 
        npc_scan_bounds.position( pos_cmp.position ); 
      });
    // clang-format on
  }
}

void NpcSystem::on_npc_death( const Events::NpcDeathEvent &event )
{
  SPDLOG_DEBUG( "NPC Death Event received" );
  remove_npc_entity( event.npc_entity );
}
void NpcSystem::on_npc_creation( const Events::NpcCreationEvent &event )
{
  SPDLOG_DEBUG( "NPC Creation Event received" );
  add_npc_entity( event.position_entity );
}

} // namespace ProceduralMaze::Sys