#ifndef SRC_SYSTEMS_SYSTEMSTORE_HPP_
#define SRC_SYSTEMS_SYSTEMSTORE_HPP_

#include <SceneControl/SceneInputRouter.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CollisionSystem.hpp>
#include <Systems/CryptSystem.hpp>
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
    SceneInputRouter,
    RenderGameSystem,
    PersistentSystem,
    PlayerSystem,
    PathFindSystem,
    NpcSystem,
    CollisionSystem,
    DiggingSystem,
    RenderOverlaySystem,
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
    CellAutomataSystem,
    CryptSystem,
  };

  SystemStore( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank, entt::dispatcher &nav_event_dispatcher,
               entt::dispatcher &scenemanager_event_dispatcher )
  {
    // clang-format off
    m_sysmap.emplace( Type::RenderMenuSystem, std::make_unique<RenderMenuSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::SceneInputRouter, std::make_unique<SceneInputRouter>( m_initial_reg, window, sprite_factory, sound_bank, nav_event_dispatcher, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::RenderGameSystem, std::make_unique<RenderGameSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::PersistentSystem, std::make_unique<PersistentSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::PlayerSystem, std::make_unique<PlayerSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::PathFindSystem, std::make_unique<PathFindSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::NpcSystem, std::make_unique<NpcSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CollisionSystem, std::make_unique<CollisionSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::DiggingSystem, std::make_unique<DiggingSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderOverlaySystem, std::make_unique<RenderOverlaySystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::BombSystem, std::make_unique<BombSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::AnimSystem, std::make_unique<AnimSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::SinkHoleHazardSystem, std::make_unique<SinkHoleHazardSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CorruptionHazardSystem, std::make_unique<CorruptionHazardSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::WormholeSystem, std::make_unique<WormholeSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::ExitSystem, std::make_unique<ExitSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::FootstepSystem, std::make_unique<FootstepSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::LargeObstacleSystem, std::make_unique<LargeObstacleSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::LootSystem, std::make_unique<LootSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RandomLevelGenerator, std::make_unique<ProcGen::RandomLevelGenerator>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CellAutomataSystem, std::make_unique<ProcGen::CellAutomataSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CryptSystem, std::make_unique<ProceduralMaze::Sys::CryptSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    // clang-format on
  }

  template <Type T>
  auto &find()
  {
    if constexpr ( T == Type::RenderGameSystem ) { return static_cast<RenderGameSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::SceneInputRouter ) { return static_cast<SceneInputRouter &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::RenderMenuSystem ) { return static_cast<RenderMenuSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::PersistentSystem ) { return static_cast<PersistentSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::PlayerSystem ) { return static_cast<PlayerSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::PathFindSystem ) { return static_cast<PathFindSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::NpcSystem ) { return static_cast<NpcSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::CollisionSystem ) { return static_cast<CollisionSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::DiggingSystem ) { return static_cast<DiggingSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::RenderOverlaySystem ) { return static_cast<RenderOverlaySystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::BombSystem ) { return static_cast<BombSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::AnimSystem ) { return static_cast<AnimSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::SinkHoleHazardSystem ) { return static_cast<SinkHoleHazardSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::CorruptionHazardSystem ) { return static_cast<CorruptionHazardSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::WormholeSystem ) { return static_cast<WormholeSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::ExitSystem ) { return static_cast<ExitSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::FootstepSystem ) { return static_cast<FootstepSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::LargeObstacleSystem ) { return static_cast<LargeObstacleSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::LootSystem ) { return static_cast<LootSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::RandomLevelGenerator ) { return static_cast<ProcGen::RandomLevelGenerator &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::CellAutomataSystem ) { return static_cast<ProcGen::CellAutomataSystem &>( *m_sysmap[T] ); }
    else if constexpr ( T == Type::CryptSystem ) { return static_cast<ProceduralMaze::Sys::CryptSystem &>( *m_sysmap[T] ); }
    // ... add other systems as needed
    else { static_assert( false, "Unknown system type" ); }
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
  entt::registry m_initial_reg; // Temporary registry for initialization
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_SYSTEMSTORE_HPP_