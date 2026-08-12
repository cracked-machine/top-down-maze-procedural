#ifndef SRC_SYSTEMS_STORES_SYSTEMSTORE_HPP__
#define SRC_SYSTEMS_STORES_SYSTEMSTORE_HPP__

#include <entt/fwd.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

// Forward declarations only
// clang-format off
namespace Game::Sprites { class SpriteFactory; }
namespace Game::Audio { class SoundBank; }
namespace sf { class RenderWindow; }
namespace Game::Sys::ProcGen { class CellAutomataSystem; class DLASystem; class LevelGenerator; }
// clang-format on

namespace Game::Sys
{

// Forward declare all system classes
class BaseSystem;
class AnimSystem;
class ArrowSystem;
class AltarSystem;
class BombSystem;
class CorruptionHazardSystem;
class CryptSystem;
class ActionSystem;
class ExitSystem;
class FootstepSystem;
class GraveSystem;
class GrimoireSystem;
class HealingSpringSystem;
class ItemStore;
class ItemSystem;
class NpcStore;
class LootSystem;
class NpcSystem;
class ParticleSystem;
class PassageSystem;
class PersistSystem;
class PlayerSystem;
class RenderGameSystem;
class RenderMenuSystem;
class RenderOverlaySystem;
class SceneInputRouter;
class SinkHoleHazardSystem;
class ShaderSystem;
class ShockwaveSystem;
class ShopSystem;
class LightningSystem;
class RuinSystem;
class WormholeSystem;

class Store
{
public:
  //! @brief Key for the SystemStore concrete classes
  enum class Type {
    AnimSystem,
    AltarSystem,
    ArrowSystem,
    BombSystem,
    CellAutomataSystem,
    CorruptionHazardSystem,
    CryptSystem,
    DiffusionLtdAggrSystem,
    ActionSystem,
    ExitSystem,
    FootstepSystem,
    GraveSystem,
    GrimoireSystem,
    HealingSpringSystem,
    ItemStore,
    ItemSystem,
    NpcStore,
    RuinSystem,
    LightningSystem,
    LootSystem,
    NpcSystem,
    ParticleSystem,
    PassageSystem,
    PersistSystem,
    PlayerSystem,
    LevelGenerator,
    RenderGameSystem,
    RenderMenuSystem,
    RenderOverlaySystem,
    SceneInputRouter,
    ShaderSystem,
    ShockwaveSystem,
    ShopSystem,
    SinkHoleHazardSystem,
    WormholeSystem,
  };

  //! @brief System type traits - Maps each Sys::Store::Type enum value to its concrete class.
  //! @tparam T
  template <Type T>
  struct SystemTraits;

