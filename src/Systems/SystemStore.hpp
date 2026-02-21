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

class Store
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

  Store( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank, entt::dispatcher &nav_event_dispatcher,
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
template<> struct Store::SystemTraits<Store::Type::AltarSystem>            { using type = AltarSystem; };
template<> struct Store::SystemTraits<Store::Type::AnimSystem>             { using type = AnimSystem; };
template<> struct Store::SystemTraits<Store::Type::BombSystem>             { using type = BombSystem; };
template<> struct Store::SystemTraits<Store::Type::CellAutomataSystem>     { using type = ProcGen::CellAutomataSystem; };
template<> struct Store::SystemTraits<Store::Type::CorruptionHazardSystem> { using type = CorruptionHazardSystem; };
template<> struct Store::SystemTraits<Store::Type::CryptSystem>            { using type = ProceduralMaze::Sys::CryptSystem; };
template<> struct Store::SystemTraits<Store::Type::DiggingSystem>          { using type = DiggingSystem; };
template<> struct Store::SystemTraits<Store::Type::ExitSystem>             { using type = ExitSystem; };
template<> struct Store::SystemTraits<Store::Type::FootstepSystem>         { using type = FootstepSystem; };
template<> struct Store::SystemTraits<Store::Type::GraveSystem>            { using type = GraveSystem; };
template<> struct Store::SystemTraits<Store::Type::HolyWellSystem>         { using type = HolyWellSystem; };
template<> struct Store::SystemTraits<Store::Type::RuinSystem>         { using type = RuinSystem; };
template<> struct Store::SystemTraits<Store::Type::LootSystem>             { using type = LootSystem; };
template<> struct Store::SystemTraits<Store::Type::NpcSystem>              { using type = NpcSystem; };
template<> struct Store::SystemTraits<Store::Type::PassageSystem>          { using type = ProceduralMaze::Sys::PassageSystem; };
template<> struct Store::SystemTraits<Store::Type::PersistSystem>          { using type = PersistSystem; };
template<> struct Store::SystemTraits<Store::Type::PlayerSystem>           { using type = PlayerSystem; };
template<> struct Store::SystemTraits<Store::Type::RandomLevelGenerator>   { using type = ProcGen::RandomLevelGenerator; };
template<> struct Store::SystemTraits<Store::Type::RenderGameSystem>       { using type = RenderGameSystem; };
template<> struct Store::SystemTraits<Store::Type::RenderOverlaySystem>    { using type = RenderOverlaySystem; };
template<> struct Store::SystemTraits<Store::Type::RenderMenuSystem>       { using type = RenderMenuSystem; };
template<> struct Store::SystemTraits<Store::Type::SceneInputRouter>       { using type = SceneInputRouter; };
template<> struct Store::SystemTraits<Store::Type::ShockwaveSystem>        { using type = ShockwaveSystem; };
template<> struct Store::SystemTraits<Store::Type::SinkHoleHazardSystem>   { using type = SinkHoleHazardSystem; };
template<> struct Store::SystemTraits<Store::Type::WormholeSystem>         { using type = WormholeSystem; };
// clang-format on

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_SYSTEMSTORE_HPP_