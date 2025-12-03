#ifndef SRC_SYSTEMS_SYSTEMSTORE_HPP_
#define SRC_SYSTEMS_SYSTEMSTORE_HPP_

#include <SceneControl/SceneInputRouter.hpp>
#include <Systems/AltarSystem.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/GraveSystem.hpp>
#include <Systems/LootSystem.hpp>
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
#include <stdexcept>
#include <string>

namespace ProceduralMaze::Sys
{

class SystemStore
{
public:
  enum class Type
  {
    AnimSystem,
    AltarSystem,
    BombSystem,
    CellAutomataSystem,
    CorruptionHazardSystem,
    CryptSystem,
    DiggingSystem,
    ExitSystem,
    FootstepSystem,
    GraveSystem,
    LootSystem,
    NpcSystem,
    PersistentSystem,
    PlayerSystem,
    RandomLevelGenerator,
    RenderGameSystem,
    RenderMenuSystem,
    RenderOverlaySystem,
    SceneInputRouter,
    SinkHoleHazardSystem,
    WormholeSystem,
  };

  // System type traits - explicit specializations
  template <Type T>
  struct SystemTraits;

  SystemStore( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank, entt::dispatcher &nav_event_dispatcher,
               entt::dispatcher &scenemanager_event_dispatcher )
  {
    // clang-format off
    m_sysmap.emplace( Type::AltarSystem, std::make_unique<AltarSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::AnimSystem, std::make_unique<AnimSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::BombSystem, std::make_unique<BombSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CellAutomataSystem, std::make_unique<ProcGen::CellAutomataSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CorruptionHazardSystem, std::make_unique<CorruptionHazardSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CryptSystem, std::make_unique<ProceduralMaze::Sys::CryptSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::DiggingSystem, std::make_unique<DiggingSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::ExitSystem, std::make_unique<ExitSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::FootstepSystem, std::make_unique<FootstepSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::GraveSystem, std::make_unique<GraveSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::LootSystem, std::make_unique<LootSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::NpcSystem, std::make_unique<NpcSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::PlayerSystem, std::make_unique<PlayerSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::PersistentSystem, std::make_unique<PersistentSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RandomLevelGenerator, std::make_unique<ProcGen::RandomLevelGenerator>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderGameSystem, std::make_unique<RenderGameSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderMenuSystem, std::make_unique<RenderMenuSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderOverlaySystem, std::make_unique<RenderOverlaySystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::SinkHoleHazardSystem, std::make_unique<SinkHoleHazardSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::SceneInputRouter, std::make_unique<SceneInputRouter>( m_initial_reg, window, sprite_factory, sound_bank, nav_event_dispatcher, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::WormholeSystem, std::make_unique<WormholeSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    // clang-format on
  }

  template <Type T>
  auto &find()
  {
    using SystemType = typename SystemTraits<T>::type;

    auto it = m_sysmap.find( T );
    if ( it == m_sysmap.end() ) { throw std::runtime_error( "System not found in store: " + std::to_string( static_cast<int>( T ) ) ); }
    if ( !it->second ) { throw std::runtime_error( "System pointer is null: " + std::to_string( static_cast<int>( T ) ) ); }

    return static_cast<SystemType &>( *it->second );
  }

  auto begin() { return m_sysmap.begin(); }
  auto end() { return m_sysmap.end(); }
  auto size() const { return m_sysmap.size(); }

private:
  std::map<Type, std::unique_ptr<BaseSystem>> m_sysmap;
  entt::registry m_initial_reg; // Temporary registry for initialization
};

// Explicit template specializations for SystemTraits
// clang-format off
  template<> struct SystemStore::SystemTraits<SystemStore::Type::AltarSystem>            { using type = AltarSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::AnimSystem>             { using type = AnimSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::BombSystem>             { using type = BombSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::CellAutomataSystem>     { using type = ProcGen::CellAutomataSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::CorruptionHazardSystem> { using type = CorruptionHazardSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::CryptSystem>            { using type = ProceduralMaze::Sys::CryptSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::DiggingSystem>          { using type = DiggingSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::ExitSystem>             { using type = ExitSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::FootstepSystem>         { using type = FootstepSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::GraveSystem>           { using type = GraveSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::LootSystem>             { using type = LootSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::NpcSystem>              { using type = NpcSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::PersistentSystem>       { using type = PersistentSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::PlayerSystem>           { using type = PlayerSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::RandomLevelGenerator>   { using type = ProcGen::RandomLevelGenerator; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::RenderGameSystem>       { using type = RenderGameSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::RenderOverlaySystem>    { using type = RenderOverlaySystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::RenderMenuSystem>       { using type = RenderMenuSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::SceneInputRouter>       { using type = SceneInputRouter; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::SinkHoleHazardSystem>   { using type = SinkHoleHazardSystem; };
  template<> struct SystemStore::SystemTraits<SystemStore::Type::WormholeSystem>         { using type = WormholeSystem; };
// clang-format on

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_SYSTEMSTORE_HPP_