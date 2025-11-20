#ifndef SRC_SYSTEMSTORE_HPP_
#define SRC_SYSTEMSTORE_HPP_

#include <EventHandler.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CollisionSystem.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/LargeObstacleSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PathFindSystem.hpp>
#include <Systems/PersistentSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/Render/RenderPlayerSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/WormholeSystem.hpp>

#include <map>
#include <memory>

namespace ProceduralMaze::Sys
{

class SystemStore
{
public:
  enum class Type
  {
    RenderMenuSystem,
    EventHandler,
    RenderGameSystem,
    PersistentSystem,
    PlayerSystem,
    PathFindSystem,
    NpcSystem,
    CollisionSystem,
    DiggingSystem,
    RenderOverlaySystem,
    RenderPlayerSystem,
    BombSystem,
    AnimSystem,
    SinkHoleHazardSystem,
    CorruptionHazardSystem,
    WormholeSystem,
    ExitSystem,
    FootstepSystem,
    LargeObstacleSystem,
    LootSystem,
    RandomLevelGenerator,
    CellAutomataSystem
  };

  SystemStore( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
  {
    // clang-format off
    m_sysmap.emplace( Type::RenderMenuSystem, std::make_unique<RenderMenuSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::EventHandler, std::make_unique<EventHandler>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderGameSystem, std::make_unique<RenderGameSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::PersistentSystem, std::make_unique<PersistentSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::PlayerSystem, std::make_unique<PlayerSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::PathFindSystem, std::make_unique<PathFindSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::NpcSystem, std::make_unique<NpcSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CollisionSystem, std::make_unique<CollisionSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::DiggingSystem, std::make_unique<DiggingSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderOverlaySystem, std::make_unique<RenderOverlaySystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderPlayerSystem, std::make_unique<RenderPlayerSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::BombSystem, std::make_unique<BombSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::AnimSystem, std::make_unique<AnimSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::SinkHoleHazardSystem, std::make_unique<SinkHoleHazardSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CorruptionHazardSystem, std::make_unique<CorruptionHazardSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::WormholeSystem, std::make_unique<WormholeSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::ExitSystem, std::make_unique<ExitSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::FootstepSystem, std::make_unique<FootstepSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::LargeObstacleSystem, std::make_unique<LargeObstacleSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::LootSystem, std::make_unique<LootSystem>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RandomLevelGenerator, std::make_unique<ProcGen::RandomLevelGenerator>( window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CellAutomataSystem, std::make_unique<ProcGen::CellAutomataSystem>( window, sprite_factory, sound_bank ) );
    // clang-format on
  }

  BaseSystem &find( Type t )
  {
    if ( auto search = m_sysmap.find( t ); search != m_sysmap.end() ) { return *search->second; }
    throw std::runtime_error( "Unknown system type: " + std::to_string( static_cast<int>( t ) ) );
  }

  auto begin() { return m_sysmap.begin(); }
  auto end() { return m_sysmap.end(); }
  auto size() const { return m_sysmap.size(); }

private:
  std::map<Type, std::unique_ptr<BaseSystem>> m_sysmap;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMSTORE_HPP_