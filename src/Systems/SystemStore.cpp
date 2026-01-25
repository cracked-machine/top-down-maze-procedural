#include <Systems/HolyWellSystem.hpp>
#include <Systems/SystemStore.hpp>

#include <SceneControl/SceneInputRouter.hpp>
#include <Systems/AltarSystem.hpp>
#include <Systems/AnimSystem.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/CryptSystem.hpp>
#include <Systems/DiggingSystem.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/FootstepSystem.hpp>
#include <Systems/GraveSystem.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/LootSystem.hpp>
#include <Systems/PassageSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PlayerSystem.hpp>
#include <Systems/ProcGen/CellAutomataSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Systems/Threats/HazardFieldSystemImpl.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/ShockwaveSystem.hpp>
#include <Systems/Threats/WormholeSystem.hpp>

namespace ProceduralMaze::Sys
{

SystemStore::SystemStore( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                          entt::dispatcher &nav_event_dispatcher, entt::dispatcher &scenemanager_event_dispatcher )
{
  // clang-format off
    m_sysmap.emplace( Type::AltarSystem, std::make_unique<AltarSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::AnimSystem, std::make_unique<AnimSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::BombSystem, std::make_unique<BombSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CellAutomataSystem, std::make_unique<ProcGen::CellAutomataSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CorruptionHazardSystem, std::make_unique<CorruptionHazardSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::CryptSystem, std::make_unique<ProceduralMaze::Sys::CryptSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::DiggingSystem, std::make_unique<DiggingSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::ExitSystem, std::make_unique<ExitSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::FootstepSystem, std::make_unique<FootstepSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::GraveSystem, std::make_unique<GraveSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::HolyWellSystem, std::make_unique<HolyWellSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::RuinSystem, std::make_unique<RuinSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::LootSystem, std::make_unique<LootSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::NpcSystem, std::make_unique<NpcSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::PassageSystem, std::make_unique<PassageSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::PlayerSystem, std::make_unique<PlayerSystem>( m_initial_reg, window, sprite_factory, sound_bank, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::PersistSystem, std::make_unique<PersistSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RandomLevelGenerator, std::make_unique<ProcGen::RandomLevelGenerator>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderGameSystem, std::make_unique<RenderGameSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderMenuSystem, std::make_unique<RenderMenuSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::RenderOverlaySystem, std::make_unique<RenderOverlaySystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::SceneInputRouter, std::make_unique<SceneInputRouter>( m_initial_reg, window, sprite_factory, sound_bank, nav_event_dispatcher, scenemanager_event_dispatcher ) );
    m_sysmap.emplace( Type::ShockwaveSystem, std::make_unique<ShockwaveSystem>( m_initial_reg, window, sprite_factory, sound_bank) );
    m_sysmap.emplace( Type::SinkHoleHazardSystem, std::make_unique<SinkHoleHazardSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
    m_sysmap.emplace( Type::WormholeSystem, std::make_unique<WormholeSystem>( m_initial_reg, window, sprite_factory, sound_bank ) );
  // clang-format on
}

} // namespace ProceduralMaze::Sys