#include <Components/Position.hpp>
#include <Constants.hpp>
#include <FootStepTimer.hpp>
#include <Inventory/CarryItem.hpp>
#include <Npc/Npc.hpp>
#include <Pathfinding/NeighbourSearchArea.hpp>
#include <Player.hpp>
#include <Player/PlayerCharacter.hpp>
#include <RectBounds.hpp>
#include <Systems/PathSystem.hpp>
#include <Utils/Npc.hpp>

namespace ProceduralMaze::Sys
{

PathSystem::PathSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "PathSystem initialized" );
}

void PathSystem::update( [[maybe_unused]] sf::Time dt ) {}

} // namespace ProceduralMaze::Sys