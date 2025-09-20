#ifndef __SYS_NPCSYSTEM_HPP__
#define __SYS_NPCSYSTEM_HPP__

#include <BaseSystem.hpp>
#include <Direction.hpp>
#include <Events/NpcCreationEvent.hpp>
#include <Events/NpcDeathEvent.hpp>
#include <LerpPosition.hpp>
#include <Movement.hpp>
#include <NPC.hpp>
#include <NPCScanBounds.hpp>
#include <Persistent/NPCScanScale.hpp>
#include <Position.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class NpcSystem : public BaseSystem
{
public:
  NpcSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg )
      : BaseSystem( reg )
  {
  }

  void init_context()
  {
    if ( not m_reg->ctx().contains<Cmp::Persistent::NPCScanScale>() )
    {
      m_reg->ctx().emplace<Cmp::Persistent::NPCScanScale>();
    }
  }

  void add_npc_entity( sf::Vector2f position )
  {
    auto new_npc_entity = m_reg->create();
    m_reg->emplace<Cmp::NPC>( new_npc_entity, true );
    m_reg->emplace<Cmp::Position>( new_npc_entity, position );
    auto &npc_scan_scale = m_reg->ctx().get<Cmp::Persistent::NPCScanScale>();
    m_reg->emplace<Cmp::NPCScanBounds>( new_npc_entity, position,
                                        sf::Vector2f{ Sprites::MultiSprite::DEFAULT_SPRITE_SIZE }, npc_scan_scale() );
  }

  void remove_npc_entity( entt::entity npc_entity )
  {
    // kill npc
    m_reg->remove<Cmp::NPC>( npc_entity );
    m_reg->remove<Cmp::Position>( npc_entity );
    m_reg->remove<Cmp::NPCScanBounds>( npc_entity );
    SPDLOG_DEBUG( "NPC entity {} killed by explosion", static_cast<int>( npc_entity ) );
  }

  void lerp_movement( sf::Time dt )
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

  void on_npc_death( const Events::NpcDeathEvent &event )
  {
    SPDLOG_DEBUG( "NPC Death Event received" );
    remove_npc_entity( event.npc_entity );
  }
  void on_npc_creation( const Events::NpcCreationEvent &event )
  {
    SPDLOG_DEBUG( "NPC Creation Event received" );
    add_npc_entity( event.position );
  }
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_NPCSYSTEM_HPP__