  //! @brief Construct a new Store object
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  //! @param nav_event_dispatcher
  //! @param scenemanager_event_dispatcher
  Store( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank, entt::dispatcher &nav_event_dispatcher,
         entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief Find the concrete class using the Sys::Store::Type enum
  //! @tparam T
  //! @return auto&
  template <Type T>
  auto &find()
  {
    using SystemType = typename SystemTraits<T>::type;

    auto it = m_sysmap.find( T );
    if ( it == m_sysmap.end() ) { throw std::runtime_error( "System not found in store: " + std::to_string( static_cast<int>( T ) ) ); }
    if ( !it->second ) { throw std::runtime_error( "System pointer is null: " + std::to_string( static_cast<int>( T ) ) ); }

    return static_cast<SystemType &>( *it->second );
  }

  //! @brief Get the first element of the SystemStore
  auto begin() { return m_sysmap.begin(); }

  //! @brief Get the last element of the SystemStore
  auto end() { return m_sysmap.end(); }

  //! @brief Get the size of the SystemStore
  [[nodiscard]] auto size() const { return m_sysmap.size(); }

private:
  //! @brief The SystemStore container keyed by Type enum
  std::map<Type, std::unique_ptr<BaseSystem>> m_sysmap;

  //! @brief Empty registry for initialization
  entt::registry m_initial_reg;

  //! @brief Wrapper function that ensures matching pairs are emplaced into SystemStore.
  //! @tparam T
  //! @tparam Args
  //! @param args
  template <Type T, typename... Args>
  void emplace( Args &&...args )
  {
    using SystemType = typename SystemTraits<T>::type;
    m_sysmap.emplace( T, std::make_unique<SystemType>( std::forward<Args>( args )... ) );
  }
};

// Explicit template specializations for SystemTraits
// clang-format off
template<> struct Store::SystemTraits<Store::Type::ActionSystem>           { using type = ActionSystem; };
template<> struct Store::SystemTraits<Store::Type::AltarSystem>            { using type = AltarSystem; };
template<> struct Store::SystemTraits<Store::Type::AnimSystem>             { using type = AnimSystem; };
template<> struct Store::SystemTraits<Store::Type::ArrowSystem>            { using type = ArrowSystem; };
template<> struct Store::SystemTraits<Store::Type::BombSystem>             { using type = BombSystem; };
template<> struct Store::SystemTraits<Store::Type::CellAutomataSystem>     { using type = ProcGen::CellAutomataSystem; };
template<> struct Store::SystemTraits<Store::Type::CorruptionHazardSystem> { using type = CorruptionHazardSystem; };
template<> struct Store::SystemTraits<Store::Type::CryptSystem>            { using type = Game::Sys::CryptSystem; };
template<> struct Store::SystemTraits<Store::Type::DiffusionLtdAggrSystem> { using type = ProcGen::DLASystem; };
template<> struct Store::SystemTraits<Store::Type::ExitSystem>             { using type = ExitSystem; };
template<> struct Store::SystemTraits<Store::Type::FootstepSystem>         { using type = FootstepSystem; };
template<> struct Store::SystemTraits<Store::Type::GraveSystem>            { using type = GraveSystem; };
template<> struct Store::SystemTraits<Store::Type::GrimoireSystem>         { using type = GrimoireSystem; };
template<> struct Store::SystemTraits<Store::Type::HealingSpringSystem>    { using type = HealingSpringSystem; };
template<> struct Store::SystemTraits<Store::Type::ItemStore>              { using type = ItemStore; };
template<> struct Store::SystemTraits<Store::Type::ItemSystem>             { using type = ItemSystem; };
template<> struct Store::SystemTraits<Store::Type::NpcStore>               { using type = NpcStore; };
template<> struct Store::SystemTraits<Store::Type::RuinSystem>             { using type = RuinSystem; };
template<> struct Store::SystemTraits<Store::Type::LootSystem>             { using type = LootSystem; };
template<> struct Store::SystemTraits<Store::Type::LightningSystem>        { using type = LightningSystem; };
template<> struct Store::SystemTraits<Store::Type::NpcSystem>              { using type = NpcSystem; };
template<> struct Store::SystemTraits<Store::Type::ParticleSystem>         { using type = ParticleSystem; };
template<> struct Store::SystemTraits<Store::Type::PassageSystem>          { using type = Game::Sys::PassageSystem; };
template<> struct Store::SystemTraits<Store::Type::PersistSystem>          { using type = PersistSystem; };
template<> struct Store::SystemTraits<Store::Type::PlayerSystem>           { using type = PlayerSystem; };
template<> struct Store::SystemTraits<Store::Type::LevelGenerator>         { using type = ProcGen::LevelGenerator; };
template<> struct Store::SystemTraits<Store::Type::RenderGameSystem>       { using type = RenderGameSystem; };
template<> struct Store::SystemTraits<Store::Type::RenderOverlaySystem>    { using type = RenderOverlaySystem; };
template<> struct Store::SystemTraits<Store::Type::RenderMenuSystem>       { using type = RenderMenuSystem; };
template<> struct Store::SystemTraits<Store::Type::SceneInputRouter>       { using type = SceneInputRouter; };
template<> struct Store::SystemTraits<Store::Type::ShaderSystem>           { using type = ShaderSystem; };
template<> struct Store::SystemTraits<Store::Type::ShockwaveSystem>        { using type = ShockwaveSystem; };
template<> struct Store::SystemTraits<Store::Type::ShopSystem>             { using type = ShopSystem; };
template<> struct Store::SystemTraits<Store::Type::SinkHoleHazardSystem>   { using type = SinkHoleHazardSystem; };
template<> struct Store::SystemTraits<Store::Type::WormholeSystem>         { using type = WormholeSystem; };
// clang-format on

} // namespace Game::Sys

#endif // SRC_SYSTEMS_STORES_SYSTEMSTORE_HPP__
