#ifndef __SYS_NPCSYSTEM_HPP__
#define __SYS_NPCSYSTEM_HPP__

#include <BaseSystem.hpp>
#include <Direction.hpp>
#include <Events/NpcCreationEvent.hpp>
#include <Events/NpcDeathEvent.hpp>
#include <LerpPosition.hpp>

#include <NPC.hpp>
#include <NPCScanBounds.hpp>
#include <Persistent/NpcActivateScale.hpp>
#include <Persistent/NpcScanScale.hpp>
#include <Position.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

class NpcSystem : public BaseSystem
{
public:
  NpcSystem( ProceduralMaze::SharedEnttRegistry reg );

  void add_npc_entity( entt::entity npc_entity );
  void remove_npc_entity( entt::entity npc_entity );

  void lerp_movement( sf::Time dt );

  void on_npc_death( const Events::NpcDeathEvent &event );
  void on_npc_creation( const Events::NpcCreationEvent &event );
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_NPCSYSTEM_HPP__
