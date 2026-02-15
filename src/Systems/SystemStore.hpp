#ifndef SRC_SYSTEMS_SYSTEMSTORE_HPP_
#define SRC_SYSTEMS_SYSTEMSTORE_HPP_

#include <Systems/RuinSystem.hpp>

#include <entt/fwd.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

// Forward declarations only
// clang-format off
namespace ProceduralMaze::Sprites { class SpriteFactory; }
namespace ProceduralMaze::Audio { class SoundBank; }
namespace sf { class RenderWindow; }
namespace ProceduralMaze::Sys::ProcGen { class CellAutomataSystem; class RandomLevelGenerator; }
// clang-format on
namespace ProceduralMaze::Sys
{

// Forward declare all system classes
class BaseSystem;
class AnimSystem;
class AltarSystem;
class BombSystem;
class CorruptionHazardSystem;
class CryptSystem;
class DiggingSystem;
class ExitSystem;
class FootstepSystem;
class GraveSystem;
class HolyWellSystem;
class LootSystem;
class NpcSystem;
class PassageSystem;
class PersistSystem;
class PlayerSystem;
class RenderGameSystem;
class RenderMenuSystem;
class RenderOverlaySystem;
class SceneInputRouter;
class SinkHoleHazardSystem;
class ShockwaveSystem;
class WormholeSystem;

class SystemStore
{
public:
  enum class Type {
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
    HolyWellSystem,
    RuinSystem,
    LootSystem,
    NpcSystem,
    PassageSystem,
    PersistSystem,
    PlayerSystem,
    RandomLevelGenerator,
    RenderGameSystem,
    RenderMenuSystem,
    RenderOverlaySystem,
    SceneInputRouter,
    ShockwaveSystem,
    SinkHoleHazardSystem,
    WormholeSystem,
  };

  // System type traits - explicit specializations
  template <Type T>
  struct SystemTraits;

  SystemStore( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank, entt::dispatcher &nav_event_dispatcher,
               entt::dispatcher &scenemanager_event_dispatcher );

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
template<> struct SystemStore::SystemTraits<SystemStore::Type::GraveSystem>            { using type = GraveSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::HolyWellSystem>         { using type = HolyWellSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::RuinSystem>         { using type = RuinSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::LootSystem>             { using type = LootSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::NpcSystem>              { using type = NpcSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::PassageSystem>          { using type = ProceduralMaze::Sys::PassageSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::PersistSystem>          { using type = PersistSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::PlayerSystem>           { using type = PlayerSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::RandomLevelGenerator>   { using type = ProcGen::RandomLevelGenerator; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::RenderGameSystem>       { using type = RenderGameSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::RenderOverlaySystem>    { using type = RenderOverlaySystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::RenderMenuSystem>       { using type = RenderMenuSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::SceneInputRouter>       { using type = SceneInputRouter; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::ShockwaveSystem>        { using type = ShockwaveSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::SinkHoleHazardSystem>   { using type = SinkHoleHazardSystem; };
template<> struct SystemStore::SystemTraits<SystemStore::Type::WormholeSystem>         { using type = WormholeSystem; };
// clang-format on

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_SYSTEMSTORE_HPP_