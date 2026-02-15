#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Components/Persistent/CameraSmoothSpeed.hpp>
#include <Components/Persistent/CryptNpcSpawnCount.hpp>
#include <Components/Persistent/NpcShockwaveFreq.hpp>
#include <Components/Persistent/NpcShockwaveMaxRadius.hpp>
#include <Components/Persistent/NpcShockwaveResolution.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Components/Persistent/PlayerLerpInterruptThreshold.hpp>
#include <Systems/PersistSystem.hpp>

#define JSON_NOEXCEPTION
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/NpcGhostAnimFramerate.hpp>
#include <Components/Persistent/NpcLerpSpeed.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Components/Persistent/NpcSkeleAnimFramerate.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerAnimFramerate.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerFootstepAddDelay.hpp>
#include <Components/Persistent/PlayerFootstepFadeDelay.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/Persistent/WormholeSeed.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>

namespace ProceduralMaze::Sys
{

PersistSystem::PersistSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  std::ignore = get_systems_event_queue().sink<Events::SaveSettingsEvent>().connect<&Sys::PersistSystem::on_save_settings_event>( this );
  SPDLOG_DEBUG( "PersistSystem initialized" );
}

void PersistSystem::initializeTypeRegistry()
{
  using namespace Cmp::Persist;

  //! @brief Helper function for SerDe and for initializing the component in the registry.
  auto reg = [this]<typename T>( const char *name )
  {
    m_type_registry[name] = [this, name]( const nlohmann::json &json )
    {
      Sys::PersistSystem::add_persist_cmp<T>( getReg() );
      Sys::PersistSystem::get_persist_cmp<T>( getReg() ).deserialize( json );
      registerTypes<T>( name );
    };
  };

  // clang-format off
  reg.operator()<ArmedOffDelay>("ArmedOffDelay");
  reg.operator()<ArmedOnDelay>("ArmedOnDelay");
  reg.operator()<BlastRadius>("BlastRadius");
  reg.operator()<BombBonus>("BombBonus");
  reg.operator()<BombDamage>("BombDamage");
  reg.operator()<BombInventory>("BombInventory");
  reg.operator()<CameraSmoothSpeed>("CameraSmoothSpeed");
  reg.operator()<CorruptionDamage>("CorruptionDamage");
  reg.operator()<CryptNpcSpawnCount>("CryptNpcSpawnCount");
  reg.operator()<DiggingCooldownThreshold>("DiggingCooldownThreshold");
  reg.operator()<DiggingDamagePerHit>("DiggingDamagePerHit");
  reg.operator()<DisplayResolution>("DisplayResolution");
  reg.operator()<EffectsVolume>("EffectsVolume");
  reg.operator()<ExitKeyRequirement>("ExitKeyRequirement");
  reg.operator()<FuseDelay>("FuseDelay");
  reg.operator()<GraveNumMultiplier>("GraveNumMultiplier");
  reg.operator()<HealthBonus>("HealthBonus");
  reg.operator()<MaxNumAltars>("MaxNumAltars");
  reg.operator()<MaxNumCrypts>("MaxNumCrypts");
  reg.operator()<MusicVolume>("MusicVolume");
  reg.operator()<NpcActivateScale>("NpcActivateScale");
  reg.operator()<NpcDamage>("NpcDamage");
  reg.operator()<NpcDeathAnimFramerate>("NpcDeathAnimFramerate");
  reg.operator()<NpcGhostAnimFramerate>("NpcGhostAnimFramerate");
  reg.operator()<NpcLerpSpeed>("NpcLerpSpeed");
  reg.operator()<NpcPushBack>("NpcPushBack");
  reg.operator()<NpcScanScale>("NpcScanScale");
  reg.operator()<NpcShockwaveFreq>("NpcShockwaveFreq");
  reg.operator()<NpcShockwaveMaxRadius>("NpcShockwaveMaxRadius");
  reg.operator()<NpcShockwaveResolution>("NpcShockwaveResolution");
  reg.operator()<NpcShockwaveSpeed>("NpcShockwaveSpeed");
  reg.operator()<NpcSkeleAnimFramerate>("NpcSkeleAnimFramerate");
  reg.operator()<PcDamageDelay>("PcDamageDelay");
  reg.operator()<PlayerAnimFramerate>("PlayerAnimFramerate");
  reg.operator()<PlayerDetectionScale>("PlayerDetectionScale");
  reg.operator()<PlayerDiagonalLerpSpeedModifier>("PlayerDiagonalLerpSpeedModifier");
  reg.operator()<PlayerFootstepAddDelay>("PlayerFootstepAddDelay");
  reg.operator()<PlayerFootstepFadeDelay>("PlayerFootstepFadeDelay");
  reg.operator()<PlayerLerpInterruptThreshold>("PlayerLerpInterruptThreshold");
  reg.operator()<PlayerLerpSpeed>("PlayerLerpSpeed");
  reg.operator()<PlayerShortcutLerpSpeedModifier>("PlayerShortcutLerpSpeedModifier");
  reg.operator()<PlayerStartPosition>("PlayerStartPosition");
  reg.operator()<WeaponDegradePerHit>("WeaponDegradePerHit");
  reg.operator()<WormholeAnimFramerate>("WormholeAnimFramerate");
  // clang-format on
}

