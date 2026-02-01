// #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptLever.hpp>
#include <Components/HolyWell/HolyWellSegment.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Systems/BaseSystem.hpp>

#include <Audio/SoundBank.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Wall.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Utils/Utils.hpp>

#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sys
{

BaseSystem::BaseSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : m_reg( reg ),
      m_window( window ),
      m_sprite_factory( sprite_factory ),
      m_sound_bank( sound_bank )
{
  SPDLOG_DEBUG( "BaseSystem constructor called" );
}

// initialised by first call to getEventDispatcher()
std::unique_ptr<entt::dispatcher> BaseSystem::m_systems_event_queue = nullptr;

} // namespace ProceduralMaze::Sys