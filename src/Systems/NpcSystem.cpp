#include <NpcSystem.hpp>
#include <SpriteAnimation.hpp>

namespace ProceduralMaze::Sys {

NpcSystem::NpcSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  std::ignore = Sys::BaseSystem::getEventDispatcher()
                    .sink<Events::NpcCreationEvent>()
                    .connect<&Sys::NpcSystem::on_npc_creation>( this );
  std::ignore = Sys::BaseSystem::getEventDispatcher()
                    .sink<Events::NpcDeathEvent>()
                    .connect<&Sys::NpcSystem::on_npc_death>( this );
}

void NpcSystem::add_npc_entity( sf::Vector2f position )
{
  auto new_npc_entity = m_reg->create();
  m_reg->emplace<Cmp::NPC>( new_npc_entity );
  m_reg->emplace<Cmp::Direction>( new_npc_entity, sf::Vector2f{ 0, 0 } );
  m_reg->emplace<Cmp::Position>( new_npc_entity, position );
  auto &npc_scan_scale = get_persistent_component<Cmp::Persistent::NpcScanScale>();
  m_reg->emplace<Cmp::NPCScanBounds>(
      new_npc_entity, position, sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions }, npc_scan_scale() );

  m_reg->emplace<Cmp::SpriteAnimation>( new_npc_entity, 10, sf::seconds( 0.1f ) );

  SPDLOG_DEBUG( "Creating NPC entity {} at position ({}, {})", static_cast<int>( new_npc_entity ), position.x,
                position.y );
}

void NpcSystem::remove_npc_entity( entt::entity npc_entity )
{
  // kill npc
  m_reg->remove<Cmp::NPC>( npc_entity );
  m_reg->remove<Cmp::Position>( npc_entity );
  m_reg->remove<Cmp::NPCScanBounds>( npc_entity );
  m_reg->remove<Cmp::SpriteAnimation>( npc_entity );
  m_reg->remove<Cmp::Direction>( npc_entity );
  SPDLOG_DEBUG( "NPC entity {} killed by explosion", static_cast<int>( npc_entity ) );
}

void NpcSystem::lerp_movement( sf::Time dt )
{
  auto view = m_reg->view<Cmp::Position, Cmp::LerpPosition, Cmp::NPCScanBounds>();

  for ( auto [entity, pos, target, npc_scan_bounds] : view.each() )
  {
    // If this is the first update, store the start position
    if ( target.m_lerp_factor == 0.0f ) { target.m_start = pos; }

    target.m_lerp_factor += target.m_lerp_speed * dt.asSeconds();

    if ( target.m_lerp_factor >= 1.0f )
    {
      pos = target.m_target;
      m_reg->remove<Cmp::LerpPosition>( entity );
    }
    else
    {
      // Lerp from start to target directly
      pos.x = std::lerp( target.m_start.x, target.m_target.x, target.m_lerp_factor );
      pos.y = std::lerp( target.m_start.y, target.m_target.y, target.m_lerp_factor );
    }

    m_reg->patch<Cmp::NPCScanBounds>( entity, [&]( auto &npc_scan_bounds ) { npc_scan_bounds.position( pos ); } );
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
  add_npc_entity( event.position );
}

} // namespace ProceduralMaze::Sys