void PersistSystem::initializeComponentRegistry()
{

  // First, set up the type registry (maps type names to factory functions)
  initializeTypeRegistry();

  // Load component definitions from JSON
  std::ifstream inputFile( "res/json/persistent_components.json" );
  if ( !inputFile.is_open() )
  {
    SPDLOG_ERROR( "Failed to open persistent_components.json" );
    return;
  }

  nlohmann::json definitions;
  inputFile >> definitions;
  inputFile.close();

  // Register each component from the JSON file
  for ( const auto &[name, config] : definitions.items() )
  {
    if ( m_type_registry.contains( name ) ) { m_type_registry.at( name )( config ); }
    else { SPDLOG_WARN( "Unknown component type in definitions: {}", name ); }
  }

  // Add components not stored in JSON (runtime-only seeds)
  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::WormholeSeed>( getReg(), 0 );
  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::SinkholeSeed>( getReg(), 0 );
  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::CorruptionSeed>( getReg(), 0 );
}

void PersistSystem::load_state()
{
  SPDLOG_DEBUG( "Loading persistent state..." );
  nlohmann::json jsonData;
  std::ifstream inputFile( "res/json/persistent_components.json" );
  if ( inputFile.is_open() )
  {
    inputFile >> jsonData;
    inputFile.close();
  }

  for ( const auto &[key, value] : jsonData.items() )
  {
    if ( m_component_loaders.contains( key ) ) { m_component_loaders.at( key )( value ); }
    else { SPDLOG_WARN( "Unknown component: {}", key ); }
  }
}

void PersistSystem::save_state()
{
  SPDLOG_DEBUG( "Saving persistent state..." );
  nlohmann::json jsonData;

  // Use the serializers registered during initializeComponentRegistry
  for ( const auto &[key, serializer] : m_component_serializers )
  {
    try
    {
      auto result = serializer();
      if ( !result.is_null() ) { jsonData[key] = result; }
    } catch ( const std::exception &e )
    {
      SPDLOG_WARN( "Failed to serialize component {}: {}", key, e.what() );
    }
  }

  std::ofstream outputFile( "res/json/persistent_components.json" );
  if ( outputFile.is_open() )
  {
    outputFile << jsonData.dump( 4 );
    outputFile.close();
    SPDLOG_INFO( "Persistent state saved successfully" );
  }
  else { SPDLOG_ERROR( "Failed to open file for saving persistent state" ); }
}

} // namespace ProceduralMaze::Sys